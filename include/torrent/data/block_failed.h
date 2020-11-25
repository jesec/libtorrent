// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2005-2011, Jari Sundell <jaris@ifi.uio.no>

#ifndef LIBTORRENT_BLOCK_FAILED_H
#define LIBTORRENT_BLOCK_FAILED_H

#include <algorithm>
#include <functional>
#include <torrent/common.h>
#include <vector>

namespace torrent {

class BlockFailed : public std::vector<std::pair<char*, uint32_t>> {
public:
  typedef std::vector<std::pair<char*, uint32_t>> base_type;

  using base_type::difference_type;
  using base_type::reference;
  using base_type::size_type;
  using base_type::value_type;

  using base_type::empty;
  using base_type::iterator;
  using base_type::reverse_iterator;
  using base_type::size;

  using base_type::begin;
  using base_type::end;
  using base_type::rbegin;
  using base_type::rend;

  using base_type::operator[];

  static const uint32_t invalid_index = ~uint32_t();

  BlockFailed()
    : m_current(invalid_index) {}
  ~BlockFailed();

  size_type current() const {
    return m_current;
  }
  iterator current_iterator() {
    return begin() + m_current;
  }
  reverse_iterator current_reverse_iterator() {
    return reverse_iterator(begin() + m_current + 1);
  }

  void set_current(size_type idx) {
    m_current = idx;
  }
  void set_current(iterator itr) {
    m_current = itr - begin();
  }
  void set_current(reverse_iterator itr) {
    m_current = itr.base() - begin() - 1;
  }

  iterator         max_element();
  reverse_iterator reverse_max_element();

private:
  BlockFailed(const BlockFailed&);
  void operator=(const BlockFailed&);

  static void delete_entry(value_type e) {
    delete[] e.first;
  }
  static bool compare_entries(value_type e1, value_type e2) {
    return e1.second < e2.second;
  }

  size_type m_current;
};

inline BlockFailed::~BlockFailed() {
  std::for_each(begin(), end(), [](std::pair<char*, uint32_t> p) {
    return delete_entry(p);
  });
}

inline BlockFailed::iterator
BlockFailed::max_element() {
  return std::max_element(
    begin(),
    end(),
    [](std::pair<char*, uint32_t> p1, std::pair<char*, uint32_t> p2) {
      return compare_entries(p1, p2);
    });
}

inline BlockFailed::reverse_iterator
BlockFailed::reverse_max_element() {
  return std::max_element(
    rbegin(),
    rend(),
    [](std::pair<char*, uint32_t> p1, std::pair<char*, uint32_t> p2) {
      return compare_entries(p1, p2);
    });
}

} // namespace torrent

#endif
