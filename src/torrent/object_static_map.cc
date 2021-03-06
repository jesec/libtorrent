// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2005-2011, Jari Sundell <jaris@ifi.uio.no>

#include "torrent/utils/algorithm.h"

#include "torrent/object_static_map.h"

namespace torrent {

const static_map_key_search_result
find_key_match(const static_map_mapping_type* first,
               const static_map_mapping_type* last,
               const char*                    key_first,
               const char*                    key_last) {
  //  unsigned int key_length = strlen(key);
  const static_map_mapping_type* itr = first;

  while (itr != last) {
    unsigned int base = utils::count_base(
      key_first, key_last, itr->key, itr->key + itr->max_key_size);

    if (key_first[base] != '\0') {
      // Return not found here if we know the entry won't come after
      // this.
      itr++;
      continue;
    }

    if (itr->key[base] == '\0' || itr->key[base] == '*' ||
        (itr->key[base] == ':' && itr->key[base + 1] == ':') ||
        (itr->key[base] == '[' && itr->key[base + 1] == ']'))
      return static_map_key_search_result(itr, base);

    break;
  }

  return static_map_key_search_result(first, 0);
}

} // namespace torrent
