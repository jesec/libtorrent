// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2005-2011, Jari Sundell <jaris@ifi.uio.no>

#ifndef LIBTORRENT_DATA_HASH_TORRENT_H
#define LIBTORRENT_DATA_HASH_TORRENT_H

#include <cinttypes>
#include <functional>
#include <string>

#include "data/chunk_handle.h"
#include "torrent/utils/priority_queue_default.h"
#include "torrent/utils/ranges.h"

namespace torrent {

class ChunkList;

class HashTorrent {
public:
  using Ranges = ranges<uint32_t>;

  using slot_chunk_handle = std::function<void(ChunkHandle)>;

  HashTorrent(ChunkList* c);
  ~HashTorrent() {
    clear();
  }

  bool start(bool try_quick);
  void clear();

  bool is_checking() {
    return m_outstanding >= 0;
  }
  bool is_checked();

  void confirm_checked();

  Ranges& hashing_ranges() {
    return m_ranges;
  }
  uint32_t position() const {
    return m_position;
  }
  uint32_t outstanding() const {
    return m_outstanding;
  }

  std::errc error_number() const {
    return m_errno;
  }

  slot_chunk_handle& slot_check_chunk() {
    return m_slot_check_chunk;
  }

  utils::priority_item& delay_checked() {
    return m_delayChecked;
  }

  void receive_chunkdone(uint32_t index);
  void receive_chunk_cleared(uint32_t index);

private:
  void queue(bool quick);

  unsigned int m_position;
  int          m_outstanding;
  Ranges       m_ranges;

  std::errc m_errno;

  ChunkList* m_chunk_list;

  slot_chunk_handle m_slot_check_chunk;

  utils::priority_item m_delayChecked;
};

} // namespace torrent

#endif
