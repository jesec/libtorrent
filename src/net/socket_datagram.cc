// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2005-2011, Jari Sundell <jaris@ifi.uio.no>

#include <cerrno>
#include <sys/socket.h>
#include <sys/types.h>

#include "net/socket_datagram.h"
#include "rak/socket_address.h"
#include "torrent/exceptions.h"

namespace torrent {

int
SocketDatagram::read_datagram(void*                buffer,
                              unsigned int         length,
                              rak::socket_address* sa) {
  if (length == 0)
    throw internal_error("Tried to receive buffer length 0");

  int       r;
  socklen_t fromlen;

  if (sa != NULL) {
    fromlen = sizeof(rak::socket_address);
    r = ::recvfrom(m_fileDesc, buffer, length, 0, sa->c_sockaddr(), &fromlen);
  } else {
    r = ::recv(m_fileDesc, buffer, length, 0);
  }

  return r;
}

int
SocketDatagram::write_datagram(const void*          buffer,
                               unsigned int         length,
                               rak::socket_address* sa) {
  if (length == 0)
    throw internal_error("Tried to send buffer length 0");

  int r;

  if (sa != NULL) {
    if (m_ipv6_socket && sa->family() == rak::socket_address::pf_inet) {
      rak::socket_address_inet6 sa_mapped = sa->sa_inet()->to_mapped_address();
      r                                   = ::sendto(m_fileDesc,
                   buffer,
                   length,
                   0,
                   sa_mapped.c_sockaddr(),
                   sizeof(rak::socket_address_inet6));
    } else {
      r =
        ::sendto(m_fileDesc, buffer, length, 0, sa->c_sockaddr(), sa->length());
    }
  } else {
    r = ::send(m_fileDesc, buffer, length, 0);
  }

  return r;
}

} // namespace torrent
