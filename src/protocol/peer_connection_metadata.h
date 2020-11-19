// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2005-2011, Jari Sundell <jaris@ifi.uio.no>

#ifndef LIBTORRENT_PROTOCOL_PEER_CONNECTION_METADATA_H
#define LIBTORRENT_PROTOCOL_PEER_CONNECTION_METADATA_H

#include "peer_connection_base.h"

#include "torrent/download.h"

namespace torrent {

class PeerConnectionMetadata : public PeerConnectionBase {
public:
  ~PeerConnectionMetadata();

  virtual void initialize_custom();
  virtual void update_interested();
  virtual bool receive_keepalive();

  virtual void event_read();
  virtual void event_write();

  virtual void receive_metadata_piece(uint32_t    piece,
                                      const char* data,
                                      uint32_t    length);

private:
  inline bool read_message();

  bool read_skip_bitfield();

  bool try_request_metadata_pieces();

  inline void fill_write_buffer();

  uint32_t m_skipLength;
};

} // namespace torrent

#endif
