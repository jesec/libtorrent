// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2005-2011, Jari Sundell <jaris@ifi.uio.no>

#include <algorithm>
#include <functional>

#include "download/download_main.h"
#include "torrent/download/choke_group.h"
#include "torrent/download/choke_queue.h"
#include "torrent/download/resource_manager.h" // TODO: Put resource_manager_entry in a separate file.
#include "torrent/exceptions.h"

namespace torrent {

choke_group::choke_group()
  : m_tracker_mode(TRACKER_MODE_NORMAL)
  , m_down_queue(choke_queue::flag_unchoke_all_new)
  , m_first(NULL)
  , m_last(NULL) {}

uint64_t
choke_group::up_rate() const {
  uint64_t result = 0;

  std::for_each(m_first, m_last, [&result](resource_manager_entry e) {
    result += e.up_rate()->rate();
  });

  return result;
}

uint64_t
choke_group::down_rate() const {
  uint64_t result = 0;

  std::for_each(m_first, m_last, [&result](resource_manager_entry e) {
    result += e.down_rate()->rate();
  });

  return result;
}

} // namespace torrent
