// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2005-2011, Jari Sundell <jaris@ifi.uio.no>

#ifndef LIBTORRENT_DATA_CHUNK_LIST_NODE_H
#define LIBTORRENT_DATA_CHUNK_LIST_NODE_H

#include "torrent/buildinfo.h"

#include <cinttypes>
#include <cstdlib>
#include <rak/timer.h>

namespace torrent {

class Chunk;

// ChunkNode can contain information like how long since it was last
// used, last synced, last checked with mincore and how many
// references there are to it.
//
// ChunkList will make sure all the nodes are cleaned up properly, so
// no dtor is needed.

class lt_cacheline_aligned ChunkListNode {
public:
  static const uint32_t invalid_index = ~uint32_t();

  ChunkListNode()
    : m_index(invalid_index)
    , m_chunk(NULL)
    , m_references(0)
    , m_writable(0)
    , m_blocking(0)
    , m_asyncTriggered(false) {}

  bool is_valid() const {
    return m_chunk != NULL;
  }

  uint32_t index() const {
    return m_index;
  }
  void set_index(uint32_t idx) {
    m_index = idx;
  }

  Chunk* chunk() const {
    return m_chunk;
  }
  void set_chunk(Chunk* c) {
    m_chunk = c;
  }

  const rak::timer& time_modified() const {
    return m_timeModified;
  }
  void set_time_modified(rak::timer t) {
    m_timeModified = t;
  }

  const rak::timer& time_preloaded() const {
    return m_timePreloaded;
  }
  void set_time_preloaded(rak::timer t) {
    m_timePreloaded = t;
  }

  bool sync_triggered() const {
    return m_asyncTriggered;
  }
  void set_sync_triggered(bool v) {
    m_asyncTriggered = v;
  }

  int references() const {
    return m_references;
  }
  int dec_references() {
    return --m_references;
  }
  int inc_references() {
    return ++m_references;
  }

  int writable() const {
    return m_writable;
  }
  int dec_writable() {
    return --m_writable;
  }
  int inc_writable() {
    return ++m_writable;
  }

  int blocking() const {
    return m_blocking;
  }
  int dec_blocking() {
    return --m_blocking;
  }
  int inc_blocking() {
    return ++m_blocking;
  }

  void inc_rw() {
    inc_writable();
    inc_references();
  }
  void dec_rw() {
    dec_writable();
    dec_references();
  }

private:
  uint32_t m_index;
  Chunk*   m_chunk;

  int m_references;
  int m_writable;
  int m_blocking;

  bool m_asyncTriggered;

  rak::timer m_timeModified;
  rak::timer m_timePreloaded;
};

} // namespace torrent

#endif
