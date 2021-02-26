// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2005-2011, Jari Sundell <jaris@ifi.uio.no>

#include <algorithm>

#include <stdexcept>
#include <sys/time.h>
#include <unistd.h>

#include "net/socket_set.h"
#include "torrent/event.h"
#include "torrent/exceptions.h"
#include "torrent/poll_select.h"
#include "torrent/torrent.h"
#include "torrent/utils/cacheline.h"
#include "torrent/utils/error_number.h"
#include "torrent/utils/log.h"
#include "torrent/utils/thread_base.h"
#include "torrent/utils/timer.h"

#define LT_LOG_EVENT(event, log_level, log_fmt, ...)                           \
  lt_log_print(LOG_SOCKET_##log_level,                                         \
               "select->%s(%i): " log_fmt,                                     \
               event->type_name(),                                             \
               event->file_descriptor(),                                       \
               __VA_ARGS__);

namespace torrent {

typedef struct lt_cacheline_aligned {
  torrent::SocketSet lt_cacheline_aligned readSet;
  torrent::SocketSet lt_cacheline_aligned writeSet;
  torrent::SocketSet lt_cacheline_aligned exceptSet;
} set_block_type;

Poll::slot_poll Poll::m_slot_create_poll;

template<typename _Operation>
struct poll_check_t {
  poll_check_t(Poll* p, fd_set* s, _Operation op)
    : m_poll(p)
    , m_set(s)
    , m_op(op) {}

  bool operator()(Event* s) {
    // This check is nessesary as other events may remove a socket
    // from the set.
    if (s == nullptr)
      return false;

    // This check is not nessesary, just for debugging.
    if (s->file_descriptor() < 0)
      throw internal_error("poll_check: s->fd < 0");

    if (FD_ISSET(s->file_descriptor(), m_set)) {
      m_op(s);

      // We waive the global lock after an event has been processed in
      // order to ensure that 's' doesn't get removed before the op is
      // called.
      if ((m_poll->flags() & Poll::flag_waive_global_lock) &&
          thread_base::global_queue_size() != 0)
        thread_base::waive_global_lock();

      return true;
    } else {
      return false;
    }
  }

  Poll*      m_poll;
  fd_set*    m_set;
  _Operation m_op;
};

template<typename _Operation>
inline poll_check_t<_Operation>
poll_check(Poll* p, fd_set* s, _Operation op) {
  return poll_check_t<_Operation>(p, s, op);
}

struct poll_mark {
  poll_mark(fd_set* s, unsigned int* m)
    : m_max(m)
    , m_set(s) {}

  void operator()(Event* s) {
    // Neither of these checks are nessesary, just for debugging.
    if (s == nullptr)
      throw internal_error("poll_mark: s == nullptr");

    if (s->file_descriptor() < 0)
      throw internal_error("poll_mark: s->fd < 0");

    *m_max = std::max(*m_max, (unsigned int)s->file_descriptor());

    FD_SET(s->file_descriptor(), m_set);
  }

  unsigned int* m_max;
  fd_set*       m_set;
};

PollSelect*
PollSelect::create(int maxOpenSockets) {
  if (maxOpenSockets <= 0)
    throw internal_error(
      "PollSelect::set_open_max(...) received an invalid value");

  PollSelect* p = new PollSelect;

  // Just a temp hack, make some special template function for this...
  //
  // Also consider how portable this is for specialized C++
  // allocators.
  set_block_type* block = new set_block_type;

  p->m_readSet   = &block->readSet;
  p->m_writeSet  = &block->writeSet;
  p->m_exceptSet = &block->exceptSet;

  p->m_readSet->reserve(maxOpenSockets);
  p->m_writeSet->reserve(maxOpenSockets);
  p->m_exceptSet->reserve(maxOpenSockets);

  return p;
}

PollSelect::~PollSelect() {
  m_readSet->prepare();
  m_writeSet->prepare();
  m_exceptSet->prepare();

  delete reinterpret_cast<set_block_type*>(m_readSet);
}

uint32_t
PollSelect::open_max() const {
  return m_readSet->max_size();
}

unsigned int
PollSelect::fdset(fd_set* readSet, fd_set* writeSet, fd_set* exceptSet) {
  unsigned int maxFd = 0;

  m_readSet->prepare();
  std::for_each(
    m_readSet->begin(), m_readSet->end(), poll_mark(readSet, &maxFd));

  m_writeSet->prepare();
  std::for_each(
    m_writeSet->begin(), m_writeSet->end(), poll_mark(writeSet, &maxFd));

  m_exceptSet->prepare();
  std::for_each(
    m_exceptSet->begin(), m_exceptSet->end(), poll_mark(exceptSet, &maxFd));

  return maxFd;
}

unsigned int
PollSelect::perform(fd_set* readSet, fd_set* writeSet, fd_set* exceptSet) {
  unsigned int count = 0;

  // Make sure we don't do read/write on fd's that are in except. This should
  // not be a problem as any except call should remove it from the m_*Set's.
  m_exceptSet->prepare();
  count += std::count_if(
    m_exceptSet->begin(),
    m_exceptSet->end(),
    poll_check(this, exceptSet, std::mem_fn(&Event::event_error)));

  m_readSet->prepare();
  count +=
    std::count_if(m_readSet->begin(),
                  m_readSet->end(),
                  poll_check(this, readSet, std::mem_fn(&Event::event_read)));

  m_writeSet->prepare();
  count +=
    std::count_if(m_writeSet->begin(),
                  m_writeSet->end(),
                  poll_check(this, writeSet, std::mem_fn(&Event::event_write)));

  return count;
}

unsigned int
PollSelect::do_poll(int64_t timeout_usec, int flags) {
  unsigned int result = 0;

  utils::timer timeout = utils::timer(timeout_usec);

  timeout += 10;

  uint32_t set_size = open_max();

  char* read_set_buffer  = static_cast<char*>(malloc(set_size * sizeof(char)));
  char* write_set_buffer = static_cast<char*>(malloc(set_size * sizeof(char)));
  char* error_set_buffer = static_cast<char*>(malloc(set_size * sizeof(char)));

  fd_set* read_set  = (fd_set*)read_set_buffer;
  fd_set* write_set = (fd_set*)write_set_buffer;
  fd_set* error_set = (fd_set*)error_set_buffer;

  std::memset(read_set_buffer, 0, set_size);
  std::memset(write_set_buffer, 0, set_size);
  std::memset(error_set_buffer, 0, set_size);

  unsigned int maxFd = fdset(read_set, write_set, error_set);
  timeval      t     = timeout.tval();

  if (!(flags & poll_worker_thread)) {
    thread_base::release_global_lock();
  }

  int status = select(maxFd + 1, read_set, write_set, error_set, &t);

  if (!(flags & poll_worker_thread)) {
    thread_base::acquire_global_lock();
  }

  if (status == -1) {
    free(read_set_buffer);
    free(write_set_buffer);
    free(error_set_buffer);

    if (utils::error_number::current().value() != std::errc::interrupted) {
      throw std::runtime_error("PollSelect::work(): " +
                               utils::error_number::current().message());
    }

    return 0;
  }

  result = perform(read_set, write_set, error_set);

  free(read_set_buffer);
  free(write_set_buffer);
  free(error_set_buffer);

  return result;
}

#ifdef LT_LOG_POLL_OPEN
inline static void
log_poll_open(Event* event) {
  static int log_fd = -1;
  char       buffer[256];

  if (log_fd == -1) {
    snprintf(buffer, 256, LT_LOG_POLL_OPEN, getpid());

    if ((log_fd = open(buffer, O_WRONLY | O_CREAT | O_TRUNC)) == -1)
      throw internal_error("Could not open poll open log file.");
  }

  unsigned int buf_lenght = snprintf(buffer, 256, "open %i\n", event->fd());
}
#endif

void
PollSelect::open(Event* event) {
  LT_LOG_EVENT(event, DEBUG, "Open event.", 0);

  if ((uint32_t)event->file_descriptor() >= m_readSet->max_size())
    throw internal_error("Tried to add a socket to PollSelect that is larger "
                         "than PollSelect::get_open_max()");

  if (in_read(event) || in_write(event) || in_error(event))
    throw internal_error("PollSelect::open(...) called on an inserted event");
}

void
PollSelect::close(Event* event) {
  LT_LOG_EVENT(event, DEBUG, "Close event.", 0);

  if ((uint32_t)event->file_descriptor() >= m_readSet->max_size())
    throw internal_error(
      "PollSelect::close(...) called with an invalid file descriptor");

  if (in_read(event) || in_write(event) || in_error(event))
    throw internal_error("PollSelect::close(...) called on an inserted event");
}

void
PollSelect::closed(Event* event) {
  LT_LOG_EVENT(event, DEBUG, "Closed event.", 0);

  // event->get_fd() was closed, remove it from the sets.
  m_readSet->erase(event);
  m_writeSet->erase(event);
  m_exceptSet->erase(event);
}

bool
PollSelect::in_read(Event* event) {
  return m_readSet->find(event) != m_readSet->end();
}

bool
PollSelect::in_write(Event* event) {
  return m_writeSet->find(event) != m_writeSet->end();
}

bool
PollSelect::in_error(Event* event) {
  return m_exceptSet->find(event) != m_exceptSet->end();
}

void
PollSelect::insert_read(Event* event) {
  LT_LOG_EVENT(event, DEBUG, "Insert read.", 0);
  m_readSet->insert(event);
}

void
PollSelect::insert_write(Event* event) {
  LT_LOG_EVENT(event, DEBUG, "Insert write.", 0);
  m_writeSet->insert(event);
}

void
PollSelect::insert_error(Event* event) {
  LT_LOG_EVENT(event, DEBUG, "Insert error.", 0);
  m_exceptSet->insert(event);
}

void
PollSelect::remove_read(Event* event) {
  LT_LOG_EVENT(event, DEBUG, "Remove read.", 0);
  m_readSet->erase(event);
}

void
PollSelect::remove_write(Event* event) {
  LT_LOG_EVENT(event, DEBUG, "Remove write.", 0);
  m_writeSet->erase(event);
}

void
PollSelect::remove_error(Event* event) {
  LT_LOG_EVENT(event, DEBUG, "Remove error.", 0);
  m_exceptSet->erase(event);
}

} // namespace torrent
