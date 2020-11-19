// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2005-2011, Jari Sundell <jaris@ifi.uio.no>

#include "config.h"

#include "protocol/peer_connection_leech.h"
#include "protocol/peer_connection_metadata.h"
#include "protocol/peer_factory.h"

namespace torrent {

PeerConnectionBase*
createPeerConnectionDefault(bool) {
  PeerConnectionBase* pc = new PeerConnection<Download::CONNECTION_LEECH>;

  return pc;
}

PeerConnectionBase*
createPeerConnectionSeed(bool) {
  PeerConnectionBase* pc = new PeerConnection<Download::CONNECTION_SEED>;

  return pc;
}

PeerConnectionBase*
createPeerConnectionInitialSeed(bool) {
  PeerConnectionBase* pc =
    new PeerConnection<Download::CONNECTION_INITIAL_SEED>;

  return pc;
}

PeerConnectionBase*
createPeerConnectionMetadata(bool) {
  PeerConnectionBase* pc = new PeerConnectionMetadata;

  return pc;
}

} // namespace torrent
