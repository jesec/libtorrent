// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2005-2011, Jari Sundell <jaris@ifi.uio.no>

#include "torrent/buildinfo.h"

#include <cassert>

#include <algorithm>
#include <stdexcept>
#include <unistd.h>

#ifdef LT_USE_KQUEUE
#include <sys/event.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#endif

#include "torrent/event.h"
#include "torrent/exceptions.h"
#include "torrent/poll_kqueue.h"
#include "torrent/torrent.h"
#include "torrent/utils/error_number.h"
#include "torrent/utils/log.h"
#include "torrent/utils/thread_base.h"
#include "torrent/utils/timer.h"

#define LT_LOG_EVENT(event, log_level, log_fmt, ...)                           \
  lt_log_print(LOG_SOCKET_##log_level,                                         \
               "kqueue->%s(%i): " log_fmt,                                     \
               event->type_name(),                                             \
               event->file_descriptor(),                                       \
               __VA_ARGS__);

namespace torrent {

#ifdef LT_USE_KQUEUE

inline uint32_t
PollKQueue::event_mask(Event* e) {
  assert(e->file_descriptor() != -1);

  Table::value_type entry = m_table[e->file_descriptor()];
  return entry.second != e ? 0 : entry.first;
}

inline void
PollKQueue::set_event_mask(Event* e, uint32_t m) {
  assert(e->file_descriptor() != -1);

  m_table[e->file_descriptor()] = Table::value_type(m, e);
}

void
PollKQueue::flush_events() {
  timespec timeout = { 0, 0 };

  int nfds = kevent(m_fd,
                    m_changes,
                    m_changedEvents,
                    m_events + m_waitingEvents,
                    m_maxEvents - m_waitingEvents,
                    &timeout);
  if (nfds == -1)
    throw internal_error("PollKQueue::flush_events() error: " +
                         utils::error_number::current().message());

  m_changedEvents = 0;
  m_waitingEvents += nfds;
}

void
PollKQueue::modify(Event* event, unsigned short op, short mask) {
  LT_LOG_EVENT(event,
               DEBUG,
               "Modify event: op:%hx mask:%hx changed:%u.",
               op,
               mask,
               m_changedEvents);

  // Flush the changed filters to the kernel if the buffer is full.
  if (m_changedEvents == m_maxEvents) {
    if (kevent(m_fd, m_changes, m_changedEvents, NULL, 0, NULL) == -1)
      throw internal_error("PollKQueue::modify() error: " +
                           utils::error_number::current().message());

    m_changedEvents = 0;
  }

  struct kevent* itr = m_changes + (m_changedEvents++);

  assert(event == m_table[event->file_descriptor()].second);
  EV_SET(itr, event->file_descriptor(), mask, op, 0, 0, event);
}

PollKQueue*
PollKQueue::create(int maxOpenSockets) {
  int fd = kqueue();

  if (fd == -1)
    return nullptr;

  return new PollKQueue(fd, 1024, maxOpenSockets);
}

PollKQueue::PollKQueue(int fd, int maxEvents, int maxOpenSockets)
  : m_fd(fd)
  , m_maxEvents(maxEvents)
  , m_waitingEvents(0)
  , m_changedEvents(0)
  , m_stdinEvent(nullptr) {

  m_events  = new struct kevent[m_maxEvents];
  m_changes = new struct kevent[maxOpenSockets];

  m_table.resize(maxOpenSockets);
}

PollKQueue::~PollKQueue() {
  m_table.clear();
  delete[] m_events;
  delete[] m_changes;

  ::close(m_fd);
}

int
PollKQueue::poll(int msec) {
#if LT_KQUEUE_SOCKET_ONLY
  if (m_stdinEvent != nullptr) {
    // Flush all changes to the kqueue poll before we start select
    // polling, so that they get included.
    if (m_changedEvents != 0)
      flush_events();

    if (poll_select(msec) == -1)
      return -1;

    // The timeout was already handled in select().
    msec = 0;
  }
#endif

  timespec timeout = { msec / 1000, (msec % 1000) * 1000000 };

  int nfds = kevent(m_fd,
                    m_changes,
                    m_changedEvents,
                    m_events + m_waitingEvents,
                    m_maxEvents - m_waitingEvents,
                    &timeout);

  // Clear the changed events even on fail as we might have received a
  // signal or similar, and the changed events have already been
  // consumed.
  //
  // There's a chance a bad changed event could make kevent return -1,
  // but it won't as long as there is room enough in m_events.
  m_changedEvents = 0;

  if (nfds == -1)
    return -1;

  m_waitingEvents += nfds;

  return nfds;
}

#if LT_KQUEUE_SOCKET_ONLY
int
PollKQueue::poll_select(int msec) {
  if (m_waitingEvents >= m_maxEvents)
    return 0;

  timeval selectTimeout = { msec / 1000, (msec % 1000) * 1000 };

  // If m_fd isn't the first FD opened by the client and has
  // a low number, using ::poll() here would perform better.
  //
  // This kinda assumes fd_set's internal type is int.
  auto    readBuffer = new int[m_fd + 1];
  fd_set* readSet    = (fd_set*)&readBuffer;

  std::memset(readBuffer, 0, m_fd + 1);
  FD_SET(0, readSet);
  FD_SET(m_fd, readSet);

  int nfds = select(m_fd + 1, readSet, NULL, NULL, &selectTimeout);

  if (nfds == -1)
    return nfds;

  if (FD_ISSET(0, readSet)) {
    m_events[m_waitingEvents].ident  = 0;
    m_events[m_waitingEvents].filter = EVFILT_READ;
    m_events[m_waitingEvents].flags  = 0;
    m_waitingEvents++;
  }

  delete[] readBuffer;

  return nfds;
}
#endif

unsigned int
PollKQueue::perform() {
  unsigned int count = 0;

  for (struct kevent *itr = m_events, *last = m_events + m_waitingEvents;
       itr != last;
       ++itr) {
    if (itr->ident >= m_table.size())
      continue;

    if ((flags() & flag_waive_global_lock) &&
        thread_base::global_queue_size() != 0)
      thread_base::waive_global_lock();

    Table::iterator evItr = m_table.begin() + itr->ident;

    if ((itr->flags & EV_ERROR) && evItr->second != nullptr) {
      if (evItr->first & flag_error)
        evItr->second->event_error();

      count++;
      continue;
    }

    // Also check current mask.

    if (itr->filter == EVFILT_READ && evItr->second != nullptr &&
        evItr->first & flag_read) {
      count++;
      evItr->second->event_read();
    }

    if (itr->filter == EVFILT_WRITE && evItr->second != nullptr &&
        evItr->first & flag_write) {
      count++;
      evItr->second->event_write();
    }
  }

  m_waitingEvents = 0;
  return count;
}

unsigned int
PollKQueue::do_poll(int64_t timeout_usec, int flags) {
  utils::timer timeout = utils::timer(timeout_usec);
  timeout += 10;

  if (!(flags & poll_worker_thread)) {
    thread_base::release_global_lock();
  }

  int status = poll((timeout.usec() + 999) / 1000);

  if (!(flags & poll_worker_thread)) {
    thread_base::acquire_global_lock();
  }

  if (status == -1) {
    if (utils::error_number::current().value() != std::errc::interrupted) {
      throw std::runtime_error("PollKQueue::work(): " +
                               utils::error_number::current().message());
    }

    return 0;
  }

  return perform();
}

uint32_t
PollKQueue::open_max() const {
  return m_table.size();
}

void
PollKQueue::open(Event* event) {
  LT_LOG_EVENT(event, DEBUG, "Open event.", 0);

  if (event_mask(event) != 0)
    throw internal_error(
      "PollKQueue::open(...) called but the file descriptor is active");
}

void
PollKQueue::close(Event* event) {
  LT_LOG_EVENT(event, DEBUG, "close event", 0);

#if LT_KQUEUE_SOCKET_ONLY
  if (event->file_descriptor() == 0) {
    m_stdinEvent = nullptr;
    return;
  }
#endif

  if (event_mask(event) != 0)
    throw internal_error(
      "PollKQueue::close(...) called but the file descriptor is active");

  m_table[event->file_descriptor()] = Table::value_type();

  // Shouldn't be needed anymore.
  for (struct kevent *itr = m_events, *last = m_events + m_waitingEvents;
       itr != last;
       ++itr)
    if (itr->udata == event)
      itr->udata = NULL;

  m_changedEvents =
    std::remove_if(m_changes,
                   m_changes + m_changedEvents,
                   [event](struct kevent& e) { return event == e.udata; }) -
    m_changes;
}

void
PollKQueue::closed(Event* event) {
  LT_LOG_EVENT(event, DEBUG, "closed event", 0);

#if LT_KQUEUE_SOCKET_ONLY
  if (event->file_descriptor() == 0) {
    m_stdinEvent = nullptr;
    return;
  }
#endif

  // Kernel removes closed FDs automatically, so just clear the mask
  // and remove it from pending calls.  Don't touch if the FD was
  // re-used before we received the close notification.
  if (m_table[event->file_descriptor()].second == event)
    m_table[event->file_descriptor()] = Table::value_type();

  // Shouldn't be needed anymore.
  for (struct kevent *itr = m_events, *last = m_events + m_waitingEvents;
       itr != last;
       ++itr)
    if (itr->udata == event)
      itr->udata = NULL;

  m_changedEvents =
    std::remove_if(m_changes,
                   m_changes + m_changedEvents,
                   [event](struct kevent& e) { return event == e.udata; }) -
    m_changes;
}

// Use custom defines for EPOLL* to make the below code compile with
// and with epoll.
bool
PollKQueue::in_read(Event* event) {
  return event_mask(event) & flag_read;
}

bool
PollKQueue::in_write(Event* event) {
  return event_mask(event) & flag_write;
}

bool
PollKQueue::in_error(Event* event) {
  return event_mask(event) & flag_error;
}

void
PollKQueue::insert_read(Event* event) {
  if (event_mask(event) & flag_read)
    return;

  LT_LOG_EVENT(event, DEBUG, "Insert read.", 0);
  set_event_mask(event, event_mask(event) | flag_read);

#if LT_KQUEUE_SOCKET_ONLY
  if (event->file_descriptor() == 0) {
    m_stdinEvent = event;
    return;
  }
#endif

  modify(event, EV_ADD, EVFILT_READ);
}

void
PollKQueue::insert_write(Event* event) {
  if (event_mask(event) & flag_write)
    return;

  LT_LOG_EVENT(event, DEBUG, "Insert write.", 0);
  set_event_mask(event, event_mask(event) | flag_write);
  modify(event, EV_ADD, EVFILT_WRITE);
}

void
PollKQueue::insert_error(Event* event) {
  LT_LOG_EVENT(event, DEBUG, "Insert error.", 0);
}

void
PollKQueue::remove_read(Event* event) {
  if (!(event_mask(event) & flag_read))
    return;

  LT_LOG_EVENT(event, DEBUG, "Remove read.", 0);
  set_event_mask(event, event_mask(event) & ~flag_read);

#if LT_KQUEUE_SOCKET_ONLY
  if (event->file_descriptor() == 0) {
    m_stdinEvent = nullptr;
    return;
  }
#endif

  modify(event, EV_DELETE, EVFILT_READ);
}

void
PollKQueue::remove_write(Event* event) {
  if (!(event_mask(event) & flag_write))
    return;

  LT_LOG_EVENT(event, DEBUG, "Remove write.", 0);
  set_event_mask(event, event_mask(event) & ~flag_write);
  modify(event, EV_DELETE, EVFILT_WRITE);
}

void
PollKQueue::remove_error(Event* event) {
  LT_LOG_EVENT(event, DEBUG, "Remove error.", 0);
}

#else // USE_QUEUE

PollKQueue*
PollKQueue::create(int) {
  return nullptr;
}

int
PollKQueue::poll(int) {
  throw internal_error(
    "An PollKQueue function was called, but it is disabled.");
}

unsigned int
PollKQueue::perform() {
  throw internal_error(
    "An PollKQueue function was called, but it is disabled.");
}

unsigned int
PollKQueue::do_poll(int64_t, int) {
  throw internal_error(
    "An PollKQueue function was called, but it is disabled.");
}

uint32_t
PollKQueue::open_max() const {
  throw internal_error(
    "An PollKQueue function was called, but it is disabled.");
}

void
PollKQueue::open(torrent::Event*) {}

void
PollKQueue::close(torrent::Event*) {}

void
PollKQueue::closed(torrent::Event*) {}

bool
PollKQueue::in_read(torrent::Event*) {
  throw internal_error(
    "An PollKQueue function was called, but it is disabled.");
}

bool
PollKQueue::in_write(torrent::Event*) {
  throw internal_error(
    "An PollKQueue function was called, but it is disabled.");
}

bool
PollKQueue::in_error(torrent::Event*) {
  throw internal_error(
    "An PollKQueue function was called, but it is disabled.");
}

void
PollKQueue::insert_read(torrent::Event*) {}

void
PollKQueue::insert_write(torrent::Event*) {}

void
PollKQueue::insert_error(torrent::Event*) {}

void
PollKQueue::remove_read(torrent::Event*) {}

void
PollKQueue::remove_write(torrent::Event*) {}

void
PollKQueue::remove_error(torrent::Event*) {}

PollKQueue::PollKQueue(int, int, int) {
  throw internal_error(
    "An PollKQueue function was called, but it is disabled.");
}

PollKQueue::~PollKQueue() = default;

#endif // LT_USE_KQUEUE

} // namespace torrent
