// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2005-2011, Jari Sundell <jaris@ifi.uio.no>

#include <algorithm>
#include <cstdlib>
#include <iterator>

#include "download/available_list.h"
#include "torrent/exceptions.h"

namespace torrent {

AvailableList::value_type
AvailableList::pop_random() {
  if (empty())
    throw internal_error(
      "AvailableList::pop_random() called on an empty container");

  size_type idx = random() % size();

  value_type tmp   = *(begin() + idx);
  *(begin() + idx) = back();

  pop_back();

  return tmp;
}

void
AvailableList::push_back(const utils::socket_address* sa) {
  if (std::find(begin(), end(), *sa) != end())
    return;

  base_type::push_back(*sa);
}

void
AvailableList::insert(AddressList* l) {
  if (!want_more())
    return;

  std::sort(begin(), end());

  // Can i use use the std::remove* semantics for this, and just copy
  // to 'l'?.
  //
  // 'l' is guaranteed to be sorted, so we can just do
  // std::set_difference.
  AddressList difference;
  std::set_difference(
    l->begin(), l->end(), begin(), end(), std::back_inserter(difference));

  std::copy(difference.begin(),
            difference.end(),
            std::back_inserter(*static_cast<base_type*>(this)));
}

void
AvailableList::erase(const utils::socket_address& sa) {
  iterator itr = std::find(begin(), end(), sa);

  if (itr != end()) {
    *itr = back();
    pop_back();
  }
}

} // namespace torrent
