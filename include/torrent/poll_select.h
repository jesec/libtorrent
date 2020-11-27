// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2005-2011, Jari Sundell <jaris@ifi.uio.no>

#ifndef LIBTORRENT_NET_POLL_SELECT_H
#define LIBTORRENT_NET_POLL_SELECT_H

#include <sys/types.h>
#include <torrent/common.h>
#include <torrent/poll.h>

namespace torrent {

// The default Poll implementation using fd_set's.
//
// You should call torrent::perform() (or whatever the function will
// be called) immidiately before and after the call to work(...). This
// ensures we dealt with scheduled tasks and updated the cache'ed time.

class LIBTORRENT_EXPORT PollSelect : public Poll {
public:
  static PollSelect* create(int maxOpenSockets);
  virtual ~PollSelect();

  virtual uint32_t open_max() const;

  // Returns the largest fd marked.
  unsigned int fdset(fd_set* readSet, fd_set* writeSet, fd_set* exceptSet);
  unsigned int perform(fd_set* readSet, fd_set* writeSet, fd_set* exceptSet);

  unsigned int do_poll(int64_t timeout_usec, int flags = 0);

  virtual void open(Event* event);
  virtual void close(Event* event);

  virtual void closed(Event* event);

  virtual bool in_read(Event* event);
  virtual bool in_write(Event* event);
  virtual bool in_error(Event* event);

  virtual void insert_read(Event* event);
  virtual void insert_write(Event* event);
  virtual void insert_error(Event* event);

  virtual void remove_read(Event* event);
  virtual void remove_write(Event* event);
  virtual void remove_error(Event* event);

private:
  PollSelect() {}
  PollSelect(const PollSelect&);
  void operator=(const PollSelect&);

  SocketSet* m_readSet;
  SocketSet* m_writeSet;
  SocketSet* m_exceptSet;
};

} // namespace torrent

#endif
