// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2005-2011, Jari Sundell <jaris@ifi.uio.no>

#ifndef LIBTORRENT_PROTOCOL_PEER_CHUNKS_H
#define LIBTORRENT_PROTOCOL_PEER_CHUNKS_H

#include <list>

#include "net/throttle_node.h"
#include "torrent/bitfield.h"
#include "torrent/data/piece.h"
#include "torrent/rate.h"
#include "torrent/utils/partial_queue.h"
#include "torrent/utils/timer.h"

namespace torrent {

class PeerInfo;

class PeerChunks {
public:
  using piece_list_type = std::list<Piece>;

  bool is_seeder() const {
    return m_bitfield.is_all_set();
  }

  PeerInfo* peer_info() {
    return m_peerInfo;
  }
  const PeerInfo* peer_info() const {
    return m_peerInfo;
  }
  void set_peer_info(PeerInfo* p) {
    m_peerInfo = p;
  }

  bool using_counter() const {
    return m_usingCounter;
  }
  void set_using_counter(bool state) {
    m_usingCounter = state;
  }

  Bitfield* bitfield() {
    return &m_bitfield;
  }
  const Bitfield* bitfield() const {
    return &m_bitfield;
  }

  utils::partial_queue* download_cache() {
    return &m_downloadCache;
  }

  piece_list_type* upload_queue() {
    return &m_uploadQueue;
  }
  const piece_list_type* upload_queue() const {
    return &m_uploadQueue;
  }
  piece_list_type* cancel_queue() {
    return &m_cancelQueue;
  }

  // Timer used to figure out what HAVE_PIECE messages have not been
  // sent.
  utils::timer have_timer() const {
    return m_haveTimer;
  }
  void set_have_timer(utils::timer t) {
    m_haveTimer = t;
  }

  Rate* peer_rate() {
    return &m_peerRate;
  }
  const Rate* peer_rate() const {
    return &m_peerRate;
  }

  ThrottleNode* download_throttle() {
    return &m_downloadThrottle;
  }
  const ThrottleNode* download_throttle() const {
    return &m_downloadThrottle;
  }
  ThrottleNode* upload_throttle() {
    return &m_uploadThrottle;
  }
  const ThrottleNode* upload_throttle() const {
    return &m_uploadThrottle;
  }

private:
  PeerInfo* m_peerInfo{ nullptr };

  bool m_usingCounter{ false };

  Bitfield m_bitfield;

  utils::partial_queue m_downloadCache;

  piece_list_type m_uploadQueue;
  piece_list_type m_cancelQueue;

  utils::timer m_haveTimer;

  Rate m_peerRate{ 600 };

  ThrottleNode m_downloadThrottle{ 30 };
  ThrottleNode m_uploadThrottle{ 30 };
};

} // namespace torrent

#endif
