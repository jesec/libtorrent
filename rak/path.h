// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2005-2007, Jari Sundell <jaris@ifi.uio.no>

// Various functions for manipulating file paths. Also consider making
// a directory iterator.

#ifndef RAK_PATH_H
#define RAK_PATH_H

#include <cstdlib>
#include <string>

namespace rak {

inline std::string
path_expand(const std::string& path) {
  if (path.empty() || path[0] != '~')
    return path;

  char* home = std::getenv("HOME");

  if (home == NULL)
    return path;

  return home + path.substr(1);
}

// Don't inline this...
//
// Same strlcpy as found in *bsd.
inline size_t
strlcpy(char* dest, const char* src, size_t size) {
  size_t      n     = size;
  const char* first = src;

  if (n != 0) {
    while (--n != 0)
      if ((*dest++ = *src++) == '\0')
        break;
  }

  if (n == 0) {
    if (size != 0)
      *dest = '\0';

    while (*src++)
      ;
  }

  return src - first - 1;
}

inline char*
path_expand(const char* src, char* first, char* last) {
  if (*src == '~') {
    char* home = std::getenv("HOME");

    if (home == NULL)
      return first;

    first += strlcpy(first, home, std::distance(first, last));

    if (first > last)
      return last;

    src++;
  }

  return std::min(first + strlcpy(first, src, std::distance(first, last)),
                  last);
}

} // namespace rak

#endif
