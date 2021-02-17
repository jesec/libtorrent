// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2005-2011, Jari Sundell <jaris@ifi.uio.no>

#ifndef LIBTORRENT_DATA_HASH_QUEUE_NODE_H
#define LIBTORRENT_DATA_HASH_QUEUE_NODE_H

#include <cinttypes>
#include <functional>
#include <string>
#include <utility>

#include "chunk_handle.h"
#include "hash_chunk.h"

namespace torrent {

class download_data;

class HashQueueNode {
public:
  using slot_done_type = std::function<void(ChunkHandle, const char*)>;
  using id_type        = download_data*;

  HashQueueNode(id_type id, HashChunk* c, slot_done_type d)
    : m_id(id)
    , m_chunk(c)
    , m_willneed(false)
    , m_slot_done(std::move(d)) {}

  id_type id() const {
    return m_id;
  }
  ChunkHandle& handle() {
    return *m_chunk->chunk();
  }

  uint32_t get_index() const;

  HashChunk* get_chunk() {
    return m_chunk;
  }
  bool get_willneed() const {
    return m_willneed;
  }

  bool perform_remaining(bool force) {
    return m_chunk->perform(m_chunk->remaining(), force);
  }

  void clear();

  // Does not call multiple times on the same chunk. Returns the
  // number of bytes not checked in this chunk.
  uint32_t call_willneed();

  slot_done_type& slot_done() {
    return m_slot_done;
  }

private:
  id_type    m_id;
  HashChunk* m_chunk;
  bool       m_willneed;

  slot_done_type m_slot_done;
};

} // namespace torrent

#endif
