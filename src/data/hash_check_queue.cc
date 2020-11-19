// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2005-2011, Jari Sundell <jaris@ifi.uio.no>

#include "config.h"

#include "hash_check_queue.h"

#include "data/hash_chunk.h"
#include "torrent/hash_string.h"
#include "utils/instrumentation.h"

namespace torrent {

HashCheckQueue::HashCheckQueue() {
  pthread_mutex_init(&m_lock, NULL);
}

HashCheckQueue::~HashCheckQueue() {
  pthread_mutex_destroy(&m_lock);
}

// Always poke thread_disk after calling this.
void
HashCheckQueue::push_back(HashChunk* hash_chunk) {
  if (hash_chunk == NULL || !hash_chunk->chunk()->is_loaded() ||
      !hash_chunk->chunk()->is_blocking())
    throw internal_error("Invalid hash chunk passed to HashCheckQueue.");

  pthread_mutex_lock(&m_lock);

  // Set blocking...(? this needs to be possible to do after getting
  // the chunk) When doing this make sure we verify that the handle is
  // not previously blocked.

  base_type::push_back(hash_chunk);

  int64_t size = hash_chunk->chunk()->chunk()->chunk_size();
  instrumentation_update(INSTRUMENTATION_MEMORY_HASHING_CHUNK_COUNT, 1);
  instrumentation_update(INSTRUMENTATION_MEMORY_HASHING_CHUNK_USAGE, size);

  pthread_mutex_unlock(&m_lock);
}

// erase...
//
// The erasing function should call slot, perhaps return a bool if we
// deleted, or in the rare case it has already been hashed and will
// arraive in the near future?
//
// We could handle this by polling the done chunks on false return
// values.

// Lock the chunk list and increment blocking only when starting the
// checking.

// No removal of entries, only clearing.

bool
HashCheckQueue::remove(HashChunk* hash_chunk) {
  pthread_mutex_lock(&m_lock);

  bool     result;
  iterator itr = std::find(begin(), end(), hash_chunk);

  if (itr != end()) {
    base_type::erase(itr);
    result = true;

    int64_t size = hash_chunk->chunk()->chunk()->chunk_size();
    instrumentation_update(INSTRUMENTATION_MEMORY_HASHING_CHUNK_COUNT, -1);
    instrumentation_update(INSTRUMENTATION_MEMORY_HASHING_CHUNK_USAGE, -size);

  } else {
    result = false;
  }

  pthread_mutex_unlock(&m_lock);
  return result;
}

void
HashCheckQueue::perform() {
  pthread_mutex_lock(&m_lock);

  while (!empty()) {
    HashChunk* hash_chunk = base_type::front();
    base_type::pop_front();

    if (!hash_chunk->chunk()->is_loaded())
      throw internal_error(
        "HashCheckQueue::perform(): !entry.node->is_loaded().");

    int64_t size = hash_chunk->chunk()->chunk()->chunk_size();
    instrumentation_update(INSTRUMENTATION_MEMORY_HASHING_CHUNK_COUNT, -1);
    instrumentation_update(INSTRUMENTATION_MEMORY_HASHING_CHUNK_USAGE, -size);

    pthread_mutex_unlock(&m_lock);

    if (!hash_chunk->perform(~uint32_t(), true))
      throw internal_error("HashCheckQueue::perform(): "
                           "!hash_chunk->perform(~uint32_t(), true).");

    HashString hash;
    hash_chunk->hash_c(hash.data());

    m_slot_chunk_done(hash_chunk, hash);
    pthread_mutex_lock(&m_lock);
  }

  pthread_mutex_unlock(&m_lock);
}

} // namespace torrent
