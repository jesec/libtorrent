// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2005-2011, Jari Sundell <jaris@ifi.uio.no>

#ifndef LIBTORRENT_NET_THROTTLE_NODE_H
#define LIBTORRENT_NET_THROTTLE_NODE_H

#include <functional>

#include "torrent/rate.h"

#include "throttle_list.h"

namespace torrent {

class SocketBase;

class ThrottleNode {
public:
  typedef ThrottleList::iterator       iterator;
  typedef ThrottleList::const_iterator const_iterator;

  typedef std::function<void()> slot_void;

  ThrottleNode(uint32_t rateSpan)
    : m_rate(rateSpan) {
    clear_quota();
  }

  Rate* rate() {
    return &m_rate;
  }
  const Rate* rate() const {
    return &m_rate;
  }

  uint32_t quota() const {
    return m_quota;
  }
  void clear_quota() {
    m_quota = 0;
  }
  void set_quota(uint32_t q) {
    m_quota = q;
  }

  iterator list_iterator() {
    return m_listIterator;
  }
  const_iterator list_iterator() const {
    return m_listIterator;
  }
  void set_list_iterator(iterator itr) {
    m_listIterator = itr;
  }

  void activate() {
    if (m_slot_activate)
      m_slot_activate();
  }

  slot_void& slot_activate() {
    return m_slot_activate;
  }

private:
  ThrottleNode(const ThrottleNode&);
  void operator=(const ThrottleNode&);

  uint32_t m_quota;
  iterator m_listIterator;

  Rate      m_rate;
  slot_void m_slot_activate;
};

} // namespace torrent

#endif
