// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2005-2011, Jari Sundell <jaris@ifi.uio.no>

#ifndef LIBTORRENT_DATA_HASH_CHECK_QUEUE_H
#define LIBTORRENT_DATA_HASH_CHECK_QUEUE_H

#include <deque>
#include <functional>
#include <mutex>

#include "torrent/utils/allocators.h"
#include "torrent/utils/cacheline.h"

namespace torrent {

class HashString;
class HashChunk;

class lt_cacheline_aligned HashCheckQueue
  : private std::deque<HashChunk*, utils::cacheline_allocator<HashChunk*>> {
public:
  using base_type =
    std::deque<HashChunk*, utils::cacheline_allocator<HashChunk*>>;
  using slot_chunk_handle = std::function<void(HashChunk*, const HashString&)>;

  using base_type::iterator;

  using base_type::empty;
  using base_type::size;

  using base_type::back;
  using base_type::begin;
  using base_type::end;
  using base_type::front;

  // Guarded functions for adding new...

  void push_back(HashChunk* node);
  void perform();

  bool remove(HashChunk* node);

  slot_chunk_handle& slot_chunk_done() {
    return m_slot_chunk_done;
  }

private:
  slot_chunk_handle m_slot_chunk_done;
  std::mutex        m_lock;
};

} // namespace torrent

#endif
