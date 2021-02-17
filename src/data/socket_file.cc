// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2005-2011, Jari Sundell <jaris@ifi.uio.no>

#include "torrent/buildinfo.h"

#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>

#ifdef LT_HAVE_FALLOCATE
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <linux/falloc.h>
#endif

#include "data/socket_file.h"
#include "torrent/exceptions.h"
#include "torrent/utils/error_number.h"
#include "torrent/utils/file_stat.h"

namespace torrent {

bool
SocketFile::open(const std::string& path, int prot, int flags, mode_t mode) {
  close();

  if (prot & MemoryChunk::prot_read && prot & MemoryChunk::prot_write)
    flags |= O_RDWR;
  else if (prot & MemoryChunk::prot_read)
    flags |= O_RDONLY;
  else if (prot & MemoryChunk::prot_write)
    flags |= O_WRONLY;
  else
    throw internal_error("torrent::SocketFile::open(...) Tried to open file "
                         "with no protection flags");

#ifdef O_LARGEFILE
  fd_type fd = ::open(path.c_str(), flags | O_LARGEFILE, mode);
#else
  fd_type fd = ::open(path.c_str(), flags, mode);
#endif

  if (fd == invalid_fd)
    return false;

  m_fd = fd;
  return true;
}

void
SocketFile::close() {
  if (!is_open())
    return;

  ::close(m_fd);

  m_fd = invalid_fd;
}

uint64_t
SocketFile::size() const {
  if (!is_open())
    throw internal_error("SocketFile::size() called on a closed file");

  utils::file_stat fs;

  return fs.update(m_fd) ? fs.size() : 0;
}

bool
SocketFile::set_size(uint64_t size, int flags) const {
  if (!is_open())
    throw internal_error("SocketFile::set_size() called on a closed file");

#if defined(LT_HAVE_FALLOCATE)
  if (flags & flag_fallocate && fallocate(m_fd, 0, 0, size) == 0)
    return true;
#elif defined(LT_USE_POSIX_FALLOCATE)
  if (flags & flag_fallocate && flags & flag_fallocate_blocking &&
      posix_fallocate(m_fd, 0, size) == 0)
    return true;
#elif defined(__APPLE__)
  if (flags & flag_fallocate) {
    fstore_t fstore;
    fstore.fst_flags      = F_ALLOCATECONTIG;
    fstore.fst_posmode    = F_PEOFPOSMODE;
    fstore.fst_offset     = 0;
    fstore.fst_length     = size;
    fstore.fst_bytesalloc = 0;

    // Hmm... this shouldn't really be something we fail the set_size
    // on...
    //
    // Yet is somehow fails with ENOSPC...
    //     if (fcntl(m_fd, F_PREALLOCATE, &fstore) == -1)
    //       throw internal_error("hack: fcntl failed" +
    //       std::string(strerror(errno)));

    fcntl(m_fd, F_PREALLOCATE, &fstore); // Ignore result for now...
  }
#endif

  if (ftruncate(m_fd, size) == 0)
    return true;

  // Use workaround to resize files on vfat. It causes the whole
  // client to block while it is resizing the files, this really
  // should be in a seperate thread.
  if (size != 0 && lseek(m_fd, size - 1, SEEK_SET) == (off_t)(size - 1) &&
      write(m_fd, &size, 1) == 1)
    return true;

  return false;
}

MemoryChunk
SocketFile::create_chunk(uint64_t offset,
                         uint32_t length,
                         int      prot,
                         int      flags) const {
  if (!is_open())
    throw internal_error("SocketFile::get_chunk() called on a closed file");

  // For some reason mapping beyond the extent of the file does not
  // cause mmap to complain, so we need to check manually here.
  if (length == 0 || offset > size() || offset + length > size())
    return MemoryChunk();

  uint64_t align = offset % MemoryChunk::page_size();

  char* ptr =
    (char*)mmap(nullptr, length + align, prot, flags, m_fd, offset - align);

  if (ptr == MAP_FAILED)
    return MemoryChunk();

  return MemoryChunk(ptr, ptr + align, ptr + align + length, prot, flags);
}

} // namespace torrent
