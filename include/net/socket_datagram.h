// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2005-2011, Jari Sundell <jaris@ifi.uio.no>

#ifndef LIBTORRENT_NET_SOCKET_DGRAM_H
#define LIBTORRENT_NET_SOCKET_DGRAM_H

#include "socket_base.h"

namespace torrent {

class SocketDatagram : public SocketBase {
public:
  // TODO: Make two seperate functions depending on whetever sa is
  // used.
  int read_datagram(void*                  buffer,
                    unsigned int           length,
                    utils::socket_address* sa = nullptr);
  int write_datagram(const void*            buffer,
                     unsigned int           length,
                     utils::socket_address* sa = nullptr);
};

} // namespace torrent

#endif
