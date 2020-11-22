// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2005-2007, Jari Sundell <jaris@ifi.uio.no>

#ifndef LIBTORRENT_UTILS_FS_STAT_H
#define LIBTORRENT_UTILS_FS_STAT_H

#include <cinttypes>
#include <string>

#include <torrent/buildinfo.h>
#include <torrent/utils/error_number.h>

#if LT_HAVE_STATVFS

#include <sys/statfs.h>
#include <sys/statvfs.h>
#include <sys/vfs.h>

#define FS_STAT_FD         fstatvfs(fd, &m_stat) == 0
#define FS_STAT_FN         statvfs(fn, &m_stat) == 0
#define FS_STAT_STRUCT     struct statvfs
#define FS_STAT_SIZE_TYPE  unsigned long
#define FS_STAT_COUNT_TYPE fsblkcnt_t
#define FS_STAT_BLOCK_SIZE (m_stat.f_frsize)

#elif LT_HAVE_STATFS

#include <sys/mount.h>
#include <sys/param.h>
#include <sys/statfs.h>

#define FS_STAT_FD         fstatfs(fd, &m_stat) == 0
#define FS_STAT_FN         statfs(fn, &m_stat) == 0
#define FS_STAT_STRUCT     struct statfs
#define FS_STAT_SIZE_TYPE  long
#define FS_STAT_COUNT_TYPE long
#define FS_STAT_BLOCK_SIZE (m_stat.f_bsize)

#else

#define FS_STAT_FD (errno = ENOSYS) == 0
#define FS_STAT_FN (errno = ENOSYS) == 0
#define FS_STAT_STRUCT                                                         \
  struct {                                                                     \
    blocksize_type  f_bsize;                                                   \
    blockcount_type f_bavail;                                                  \
  }
#define FS_STAT_SIZE_TYPE  int
#define FS_STAT_COUNT_TYPE int
#define FS_STAT_BLOCK_SIZE 4096

#endif

namespace torrent {
namespace utils {

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

} // namespace utils
} // namespace torrent

#endif
