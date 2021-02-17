// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2005-2011, Jari Sundell <jaris@ifi.uio.no>

#ifndef LIBTORRENT_DATA_STORAGE_CHUNK_PART_H
#define LIBTORRENT_DATA_STORAGE_CHUNK_PART_H

#include "memory_chunk.h"
#include "torrent/utils/cacheline.h"

namespace torrent {

class File;

class lt_cacheline_aligned ChunkPart {
public:
  using mapped_type = enum { MAPPED_MMAP, MAPPED_STATIC };

  ChunkPart(mapped_type mapped, const MemoryChunk& c, uint32_t pos)
    : m_mapped(mapped)
    , m_chunk(c)
    , m_position(pos)
    , m_file(nullptr)
    , m_file_offset(0) {}

  bool is_valid() const {
    return m_chunk.is_valid();
  }
  bool is_contained(uint32_t p) const {
    return p >= m_position && p < m_position + size();
  }

  bool has_address(void* p) const {
    return (char*)p >= m_chunk.begin() && p < m_chunk.end();
  }

  void clear();

  mapped_type mapped() const {
    return m_mapped;
  }

  MemoryChunk& chunk() {
    return m_chunk;
  }
  const MemoryChunk& chunk() const {
    return m_chunk;
  }

  uint32_t size() const {
    return m_chunk.size();
  }
  uint32_t position() const {
    return m_position;
  }

  uint32_t remaining_from(uint32_t pos) const {
    return size() - (pos - m_position);
  }

  File* file() const {
    return m_file;
  }
  uint64_t file_offset() const {
    return m_file_offset;
  }

  void set_file(File* f, uint64_t f_offset) {
    m_file        = f;
    m_file_offset = f_offset;
  }

  bool     is_incore(uint32_t pos, uint32_t length = ~uint32_t());
  uint32_t incore_length(uint32_t pos, uint32_t length = ~uint32_t());

private:
  mapped_type m_mapped;

  MemoryChunk m_chunk;
  uint32_t    m_position;

  // Currently used to figure out what file and where a SIGBUS
  // occurred. Can also be used in the future to indicate if a part is
  // temporary storage, etc.
  File*    m_file;
  uint64_t m_file_offset;
};

} // namespace torrent

#endif
