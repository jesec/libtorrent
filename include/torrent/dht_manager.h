// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2005-2011, Jari Sundell <jaris@ifi.uio.no>

// Add some helpfull words here.

#ifndef LIBTORRENT_DHT_MANAGER_H
#define LIBTORRENT_DHT_MANAGER_H

#include <string>

#include <torrent/common.h>
#include <torrent/connection_manager.h>

namespace torrent {

class ThrottleList;

class LIBTORRENT_EXPORT DhtManager {
public:
  using port_type = ConnectionManager::port_type;

  struct statistics_type {
    // Cycle; 0=inactive, 1=initial bootstrapping, 2 and up=normal operation
    unsigned int cycle;

    // UDP transfer rates.
    const Rate& up_rate;
    const Rate& down_rate;

    // DHT query statistics.
    unsigned int queries_received;
    unsigned int queries_sent;
    unsigned int replies_received;
    unsigned int errors_received;
    unsigned int errors_caught;

    // DHT node info.
    unsigned int num_nodes;
    unsigned int num_buckets;

    // DHT tracker info.
    unsigned int num_peers;
    unsigned int max_peers;
    unsigned int num_trackers;

    statistics_type(const Rate& up, const Rate& down)
      : up_rate(up)
      , down_rate(down) {}
  };

  ~DhtManager();

  void initialize(const Object& dhtCache);

  bool start(port_type port);
  void stop();

  // Store DHT cache in the given container and return the container.
  Object* store_cache(Object* container) const;

  bool is_valid() const {
    return m_router;
  }
  bool is_active() const;

  port_type port() const {
    return m_port;
  }

  bool can_receive_queries() const {
    return m_canReceive;
  }

  // Call this after sending the port to a client, so the router knows
  // that it should be getting requests.
  void port_sent() {
    m_portSent++;
  }

  // Add a node by host (from a torrent file), or by address from explicit
  // add_node command or the BT PORT message.
  void add_node(const std::string& host, int port);
  void add_node(const sockaddr* addr, int port);

  statistics_type get_statistics() const;
  void            reset_statistics();

  void set_upload_throttle(Throttle* t);
  void set_download_throttle(Throttle* t);

  // To be called if upon examining the statistics, the client decides that
  // we can't receive outside requests and therefore shouldn't advertise our
  // UDP port after the BT handshake.
  void set_can_receive(bool can) {
    m_canReceive = can;
  }

  // Internal libTorrent use only
  DhtRouter* router() {
    return m_router;
  }

private:
  DhtRouter* m_router{ nullptr };
  port_type  m_port;

  int  m_portSent{ 0 };
  bool m_canReceive{ true };
};

} // namespace torrent

#endif
