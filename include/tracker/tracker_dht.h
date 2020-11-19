// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2005-2011, Jari Sundell <jaris@ifi.uio.no>

#ifndef LIBTORRENT_TRACKER_TRACKER_DHT_H
#define LIBTORRENT_TRACKER_TRACKER_DHT_H

#include "net/address_list.h"
#include "torrent/object.h"
#include "torrent/tracker.h"

namespace torrent {

class TrackerDht : public Tracker {
public:
  TrackerDht(TrackerList* parent, const std::string& url, int flags);
  ~TrackerDht();

  typedef enum {
    state_idle,
    state_searching,
    state_announcing,
  } state_type;

  static const char* states[];

  static bool is_allowed();

  virtual bool is_busy() const;
  virtual bool is_usable() const;

  virtual void send_state(int state);
  virtual void close();
  virtual void disown();

  virtual Type type() const;

  virtual void get_status(char* buffer, int length);

  void set_state(state_type state) {
    m_state = state;
  }
  state_type get_state() const {
    return m_state;
  }

  bool has_peers() const {
    return !m_peers.empty();
  }

  void receive_peers(raw_list peers);
  void receive_success();
  void receive_failed(const char* msg);
  void receive_progress(int replied, int contacted);

private:
  AddressList m_peers;
  state_type  m_state;

  int m_replied;
  int m_contacted;
};

} // namespace torrent

#endif
