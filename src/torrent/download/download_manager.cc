// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2005-2011, Jari Sundell <jaris@ifi.uio.no>

#include "config.h"

#include "download/download_wrapper.h"
#include "rak/functional.h"
#include "torrent/download/download_manager.h"
#include "torrent/exceptions.h"

namespace torrent {

DownloadManager::iterator
DownloadManager::insert(DownloadWrapper* d) {
  if (find(d->info()->hash()) != end())
    throw internal_error("Could not add torrent as it already exists.");

  return base_type::insert(end(), d);
}

DownloadManager::iterator
DownloadManager::erase(DownloadWrapper* d) {
  iterator itr = std::find(begin(), end(), d);

  if (itr == end())
    throw internal_error("Tried to remove a torrent that doesn't exist");

  delete *itr;
  return base_type::erase(itr);
}

void
DownloadManager::clear() {
  while (!empty()) {
    delete base_type::back();
    base_type::pop_back();
  }
}

DownloadManager::iterator
DownloadManager::find(const std::string& hash) {
  return std::find_if(begin(),
                      end(),
                      rak::equal(*HashString::cast_from(hash),
                                 rak::on(std::mem_fun(&DownloadWrapper::info),
                                         std::mem_fun(&DownloadInfo::hash))));
}

DownloadManager::iterator
DownloadManager::find(const HashString& hash) {
  return std::find_if(begin(),
                      end(),
                      rak::equal(hash,
                                 rak::on(std::mem_fun(&DownloadWrapper::info),
                                         std::mem_fun(&DownloadInfo::hash))));
}

DownloadManager::iterator
DownloadManager::find(DownloadInfo* info) {
  return std::find_if(
    begin(), end(), rak::equal(info, std::mem_fun(&DownloadWrapper::info)));
}

DownloadManager::iterator
DownloadManager::find_chunk_list(ChunkList* cl) {
  return std::find_if(
    begin(), end(), rak::equal(cl, std::mem_fun(&DownloadWrapper::chunk_list)));
}

DownloadMain*
DownloadManager::find_main(const char* hash) {
  iterator itr =
    std::find_if(begin(),
                 end(),
                 rak::equal(*HashString::cast_from(hash),
                            rak::on(std::mem_fun(&DownloadWrapper::info),
                                    std::mem_fun(&DownloadInfo::hash))));

  if (itr == end())
    return NULL;
  else
    return (*itr)->main();
}

DownloadMain*
DownloadManager::find_main_obfuscated(const char* hash) {
  iterator itr = std::find_if(
    begin(),
    end(),
    rak::equal(*HashString::cast_from(hash),
               rak::on(std::mem_fun(&DownloadWrapper::info),
                       std::mem_fun(&DownloadInfo::hash_obfuscated))));

  if (itr == end())
    return NULL;
  else
    return (*itr)->main();
}

} // namespace torrent
