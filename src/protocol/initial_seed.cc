// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2005-2011, Jari Sundell <jaris@ifi.uio.no>

#include <cstring>

#include "download/chunk_statistics.h"
#include "protocol/initial_seed.h"
#include "protocol/peer_connection_leech.h"
#include "torrent/download/choke_group.h"
#include "torrent/download/choke_queue.h"

namespace torrent {

PeerInfo* const InitialSeeding::chunk_unsent  = (PeerInfo*)nullptr;
PeerInfo* const InitialSeeding::chunk_unknown = (PeerInfo*)1;
PeerInfo* const InitialSeeding::chunk_done    = (PeerInfo*)2;

InitialSeeding::InitialSeeding(DownloadMain* download)
  : m_nextChunk(0)
  , m_chunksLeft(download->file_list()->size_chunks())
  , m_download(download)
  , m_peerChunks(new PeerInfo*[m_chunksLeft]) {

  memset(m_peerChunks, 0, m_chunksLeft * sizeof(m_peerChunks[0]));
}

InitialSeeding::~InitialSeeding() {
  unblock_all();
  delete[] m_peerChunks;
}

inline bool
InitialSeeding::valid_peer(PeerInfo* peer) {
  return peer > chunk_done;
}

void
InitialSeeding::clear_peer(PeerInfo* peer) {
  if (!valid_peer(peer))
    return;

  peer->unset_flags(PeerInfo::flag_blocked);

  // If peer is still connected, offer new piece right away.
  if (peer->connection() != nullptr)
    peer->connection()->write_insert_poll_safe();
}

void
InitialSeeding::chunk_seen(uint32_t index, PeerConnectionBase* pcb) {
  // When we have two other seeds, trust that the download will
  // be sufficiently seeded and switch to normal seeding. This is
  // mainly for when the user accidentally enables initial seeding.
  if (m_download->chunk_statistics()->complete() > 1)
    complete(pcb);

  PeerInfo* peer = pcb->mutable_peer_info();
  PeerInfo* old  = m_peerChunks[index];

  // We didn't send this chunk. Is someone else initial seeding too?
  // Or maybe we restarted and the peer got this chunk from someone
  // we did send it to. Either way, we don't know who it belongs to.
  // Don't mark it done until we see it from someone else, though.
  if (old == chunk_unsent) {
    m_peerChunks[index] = chunk_unknown;
    return;
  }

  if (old == peer || old == chunk_done)
    return;

  // We've seen two peers on the swarm receive this chunk.
  m_peerChunks[index] = chunk_done;
  if (--m_chunksLeft == 0)
    complete(pcb);

  // The peer we sent it to originally may now receive another chunk.
  clear_peer(old);
}

void
InitialSeeding::chunk_complete(uint32_t index, PeerConnectionBase* pcb) {
  clear_peer(m_peerChunks[index]);
  m_peerChunks[index] = chunk_unknown;
  chunk_seen(index, pcb);
}

void
InitialSeeding::new_peer(PeerConnectionBase* pcb) {
  PeerInfo* peer = pcb->mutable_peer_info();
  if (peer->is_blocked())
    peer->set_flags(PeerInfo::flag_restart);

  // We don't go through the peer's entire bitfield here. This eliminates
  // cheating by sending a bogus bitfield if it figures out we are initial
  // seeding, to drop us out of it. We should see HAVE messages for pieces
  // it has that we were waiting for anyway. We will check individual chunks
  // as we are about to offer them, to avoid the overhead of checking each
  // peer's bitfield as well. If it really was cheating, the pieces it isn't
  // sharing will be sent during the second round of initial seeding.

  // If we're on the second round, don't check
  // it until we're about to offer a chunk.
  if (m_peerChunks[m_nextChunk] != chunk_unsent)
    return;

  // But during primary initial seeding (some chunks not sent at all),
  // check that nobody already has the next chunk we were going to send.
  while (m_peerChunks[m_nextChunk] == chunk_unsent &&
         (*m_download->chunk_statistics())[m_nextChunk]) {
    // Could set to chunk_done if enough peers have it, but if that was the
    // last one it could cause initial seeding to end and all connections to
    // be closed, and now is a bad time for that (still being set up). Plus
    // this gives us the opportunity to wait for HAVE messages and resend
    // the chunk if it's not being shared.
    m_peerChunks[m_nextChunk] = chunk_unknown;
    find_next(false, pcb);
  }
}

uint32_t
InitialSeeding::chunk_offer(PeerConnectionBase* pcb, uint32_t chunkDone) {
  PeerInfo* peer = pcb->mutable_peer_info();

  // If this peer completely downloaded the chunk we offered and we have too
  // many unused upload slots, give it another chunk to download for free.
  if (peer->is_blocked() && chunkDone != no_offer &&
      m_peerChunks[chunkDone] == peer &&
      m_download->choke_group()->up_queue()->size_total() * 10 <
        9 * m_download->choke_group()->up_queue()->max_unchoked()) {
    m_peerChunks[chunkDone] = chunk_unknown;
    peer->unset_flags(PeerInfo::flag_blocked);

    // Otherwise check if we can offer a chunk normally.
  } else if (peer->is_blocked()) {
    if (!peer->is_restart())
      return no_offer;

    peer->unset_flags(PeerInfo::flag_restart);

    // Re-connection of a peer we already sent a chunk.
    // Offer the same chunk again.
    PeerInfo** peerChunksEnd =
      m_peerChunks + m_download->file_list()->size_chunks();
    PeerInfo** itr =
      std::find_if(m_peerChunks, peerChunksEnd, [peer](PeerInfo* p) {
        return std::equal_to<PeerInfo*>()(p, peer);
      });
    if (itr != peerChunksEnd)
      return itr - m_peerChunks;

    // Couldn't find the chunk, we probably sent it to someone
    // else since the disconnection. So offer a new one.
  }

  uint32_t index     = m_nextChunk;
  bool     secondary = false;

  // If we already sent this chunk to someone else, we're on the second
  // (or more) round. We might have already found this chunk elsewhere on
  // the swarm since then and need to find a different one if so.
  if (m_peerChunks[index] != chunk_unsent) {
    secondary = true;

    // Accounting for peers whose bitfield we didn't check when connecting.
    // If the chunk stats say there are enough peers who have it, believe that.
    if (m_peerChunks[index] != chunk_done &&
        (*m_download->chunk_statistics())[index] > 1)
      chunk_complete(index, pcb);

    if (m_peerChunks[index] == chunk_done)
      index = find_next(true, pcb);
  }

  // When we only have one chunk left and we already offered it
  // to someone who hasn't shared it yet, offer it to everyone
  // else. We do not override the peer we sent it to, so they
  // cannot be unblocked, but when initial seeding completes
  // everyone is unblocked anyway.
  if (m_chunksLeft == 1 && valid_peer(m_peerChunks[index])) {
    peer->set_flags(PeerInfo::flag_blocked);
    return index;
  }

  // Make sure we don't accidentally offer a chunk it has
  // already, or it would never even request it from us.
  // We'll just offer it to the next peer instead.
  if (pcb->bitfield()->get(index))
    return no_offer;

  m_peerChunks[index] = peer;
  peer->set_flags(PeerInfo::flag_blocked);
  find_next(secondary, pcb);
  return index;
}

bool
InitialSeeding::should_upload(uint32_t index) {
  return m_peerChunks[index] != chunk_done;
}

uint32_t
InitialSeeding::find_next(bool secondary, PeerConnectionBase* pcb) {
  if (!secondary) {
    // Primary seeding: find next chunk not sent yet.
    while (++m_nextChunk < m_download->file_list()->size_chunks()) {
      if (m_peerChunks[m_nextChunk] == chunk_unsent) {
        if (!(*m_download->chunk_statistics())[m_nextChunk])
          return m_nextChunk;

        // Someone has this one already. We don't know if we sent it or not.
        m_peerChunks[m_nextChunk] = chunk_unknown;
      }
    }

    // Went through all chunks. Continue with secondary seeding.
    m_nextChunk--;
  }

  // Secondary seeding: find next chunk that's not done yet.
  do {
    if (++m_nextChunk == m_download->file_list()->size_chunks())
      m_nextChunk = 0;

    if (m_peerChunks[m_nextChunk] != chunk_done &&
        (*m_download->chunk_statistics())[m_nextChunk] > 1)
      chunk_complete(m_nextChunk, pcb);

  } while (m_peerChunks[m_nextChunk] == chunk_done);

  return m_nextChunk;
}

void
InitialSeeding::complete(PeerConnectionBase* pcb) {
  unblock_all();
  m_chunksLeft = 0;
  m_nextChunk  = no_offer;

  // We think all chunks should be well seeded now. Check to make sure.
  for (uint32_t i = 0; i < m_download->file_list()->size_chunks(); i++) {
    if (m_download->chunk_statistics()->complete() +
          (*m_download->chunk_statistics())[i] <
        2) {
      // Chunk too rare, send it again before switching to normal seeding.
      m_chunksLeft++;
      m_peerChunks[i] = chunk_unsent;
      if (m_nextChunk == no_offer)
        m_nextChunk = i;
    }
  }

  if (m_chunksLeft)
    return;

  m_download->initial_seeding_done(pcb);
}

void
InitialSeeding::unblock_all() {
  for (const auto& peer_pair : *m_download->peer_list())
    peer_pair.second->unset_flags(PeerInfo::flag_blocked);
}

} // namespace torrent
