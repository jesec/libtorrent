// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2005-2011, Jari Sundell <jaris@ifi.uio.no>

#ifndef LIBTORRENT_UTILS_THREAD_INTERRUPT_H
#define LIBTORRENT_UTILS_THREAD_INTERRUPT_H

#include <torrent/event.h>
#include <utility>

namespace torrent {

class SocketFd;

class LIBTORRENT_EXPORT lt_cacheline_aligned thread_interrupt : public Event {
public:
  typedef std::pair<thread_interrupt*, thread_interrupt*> pair_type;

  ~thread_interrupt();

  static pair_type create_pair();

  bool is_poking() const;

  bool poke();

  void event_read();
  void event_write() {}
  void event_error() {}

private:
  thread_interrupt(int fd);

  SocketFd& get_fd() {
    return *reinterpret_cast<SocketFd*>(&m_fileDesc);
  }

  thread_interrupt* m_other;
  bool m_poking     lt_cacheline_aligned;
};

inline bool
thread_interrupt::is_poking() const {
  return m_poking;
}

} // namespace torrent

#endif
