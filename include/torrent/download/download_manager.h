// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2005-2011, Jari Sundell <jaris@ifi.uio.no>

#ifndef LIBTORRENT_DOWNLOAD_MANAGER_H
#define LIBTORRENT_DOWNLOAD_MANAGER_H

#include <torrent/common.h>
#include <vector>

namespace torrent {

class ChunkList;
class DownloadWrapper;
class DownloadInfo;
class DownloadMain;

class LIBTORRENT_EXPORT DownloadManager
  : private std::vector<DownloadWrapper*> {
public:
  using base_type = std::vector<DownloadWrapper*>;

  using value_type      = base_type::value_type;
  using pointer         = base_type::pointer;
  using const_pointer   = base_type::const_pointer;
  using reference       = base_type::reference;
  using const_reference = base_type::const_reference;
  using size_type       = base_type::size_type;

  using iterator               = base_type::iterator;
  using reverse_iterator       = base_type::reverse_iterator;
  using const_iterator         = base_type::const_iterator;
  using const_reverse_iterator = base_type::const_reverse_iterator;

  using base_type::empty;
  using base_type::size;

  using base_type::begin;
  using base_type::end;
  using base_type::rbegin;
  using base_type::rend;

  ~DownloadManager() {
    clear();
  }

  iterator find(const std::string& hash);
  iterator find(const HashString& hash);
  iterator find(DownloadInfo* info);

  iterator find_chunk_list(ChunkList* cl);

  DownloadMain* find_main(const char* hash);
  DownloadMain* find_main_obfuscated(const char* hash);

  //
  // Don't export:
  //

  iterator insert(DownloadWrapper* d) LIBTORRENT_NO_EXPORT;
  iterator erase(DownloadWrapper* d) LIBTORRENT_NO_EXPORT;

  void clear() LIBTORRENT_NO_EXPORT;
};

} // namespace torrent

#endif
