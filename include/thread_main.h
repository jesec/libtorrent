// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2005-2011, Jari Sundell <jaris@ifi.uio.no>

#ifndef LIBTORRENT_THREAD_MAIN_H
#define LIBTORRENT_THREAD_MAIN_H

#include "data/hash_check_queue.h"
#include "torrent/utils/thread_base.h"

namespace torrent {

class thread_main : public thread_base {
public:
  const char* name() const override {
    return "rtorrent main";
  }

  void init_thread() override;

protected:
  void    call_events() override;
  int64_t next_timeout_usec() override;
};

} // namespace torrent

#endif
