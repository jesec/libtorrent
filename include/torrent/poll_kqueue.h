// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2005-2011, Jari Sundell <jaris@ifi.uio.no>

#ifndef LIBTORRENT_TORRENT_POLL_KQUEUE_H
#define LIBTORRENT_TORRENT_POLL_KQUEUE_H

#include <torrent/poll.h>
#include <vector>

struct kevent;

namespace torrent {

class LIBTORRENT_EXPORT PollKQueue : public torrent::Poll {
public:
  using Table = std::vector<std::pair<uint32_t, Event*>>;

  static const uint32_t flag_read  = (1 << 0);
  static const uint32_t flag_write = (1 << 1);
  static const uint32_t flag_error = (1 << 2);

  static PollKQueue* create(int maxOpenSockets);
  ~PollKQueue() override = default;

  int          poll(int msec);
  unsigned int perform();

  unsigned int do_poll(int64_t timeout_usec, int flags = 0) override;

  int file_descriptor() {
    return m_fd;
  }

  uint32_t open_max() const override;

  // torrent::Event::get_fd() is guaranteed to be valid and remain constant
  // from open(...) is called to close(...) returns.
  void open(torrent::Event* event) override;
  void close(torrent::Event* event) override;

  // torrent::Event::get_fd() was closed outside of our control.
  void closed(torrent::Event* event) override;

  // Functions for checking whetever the torrent::Event is listening to r/w/e?
  bool in_read(torrent::Event* event) override;
  bool in_write(torrent::Event* event) override;
  bool in_error(torrent::Event* event) override;

  // These functions may be called on 'event's that might, or might
  // not, already be in the set.
  void insert_read(torrent::Event* event) override;
  void insert_write(torrent::Event* event) override;
  void insert_error(torrent::Event* event) override;

  void remove_read(torrent::Event* event) override;
  void remove_write(torrent::Event* event) override;
  void remove_error(torrent::Event* event) override;

private:
  PollKQueue(int fd, int maxEvents, int maxOpenSockets) LIBTORRENT_NO_EXPORT;

  inline uint32_t event_mask(Event* e) LIBTORRENT_NO_EXPORT;
  inline void     set_event_mask(Event* e, uint32_t m) LIBTORRENT_NO_EXPORT;

  int poll_select(int msec) LIBTORRENT_NO_EXPORT;

  void flush_events() LIBTORRENT_NO_EXPORT;
  void modify(torrent::Event* event,
              unsigned short  op,
              short           mask) LIBTORRENT_NO_EXPORT;

  int m_fd;

  unsigned int m_maxEvents;
  unsigned int m_waitingEvents;
  unsigned int m_changedEvents;

  Table          m_table;
  struct kevent* m_events;
  struct kevent* m_changes;

  // Work-around the stdin bug in MacOSX's kqueue implementation.
  Event* m_stdinEvent;
};

} // namespace torrent

#endif
