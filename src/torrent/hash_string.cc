// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2005-2011, Jari Sundell <jaris@ifi.uio.no>

#include <cctype>

#include "torrent/hash_string.h"
#include "torrent/utils/string_manip.h"

namespace torrent {

const char*
hash_string_from_hex_c_str(const char* first, HashString& hash) {
  const char* hash_first = first;

  torrent::HashString::iterator itr = hash.begin();

  while (itr != hash.end()) {
    if (!std::isxdigit(*first) || !std::isxdigit(*(first + 1)))
      return hash_first;

    *itr++ = (utils::hexchar_to_value(*first) << 4) +
             utils::hexchar_to_value(*(first + 1));
    first += 2;
  }

  return first;
}

char*
hash_string_to_hex(const HashString& hash, char* first) {
  return utils::transform_hex(hash.begin(), hash.end(), first);
}

std::string
hash_string_to_hex_str(const HashString& hash) {
  std::string str(HashString::size_data * 2, '\0');
  utils::transform_hex(hash.begin(), hash.end(), str.begin());

  return str;
}

} // namespace torrent
