// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2005-2011, Jari Sundell <jaris@ifi.uio.no>

#include <algorithm>
#include <csetjmp>
#include <csignal>
#include <cstring>
#include <functional>

#include "data/chunk.h"
#include "data/chunk_iterator.h"
#include "torrent/exceptions.h"

sigjmp_buf jmp_disk_full;

void
bus_handler(int, siginfo_t* si, void*) {
  if (si->si_code == BUS_ADRERR)
    siglongjmp(jmp_disk_full, 1);
}

namespace torrent {

static inline int
intercept_sigbus(struct sigaction* oldact) noexcept(false) {
  struct sigaction sa;
  std::memset(&sa, 0, sizeof(sa));
  sa.sa_sigaction = bus_handler;
  sa.sa_flags     = SA_SIGINFO;
  sigfillset(&sa.sa_mask);
  sigaction(SIGBUS, &sa, oldact);

  if (sigsetjmp(jmp_disk_full, 1)) {
    return 1;
  }

  return 0;
}

bool
Chunk::is_all_valid() const {
  return !empty() && std::all_of(begin(), end(), [](const ChunkPart& c) {
    return c.is_valid();
  });
}

void
Chunk::clear() {
  std::for_each(begin(), end(), std::mem_fn(&ChunkPart::clear));

  m_chunkSize = 0;
  m_prot      = ~0;
  base_type::clear();
}

// Each add calls vector's reserve adding 1. This should keep
// the size of the vector at exactly what we need. Though it
// will require a few more cycles, it won't matter as we only
// rarely have more than 1 or 2 nodes.
//
// If the user knows how many chunk parts he is going to add, then he
// may call reserve prior to this.
void
Chunk::push_back(value_type::mapped_type mapped, const MemoryChunk& c) {
  m_prot &= c.get_prot();

  // Gcc starts the reserved size at 1 for the first insert, so we
  // won't be wasting any space in the general case.
  base_type::insert(end(), ChunkPart(mapped, c, m_chunkSize));

  m_chunkSize += c.size();
}

Chunk::iterator
Chunk::at_position(uint32_t pos) {
  if (pos >= m_chunkSize)
    throw internal_error(
      "Chunk::at_position(...) tried to get Chunk position out of range.");

  iterator itr = std::find_if(
    begin(), end(), [pos](ChunkPart& p) { return p.is_contained(pos); });

  if (itr == end())
    throw internal_error("Chunk::at_position(...) might be mangled, "
                         "at_position failed horribly");

  if (itr->size() == 0)
    throw internal_error(
      "Chunk::at_position(...) tried to return a node with length 0");

  return itr;
}

Chunk::data_type
Chunk::at_memory(uint32_t offset, iterator part) {
  if (part == end())
    throw internal_error("Chunk::at_memory(...) reached end.");

  if (!part->chunk().is_valid())
    throw internal_error("Chunk::at_memory(...) chunk part isn't valid.");

  if (offset < part->position() || offset >= part->position() + part->size())
    throw internal_error("Chunk::at_memory(...) out of range.");

  offset -= part->position();

  return data_type(part->chunk().begin() + offset, part->size() - offset);
}

bool
Chunk::is_incore(uint32_t pos, uint32_t length) {
  iterator itr = at_position(pos);

  if (itr == end())
    throw internal_error("Chunk::incore_length(...) at end()");

  uint32_t last = pos + std::min(length, chunk_size() - pos);

  while (itr->is_incore(pos, last - pos)) {
    if (++itr == end() || itr->position() >= last)
      return true;

    pos = itr->position();
  }

  return false;
}

// TODO: Buggy, hitting internal_error. Likely need to fix
// ChunkPart::incore_length's length align.
uint32_t
Chunk::incore_length(uint32_t pos, uint32_t length) {
  uint32_t result = 0;
  iterator itr    = at_position(pos);

  if (itr == end())
    throw internal_error("Chunk::incore_length(...) at end()");

  length = std::min(length, chunk_size() - pos);

  do {
    uint32_t incore_len = itr->incore_length(pos, length);

    if (incore_len > length)
      throw internal_error("Chunk::incore_length(...) incore_len > length.");

    pos += incore_len;
    length -= incore_len;
    result += incore_len;

  } while (pos == itr->position() + itr->size() && ++itr != end());

  return result;
}

bool
Chunk::sync(int flags) {
  bool success = true;

  for (auto& part : *this) {
    if (!part.chunk().sync(0, part.chunk().size(), flags)) {
      success = false;
    }
  }

  return success;
}

void
Chunk::preload(uint32_t position, uint32_t length, bool useAdvise) {
  if (position >= m_chunkSize)
    throw internal_error("Chunk::preload(...) position > m_chunkSize.");

  if (length == 0)
    return;

  Chunk::data_type data;
  ChunkIterator    itr(
    this, position, position + std::min(length, m_chunkSize - position));

  do {
    data = itr.data();

    // Don't do preloading for zero-length chunks.

    if (useAdvise) {
      itr.memory_chunk()->advise(
        itr.memory_chunk_first(), data.second, MemoryChunk::advice_willneed);

    } else {
      for (char *first = (char*)data.first,
                *last  = (char*)data.first + data.second;
           first < last;
           first += 4096)
        volatile char __attribute__((unused)) touchChunk = *(char*)data.first;

      // Make sure we touch the last page in the range.
      volatile char __attribute__((unused)) touchChunk =
        *((char*)data.first + data.second - 1);
    }

  } while (itr.next());
}

// Consider using uint32_t returning first mismatch or length if
// matching.
bool
Chunk::to_buffer(void* buffer, uint32_t position, uint32_t length) {
  if (position + length > m_chunkSize)
    throw internal_error(
      "Chunk::to_buffer(...) position + length > m_chunkSize.");

  if (length == 0)
    return true;

  Chunk::data_type data;
  ChunkIterator    itr(this, position, position + length);

  do {
    data = itr.data();
    std::memcpy(buffer, data.first, data.second);

    buffer = static_cast<char*>(buffer) + data.second;
  } while (itr.next());

  return true;
}

// Consider using uint32_t returning first mismatch or length if
// matching.
bool
Chunk::from_buffer(const void* buffer, uint32_t position, uint32_t length) {
  if (position + length > m_chunkSize)
    throw internal_error(
      "Chunk::from_buffer(...) position + length > m_chunkSize.");

  if (length == 0)
    return true;

  Chunk::data_type data;
  ChunkIterator    itr(this, position, position + length);

  // Start to intercept SIGBUS
  struct sigaction oldact;
  if (intercept_sigbus(&oldact)) {
    // Stop intercepting SIGBUS
    sigaction(SIGBUS, &oldact, nullptr);
    throw storage_error("no space left on disk");
  }

  do {
    data = itr.data();
    std::memcpy(data.first, buffer, data.second);

    buffer = static_cast<const char*>(buffer) + data.second;
  } while (itr.next());

  // Stop intercepting SIGBUS
  sigaction(SIGBUS, &oldact, nullptr);

  return true;
}

// Consider using uint32_t returning first mismatch or length if
// matching.
bool
Chunk::compare_buffer(const void* buffer, uint32_t position, uint32_t length) {
  if (position + length > m_chunkSize)
    throw internal_error(
      "Chunk::compare_buffer(...) position + length > m_chunkSize.");

  if (length == 0)
    return true;

  Chunk::data_type data;
  ChunkIterator    itr(this, position, position + length);

  do {
    data = itr.data();

    if (std::memcmp(data.first, buffer, data.second) != 0)
      return false;

    buffer = static_cast<const char*>(buffer) + data.second;
  } while (itr.next());

  return true;
}

} // namespace torrent
