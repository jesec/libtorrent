// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2005-2011, Jari Sundell <jaris@ifi.uio.no>

#ifndef LIBTORRENT_COMMON_H
#define LIBTORRENT_COMMON_H

#include <cinttypes>
#include <cstddef>

struct sockaddr;
struct sockaddr_in;
struct sockaddr_in6;
struct sockaddr_un;

namespace torrent {

enum priority_enum { PRIORITY_OFF = 0, PRIORITY_NORMAL, PRIORITY_HIGH };

using priority_t = priority_enum;

// Just forward declare everything here so we can keep the actual
// headers clean.
class AvailableList;
class Bitfield;
class Block;
class BlockFailed;
class BlockList;
class BlockTransfer;
class Chunk;
class ChunkList;
class ChunkManager;
class ChunkSelector;
class ClientInfo;
class ClientList;
class ConnectionList;
class ConnectionManager;
class DhtManager;
class DhtRouter;
class Download;
class DownloadMain;
class DownloadWrapper;
class FileList;
class Event;
class File;
class FileList;
class Handshake;
class HandshakeManager;
class HashString;
class Listen;
class MemoryChunk;
class Object;
class Path;
class Peer;
class PeerConnectionBase;
class PeerInfo;
class PeerList;
class Piece;
class Poll;
class ProtocolExtension;
class Rate;
class SocketSet;
class Throttle;
class Tracker;
class TrackerList;
class TransferList;

// This should only need to be set when compiling libtorrent.
#ifdef EXPORT_LIBTORRENT_SYMBOLS
#define LIBTORRENT_NO_EXPORT __attribute__((visibility("hidden")))
#define LIBTORRENT_EXPORT    __attribute__((visibility("default")))
#else
#define LIBTORRENT_NO_EXPORT
#define LIBTORRENT_EXPORT
#endif

} // namespace torrent

#endif
