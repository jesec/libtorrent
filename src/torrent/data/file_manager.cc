// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2005-2011, Jari Sundell <jaris@ifi.uio.no>

#include <algorithm>

#include "data/socket_file.h"
#include "manager.h"
#include "torrent/data/file.h"
#include "torrent/data/file_manager.h"
#include "torrent/exceptions.h"

namespace torrent {

FileManager::FileManager()
  : m_maxOpenFiles(0)
  , m_filesOpenedCounter(0)
  , m_filesClosedCounter(0)
  , m_filesFailedCounter(0) {}

FileManager::~FileManager() {
  if (!empty())
    deconstruct_error("FileManager::~FileManager() called but empty() != true.");
}

void
FileManager::set_max_open_files(size_type s) {
  if (s < 4 || s > (1 << 16))
    throw input_error("Max open files must be between 4 and 2^16.");

  m_maxOpenFiles = s;

  while (size() > m_maxOpenFiles)
    close_least_active();
}

bool
FileManager::open(value_type file, int prot, int flags) {
  if (file->is_open())
    close(file);

  if (size() > m_maxOpenFiles)
    throw internal_error(
      "FileManager::open_file(...) m_openSize > m_maxOpenFiles.");

  if (size() == m_maxOpenFiles)
    close_least_active();

  SocketFile fd;

  if (!fd.open(file->frozen_path(), prot, flags)) {
    m_filesFailedCounter++;
    return false;
  }

  file->set_protection(prot);
  file->set_file_descriptor(fd.fd());
  base_type::push_back(file);

  // Consider storing the position of the file here.

  m_filesOpenedCounter++;
  return true;
}

void
FileManager::close(value_type file) {
  if (!file->is_open())
    return;

  SocketFile(file->file_descriptor()).close();

  file->set_protection(0);
  file->set_file_descriptor(-1);

  iterator itr = std::find(begin(), end(), file);

  if (itr == end())
    throw internal_error("FileManager::close_file(...) itr == end().");

  *itr = back();
  base_type::pop_back();

  m_filesClosedCounter++;
}

struct FileManagerActivity {
  FileManagerActivity()
    : m_last(utils::timer::max().usec())
    , m_file(NULL) {}

  void operator()(File* f) {
    if (f->is_open() && f->last_touched() <= m_last) {
      m_last = f->last_touched();
      m_file = f;
    }
  }

  uint64_t m_last;
  File*    m_file;
};

void
FileManager::close_least_active() {
  close(std::for_each(begin(), end(), FileManagerActivity()).m_file);
}

} // namespace torrent
