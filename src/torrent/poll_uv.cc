// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2023, Jesse Chan <jc@linux.com>

#include "torrent/buildinfo.h"

#include <stdexcept>
#include <unistd.h>

#include "torrent/event.h"
#include "torrent/exceptions.h"
#include "torrent/poll_uv.h"
#include "torrent/torrent.h"
#include "torrent/utils/error_number.h"
#include "torrent/utils/log.h"
#include "torrent/utils/thread_base.h"
#include "torrent/utils/timer.h"

#define LT_LOG_EVENT(event, log_level, log_fmt, ...)                           \
  lt_log_print(LOG_SOCKET_##log_level,                                         \
               "uv->%s(%i): " log_fmt,                                         \
               event->type_name(),                                             \
               event->file_descriptor(),                                       \
               __VA_ARGS__);

namespace torrent {

enum EventMask {
  EV_READ  = 1,
  EV_WRITE = 2,
  EV_ERROR = 4,
};

static void
uv_event_cb(uv_poll_t* handle, int status, int events) {
  torrent::Event* e    = (torrent::Event*)handle->data;
  PollUV*         poll = (PollUV*)handle->loop->data;

  poll->event_callback(e, status, events);
}

static void
uv_timeout_cb(uv_timer_t* handle) {
  uv_stop(handle->loop);
}

static void
uv_close_timer_cb(uv_handle_t* handle) {
  delete (uv_timer_t*)handle;
}

static void
uv_close_poll_cb(uv_handle_t* handle) {
  delete (uv_poll_t*)handle;
}

PollUV::PollUV(uv_loop_t* loop, uv_timer_t* timer, int maxOpenSockets)
  : m_loop(loop)
  , m_timer(timer) {
  m_loop->data = this;
  m_table.resize(maxOpenSockets);
}

PollUV::~PollUV() {
  uv_close((uv_handle_t*)m_timer, uv_close_timer_cb);
  for (auto& [event, mask, handle] : m_table) {
    if (handle) {
      uv_close((uv_handle_t*)handle, uv_close_poll_cb);
    }
  }

  uv_run(m_loop, UV_RUN_DEFAULT);
  uv_loop_close(m_loop);
  delete m_loop;
}

PollUV*
PollUV::create(int maxOpenSockets) {
  int result = 0;

  auto* loop = new uv_loop_t;
  result     = uv_loop_init(loop);
  if (result < 0) {
    delete loop;
    throw internal_error("PollUV::create: uv_loop_init failed");
  }

  auto* timer = new uv_timer_t;
  result      = uv_timer_init(loop, timer);
  if (result < 0) {
    delete timer;
    delete loop;
    throw internal_error("PollUV::create: uv_timer_init failed");
  }

  return new PollUV(loop, timer, maxOpenSockets);
}

void
PollUV::event_callback(Event* e, int status, int events) {
  // We waive the global lock after an event has been processed in
  // order to ensure that 's' doesn't get removed before the op is
  // called.
  if ((flags() & Poll::flag_waive_global_lock) &&
      thread_base::global_queue_size() != 0)
    thread_base::waive_global_lock();

  ++m_event_count;

  int mask = event_mask(e);

  if (status < 0) {
    LT_LOG_EVENT(e, ERROR, "event_callback: %s", uv_strerror(status));
    if (mask & EV_ERROR) {
      e->event_error();
    }
  }

  if ((events & UV_READABLE) && (mask & EV_READ)) {
    e->event_read();
  }

  if ((events & UV_WRITABLE) && (mask & EV_WRITE)) {
    e->event_write();
  }
}

inline int
PollUV::event_mask(Event* e) {
  const auto& [event, mask, handle] = m_table[e->file_descriptor()];

  if (event != e) {
    return 0;
  }

  return mask;
}

inline void
PollUV::set_event_mask(Event* e, int m) {
  auto& [event, mask, handle] = m_table[e->file_descriptor()];

  event = e;
  mask  = m;
}

uint32_t
PollUV::open_max() const {
  return m_table.size();
}

unsigned int
PollUV::do_poll(long timeout_usec, int flags) {
  auto timeout = utils::timer(timeout_usec);

  timeout += 10;

  if (!(flags & poll_worker_thread)) {
    thread_base::release_global_lock();
  }

  m_event_count = 0;
  uv_timer_start(m_timer, uv_timeout_cb, (timeout.usec() + 999) / 1000, 0);
  uv_run(m_loop, UV_RUN_ONCE);

  if (!(flags & poll_worker_thread)) {
    thread_base::acquire_global_lock();
  }

  return m_event_count;
}

void
PollUV::open(torrent::Event* e) {
  LT_LOG_EVENT(e, DEBUG, "Open event.", 0);

  if (event_mask(e) != 0)
    throw internal_error(
      "PollUV::open(...) called but the file descriptor is active");
}

void
PollUV::close(torrent::Event* e) {
  LT_LOG_EVENT(e, DEBUG, "Close event.", 0);

  if (event_mask(e) != 0)
    throw internal_error(
      "PollEPoll::close(...) called but the file descriptor is active");

  auto& [event, mask, handle] = m_table[e->file_descriptor()];
  if (handle != nullptr) {
    uv_close((uv_handle_t*)handle, uv_close_poll_cb);
  }

  m_table[e->file_descriptor()] = std::make_tuple(nullptr, 0, nullptr);
}

void
PollUV::closed(torrent::Event* e) {
  LT_LOG_EVENT(e, DEBUG, "Closed event.", 0);

  auto& [event, mask, handle] = m_table[e->file_descriptor()];

  if (event != e) {
    return;
  }

  if (handle != nullptr) {
    uv_poll_stop(handle);
    uv_close((uv_handle_t*)handle, uv_close_poll_cb);
  }

  m_table[e->file_descriptor()] = std::make_tuple(nullptr, 0, nullptr);
}

void
PollUV::poll_start(torrent::Event* e) {
  auto& [event, mask, handle] = m_table[e->file_descriptor()];
  int m                       = event_mask(e);

  if (m == 0) {
    return;
  }

  int events = 0;
  if (m & EV_READ) {
    events |= UV_READABLE;
  }
  if (m & EV_WRITE) {
    events |= UV_WRITABLE;
  }

  if (events == 0) {
    return;
  }

  if (handle == nullptr) {
    handle = new uv_poll_t;
    uv_poll_init(m_loop, handle, e->file_descriptor());
    handle->data = (void*)e;
  }

  uv_poll_start(handle, events, uv_event_cb);
}

bool
PollUV::in_read(torrent::Event* event) {
  return event_mask(event) & EV_READ;
}

bool
PollUV::in_write(torrent::Event* event) {
  return event_mask(event) & EV_WRITE;
}

bool
PollUV::in_error(torrent::Event* event) {
  return event_mask(event) & EV_ERROR;
}

void
PollUV::insert_read(torrent::Event* e) {
  set_event_mask(e, event_mask(e) | EV_READ);
  poll_start(e);
}

void
PollUV::insert_write(torrent::Event* e) {
  set_event_mask(e, event_mask(e) | EV_WRITE);
  poll_start(e);
}

void
PollUV::insert_error(torrent::Event* e) {
  set_event_mask(e, event_mask(e) | EV_ERROR);
}

void
PollUV::remove_read(torrent::Event* e) {
  auto& [event, mask, handle] = m_table[e->file_descriptor()];
  set_event_mask(e, event_mask(e) & ~EV_READ);

  if (handle == nullptr) {
    return;
  }

  uv_poll_stop(handle);
  poll_start(e);
}

void
PollUV::remove_write(torrent::Event* e) {
  auto& [event, mask, handle] = m_table[e->file_descriptor()];
  set_event_mask(e, event_mask(e) & ~EV_WRITE);

  if (handle == nullptr) {
    return;
  }

  uv_poll_stop(handle);
  poll_start(e);
}

void
PollUV::remove_error(torrent::Event* e) {
  set_event_mask(e, event_mask(e) & ~EV_ERROR);
}

} // namespace torrent
