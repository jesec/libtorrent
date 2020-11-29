// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2005-2007, Jari Sundell <jaris@ifi.uio.no>

#ifndef LIBTORRENT_UTILS_FUNCTIONAL_H
#define LIBTORRENT_UTILS_FUNCTIONAL_H

#include <cstddef>
#include <functional>

namespace torrent {
namespace utils {

template<typename Container>
inline void
slot_list_call(const Container& slot_list) {
  if (slot_list.empty())
    return;

  typename Container::const_iterator first = slot_list.begin();
  typename Container::const_iterator next  = slot_list.begin();

  while (++next != slot_list.end()) {
    (*first)();
    first = next;
  }

  (*first)();
}

template<typename Container, typename Arg1>
inline void
slot_list_call(const Container& slot_list, Arg1 arg1) {
  if (slot_list.empty())
    return;

  typename Container::const_iterator first = slot_list.begin();
  typename Container::const_iterator next  = slot_list.begin();

  while (++next != slot_list.end()) {
    (*first)(arg1);
    first = next;
  }

  (*first)(arg1);
}

template<typename Container,
         typename Arg1,
         typename Arg2,
         typename Arg3,
         typename Arg4>
inline void
slot_list_call(const Container& slot_list,
               Arg1             arg1,
               Arg2             arg2,
               Arg3             arg3,
               Arg4             arg4) {
  if (slot_list.empty())
    return;

  typename Container::const_iterator first = slot_list.begin();
  typename Container::const_iterator next  = slot_list.begin();

  while (++next != slot_list.end()) {
    (*first)(arg1, arg2, arg3, arg4);
    first = next;
  }

  (*first)(arg1, arg2, arg3, arg4);
}

} // namespace utils
} // namespace torrent

#endif
