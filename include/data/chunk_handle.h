// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2005-2011, Jari Sundell <jaris@ifi.uio.no>

#ifndef LIBTORRENT_DATA_CHUNK_HANDLE_H
#define LIBTORRENT_DATA_CHUNK_HANDLE_H

#include "chunk_list_node.h"
#include "torrent/utils/error_number.h"

namespace torrent {

class ChunkListNode;

class ChunkHandle {
public:
  ChunkHandle(ChunkListNode* c = nullptr, bool wr = false, bool blk = false)
    : m_node(c)
    , m_writable(wr)
    , m_blocking(blk) {}

  bool is_valid() const {
    return m_node != nullptr;
  }
  bool is_loaded() const {
    return m_node != nullptr && m_node->is_valid();
  }
  bool is_writable() const {
    return m_writable;
  }
  bool is_blocking() const {
    return m_blocking;
  }

  void clear() {
    m_node     = nullptr;
    m_writable = false;
    m_blocking = false;
  }

  utils::error_number error_number() const {
    return m_errorNumber;
  }
  void set_error_number(utils::error_number e) {
    m_errorNumber = e;
  }

  ChunkListNode* object() const {
    return m_node;
  }
  Chunk* chunk() const {
    return m_node->chunk();
  }

  uint32_t index() const {
    return m_node->index();
  }

  static ChunkHandle from_error(utils::error_number e) {
    ChunkHandle h;
    h.set_error_number(e);
    return h;
  }

private:
  ChunkListNode* m_node;
  bool           m_writable;
  bool           m_blocking;

  utils::error_number m_errorNumber;
};

} // namespace torrent

#endif
