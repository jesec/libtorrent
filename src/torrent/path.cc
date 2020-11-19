// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2005-2011, Jari Sundell <jaris@ifi.uio.no>

#include "config.h"

#include <algorithm>

#include "torrent/exceptions.h"
#include "torrent/path.h"

namespace torrent {

void
Path::insert_path(iterator pos, const std::string& path) {
  std::string::const_iterator first = path.begin();
  std::string::const_iterator last;

  while (first != path.end()) {
    pos = insert(
      pos, std::string(first, (last = std::find(first, path.end(), '/'))));

    if (last == path.end())
      return;

    first = last;
    first++;
  }
}

std::string
Path::as_string() const {
  if (empty())
    return std::string();

  std::string s;

  for (const_iterator itr = begin(); itr != end(); ++itr) {
    s += '/';
    s += *itr;
  }

  return s;
}

} // namespace torrent
