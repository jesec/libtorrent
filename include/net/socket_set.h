// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2005-2011, Jari Sundell <jaris@ifi.uio.no>

#ifndef LIBTORRENT_NET_SOCKET_SET_H
#define LIBTORRENT_NET_SOCKET_SET_H

#include <cinttypes>
#include <list>
#include <vector>

#include "torrent/event.h"
#include "torrent/exceptions.h"
#include "torrent/utils/allocators.h"

namespace torrent {

// SocketSet's Base is a vector of active SocketBase
// instances. 'm_table' is a vector with the size 'openMax', each
// element of which points to an active instance in the Base vector.

// Propably should rename to EventSet...

class SocketSet
  : private std::vector<Event*, utils::cacheline_allocator<Event*>> {
public:
  using size_type = uint32_t;

  using base_type = std::vector<Event*, utils::cacheline_allocator<Event*>>;
  using Table = std::vector<size_type, utils::cacheline_allocator<size_type>>;

  static const size_type npos = static_cast<size_type>(-1);

  using base_type::value_type;

  using base_type::const_iterator;
  using base_type::empty;
  using base_type::iterator;
  using base_type::reverse_iterator;
  using base_type::size;

  using base_type::begin;
  using base_type::end;
  using base_type::rbegin;
  using base_type::rend;

  bool has(Event* s) const {
    return _index(s) != npos;
  }

  iterator find(Event* s);
  void     insert(Event* s);
  void     erase(Event* s);

  // Remove all erased elements from the container.
  void prepare();
  // Allocate storage for fd's with up to 'openMax' value. TODO: Remove reserve
  void reserve(size_t openMax);

  size_t max_size() const {
    return m_table.size();
  }

private:
  size_type& _index(Event* s) {
    return m_table[s->file_descriptor()];
  }
  const size_type& _index(Event* s) const {
    return m_table[s->file_descriptor()];
  }

  inline void _replace_with_last(size_type idx);

  // TODO: Table of indexes or iterators?
  Table m_table;
  Table m_erased;
};

inline SocketSet::iterator
SocketSet::find(Event* s) {
  if (_index(s) == npos)
    return end();

  return begin() + _index(s);
}

inline void
SocketSet::insert(Event* s) {
  if (static_cast<size_type>(s->file_descriptor()) >= m_table.size())
    throw internal_error(
      "Tried to insert an out-of-bounds file descriptor to SocketSet");

  if (_index(s) != npos)
    return;

  _index(s) = size();
  base_type::push_back(s);
}

inline void
SocketSet::erase(Event* s) {
  if (static_cast<size_type>(s->file_descriptor()) >= m_table.size())
    throw internal_error(
      "Tried to erase an out-of-bounds file descriptor from SocketSet");

  size_type idx = _index(s);

  if (idx == npos)
    return;

  _index(s) = npos;

  *(begin() + idx) = NULL;
  m_erased.push_back(idx);
}

} // namespace torrent

#endif
