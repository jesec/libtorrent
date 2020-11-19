// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2005-2011, Jari Sundell <jaris@ifi.uio.no>

#ifndef LIBTORRENT_PROTOCOL_INITIAL_SEED_H
#define LIBTORRENT_PROTOCOL_INITIAL_SEED_H

#include "download/download_main.h"

namespace torrent {

class InitialSeeding {
public:
  InitialSeeding(DownloadMain* download);
  ~InitialSeeding();

  static const uint32_t no_offer = ~uint32_t();

  void new_peer(PeerConnectionBase* pcb);

  // Chunk was seen distributed to a peer in the swarm.
  void chunk_seen(uint32_t index, PeerConnectionBase* pcb);

  // Returns chunk we may offer the peer or no_offer if none.
  uint32_t chunk_offer(PeerConnectionBase* pcb, uint32_t indexDone);

  // During the second stage (seeding rare chunks), return
  // false if given chunk is already well-seeded now. True otherwise.
  bool should_upload(uint32_t index);

private:
  static PeerInfo* const chunk_unsent; // Chunk never sent to anyone.
  static PeerInfo* const
    chunk_unknown; // Peer has chunk, we don't know who we sent it to.
  static PeerInfo* const chunk_done; // Chunk properly distributed by peer.

  uint32_t find_next(bool secondary, PeerConnectionBase* pcb);

  bool valid_peer(PeerInfo* peer);
  void clear_peer(PeerInfo* peer);
  void chunk_complete(uint32_t index, PeerConnectionBase* pcb);

  void complete(PeerConnectionBase* pcb);
  void unblock_all();

  uint32_t      m_nextChunk;
  uint32_t      m_chunksLeft;
  DownloadMain* m_download;
  PeerInfo**    m_peerChunks;
};

} // namespace torrent

#endif
