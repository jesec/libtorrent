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
  ~PollSelect() override;

  uint32_t open_max() const override;

  unsigned int do_poll(int64_t timeout_usec, int flags = 0) override;

  void open(Event* event) override;
  void close(Event* event) override;

  void closed(Event* event) override;

  bool in_read(Event* event) override;
  bool in_write(Event* event) override;
  bool in_error(Event* event) override;

  void insert_read(Event* event) override;
  void insert_write(Event* event) override;
  void insert_error(Event* event) override;

  void remove_read(Event* event) override;
  void remove_write(Event* event) override;
  void remove_error(Event* event) override;

private:
  // Returns the largest fd marked.
  unsigned int fdset(fd_set* readSet, fd_set* writeSet, fd_set* exceptSet);
  unsigned int perform(fd_set* readSet, fd_set* writeSet, fd_set* exceptSet);

  PollSelect()                  = default;
  PollSelect(const PollSelect&) = delete;
  void operator=(const PollSelect&) = delete;

  SocketSet* m_readSet;
  SocketSet* m_writeSet;
  SocketSet* m_exceptSet;
};

} // namespace torrent

#endif
