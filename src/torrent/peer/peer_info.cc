// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2005-2011, Jari Sundell <jaris@ifi.uio.no>

#include "config.h"

#include <cstring>

#include "protocol/extensions.h"
#include "protocol/peer_connection_base.h"
#include "rak/socket_address.h"
#include "torrent/exceptions.h"
#include "torrent/peer/peer_info.h"
#include "utils/instrumentation.h"

namespace torrent {

// Move this to peer_info.cc when these are made into the public API.
//
// TODO: Use a safer socket address parameter.
PeerInfo::PeerInfo(const sockaddr* address)
  : m_flags(0)
  ,

  m_failedCounter(0)
  , m_transferCounter(0)
  , m_lastConnection(0)
  , m_lastHandshake(0)
  , m_listenPort(0)
  ,

  m_connection(NULL) {
  rak::socket_address* sa = new rak::socket_address();
  *sa                     = *rak::socket_address::cast_from(address);

  m_address = sa->c_sockaddr();
}

PeerInfo::~PeerInfo() noexcept(false) {
  // if (m_transferCounter != 0)
  //   throw internal_error("PeerInfo::~PeerInfo() m_transferCounter != 0.");

  instrumentation_update(INSTRUMENTATION_TRANSFER_PEER_INFO_UNACCOUNTED,
                         m_transferCounter);

  if (is_blocked())
    throw internal_error("PeerInfo::~PeerInfo() peer is blocked.");

  delete rak::socket_address::cast_from(m_address);
}

void
PeerInfo::set_port(uint16_t port) {
  rak::socket_address::cast_from(m_address)->set_port(port);
}

} // namespace torrent
