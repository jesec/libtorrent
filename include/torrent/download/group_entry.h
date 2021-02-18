// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2005-2011, Jari Sundell <jaris@ifi.uio.no>

#ifndef LIBTORRENT_DOWNLOAD_GROUP_ENTRY_H
#define LIBTORRENT_DOWNLOAD_GROUP_ENTRY_H

#include <algorithm>
#include <functional>
#include <vector>

#include <torrent/common.h>
#include <torrent/exceptions.h>

namespace torrent {

class choke_queue;
class PeerConnectionBase;

struct weighted_connection {
  weighted_connection(PeerConnectionBase* pcb, uint32_t w)
    : connection(pcb)
    , weight(w) {}

  bool operator==(const PeerConnectionBase* pcb) {
    return pcb == connection;
  }
  bool operator!=(const PeerConnectionBase* pcb) {
    return pcb != connection;
  }

  PeerConnectionBase* connection;
  uint32_t            weight;
};

// TODO: Rename to choke_entry, create an new class called group entry?
class group_entry {
public:
  using container_type = std::vector<weighted_connection>;

  static constexpr uint32_t unlimited = ~uint32_t();

  uint32_t size_connections() const {
    return m_queued.size() + m_unchoked.size();
  }

  uint32_t max_slots() const {
    return m_max_slots;
  }
  uint32_t min_slots() const {
    return m_min_slots;
  }

  void set_max_slots(uint32_t s) {
    m_max_slots = s;
  }
  void set_min_slots(uint32_t s) {
    m_min_slots = s;
  }

  const container_type* queued() {
    return &m_queued;
  }
  const container_type* unchoked() {
    return &m_unchoked;
  }

protected:
  friend class choke_queue;
  friend class PeerConnectionBase;

  container_type* mutable_queued() {
    return &m_queued;
  }
  container_type* mutable_unchoked() {
    return &m_unchoked;
  }

  void connection_unchoked(PeerConnectionBase* pcb);
  void connection_choked(PeerConnectionBase* pcb);

  void connection_queued(PeerConnectionBase* pcb);
  void connection_unqueued(PeerConnectionBase* pcb);

private:
  uint32_t m_max_slots{ unlimited };
  uint32_t m_min_slots{ 0 };

  // After a cycle the end of the vector should have the
  // highest-priority connections, and any new connections get put at
  // the back so they should always good candidates for unchoking.
  container_type m_queued;
  container_type m_unchoked;
};

inline void
group_entry::connection_unchoked(PeerConnectionBase* pcb) {
  if (std::any_of(m_unchoked.begin(),
                  m_unchoked.end(),
                  [pcb](weighted_connection c) { return c == pcb; })) {
    throw internal_error("group_entry::connection_unchoked(pcb) failed.");
  }

  m_unchoked.push_back(weighted_connection(pcb, uint32_t()));
}

inline void
group_entry::connection_queued(PeerConnectionBase* pcb) {
  if (std::any_of(m_queued.begin(),
                  m_queued.end(),
                  [pcb](weighted_connection c) { return c == pcb; })) {
    throw internal_error("group_entry::connection_queued(pcb) failed.");
  }

  m_queued.push_back(weighted_connection(pcb, uint32_t()));
}

inline void
group_entry::connection_choked(PeerConnectionBase* pcb) {
  container_type::iterator itr =
    std::find_if(m_unchoked.begin(),
                 m_unchoked.end(),
                 [pcb](weighted_connection c) { return c == pcb; });

  // none_of
  if (itr == m_unchoked.end()) {
    throw internal_error("group_entry::connection_choked(pcb) failed.");
  }

  std::swap(*itr, m_unchoked.back());
  m_unchoked.pop_back();
}

inline void
group_entry::connection_unqueued(PeerConnectionBase* pcb) {
  container_type::iterator itr =
    std::find_if(m_queued.begin(),
                 m_queued.end(),
                 [pcb](weighted_connection c) { return c == pcb; });

  // none_of
  if (itr == m_queued.end()) {
    throw internal_error("group_entry::connection_unqueued(pcb) failed.");
  }

  std::swap(*itr, m_queued.back());
  m_queued.pop_back();
}

} // namespace torrent

#endif
