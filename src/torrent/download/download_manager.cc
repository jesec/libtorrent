// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2005-2011, Jari Sundell <jaris@ifi.uio.no>

#include "download/download_wrapper.h"
#include "torrent/exceptions.h"

#include "torrent/download/download_manager.h"

namespace torrent {

DownloadManager::iterator
DownloadManager::insert(DownloadWrapper* d) {
  if (find(d->info()->hash()) != end())
    throw internal_error("Could not add torrent as it already exists.");

  lookup_cache.emplace(d->info()->hash(), size());
  obfuscated_to_hash.emplace(d->info()->hash_obfuscated(), d->info()->hash());

  return base_type::insert(end(), d);
}

DownloadManager::iterator
DownloadManager::erase(DownloadWrapper* d) {
  auto itr = find(d->info()->hash());

  if (itr == end())
    throw internal_error("Tried to remove a torrent that doesn't exist");

  lookup_cache.erase(lookup_cache.find(d->info()->hash()));
  obfuscated_to_hash.erase(
    obfuscated_to_hash.find(d->info()->hash_obfuscated()));

  delete *itr;
  return base_type::erase(itr);
}

void
DownloadManager::clear() {
  base_type::clear();
  lookup_cache.clear();
  obfuscated_to_hash.clear();
}

DownloadManager::iterator
DownloadManager::find(const std::string& hash) {
  return find(*HashString::cast_from(hash));
}

DownloadManager::iterator
DownloadManager::find(const HashString& hash) {
  auto cached = lookup_cache.find(hash);

  if (cached == lookup_cache.end()) {
    return end();
  }

  auto cached_i = cached->second;

  auto itr = cached_i < size() ? begin() + cached_i : end();
  if (itr == end() || (*itr)->info()->hash() != hash) {
    itr = std::find_if(begin(), end(), [hash](DownloadWrapper* wrapper) {
      return hash == wrapper->info()->hash();
    });
  }

  lookup_cache[hash] = itr - begin();

  return itr;
}

DownloadMain*
DownloadManager::find_main(const char* hash) {
  auto itr = find(*HashString::cast_from(hash));

  if (itr == end()) {
    return nullptr;
  }

  return (*itr)->main();
}

DownloadMain*
DownloadManager::find_main_obfuscated(const char* obfuscated) {
  auto hash_itr = obfuscated_to_hash.find(*HashString::cast_from(obfuscated));

  if (hash_itr == obfuscated_to_hash.end()) {
    return nullptr;
  }

  auto itr = find(hash_itr->second);

  if (itr == end()) {
    return nullptr;
  }

  return (*itr)->main();
}

} // namespace torrent
