// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2005-2011, Jari Sundell <jaris@ifi.uio.no>

#ifndef LIBTORRENT_DOWNLOAD_CHOKE_STATUS_H
#define LIBTORRENT_DOWNLOAD_CHOKE_STATUS_H

#include <torrent/common.h>

namespace torrent {

class group_entry;

class choke_status {
public:
  choke_status()
    : m_group_entry(NULL)
    ,

    m_queued(false)
    , m_unchoked(false)
    , m_snubbed(false)
    , m_timeLastChoke(0) {}

  group_entry* entry() const {
    return m_group_entry;
  }
  void set_entry(group_entry* grp_ent) {
    m_group_entry = grp_ent;
  }

  bool queued() const {
    return m_queued;
  }
  void set_queued(bool s) {
    m_queued = s;
  }

  bool choked() const {
    return !m_unchoked;
  }
  bool unchoked() const {
    return m_unchoked;
  }
  void set_unchoked(bool s) {
    m_unchoked = s;
  }

  bool snubbed() const {
    return m_snubbed;
  }
  void set_snubbed(bool s) {
    m_snubbed = s;
  }

  int64_t time_last_choke() const {
    return m_timeLastChoke;
  }
  void set_time_last_choke(int64_t t) {
    m_timeLastChoke = t;
  }

private:
  // TODO: Use flags.
  group_entry* m_group_entry;

  bool m_queued;
  bool m_unchoked;
  bool m_snubbed;

  int64_t m_timeLastChoke;
};

} // namespace torrent

#endif
