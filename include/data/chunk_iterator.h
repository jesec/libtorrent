// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2005-2011, Jari Sundell <jaris@ifi.uio.no>

#ifndef LIBTORRENT_DATA_CHUNK_ITERATOR_H
#define LIBTORRENT_DATA_CHUNK_ITERATOR_H

#include "chunk.h"

namespace torrent {

class ChunkIterator {
public:
  ChunkIterator(Chunk* chunk, uint32_t first, uint32_t last);

  bool empty() const {
    return m_iterator == m_chunk->end() || m_first >= m_last;
  }

  // Only non-zero length ranges will be returned.
  Chunk::data_type data();

  MemoryChunk* memory_chunk() {
    return &m_iterator->chunk();
  }

  uint32_t memory_chunk_first() const {
    return m_first - m_iterator->position();
  }
  uint32_t memory_chunk_last() const {
    return m_last - m_iterator->position();
  }

  bool next();
  bool forward(uint32_t length);

private:
  Chunk*          m_chunk;
  Chunk::iterator m_iterator;

  uint32_t m_first;
  uint32_t m_last;
};

inline ChunkIterator::ChunkIterator(Chunk* chunk, uint32_t first, uint32_t last)
  : m_chunk(chunk)
  , m_iterator(chunk->at_position(first))
  ,

  m_first(first)
  , m_last(last) {}

inline Chunk::data_type
ChunkIterator::data() {
  Chunk::data_type data = m_chunk->at_memory(m_first, m_iterator);
  data.second           = std::min(data.second, m_last - m_first);

  return data;
}

inline bool
ChunkIterator::next() {
  m_first = m_iterator->position() + m_iterator->size();

  while (++m_iterator != m_chunk->end()) {
    if (m_iterator->size() != 0)
      return m_first < m_last;
  }

  return false;
}

// Returns true if the new position is on a file boundary while not at
// the edges of the chunk.
//
// Do not return true if the length was zero, in order to avoid
// getting stuck looping when no data is being read/written.
inline bool
ChunkIterator::forward(uint32_t length) {
  m_first += length;

  if (m_first >= m_last)
    return false;

  do {
    if (m_first < m_iterator->position() + m_iterator->size())
      return true;

    m_iterator++;
  } while (m_iterator != m_chunk->end());

  return false;
}

} // namespace torrent

#endif
