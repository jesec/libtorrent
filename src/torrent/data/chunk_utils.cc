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

chunk_info_result
chunk_list_address_info(void* address) {
  ChunkManager::iterator first = manager->chunk_manager()->begin();
  ChunkManager::iterator last  = manager->chunk_manager()->begin();

  while (first != last) {
    ChunkList::chunk_address_result result = (*first)->find_address(address);

    if (result.first != (*first)->end()) {
      DownloadManager::iterator d_itr =
        manager->download_manager()->find_chunk_list(*first);

      if (d_itr == manager->download_manager()->end())
        return chunk_info_result();

      chunk_info_result ci;
      ci.download    = Download(*d_itr);
      ci.chunk_index = result.first->index();
      ci.chunk_offset =
        result.second->position() +
        std::distance(result.second->chunk().begin(), (char*)address);

      ci.file_path = result.second->file()->frozen_path().c_str();
      ci.file_offset =
        result.second->file_offset() +
        std::distance(result.second->chunk().begin(), (char*)address);

      return ci;
    }

    first++;
  }

  return chunk_info_result();
}

} // namespace torrent
