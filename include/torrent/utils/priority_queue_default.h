// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2005-2007, Jari Sundell <jaris@ifi.uio.no>

#ifndef LIBTORRENT_UTILS_PRIORITY_QUEUE_DEFAULT_H
#define LIBTORRENT_UTILS_PRIORITY_QUEUE_DEFAULT_H

#include <functional>

#include <torrent/exceptions.h>
#include <torrent/utils/allocators.h>
#include <torrent/utils/priority_queue.h>
#include <torrent/utils/timer.h>

namespace torrent {
namespace utils {

class priority_item {
public:
  using slot_void = std::function<void()>;

  priority_item() = default;
  ~priority_item() {
    if (is_queued()) {
      torrent::internal_error(
        "priority_item::~priority_item() called on a queued item.");
      return;
    }

    m_time = timer();
    m_slot = slot_void();
  }

  bool is_valid() const {
    return (bool)m_slot;
  }
  bool is_queued() const {
    return m_time != timer();
  }

  slot_void& slot() {
    return m_slot;
  }

  const timer& time() const {
    return m_time;
  }
  void clear_time() {
    m_time = timer();
  }
  void set_time(const timer& t) {
    m_time = t;
  }

  bool compare(const timer& t) const {
    return m_time > t;
  }

private:
  priority_item(const priority_item&) = delete;
  void operator=(const priority_item&) = delete;

  timer     m_time;
  slot_void m_slot;
};

struct priority_compare {
  bool operator()(const priority_item* const p1,
                  const priority_item* const p2) const {
    return p1->time() > p2->time();
  }
};

using priority_equal = std::equal_to<priority_item*>;
using priority_queue_default =
  priority_queue<priority_item*,
                 priority_compare,
                 priority_equal,
                 cacheline_allocator<priority_item*>>;

inline void
priority_queue_perform(priority_queue_default* queue, timer t) {
  while (!queue->empty() && queue->top()->time() <= t) {
    priority_item* v = queue->top();
    queue->pop();

    v->clear_time();
    v->slot()();
  }
}

inline void
priority_queue_insert(priority_queue_default* queue,
                      priority_item*          item,
                      timer                   t) {
  if (t == timer())
    throw torrent::internal_error(
      "priority_queue_insert(...) received a bad timer.");

  if (!item->is_valid())
    throw torrent::internal_error(
      "priority_queue_insert(...) called on an invalid item.");

  if (item->is_queued())
    throw torrent::internal_error(
      "priority_queue_insert(...) called on an already queued item.");

  if (queue->find(item) != queue->end())
    throw torrent::internal_error(
      "priority_queue_insert(...) item found in queue.");

  item->set_time(t);
  queue->push(item);
}

inline void
priority_queue_erase(priority_queue_default* queue, priority_item* item) {
  if (!item->is_queued())
    return;

  // Check is_valid() after is_queued() so that it is safe to call
  // erase on untouched instances.
  if (!item->is_valid())
    throw torrent::internal_error(
      "priority_queue_erase(...) called on an invalid item.");

  // Clear time before erasing to force it to the top.
  item->clear_time();

  if (!queue->erase(item))
    throw torrent::internal_error(
      "priority_queue_erase(...) could not find item in queue.");

  if (queue->find(item) != queue->end())
    throw torrent::internal_error(
      "priority_queue_erase(...) item still in queue.");
}

} // namespace utils
} // namespace torrent

#endif
