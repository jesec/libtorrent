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

// Hash functions for HashString keys, and dereferencing HashString pointers.

// Since the first few bits are very similar if not identical (since the IDs
// will be close to our own node ID), we use an offset of 64 bits in the hash
// string. These bits will be uniformly distributed until the number of DHT
// nodes on the planet approaches 2^64 which is... unlikely.
// An offset of 64 bits provides 96 significant bits which is fine as long as
// the size of size_t does not exceed 12 bytes, while still having correctly
// aligned 64-bit access.
static constexpr unsigned int hashstring_hash_ofs = 8;
static_assert((hashstring_hash_ofs + sizeof(size_t)) <= HashString::size_data);

struct hashstring_ptr_hash
  : public std::unary_function<const HashString*, size_t> {
  size_t operator()(const HashString* n) const {
    size_t result = 0;
    std::memcpy(&result, n->data() + hashstring_hash_ofs, sizeof(size_t));
    return result;
  }
};

struct hashstring_hash : public std::unary_function<HashString, size_t> {
  size_t operator()(const HashString& n) const {
    size_t result = 0;
    std::memcpy(&result, n.data() + hashstring_hash_ofs, sizeof(size_t));
    return result;
  }
};

// Compare HashString pointers by dereferencing them.
struct hashstring_ptr_equal
  : public std::binary_function<const HashString*, const HashString*, bool> {
  size_t operator()(const HashString* one, const HashString* two) const {
    return *one == *two;
  }
};

class DhtNodeList
  : public std::unordered_map<const HashString*,
                              DhtNode*,
                              hashstring_ptr_hash,
                              hashstring_ptr_equal> {
public:
  using base_type = std::unordered_map<const HashString*,
                                       DhtNode*,
                                       hashstring_ptr_hash,
                                       hashstring_ptr_equal>;

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

class DhtTrackerList
  : public std::unordered_map<HashString, DhtTracker*, hashstring_hash> {
public:
  using base_type =
    std::unordered_map<HashString, DhtTracker*, hashstring_hash>;

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
