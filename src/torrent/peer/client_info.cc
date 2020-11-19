// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2005-2011, Jari Sundell <jaris@ifi.uio.no>

#include "config.h"

#include <cstring>

#include "client_info.h"

namespace torrent {

unsigned int
ClientInfo::key_size(id_type id) {
  switch (id) {
    case TYPE_AZUREUS:
      return 2;
    case TYPE_COMPACT:
    case TYPE_MAINLINE:
      return 1;

    default:
      return 0;
  }
}

unsigned int
ClientInfo::version_size(id_type id) {
  switch (id) {
    case TYPE_AZUREUS:
      return 4;
    case TYPE_COMPACT:
    case TYPE_MAINLINE:
      return 3;

    default:
      return 0;
  }
}

bool
ClientInfo::less_intersects(const ClientInfo& left, const ClientInfo& right) {
  if (left.type() > right.type())
    return false;
  else if (left.type() < right.type())
    return true;

  int keyComp = std::memcmp(left.key(), right.key(), ClientInfo::max_key_size);

  return keyComp < 0 ||
         (keyComp == 0 && std::memcmp(left.upper_version(),
                                      right.version(),
                                      ClientInfo::max_version_size) < 0);
}

bool
ClientInfo::less_disjoint(const ClientInfo& left, const ClientInfo& right) {
  if (left.type() > right.type())
    return false;
  else if (left.type() < right.type())
    return true;

  int keyComp = std::memcmp(left.key(), right.key(), ClientInfo::max_key_size);

  return keyComp < 0 ||
         (keyComp == 0 && std::memcmp(left.version(),
                                      right.upper_version(),
                                      ClientInfo::max_version_size) < 0);
}

bool
ClientInfo::greater_intersects(const ClientInfo& left,
                               const ClientInfo& right) {
  return less_intersects(right, left);
}

bool
ClientInfo::greater_disjoint(const ClientInfo& left, const ClientInfo& right) {
  return less_disjoint(right, left);
}

bool
ClientInfo::intersects(const ClientInfo& left, const ClientInfo& right) {
  return left.type() == right.type() &&
         std::memcmp(left.key(), right.key(), ClientInfo::max_key_size) == 0 &&

         std::memcmp(left.version(),
                     right.upper_version(),
                     ClientInfo::max_version_size) <= 0 &&
         std::memcmp(left.upper_version(),
                     right.version(),
                     ClientInfo::max_version_size) >= 0;
}

inline bool
ClientInfo::equal_to(const ClientInfo& left, const ClientInfo& right) {
  return left.type() == right.type() &&
         std::memcmp(left.key(), right.key(), ClientInfo::max_key_size) == 0 &&

         std::memcmp(left.version(),
                     right.version(),
                     ClientInfo::max_version_size) == 0 &&
         std::memcmp(left.upper_version(),
                     right.upper_version(),
                     ClientInfo::max_version_size) == 0;
}

} // namespace torrent
