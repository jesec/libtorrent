// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2005-2011, Jari Sundell <jaris@ifi.uio.no>

// Fucked up ugly piece of hack, this code.

#include <algorithm>
#include <cinttypes>

#include "download/delegator.h"
#include "protocol/peer_chunks.h"
#include "torrent/bitfield.h"
#include "torrent/data/block.h"
#include "torrent/data/block_list.h"
#include "torrent/data/block_transfer.h"
#include "torrent/exceptions.h"

namespace torrent {

struct DelegatorCheckAffinity {
  DelegatorCheckAffinity(Delegator*      delegator,
                         Block**         target,
                         unsigned int    index,
                         const PeerInfo* peerInfo)
    : m_delegator(delegator)
    , m_target(target)
    , m_index(index)
    , m_peerInfo(peerInfo) {}

  bool operator()(BlockList* d) {
    return m_index == d->index() &&
           (*m_target = m_delegator->delegate_piece(d, m_peerInfo)) != nullptr;
  }

  Delegator*      m_delegator;
  Block**         m_target;
  unsigned int    m_index;
  const PeerInfo* m_peerInfo;
};

struct DelegatorCheckSeeder {
  DelegatorCheckSeeder(Delegator*      delegator,
                       Block**         target,
                       const PeerInfo* peerInfo)
    : m_delegator(delegator)
    , m_target(target)
    , m_peerInfo(peerInfo) {}

  bool operator()(BlockList* d) {
    return d->by_seeder() &&
           (*m_target = m_delegator->delegate_piece(d, m_peerInfo)) != nullptr;
  }

  Delegator*      m_delegator;
  Block**         m_target;
  const PeerInfo* m_peerInfo;
};

struct DelegatorCheckPriority {
  DelegatorCheckPriority(Delegator*        delegator,
                         Block**           target,
                         priority_t        p,
                         const PeerChunks* peerChunks)
    : m_delegator(delegator)
    , m_target(target)
    , m_priority(p)
    , m_peerChunks(peerChunks) {}

  bool operator()(BlockList* d) {
    return m_priority == d->priority() &&
           m_peerChunks->bitfield()->get(d->index()) &&
           (*m_target = m_delegator->delegate_piece(
              d, m_peerChunks->peer_info())) != nullptr;
  }

  Delegator*        m_delegator;
  Block**           m_target;
  priority_t        m_priority;
  const PeerChunks* m_peerChunks;
};

// TODO: Should this ensure we don't download pieces that are priority off?
struct DelegatorCheckAggressive {
  DelegatorCheckAggressive(Delegator*        delegator,
                           Block**           target,
                           uint16_t*         o,
                           const PeerChunks* peerChunks)
    : m_delegator(delegator)
    , m_target(target)
    , m_overlapp(o)
    , m_peerChunks(peerChunks) {}

  bool operator()(BlockList* d) {
    Block* tmp;

    if (!m_peerChunks->bitfield()->get(d->index()) ||
        d->priority() == PRIORITY_OFF ||
        (tmp = m_delegator->delegate_aggressive(
           d, m_overlapp, m_peerChunks->peer_info())) == nullptr)
      return false;

    *m_target = tmp;
    return m_overlapp == nullptr;
  }

  Delegator*        m_delegator;
  Block**           m_target;
  uint16_t*         m_overlapp;
  const PeerChunks* m_peerChunks;
};

BlockTransfer*
Delegator::delegate(PeerChunks* peerChunks, int affinity) {
  // TODO: Make sure we don't queue the same piece several time on the same peer
  // when it timeout cancels them.
  Block* target = nullptr;

  // Find piece with same index as affinity. This affinity should ensure that we
  // never start another piece while the chunk this peer used to download is
  // still in progress.
  //
  // TODO: What if the hash failed? Don't want data from that peer again.
  if (affinity >= 0 &&
      std::any_of(m_transfers.begin(),
                  m_transfers.end(),
                  DelegatorCheckAffinity(
                    this, &target, affinity, peerChunks->peer_info())))
    return target->insert(peerChunks->peer_info());

  if (peerChunks->is_seeder() &&
      (target = delegate_seeder(peerChunks)) != nullptr)
    return target->insert(peerChunks->peer_info());

  // High priority pieces.
  if (std::any_of(
        m_transfers.begin(),
        m_transfers.end(),
        DelegatorCheckPriority(this, &target, PRIORITY_HIGH, peerChunks)))
    return target->insert(peerChunks->peer_info());

  // Find normal priority pieces.
  if ((target = new_chunk(peerChunks, true)))
    return target->insert(peerChunks->peer_info());

  // Normal priority pieces.
  if (std::any_of(
        m_transfers.begin(),
        m_transfers.end(),
        DelegatorCheckPriority(this, &target, PRIORITY_NORMAL, peerChunks)))
    return target->insert(peerChunks->peer_info());

  if ((target = new_chunk(peerChunks, false)))
    return target->insert(peerChunks->peer_info());

  if (!m_aggressive)
    return nullptr;

  // Aggressive mode, look for possible downloads that already have
  // one or more queued.

  // No more than 4 per piece.
  uint16_t overlapped = 5;

  std::find_if(
    m_transfers.begin(),
    m_transfers.end(),
    DelegatorCheckAggressive(this, &target, &overlapped, peerChunks));

  return target ? target->insert(peerChunks->peer_info()) : nullptr;
}

Block*
Delegator::delegate_seeder(PeerChunks* peerChunks) {
  Block* target = nullptr;

  if (std::any_of(m_transfers.begin(),
                  m_transfers.end(),
                  DelegatorCheckSeeder(this, &target, peerChunks->peer_info())))
    return target;

  if ((target = new_chunk(peerChunks, true)))
    return target;

  if ((target = new_chunk(peerChunks, false)))
    return target;

  return nullptr;
}

Block*
Delegator::new_chunk(PeerChunks* pc, bool highPriority) {
  uint32_t index = m_slot_chunk_find(pc, highPriority);

  if (index == ~(uint32_t)0)
    return nullptr;

  TransferList::iterator itr =
    m_transfers.insert(Piece(index, 0, m_slot_chunk_size(index)), block_size);

  (*itr)->set_by_seeder(pc->is_seeder());

  if (highPriority)
    (*itr)->set_priority(PRIORITY_HIGH);
  else
    (*itr)->set_priority(PRIORITY_NORMAL);

  return &*(*itr)->begin();
}

Block*
Delegator::delegate_piece(BlockList* blockList, const PeerInfo* peerInfo) {
  Block* p = nullptr;

  for (auto& block : *blockList) {
    if (block.is_finished() || !block.is_stalled())
      continue;

    if (block.size_all() == 0) {
      // No one is downloading this, assign.
      return &block;

    } else if (p == nullptr && block.find(peerInfo) == nullptr) {
      // Stalled but we really want to finish this piece. Check 'p' so
      // that we don't end up queuing the pieces in reverse.
      p = &block;
    }
  }

  return p;
}

Block*
Delegator::delegate_aggressive(BlockList*      c,
                               uint16_t*       overlapped,
                               const PeerInfo* peerInfo) {
  Block* p = nullptr;

  for (BlockList::iterator i = c->begin(); i != c->end() && *overlapped != 0;
       ++i)
    if (!i->is_finished() && i->size_not_stalled() < *overlapped &&
        i->find(peerInfo) == nullptr) {
      p           = &*i;
      *overlapped = i->size_not_stalled();
    }

  return p;
}

} // namespace torrent
