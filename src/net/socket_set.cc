// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2005-2011, Jari Sundell <jaris@ifi.uio.no>

#include "config.h"

#include <algorithm>
#include <functional>

#include "net/socket_set.h"

namespace torrent {

const SocketSet::size_type SocketSet::npos;

inline void
SocketSet::_replace_with_last(size_type idx) {
  while (!base_type::empty() && base_type::back() == NULL)
    base_type::pop_back();

  if (idx >= m_table.size())
    throw internal_error(
      "SocketSet::_replace_with_last(...) input out-of-bounds");

  // This should handle both npos and those that have already been
  // removed with the above while loop.
  if (idx >= size())
    return;

  *(begin() + idx)          = base_type::back();
  _index(base_type::back()) = idx;

  base_type::pop_back();
}

void
SocketSet::prepare() {
  std::for_each(
    m_erased.begin(),
    m_erased.end(),
    std::bind1st(std::mem_fun(&SocketSet::_replace_with_last), this));

  m_erased.clear();
}

void
SocketSet::reserve(size_t openMax) {
  m_table.resize(openMax, npos);

  base_type::reserve(openMax);
}

} // namespace torrent
