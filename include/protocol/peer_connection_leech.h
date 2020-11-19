// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2005-2011, Jari Sundell <jaris@ifi.uio.no>

#ifndef LIBTORRENT_PROTOCOL_PEER_CONNECTION_LEECH_H
#define LIBTORRENT_PROTOCOL_PEER_CONNECTION_LEECH_H

#include "peer_connection_base.h"

#include "torrent/download.h"

namespace torrent {

// Type-specific data.
template<Download::ConnectionType type>
struct PeerConnectionData;

template<>
struct PeerConnectionData<Download::CONNECTION_LEECH> {};

template<>
struct PeerConnectionData<Download::CONNECTION_SEED> {};

template<>
struct PeerConnectionData<Download::CONNECTION_INITIAL_SEED> {
  PeerConnectionData()
    : lastIndex(~uint32_t()) {}
  uint32_t lastIndex;
  uint32_t bytesLeft;
};

template<Download::ConnectionType type>
class PeerConnection : public PeerConnectionBase {
public:
  ~PeerConnection();

  virtual void initialize_custom();
  virtual void update_interested();
  virtual bool receive_keepalive();

  virtual void event_read();
  virtual void event_write();

private:
  inline bool read_message();
  void        read_have_chunk(uint32_t index);

  void offer_chunk();
  bool should_upload();

  inline void fill_write_buffer();

  PeerConnectionData<type> m_data;
};

} // namespace torrent

#endif
