// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2005-2011, Jari Sundell <jaris@ifi.uio.no>

#include <algorithm>
#include <unistd.h>

#include "data/chunk_part.h"
#include "torrent/exceptions.h"

namespace torrent {

void
ChunkPart::clear() {
  switch (m_mapped) {
    case MAPPED_MMAP:
      m_chunk.unmap();
      break;

    default:
    case MAPPED_STATIC:
      throw internal_error("ChunkPart::clear() only MAPPED_MMAP supported.");
      break;
  }

  m_chunk.clear();
}

bool
ChunkPart::is_incore(uint32_t pos, uint32_t length) {
  length = std::min(length, remaining_from(pos));
  pos    = pos - m_position;

  if (pos > size())
    throw internal_error("ChunkPart::is_incore(...) got invalid position.");

  if (length > size() || pos + length > size())
    throw internal_error("ChunkPart::is_incore(...) got invalid length.");

  return m_chunk.is_incore(pos, length);
}

// TODO: Buggy.
uint32_t
ChunkPart::incore_length(uint32_t pos, uint32_t length) {
  // Do we want to use this?
  length = std::min(length, remaining_from(pos));
  pos    = pos - m_position;

  if (pos >= size())
    throw internal_error("ChunkPart::incore_length(...) got invalid position");

  uint32_t touched = m_chunk.pages_touched(pos, length);
  char*    buf     = static_cast<char*>(malloc(touched * sizeof(char)));

  m_chunk.incore(buf, pos, length);

  uint32_t dist = std::distance(buf, std::find(buf, buf + touched, 0));

  free(buf);

  // This doesn't properly account for alignment when calculating the length.
  return std::min(
    dist ? (dist * m_chunk.page_size() - m_chunk.page_align()) : 0, length);
}

} // namespace torrent
