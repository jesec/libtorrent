// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2005-2011, Jari Sundell <jaris@ifi.uio.no>

#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>

#include "manager.h"
#include "net/socket_base.h"
#include "torrent/exceptions.h"
#include "torrent/poll.h"

namespace torrent {

char* SocketBase::m_nullBuffer = new char[SocketBase::null_buffer_size];

SocketBase::~SocketBase() {
  if (get_fd().is_valid())
    internal_error("SocketBase::~SocketBase() called but m_fd is still valid");
}

bool
SocketBase::read_oob(void* buffer) {
  int r = ::recv(m_fileDesc, buffer, 1, MSG_OOB);

  //   if (r < 0)
  //     m_errno = errno;

  return r == 1;
}

bool
SocketBase::write_oob(const void* buffer) {
  int r = ::send(m_fileDesc, buffer, 1, MSG_OOB);

  //   if (r < 0)
  //     m_errno = errno;

  return r == 1;
}

void
SocketBase::receive_throttle_down_activate() {
  manager->poll()->insert_read(this);
}

void
SocketBase::receive_throttle_up_activate() {
  manager->poll()->insert_write(this);
}

} // namespace torrent
