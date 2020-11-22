// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2005-2011, Jari Sundell <jaris@ifi.uio.no>

#include <algorithm>

#include "net/address_list.h"
#include "torrent/utils/functional.h"

namespace torrent {

inline utils::socket_address
AddressList::parse_address(const Object& b) {
  utils::socket_address sa;
  sa.clear();

  if (!b.is_map())
    return sa;

  if (!b.has_key_string("ip") || !sa.set_address_str(b.get_key_string("ip")))
    return sa;

  if (!b.has_key_value("port") || b.get_key_value("port") <= 0 ||
      b.get_key_value("port") >= (1 << 16))
    return sa;

  sa.set_port(b.get_key_value("port"));

  return sa;
}

void
AddressList::parse_address_normal(const Object::list_type& b) {
  std::for_each(b.begin(),
                b.end(),
                utils::on(std::ptr_fun(&AddressList::parse_address),
                          AddressList::add_address(this)));
}

void
AddressList::parse_address_compact(raw_string s) {
  if (sizeof(const SocketAddressCompact) != 6)
    throw internal_error("ConnectionList::AddressList::parse_address_compact(.."
                         ".) bad struct size.");

  std::copy(reinterpret_cast<const SocketAddressCompact*>(s.data()),
            reinterpret_cast<const SocketAddressCompact*>(
              s.data() + s.size() - s.size() % sizeof(SocketAddressCompact)),
            std::back_inserter(*this));
}

void
AddressList::parse_address_compact_ipv6(const std::string& s) {
  if (sizeof(const SocketAddressCompact6) != 18)
    throw internal_error("ConnectionList::AddressList::parse_address_compact_"
                         "ipv6(...) bad struct size.");

  std::copy(reinterpret_cast<const SocketAddressCompact6*>(s.c_str()),
            reinterpret_cast<const SocketAddressCompact6*>(
              s.c_str() + s.size() - s.size() % sizeof(SocketAddressCompact6)),
            std::back_inserter(*this));
}

void
AddressList::parse_address_bencode(raw_list s) {
  if (sizeof(const SocketAddressCompact) != 6)
    throw internal_error(
      "AddressList::parse_address_bencode(...) bad struct size.");

  for (raw_list::const_iterator itr = s.begin();
       itr + 2 + sizeof(SocketAddressCompact) <= s.end();
       itr += sizeof(SocketAddressCompact)) {
    if (*itr++ != '6' || *itr++ != ':')
      break;

    insert(end(), *reinterpret_cast<const SocketAddressCompact*>(s.data()));
  }
}

} // namespace torrent
