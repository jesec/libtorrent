// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2005-2011, Jari Sundell <jaris@ifi.uio.no>

#ifndef LIBTORRENT_PEER_PEER_FACTORY_H
#define LIBTORRENT_PEER_PEER_FACTORY_H

namespace torrent {

class PeerConnectionBase;

PeerConnectionBase*
createPeerConnectionDefault(bool encrypted);
PeerConnectionBase*
createPeerConnectionSeed(bool encrypted);
PeerConnectionBase*
createPeerConnectionInitialSeed(bool encrypted);
PeerConnectionBase*
createPeerConnectionMetadata(bool encrypted);

} // namespace torrent

#endif
