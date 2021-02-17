// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2005-2011, Jari Sundell <jaris@ifi.uio.no>

#include "torrent/buildinfo.h"

#include <stdexcept>
#include <unistd.h>

#include "torrent/event.h"
#include "torrent/exceptions.h"
#include "torrent/poll_epoll.h"
#include "torrent/torrent.h"
#include "torrent/utils/error_number.h"
#include "torrent/utils/log.h"
#include "torrent/utils/thread_base.h"
#include "torrent/utils/timer.h"

#ifdef LT_USE_EPOLL
#include <sys/epoll.h>
#endif

#define LT_LOG_EVENT(event, log_level, log_fmt, ...)                           \
  lt_log_print(LOG_SOCKET_##log_level,                                         \
               "epoll->%s(%i): " log_fmt,                                      \
               event->type_name(),                                             \
               event->file_descriptor(),                                       \
               __VA_ARGS__);

namespace torrent {

#ifdef LT_USE_EPOLL

inline uint32_t
PollEPoll::event_mask(Event* e) {
  Table::value_type entry = m_table[e->file_descriptor()];
  return entry.second != e ? 0 : entry.first;
}

inline void
PollEPoll::set_event_mask(Event* e, uint32_t m) {
  m_table[e->file_descriptor()] = Table::value_type(m, e);
}

inline void
PollEPoll::modify(Event* event, int op, uint32_t mask) {
  if (event_mask(event) == mask)
    return;

  LT_LOG_EVENT(event, DEBUG, "Modify event: op:%hx mask:%hx.", op, mask);

  epoll_event e;
  e.data.u64 = 0; // Make valgrind happy? Remove please.
  e.data.fd  = event->file_descriptor();
  e.events   = mask;

  set_event_mask(event, mask);

  if (epoll_ctl(m_fd, op, event->file_descriptor(), &e)) {
    // Socket was probably already closed. Ignore this.
    if (op == EPOLL_CTL_DEL && errno == ENOENT)
      return;

    // Handle some libcurl/c-ares bugs by retrying once.
    int retry = op;

    if (op == EPOLL_CTL_ADD && errno == EEXIST) {
      retry = EPOLL_CTL_MOD;
      errno = 0;
    } else if (op == EPOLL_CTL_MOD && errno == ENOENT) {
      retry = EPOLL_CTL_ADD;
      errno = 0;
    }

    if (errno || epoll_ctl(m_fd, retry, event->file_descriptor(), &e)) {
      char errmsg[1024];
      snprintf(errmsg,
               sizeof(errmsg),
               "PollEPoll::modify(...) epoll_ctl(%d, %d -> %d, %d, [%p:%x]) = "
               "%d: %s",
               m_fd,
               op,
               retry,
               event->file_descriptor(),
               static_cast<void*>(event),
               mask,
               errno,
               utils::error_number::current().message().c_str());

      throw internal_error(errmsg);
    }
  }
}

PollEPoll*
PollEPoll::create(int maxOpenSockets) {
  int fd = epoll_create(maxOpenSockets);

  if (fd == -1)
    return nullptr;

  return new PollEPoll(fd, 1024, maxOpenSockets);
}

PollEPoll::PollEPoll(int fd, int maxEvents, int maxOpenSockets)
  : m_fd(fd)
  , m_maxEvents(maxEvents)
  , m_waitingEvents(0)
  , m_events(new epoll_event[m_maxEvents]) {

  m_table.resize(maxOpenSockets);
}

PollEPoll::~PollEPoll() {
  m_table.clear();
  delete[] m_events;

  ::close(m_fd);
}

int
PollEPoll::poll(int msec) {
  int nfds = epoll_wait(m_fd, m_events, m_maxEvents, msec);

  if (nfds == -1)
    return -1;

  return m_waitingEvents = nfds;
}

// We check m_table to make sure the Event is still listening to the
// event, so it is safe to remove Event's while in working.
//
// TODO: Do we want to guarantee if the Event has been removed from
// some event but not closed, it won't call that event? Think so...
unsigned int
PollEPoll::perform() {
  unsigned int count = 0;

  for (epoll_event *itr = m_events, *last = m_events + m_waitingEvents;
       itr != last;
       ++itr) {
    if (itr->data.fd < 0 || (size_t)itr->data.fd >= m_table.size())
      continue;

    if ((flags() & flag_waive_global_lock) &&
        thread_base::global_queue_size() != 0)
      thread_base::waive_global_lock();

    Table::iterator evItr = m_table.begin() + itr->data.fd;

    // Each branch must check for data.ptr != nullptr to allow the socket
    // to remove itself between the calls.
    //
    // TODO: Make it so that it checks that read/write is wanted, that
    // it wasn't removed from one of them but not closed.

    if (itr->events & EPOLLERR && evItr->second != nullptr &&
        evItr->first & EPOLLERR) {
      count++;
      evItr->second->event_error();
    }

    if (itr->events & EPOLLIN && evItr->second != nullptr &&
        evItr->first & EPOLLIN) {
      count++;
      evItr->second->event_read();
    }

    if (itr->events & EPOLLOUT && evItr->second != nullptr &&
        evItr->first & EPOLLOUT) {
      count++;
      evItr->second->event_write();
    }
  }

  m_waitingEvents = 0;
  return count;
}

unsigned int
PollEPoll::do_poll(int64_t timeout_usec, int flags) {
  utils::timer timeout = utils::timer(timeout_usec);

  timeout += 10;

  if (!(flags & poll_worker_thread)) {
    thread_base::release_global_lock();
    thread_base::entering_main_polling();
  }

  int status = poll((timeout.usec() + 999) / 1000);

  if (!(flags & poll_worker_thread)) {
    thread_base::leaving_main_polling();
    thread_base::acquire_global_lock();
  }

  if (status == -1) {
    if (utils::error_number::current().value() != std::errc::interrupted) {
      throw std::runtime_error("PollEPoll::work(): " +
                               utils::error_number::current().message());
    }

    return 0;
  }

  return perform();
}

uint32_t
PollEPoll::open_max() const {
  return m_table.size();
}

void
PollEPoll::open(Event* event) {
  LT_LOG_EVENT(event, DEBUG, "Open event.", 0);

  if (event_mask(event) != 0)
    throw internal_error(
      "PollEPoll::open(...) called but the file descriptor is active");
}

void
PollEPoll::close(Event* event) {
  LT_LOG_EVENT(event, DEBUG, "Close event.", 0);

  if (event_mask(event) != 0)
    throw internal_error(
      "PollEPoll::close(...) called but the file descriptor is active");

  m_table[event->file_descriptor()] = Table::value_type();

  // Clear the event list just in case we open a new socket with the
  // same fd while in the middle of calling PollEPoll::perform.
  for (epoll_event *itr = m_events, *last = m_events + m_waitingEvents;
       itr != last;
       ++itr)
    if (itr->data.fd == event->file_descriptor())
      itr->events = 0;
}

void
PollEPoll::closed(Event* event) {
  LT_LOG_EVENT(event, DEBUG, "Closed event.", 0);

  // Kernel removes closed FDs automatically, so just clear the mask and remove
  // it from pending calls. Don't touch if the FD was re-used before we received
  // the close notification.
  if (m_table[event->file_descriptor()].second == event)
    m_table[event->file_descriptor()] = Table::value_type();

  // for (epoll_event *itr = m_events, *last = m_events + m_waitingEvents; itr
  // != last; ++itr) {
  //   if (itr->data.fd == event->file_descriptor())
  //     itr->events = 0;
  // }
}

// Use custom defines for EPOLL* to make the below code compile with
// and with epoll.
bool
PollEPoll::in_read(Event* event) {
  return event_mask(event) & EPOLLIN;
}

bool
PollEPoll::in_write(Event* event) {
  return event_mask(event) & EPOLLOUT;
}

bool
PollEPoll::in_error(Event* event) {
  return event_mask(event) & EPOLLERR;
}

void
PollEPoll::insert_read(Event* event) {
  LT_LOG_EVENT(event, DEBUG, "Insert read.", 0);

  modify(event,
         event_mask(event) ? EPOLL_CTL_MOD : EPOLL_CTL_ADD,
         event_mask(event) | EPOLLIN);
}

void
PollEPoll::insert_write(Event* event) {
  LT_LOG_EVENT(event, DEBUG, "Insert write.", 0);

  modify(event,
         event_mask(event) ? EPOLL_CTL_MOD : EPOLL_CTL_ADD,
         event_mask(event) | EPOLLOUT);
}

void
PollEPoll::insert_error(Event* event) {
  LT_LOG_EVENT(event, DEBUG, "Insert error.", 0);

  modify(event,
         event_mask(event) ? EPOLL_CTL_MOD : EPOLL_CTL_ADD,
         event_mask(event) | EPOLLERR);
}

void
PollEPoll::remove_read(Event* event) {
  LT_LOG_EVENT(event, DEBUG, "Remove read.", 0);

  uint32_t mask = event_mask(event) & ~EPOLLIN;
  modify(event, mask ? EPOLL_CTL_MOD : EPOLL_CTL_DEL, mask);
}

void
PollEPoll::remove_write(Event* event) {
  LT_LOG_EVENT(event, DEBUG, "Remove write.", 0);

  uint32_t mask = event_mask(event) & ~EPOLLOUT;
  modify(event, mask ? EPOLL_CTL_MOD : EPOLL_CTL_DEL, mask);
}

void
PollEPoll::remove_error(Event* event) {
  LT_LOG_EVENT(event, DEBUG, "Remove error.", 0);

  uint32_t mask = event_mask(event) & ~EPOLLERR;
  modify(event, mask ? EPOLL_CTL_MOD : EPOLL_CTL_DEL, mask);
}

#else // LT_USE_EPOLL

PollEPoll*
PollEPoll::create(int) {
  return nullptr;
}
PollEPoll::~PollEPoll() {}

int
PollEPoll::poll(int) {
  throw internal_error("An PollEPoll function was called, but it is disabled.");
}
unsigned int
PollEPoll::perform() {
  throw internal_error("An PollEPoll function was called, but it is disabled.");
}
unsigned int
PollEPoll::do_poll(int64_t, int) {
  throw internal_error("An PollEPoll function was called, but it is disabled.");
}
uint32_t
PollEPoll::open_max() const {
  throw internal_error("An PollEPoll function was called, but it is disabled.");
}

void
PollEPoll::open(torrent::Event*) {}
void
PollEPoll::close(torrent::Event*) {}
void
PollEPoll::closed(torrent::Event*) {}

bool
PollEPoll::in_read(torrent::Event*) {
  throw internal_error("An PollEPoll function was called, but it is disabled.");
}
bool
PollEPoll::in_write(torrent::Event*) {
  throw internal_error("An PollEPoll function was called, but it is disabled.");
}
bool
PollEPoll::in_error(torrent::Event*) {
  throw internal_error("An PollEPoll function was called, but it is disabled.");
}

void
PollEPoll::insert_read(torrent::Event*) {}
void
PollEPoll::insert_write(torrent::Event*) {}
void
PollEPoll::insert_error(torrent::Event*) {}

void
PollEPoll::remove_read(torrent::Event*) {}
void
PollEPoll::remove_write(torrent::Event*) {}
void
PollEPoll::remove_error(torrent::Event*) {}

PollEPoll::PollEPoll(int, int, int) {
  throw internal_error("An PollEPoll function was called, but it is disabled.");
}

#endif // LT_USE_EPOLL

} // namespace torrent
