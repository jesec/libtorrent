// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2005-2011, Jari Sundell <jaris@ifi.uio.no>

#include "data/chunk.h"
#include "data/chunk_list.h"
#include "download/download_wrapper.h"
#include "manager.h"
#include "torrent/chunk_manager.h"
#include "torrent/download.h"
#include "torrent/download/download_manager.h"
#include "torrent/exceptions.h"

#include "torrent/data/chunk_utils.h"

namespace torrent {

std::vector<vm_mapping>
chunk_list_mapping(Download* download) {
  ChunkList* chunk_list = download->ptr()->main()->chunk_list();

  std::vector<vm_mapping> mappings;

  for (const auto& chunk : *chunk_list) {
    if (!chunk.is_valid())
      continue;

    for (const auto& part : *chunk.chunk()) {
      if (part.mapped() != ChunkPart::MAPPED_MMAP)
        continue;

      vm_mapping val = { part.chunk().ptr(), part.chunk().size_aligned() };
      mappings.push_back(val);
    }
  }

  return mappings;
}

} // namespace torrent
