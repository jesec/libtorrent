// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2005-2011, Jari Sundell <jaris@ifi.uio.no>

#ifndef LIBTORRENT_TORRENT_POLL_EPOLL_H
#define LIBTORRENT_TORRENT_POLL_EPOLL_H

#include <torrent/poll.h>
#include <vector>

struct epoll_event;

namespace torrent {

class LIBTORRENT_EXPORT PollEPoll : public torrent::Poll {
public:
  typedef std::vector<std::pair<uint32_t, Event*>> Table;

  static PollEPoll* create(int maxOpenSockets);
  virtual ~PollEPoll();

  int          poll(int msec);
  unsigned int perform();

  unsigned int do_poll(int64_t timeout_usec, int flags = 0);

  int file_descriptor() {
    return m_fd;
  }

  virtual uint32_t open_max() const;

  // torrent::Event::get_fd() is guaranteed to be valid and remain constant
  // from open(...) is called to close(...) returns.
  virtual void open(torrent::Event* event);
  virtual void close(torrent::Event* event);

  // torrent::Event::get_fd() was closed outside of our control.
  virtual void closed(torrent::Event* event);

  // Functions for checking whetever the torrent::Event is listening to r/w/e?
  virtual bool in_read(torrent::Event* event);
  virtual bool in_write(torrent::Event* event);
  virtual bool in_error(torrent::Event* event);

  // These functions may be called on 'event's that might, or might
  // not, already be in the set.
  virtual void insert_read(torrent::Event* event);
  virtual void insert_write(torrent::Event* event);
  virtual void insert_error(torrent::Event* event);

  virtual void remove_read(torrent::Event* event);
  virtual void remove_write(torrent::Event* event);
  virtual void remove_error(torrent::Event* event);

private:
  PollEPoll(int fd, int maxEvents, int maxOpenSockets);

  inline uint32_t event_mask(Event* e);
  inline void     set_event_mask(Event* e, uint32_t m);

  inline void modify(torrent::Event* event, int op, uint32_t mask);

  int m_fd;

  int m_maxEvents;
  int m_waitingEvents;

  Table        m_table;
  epoll_event* m_events;
};

} // namespace torrent

#endif
