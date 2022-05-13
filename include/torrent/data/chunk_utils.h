// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2005-2011, Jari Sundell <jaris@ifi.uio.no>

#ifndef LIBTORRENT_CHUNK_UTILS_H
#define LIBTORRENT_CHUNK_UTILS_H

#include <vector>

#include <torrent/common.h>
#include <torrent/download.h>

namespace torrent {

class ChunkList;

struct vm_mapping {
  void*    ptr;
  uint64_t length;
};

// Change to ChunkList* when that becomes part of the public API.

std::vector<vm_mapping>
chunk_list_mapping(Download* download) LIBTORRENT_EXPORT;

} // namespace torrent

#endif
