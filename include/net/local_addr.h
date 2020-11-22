// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2005-2007, Jari Sundell <jaris@ifi.uio.no>

// A routine to get a local IP address that can be presented to a tracker.
// (Does not use UPnP etc., so will not understand NAT.)
// On a machine with multiple network cards, address selection can be a
// complex process, and in general what's selected is a source/destination
// address pair. However, this routine will give an approximation that will
// be good enough for most purposes and users.

#ifndef LIBTORRENT_NET_LOCAL_ADDR_H
#define LIBTORRENT_NET_LOCAL_ADDR_H

#include <sys/socket.h>
#include <unistd.h>

#include "torrent/utils/socket_address.h"

namespace torrent {

// Note: family must currently be utils::af_inet or utils::af_inet6
// (utils::af_unspec won't do); anything else will throw an exception.
// Returns false if no address of the given family could be found,
// either because there are none, or because something went wrong in
// the process (e.g., no free file descriptors).
bool
get_local_address(sa_family_t family, utils::socket_address* address);

} // namespace torrent

#endif /* LIBTORRENT_NET_LOCAL_ADDR_H */
