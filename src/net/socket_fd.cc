// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2005-2011, Jari Sundell <jaris@ifi.uio.no>

#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>

#include "net/socket_fd.h"
#include "torrent/exceptions.h"
#include "torrent/utils/socket_address.h"

namespace torrent {

inline void
SocketFd::check_valid() const {
  if (!is_valid())
    throw internal_error("SocketFd function called on an invalid fd.");
}

bool
SocketFd::set_nonblock() {
  check_valid();

  return fcntl(m_fd, F_SETFL, O_NONBLOCK) == 0;
}

bool
SocketFd::set_priority(priority_type p) {
  check_valid();
  int opt = p;

  if (m_ipv6_socket)
    return setsockopt(m_fd, IPPROTO_IPV6, IPV6_TCLASS, &opt, sizeof(opt)) == 0;
  else
    return setsockopt(m_fd, IPPROTO_IP, IP_TOS, &opt, sizeof(opt)) == 0;
}

bool
SocketFd::set_reuse_address(bool state) {
  check_valid();
  int opt = state;

  return setsockopt(m_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == 0;
}

bool
SocketFd::set_ipv6_v6only(bool state) {
  check_valid();

  if (!m_ipv6_socket)
    return false;

  int opt = state;
  return setsockopt(m_fd, IPPROTO_IPV6, IPV6_V6ONLY, &opt, sizeof(opt)) == 0;
}

bool
SocketFd::set_send_buffer_size(uint32_t s) {
  check_valid();
  int opt = s;

  return setsockopt(m_fd, SOL_SOCKET, SO_SNDBUF, &opt, sizeof(opt)) == 0;
}

bool
SocketFd::set_receive_buffer_size(uint32_t s) {
  check_valid();
  int opt = s;

  return setsockopt(m_fd, SOL_SOCKET, SO_RCVBUF, &opt, sizeof(opt)) == 0;
}

int
SocketFd::get_error() const {
  check_valid();

  int       err;
  socklen_t length = sizeof(err);

  if (getsockopt(m_fd, SOL_SOCKET, SO_ERROR, &err, &length) == -1)
    throw internal_error("SocketFd::get_error() could not get error");

  return err;
}

bool
SocketFd::open_stream() {
  m_fd = socket(utils::socket_address::pf_inet6, SOCK_STREAM, IPPROTO_TCP);

  if (m_fd == -1) {
    m_ipv6_socket = false;
    return (m_fd = socket(
              utils::socket_address::pf_inet, SOCK_STREAM, IPPROTO_TCP)) != -1;
  }

  m_ipv6_socket = true;

  if (!set_ipv6_v6only(false)) {
    close();
    return false;
  }

  return true;
}

bool
SocketFd::open_datagram() {
  m_fd = socket(utils::socket_address::pf_inet6, SOCK_DGRAM, 0);

  if (m_fd == -1) {
    m_ipv6_socket = false;
    return (m_fd = socket(utils::socket_address::pf_inet, SOCK_DGRAM, 0)) != -1;
  }

  m_ipv6_socket = true;

  if (!set_ipv6_v6only(false)) {
    close();
    return false;
  }

  return true;
}

bool
SocketFd::open_local() {
  return (m_fd = socket(utils::socket_address::pf_local, SOCK_STREAM, 0)) != -1;
}

bool
SocketFd::open_socket_pair(int& fd1, int& fd2) {
  int result[2];

  if (socketpair(utils::socket_address::pf_local, SOCK_STREAM, 0, result) == -1)
    return false;

  fd1 = result[0];
  fd2 = result[1];

  return true;
}

void
SocketFd::close() {
  if (::close(m_fd) && errno == EBADF)
    throw internal_error(
      "SocketFd::close() called on an invalid file descriptor");
}

bool
SocketFd::bind(const utils::socket_address& sa) {
  check_valid();

  if (m_ipv6_socket && sa.family() == utils::socket_address::pf_inet) {
    utils::socket_address_inet6 sa_mapped = sa.sa_inet()->to_mapped_address();
    return !::bind(m_fd, sa_mapped.c_sockaddr(), sizeof(sa_mapped));
  }

  return !::bind(m_fd, sa.c_sockaddr(), sa.length());
}

bool
SocketFd::bind(const utils::socket_address& sa, unsigned int length) {
  check_valid();

  if (m_ipv6_socket && sa.family() == utils::socket_address::pf_inet) {
    utils::socket_address_inet6 sa_mapped = sa.sa_inet()->to_mapped_address();
    return !::bind(m_fd, sa_mapped.c_sockaddr(), sizeof(sa_mapped));
  }

  return !::bind(m_fd, sa.c_sockaddr(), length);
}

bool
SocketFd::bind_sa(const sockaddr* sa) {
  return bind(*utils::socket_address::cast_from(sa));
}

bool
SocketFd::connect(const utils::socket_address& sa) {
  check_valid();

  if (m_ipv6_socket && sa.family() == utils::socket_address::pf_inet) {
    utils::socket_address_inet6 sa_mapped = sa.sa_inet()->to_mapped_address();
    return !::connect(m_fd, sa_mapped.c_sockaddr(), sizeof(sa_mapped)) ||
           errno == EINPROGRESS;
  }

  return !::connect(m_fd, sa.c_sockaddr(), sa.length()) || errno == EINPROGRESS;
}

bool
SocketFd::connect_sa(const sockaddr* sa) {
  return connect(*utils::socket_address::cast_from(sa));
}

bool
SocketFd::getsockname(utils::socket_address* sa) {
  check_valid();

  socklen_t len = sizeof(utils::socket_address);
  if (::getsockname(m_fd, sa->c_sockaddr(), &len)) {
    return false;
  }

  if (m_ipv6_socket && sa->family() == utils::socket_address::af_inet6) {
    *sa = sa->sa_inet6()->normalize_address();
  }

  return true;
}

bool
SocketFd::listen(int size) {
  check_valid();

  return !::listen(m_fd, size);
}

SocketFd
SocketFd::accept(utils::socket_address* sa) {
  check_valid();
  socklen_t len = sizeof(utils::socket_address);

  if (sa == nullptr) {
    return SocketFd(::accept(m_fd, nullptr, &len), m_ipv6_socket);
  }

  int fd = ::accept(m_fd, sa->c_sockaddr(), &len);

  if (fd != -1 && m_ipv6_socket &&
      sa->family() == utils::socket_address::af_inet6) {
    *sa = sa->sa_inet6()->normalize_address();
  }

  return SocketFd(fd, m_ipv6_socket);
}

// unsigned int
// SocketFd::get_read_queue_size() const {
//   unsigned int v;

//   if (!is_valid() || ioctl(m_fd, SIOCINQ, &v) < 0)
//     throw internal_error("SocketFd::get_read_queue_size() could not be
//     performed");

//   return v;
// }

// unsigned int
// SocketFd::get_write_queue_size() const {
//   unsigned int v;

//   if (!is_valid() || ioctl(m_fd, SIOCOUTQ, &v) < 0)
//     throw internal_error("SocketFd::get_write_queue_size() could not be
//     performed");

//   return v;
// }

} // namespace torrent
