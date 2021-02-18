// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2005-2011, Jari Sundell <jaris@ifi.uio.no>

#ifndef LIBTORRENT_DELEGATOR_H
#define LIBTORRENT_DELEGATOR_H

#include <functional>
#include <string>
#include <vector>

#include "torrent/data/transfer_list.h"

namespace torrent {

class Block;
class BlockList;
class BlockTransfer;
class Piece;
class PeerChunks;
class PeerInfo;

class Delegator {
public:
  using slot_peer_chunk = std::function<uint32_t(PeerChunks*, bool)>;
  using slot_size       = std::function<uint32_t(uint32_t)>;

  static constexpr unsigned int block_size = 1 << 14;

  TransferList* transfer_list() {
    return &m_transfers;
  }
  const TransferList* transfer_list() const {
    return &m_transfers;
  }

  BlockTransfer* delegate(PeerChunks* peerChunks, int affinity);

  bool get_aggressive() {
    return m_aggressive;
  }
  void set_aggressive(bool a) {
    m_aggressive = a;
  }

  slot_peer_chunk& slot_chunk_find() {
    return m_slot_chunk_find;
  }
  slot_size& slot_chunk_size() {
    return m_slot_chunk_size;
  }

  // Don't call this from the outside.
  Block* delegate_piece(BlockList* c, const PeerInfo* peerInfo);
  Block* delegate_aggressive(BlockList*      c,
                             uint16_t*       overlapped,
                             const PeerInfo* peerInfo);

private:
  // Start on a new chunk, returns .end() if none possible. bf is
  // remote peer's bitfield.
  Block* new_chunk(PeerChunks* pc, bool highPriority);

  Block* delegate_seeder(PeerChunks* peerChunks);

  TransferList m_transfers;

  bool m_aggressive{ false };

  // Propably should add a m_slotChunkStart thing, which will take
  // care of enabling etc, and will be possible to listen to.
  slot_peer_chunk m_slot_chunk_find;
  slot_size       m_slot_chunk_size;
};

} // namespace torrent

#endif
