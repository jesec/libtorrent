// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2005-2007, Jari Sundell <jaris@ifi.uio.no>

#ifndef LIBTORRENT_UTILS_UNORDERED_VECTOR_H
#define LIBTORRENT_UTILS_UNORDERED_VECTOR_H

#include <vector>

namespace torrent {
namespace utils {

template<typename _Tp>
class unordered_vector : private std::vector<_Tp> {
public:
  using Base = std::vector<_Tp>;

  using value_type      = typename Base::value_type;
  using pointer         = typename Base::pointer;
  using const_pointer   = typename Base::const_pointer;
  using reference       = typename Base::reference;
  using const_reference = typename Base::const_reference;
  using size_type       = typename Base::size_type;
  using difference_type = typename Base::difference_type;
  using allocator_type  = typename Base::allocator_type;

  using iterator               = typename Base::iterator;
  using reverse_iterator       = typename Base::reverse_iterator;
  using const_iterator         = typename Base::const_iterator;
  using const_reverse_iterator = typename Base::const_reverse_iterator;

  using Base::clear;
  using Base::empty;
  using Base::reserve;
  using Base::size;

  using Base::back;
  using Base::begin;
  using Base::end;
  using Base::front;
  using Base::rbegin;
  using Base::rend;

  using Base::pop_back;
  using Base::push_back;

  // Use the range erase function, the single element erase gets
  // overloaded.
  using Base::erase;

  iterator insert(iterator position, const value_type& x);
  iterator erase(iterator position);

private:
};

template<typename _Tp>
typename unordered_vector<_Tp>::iterator
unordered_vector<_Tp>::insert(iterator, const value_type& x) {
  Base::push_back(x);

  return --end();
}

template<typename _Tp>
typename unordered_vector<_Tp>::iterator
unordered_vector<_Tp>::erase(iterator position) {
  // We don't need to check if position == end - 1 since we then copy
  // to the position we pop later.
  *position = Base::back();
  Base::pop_back();

  return position;
}

} // namespace utils
} // namespace torrent

#endif
