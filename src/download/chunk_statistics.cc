// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2005-2011, Jari Sundell <jaris@ifi.uio.no>

#include "config.h"

#include "torrent/exceptions.h"

#include "protocol/peer_chunks.h"

#include "chunk_statistics.h"

namespace torrent {

inline bool
ChunkStatistics::should_add(PeerChunks*) {
  return m_accounted < max_accounted;
}

void
ChunkStatistics::initialize(size_type s) {
  if (!empty())
    throw internal_error(
      "ChunkStatistics::initialize(...) called on an initialized object.");

  base_type::resize(s);
}

void
ChunkStatistics::clear() {
  if (m_complete != 0)
    throw internal_error("ChunkStatistics::clear() m_complete != 0.");

  base_type::clear();
}

void
ChunkStatistics::received_connect(PeerChunks* pc) {
  if (pc->using_counter())
    throw internal_error(
      "ChunkStatistics::received_connect(...) pc->using_counter() == true.");

  if (pc->bitfield()->is_all_set()) {
    pc->set_using_counter(true);
    m_complete++;

  } else if (!pc->bitfield()->is_all_unset() && should_add(pc)) {
    // There should be additional checks, so that we don't do this
    // when there's no need.
    pc->set_using_counter(true);
    m_accounted++;

    iterator itr = base_type::begin();

    // Use a bitfield iterator instead.
    for (Bitfield::size_type index = 0; index < pc->bitfield()->size_bits();
         ++index, ++itr)
      *itr += pc->bitfield()->get(index);
  }
}

void
ChunkStatistics::received_disconnect(PeerChunks* pc) {

  // The 'using_counter' of complete peers is used, but not added to
  // 'm_accounted', so that we can safely disconnect peers right after
  // receiving the bitfield without calling 'received_connect'.
  if (!pc->using_counter())
    return;

  pc->set_using_counter(false);

  if (pc->bitfield()->is_all_set()) {
    m_complete--;

  } else {
    if (m_accounted == 0)
      throw internal_error(
        "ChunkStatistics::received_disconnect(...) m_accounted == 0.");

    m_accounted--;

    iterator itr = base_type::begin();

    // Use a bitfield iterator instead.
    for (Bitfield::size_type index = 0; index < pc->bitfield()->size_bits();
         ++index, ++itr)
      *itr -= pc->bitfield()->get(index);
  }
}

void
ChunkStatistics::received_have_chunk(PeerChunks* pc,
                                     uint32_t    index,
                                     uint32_t    length) {
  // When the bitfield is empty, it is very cheap to add the peer to
  // the statistics. It needs to be done here else we need to check if
  // a connection has sent any messages, else it might send a bitfield.
  if (pc->bitfield()->is_all_unset() && should_add(pc)) {

    if (pc->using_counter())
      throw internal_error("ChunkStatistics::received_have_chunk(...) "
                           "pc->using_counter() == true.");

    pc->set_using_counter(true);
    m_accounted++;
  }

  pc->bitfield()->set(index);
  pc->peer_rate()->insert(length);

  if (pc->using_counter()) {

    base_type::operator[](index)++;

    // The below code should not cause useless work to be done in case
    // of immediate disconnect.
    if (pc->bitfield()->is_all_set()) {
      if (m_accounted == 0)
        throw internal_error(
          "ChunkStatistics::received_disconnect(...) m_accounted == 0.");

      m_complete++;
      m_accounted--;

      for (iterator itr = base_type::begin(), last = base_type::end();
           itr != last;
           ++itr)
        *itr -= 1;
    }

  } else {

    if (pc->bitfield()->is_all_set()) {
      pc->set_using_counter(true);
      m_complete++;
    }
  }
}

} // namespace torrent
