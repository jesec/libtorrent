// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2005-2011, Jari Sundell <jaris@ifi.uio.no>

#include "data/chunk_list_node.h"
#include "data/hash_chunk.h"

#include "data/hash_queue_node.h"

namespace torrent {

uint32_t
HashQueueNode::get_index() const {
  return m_chunk->chunk()->index();
}

void
HashQueueNode::clear() {
  delete m_chunk;
  m_chunk = NULL;
}

uint32_t
HashQueueNode::call_willneed() {
  if (!m_willneed) {
    m_willneed = true;
    m_chunk->advise_willneed(m_chunk->remaining());
  }

  return m_chunk->remaining();
}

} // namespace torrent
