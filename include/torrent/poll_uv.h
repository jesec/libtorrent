// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2023, Jesse Chan <jc@linux.com>

#ifndef LIBTORRENT_TORRENT_POLL_UV_H
#define LIBTORRENT_TORRENT_POLL_UV_H

#include <sys/types.h>
#include <torrent/common.h>
#include <torrent/poll.h>

#include <uv.h>

namespace torrent {

class LIBTORRENT_EXPORT PollUV : public Poll {
public:
  static PollUV* create(int maxOpenSockets);
  ~PollUV() override;

  PollUV(const PollUV&)         = delete;
  void operator=(const PollUV&) = delete;

  void event_callback(Event* e, int status, int events);

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
  PollUV(uv_loop_t* loop, uv_timer_t* timer, int maxOpenSockets);

  inline int  event_mask(Event* e);
  inline void set_event_mask(Event* e, int m);

  void poll_start(Event* e);

  // fd - [event, mask, handle]
  std::vector<std::tuple<Event*, int, uv_poll_t*>> m_table;
  uv_loop_t*                                       m_loop;
  uv_timer_t*                                      m_timer;
  size_t                                           m_event_count;
};

} // namespace torrent

#endif
