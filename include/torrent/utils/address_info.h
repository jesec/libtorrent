// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2005-2007, Jari Sundell <jaris@ifi.uio.no>

// Wrapper for addrinfo with focus on zero-copy conversion to and from
// the c-type and wrapper.
//
// Do use the wrapper on a pre-existing struct addrinfo, cast the
// pointer rather than the base type.

#ifndef LIBTORRENT_UTILS_ADDRESS_INFO_H
#define LIBTORRENT_UTILS_ADDRESS_INFO_H

#include <netdb.h>

#include <torrent/utils/socket_address.h>

namespace torrent {
namespace utils {

class address_info {
public:
  void clear() {
    std::memset(this, 0, sizeof(address_info));
  }

  int flags() const {
    return m_addrinfo.ai_flags;
  }
  void set_flags(int f) {
    m_addrinfo.ai_flags = f;
  }

  int family() const {
    return m_addrinfo.ai_family;
  }
  void set_family(int f) {
    m_addrinfo.ai_family = f;
  }

  int socket_type() const {
    return m_addrinfo.ai_socktype;
  }
  void set_socket_type(int t) {
    m_addrinfo.ai_socktype = t;
  }

  int protocol() const {
    return m_addrinfo.ai_protocol;
  }
  void set_protocol(int p) {
    m_addrinfo.ai_protocol = p;
  }

  size_t length() const {
    return m_addrinfo.ai_addrlen;
  }

  socket_address* address() {
    return reinterpret_cast<socket_address*>(m_addrinfo.ai_addr);
  }

  addrinfo* c_addrinfo() {
    return &m_addrinfo;
  }
  const addrinfo* c_addrinfo() const {
    return &m_addrinfo;
  }

  address_info* next() {
    return reinterpret_cast<address_info*>(m_addrinfo.ai_next);
  }

  static int get_address_info(const char*    node,
                              int            domain,
                              int            type,
                              address_info** ai);

  static void free_address_info(address_info* ai) {
    ::freeaddrinfo(ai->c_addrinfo());
  }

  static const char* strerror(int err) {
    return gai_strerror(err);
  }

private:
  addrinfo m_addrinfo;
};

inline int
address_info::get_address_info(const char*    node,
                               int            pfamily,
                               int            stype,
                               address_info** ai) {
  address_info hints;
  hints.clear();
  hints.set_family(pfamily);
  hints.set_socket_type(stype);

  return ::getaddrinfo(
    node, nullptr, hints.c_addrinfo(), reinterpret_cast<addrinfo**>(ai));
}

} // namespace utils
} // namespace torrent

#endif
