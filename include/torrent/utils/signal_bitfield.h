// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2005-2011, Jari Sundell <jaris@ifi.uio.no>

#ifndef LIBTORRENT_UTILS_SIGNAL_BITFIELD_H
#define LIBTORRENT_UTILS_SIGNAL_BITFIELD_H

#include <functional>

#include <torrent/common.h>
#include <torrent/utils/cacheline.h>

namespace torrent {

class LIBTORRENT_EXPORT lt_cacheline_aligned signal_bitfield {
public:
  using bitfield_type = uint32_t;
  using slot_type     = std::function<void()>;

  static constexpr unsigned int max_size = 32;

  bool has_signal(unsigned int index) const {
    return m_bitfield & (1 << index);
  }

  // Do the interrupt from the thread?
  void signal(unsigned int index) {
    __sync_or_and_fetch(&m_bitfield, 1 << index);
  }
  void work();

  unsigned int add_signal(const slot_type& slot);

private:
  bitfield_type m_bitfield{ 0 };
  unsigned int  m_size{ 0 };
  slot_type     m_slots[max_size] lt_cacheline_aligned;
};

} // namespace torrent

#endif
