// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2005-2011, Jari Sundell <jaris@ifi.uio.no>

#ifndef LIBTORRENT_FILE_H
#define LIBTORRENT_FILE_H

#include <torrent/common.h>
#include <torrent/path.h>
#include <torrent/utils/cacheline.h>

namespace torrent {

class LIBTORRENT_EXPORT lt_cacheline_aligned File {
public:
  friend class FileList;

  typedef std::pair<uint32_t, uint32_t> range_type;

  static const int flag_active             = (1 << 0);
  static const int flag_create_queued      = (1 << 1);
  static const int flag_resize_queued      = (1 << 2);
  static const int flag_fallocate          = (1 << 3);
  static const int flag_previously_created = (1 << 4);

  static const int flag_prioritize_first = (1 << 5);
  static const int flag_prioritize_last  = (1 << 6);

  File();
  ~File();

  bool is_created() const;
  bool is_open() const {
    return m_fd != -1;
  }

  bool is_correct_size() const;
  bool is_valid_position(uint64_t p) const;

  bool is_create_queued() const {
    return m_flags & flag_create_queued;
  }
  bool is_resize_queued() const {
    return m_flags & flag_resize_queued;
  }
  bool is_previously_created() const {
    return m_flags & flag_previously_created;
  }

  bool has_flags(int flags) {
    return m_flags & flags;
  }

  void set_flags(int flags);
  void unset_flags(int flags);

  bool has_permissions(int prot) const {
    return !(prot & ~m_protection);
  }

  uint64_t offset() const {
    return m_offset;
  }

  uint64_t size_bytes() const {
    return m_size;
  }
  uint32_t size_chunks() const {
    return m_range.second - m_range.first;
  }

  uint32_t completed_chunks() const {
    return m_completed;
  }
  void set_completed_chunks(uint32_t v);

  const range_type& range() const {
    return m_range;
  }
  uint32_t range_first() const {
    return m_range.first;
  }
  uint32_t range_second() const {
    return m_range.second;
  }

  priority_t priority() const {
    return m_priority;
  }
  void set_priority(priority_t t) {
    m_priority = t;
  }

  const Path* path() const {
    return &m_path;
  }
  Path* mutable_path() {
    return &m_path;
  }

  const std::string& frozen_path() const {
    return m_frozenPath;
  }

  uint32_t match_depth_prev() const {
    return m_matchDepthPrev;
  }
  uint32_t match_depth_next() const {
    return m_matchDepthNext;
  }

  // This should only be changed by libtorrent.
  int file_descriptor() const {
    return m_fd;
  }
  void set_file_descriptor(int fd) {
    m_fd = fd;
  }

  // This might actually be wanted, as it would be nice to allow the
  // File to decide if it needs to try creating the underlying file or
  // not.
  bool prepare(int prot, int flags = 0);

  int protection() const {
    return m_protection;
  }
  void set_protection(int prot) {
    m_protection = prot;
  }

  uint64_t last_touched() const {
    return m_lastTouched;
  }
  void set_last_touched(uint64_t t) {
    m_lastTouched = t;
  }

protected:
  void set_flags_protected(int flags) {
    m_flags |= flags;
  }
  void unset_flags_protected(int flags) {
    m_flags &= ~flags;
  }

  void set_frozen_path(const std::string& path) {
    m_frozenPath = path;
  }

  void set_offset(uint64_t off) {
    m_offset = off;
  }
  void set_size_bytes(uint64_t size) {
    m_size = size;
  }
  void set_range(uint32_t chunkSize);

  void set_completed_protected(uint32_t v) {
    m_completed = v;
  }
  void inc_completed_protected() {
    m_completed++;
  }

  static void set_match_depth(File* left, File* right);

  void set_match_depth_prev(uint32_t l) {
    m_matchDepthPrev = l;
  }
  void set_match_depth_next(uint32_t l) {
    m_matchDepthNext = l;
  }

private:
  File(const File&);
  void operator=(const File&);

  bool resize_file();

  int m_fd;
  int m_protection;
  int m_flags;

  Path        m_path;
  std::string m_frozenPath;

  uint64_t m_offset;
  uint64_t m_size;
  uint64_t m_lastTouched;

  range_type m_range;

  uint32_t   m_completed;
  priority_t m_priority;

  uint32_t m_matchDepthPrev;
  uint32_t m_matchDepthNext;
};

inline bool
File::is_valid_position(uint64_t p) const {
  return p >= m_offset && p < m_offset + m_size;
}

inline void
File::set_flags(int flags) {
  set_flags_protected(flags & (flag_create_queued | flag_resize_queued |
                               flag_fallocate | flag_prioritize_first |
                               flag_prioritize_last));
}

inline void
File::unset_flags(int flags) {
  unset_flags_protected(flags & (flag_create_queued | flag_resize_queued |
                                 flag_fallocate | flag_prioritize_first |
                                 flag_prioritize_last));
}

inline void
File::set_completed_chunks(uint32_t v) {
  if (!has_flags(flag_active) && v <= size_chunks())
    m_completed = v;
}

} // namespace torrent

#endif
