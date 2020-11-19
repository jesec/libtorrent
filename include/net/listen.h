// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2005-2011, Jari Sundell <jaris@ifi.uio.no>

#ifndef LIBTORRENT_LISTEN_H
#define LIBTORRENT_LISTEN_H

#include <cinttypes>
#include <functional>
#include <rak/socket_address.h>

#include "socket_base.h"
#include "socket_fd.h"

namespace torrent {

class Listen : public SocketBase {
public:
  typedef std::function<void(SocketFd, const rak::socket_address&)>
    slot_connection;

  Listen()
    : m_port(0) {}
  ~Listen() {
    close();
  }

  bool open(uint16_t                   first,
            uint16_t                   last,
            int                        backlog,
            const rak::socket_address* bindAddress);
  void close();

  bool is_open() const {
    return get_fd().is_valid();
  }

  uint16_t port() const {
    return m_port;
  }

  slot_connection& slot_accepted() {
    return m_slot_accepted;
  }

  virtual void event_read();
  virtual void event_write();
  virtual void event_error();

private:
  uint64_t m_port;

  slot_connection m_slot_accepted;
};

} // namespace torrent

#endif // LIBTORRENT_TORRENT_H
