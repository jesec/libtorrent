// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2005-2007, Jari Sundell <jaris@ifi.uio.no>

#ifndef RAK_FS_STAT_H
#define RAK_FS_STAT_H

#include <cinttypes>
#include <string>

#include <rak/error_number.h>

#if HAVE_SYS_VFS_H
#include <sys/vfs.h>
#endif
#if HAVE_SYS_STATVFS_H
#include <sys/statvfs.h>
#endif
#if HAVE_SYS_STATFS_H
#include <sys/statfs.h>
#endif
#if HAVE_SYS_PARAM_H
#include <sys/param.h>
#endif
#if HAVE_SYS_MOUNT_H
#include <sys/mount.h>
#endif

namespace rak {

class fs_stat {
public:
  typedef FS_STAT_SIZE_TYPE  blocksize_type;
  typedef FS_STAT_COUNT_TYPE blockcount_type;
  typedef FS_STAT_STRUCT     fs_stat_type;

  bool update(int fd) {
    return FS_STAT_FD;
  }
  bool update(const char* fn) {
    return FS_STAT_FN;
  }
  bool update(const std::string& filename) {
    return update(filename.c_str());
  }

  blocksize_type blocksize() {
    return FS_STAT_BLOCK_SIZE;
  }
  blockcount_type blocks_avail() {
    return m_stat.f_bavail;
  }
  int64_t bytes_avail() {
    return (int64_t)blocksize() * m_stat.f_bavail;
  }

private:
  fs_stat_type m_stat;
};

} // namespace rak

#endif
