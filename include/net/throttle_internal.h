// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2005-2011, Jari Sundell <jaris@ifi.uio.no>

#ifndef LIBTORRENT_NET_THROTTLE_INTERNAL_H
#define LIBTORRENT_NET_THROTTLE_INTERNAL_H

#include <vector>

#include "torrent/common.h"
#include "torrent/throttle.h"
#include "torrent/utils/priority_queue_default.h"

namespace torrent {

class ThrottleInternal : public Throttle {
public:
  static constexpr int flag_none = 0;
  static constexpr int flag_root = 1;

  ThrottleInternal(int flags);
  ~ThrottleInternal();

  ThrottleInternal* create_slave();

  bool is_root() {
    return m_flags & flag_root;
  }

  void enable();
  void disable();

private:
  // Fraction is a fixed-precision value with the given number of bits after the
  // decimal point.
  static constexpr uint32_t fraction_bits = 16;
  static constexpr uint32_t fraction_base = (1 << fraction_bits);

  using SlaveList = std::vector<ThrottleInternal*>;

  void receive_tick();

  // Distribute quota, return amount of quota used. May be negative
  // if it had more unused quota than is now allowed.
  int32_t receive_quota(uint32_t quota, uint32_t fraction);

  int                 m_flags;
  SlaveList           m_slaveList;
  SlaveList::iterator m_nextSlave;

  uint32_t m_unusedQuota;

  utils::timer         m_timeLastTick;
  utils::priority_item m_taskTick;
};

} // namespace torrent

#endif
