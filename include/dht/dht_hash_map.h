// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2005-2011, Jari Sundell <jaris@ifi.uio.no>

#ifndef LIBTORRENT_DHT_HASH_MAP_H
#define LIBTORRENT_DHT_HASH_MAP_H

#include "torrent/buildinfo.h"

#include <unordered_map>

#include "dht_node.h"
#include "dht_tracker.h"
#include "torrent/hash_string.h"

namespace torrent {

class DhtNodeList : public std::unordered_map<const HashString*, DhtNode*> {
public:
  using base_type = std::unordered_map<const HashString*, DhtNode*>;

  // Define accessor iterator with more convenient access to the key and
  // element values.  Allows changing the map definition more easily if needed.
  template<typename T>
  struct accessor_wrapper : public T {
    accessor_wrapper(const T& itr)
      : T(itr) {}

    const HashString& id() const {
      return *(**this).first;
    }
    DhtNode* node() const {
      return (**this).second;
    }
  };

  using const_accessor = accessor_wrapper<const_iterator>;
  using accessor       = accessor_wrapper<iterator>;

  DhtNode* add_node(DhtNode* n);
};

class DhtTrackerList : public std::unordered_map<HashString, DhtTracker*> {
public:
  using base_type = std::unordered_map<HashString, DhtTracker*>;

  template<typename T>
  struct accessor_wrapper : public T {
    accessor_wrapper(const T& itr)
      : T(itr) {}

    const HashString& id() const {
      return (**this).first;
    }
    DhtTracker* tracker() const {
      return (**this).second;
    }
  };

  using const_accessor = accessor_wrapper<const_iterator>;
  using accessor       = accessor_wrapper<iterator>;
};

inline DhtNode*
DhtNodeList::add_node(DhtNode* n) {
  insert(std::make_pair((const HashString*)n, (DhtNode*)n));
  return n;
}

} // namespace torrent

#endif
