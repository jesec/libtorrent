// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2005-2011, Jari Sundell <jaris@ifi.uio.no>

#include <cstdio>
#include <sstream>

#include "dht/dht_router.h"
#include "torrent/connection_manager.h"
#include "torrent/dht_manager.h"
#include "torrent/download_info.h"
#include "torrent/exceptions.h"
#include "torrent/tracker_list.h"
#include "torrent/utils/log.h"
#include "tracker/tracker_dht.h"

#include "globals.h"
#include "manager.h"

namespace torrent {

const char* TrackerDht::states[] = { "Idle", "Searching", "Announcing" };

bool
TrackerDht::is_allowed() {
  return manager->dht_manager()->is_valid();
}

TrackerDht::TrackerDht(TrackerList* parent, const std::string& url, int flags)
  : Tracker(parent, url, flags)
  , m_state(state_idle) {

  if (!manager->dht_manager()->is_valid())
    throw internal_error("Trying to add DHT tracker with no DHT manager.");
}

TrackerDht::~TrackerDht() {
  close();
}

bool
TrackerDht::is_busy() const {
  return m_state != state_idle;
}

bool
TrackerDht::is_complete() const {
  return !m_parent->info()->is_meta_download() &&
         m_parent->info()->slot_left()() == 0;
}

bool
TrackerDht::is_usable() const {
  return is_enabled() && manager->dht_manager()->is_active();
}

void
TrackerDht::send_state(int state) {
  if (m_parent == nullptr)
    throw internal_error(
      "TrackerDht::send_state(...) does not have a valid m_parent.");

  if (is_busy()) {
    manager->dht_manager()->router()->cancel_announce(m_parent->info(), this);
  }

  m_latest_event = state;

  if (state == DownloadInfo::STOPPED)
    return;

  m_state = state_searching;

  if (!manager->dht_manager()->is_active())
    return receive_failed("DHT server not active.");

  manager->dht_manager()->router()->announce(m_parent->info(), this);

  set_normal_interval(20 * 60);
  set_min_interval(0);
}

void
TrackerDht::close() {
  if (is_busy()) {
    manager->dht_manager()->router()->cancel_announce(m_parent->info(), this);
    m_state = state_idle;
  }
}

void
TrackerDht::disown() {
  close();
}

TrackerDht::Type
TrackerDht::type() const {
  return TRACKER_DHT;
}

void
TrackerDht::receive_peers(raw_list peers) {
  m_peers.parse_address_bencode(peers);
}

void
TrackerDht::receive_success() {
  m_state = state_idle;
  m_parent->receive_success(this, &m_peers);
  m_peers.clear();
}

void
TrackerDht::receive_failed(const char* msg) {
  m_state = state_idle;
  m_parent->receive_failed(this, msg);
  m_peers.clear();
}

void
TrackerDht::receive_progress(int replied, int contacted) {
  m_replied   = replied;
  m_contacted = contacted;
}

void
TrackerDht::get_status(char* buffer, int length) {
  snprintf(buffer,
           length,
           "[%s: %d/%d nodes replied]",
           states[m_state],
           m_replied,
           m_contacted);
}

} // namespace torrent
