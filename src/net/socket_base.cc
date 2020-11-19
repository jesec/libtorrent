// libTorrent - BitTorrent library
// Copyright (C) 2005-2011, Jari Sundell
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
// In addition, as a special exception, the copyright holders give
// permission to link the code of portions of this program with the
// OpenSSL library under certain conditions as described in each
// individual source file, and distribute linked combinations
// including the two.
//
// You must obey the GNU General Public License in all respects for
// all of the code used other than OpenSSL.  If you modify file(s)
// with this exception, you may extend this exception to your version
// of the file(s), but you are not obligated to do so.  If you do not
// wish to do so, delete this exception statement from your version.
// If you delete this exception statement from all source files in the
// program, then also delete it here.
//
// Contact:  Jari Sundell <jaris@ifi.uio.no>
//
//           Skomakerveien 33
//           3185 Skoppum, NORWAY

#include "config.h"

#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>

#include "manager.h"
#include "socket_base.h"
#include "torrent/exceptions.h"
#include "torrent/poll.h"

namespace torrent {

char* SocketBase::m_nullBuffer = new char[SocketBase::null_buffer_size];

SocketBase::~SocketBase() {
  if (get_fd().is_valid())
    throw internal_error(
      "SocketBase::~SocketBase() called but m_fd is still valid");
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
