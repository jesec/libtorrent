// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2005-2011, Jari Sundell <jaris@ifi.uio.no>

#ifndef LIBTORRENT_DATA_FILE_MANAGER_H
#define LIBTORRENT_DATA_FILE_MANAGER_H

#include <torrent/common.h>
#include <vector>

namespace torrent {

class File;

class LIBTORRENT_EXPORT FileManager : private std::vector<File*> {
public:
  using base_type = std::vector<File*>;
  using size_type = uint32_t;

  using base_type::iterator;
  using base_type::reverse_iterator;
  using base_type::value_type;

  using base_type::begin;
  using base_type::end;
  using base_type::rbegin;
  using base_type::rend;

  FileManager() = default;
  ~FileManager();

  size_type open_files() const {
    return base_type::size();
  }

  size_type max_open_files() const {
    return m_maxOpenFiles;
  }
  void set_max_open_files(size_type s);

  bool open(value_type file, int prot, int flags);
  void close(value_type file);

  void close_least_active();

  // Statistics:
  uint64_t files_opened_counter() const {
    return m_filesOpenedCounter;
  }
  uint64_t files_closed_counter() const {
    return m_filesClosedCounter;
  }
  uint64_t files_failed_counter() const {
    return m_filesFailedCounter;
  }

private:
  FileManager(const FileManager&) LIBTORRENT_NO_EXPORT = delete;
  void operator=(const FileManager&) LIBTORRENT_NO_EXPORT = delete;

  size_type m_maxOpenFiles{ 0 };

  uint64_t m_filesOpenedCounter{ 0 };
  uint64_t m_filesClosedCounter{ 0 };
  uint64_t m_filesFailedCounter{ 0 };
};

} // namespace torrent

#endif
