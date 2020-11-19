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
  typedef std::vector<DownloadWrapper*> base_type;

  typedef base_type::value_type      value_type;
  typedef base_type::pointer         pointer;
  typedef base_type::const_pointer   const_pointer;
  typedef base_type::reference       reference;
  typedef base_type::const_reference const_reference;
  typedef base_type::size_type       size_type;

  typedef base_type::iterator               iterator;
  typedef base_type::reverse_iterator       reverse_iterator;
  typedef base_type::const_iterator         const_iterator;
  typedef base_type::const_reverse_iterator const_reverse_iterator;

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
