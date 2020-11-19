// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2005-2007, Jari Sundell <jaris@ifi.uio.no>

#ifndef RAK_ERROR_NUMBER_H
#define RAK_ERROR_NUMBER_H

#include <cerrno>
#include <cstring>

namespace rak {

class error_number {
public:
  static const int e_access      = EACCES;
  static const int e_again       = EAGAIN;
  static const int e_connreset   = ECONNRESET;
  static const int e_connaborted = ECONNABORTED;
  static const int e_deadlk      = EDEADLK;

  static const int e_noent  = ENOENT;
  static const int e_nodev  = ENODEV;
  static const int e_nomem  = ENOMEM;
  static const int e_notdir = ENOTDIR;
  static const int e_isdir  = EISDIR;

  static const int e_intr = EINTR;

  error_number()
    : m_errno(0) {}
  error_number(int e)
    : m_errno(e) {}

  bool is_valid() const {
    return m_errno != 0;
  }

  int value() const {
    return m_errno;
  }
  const char* c_str() const {
    return std::strerror(m_errno);
  }

  bool is_blocked_momentary() const {
    return m_errno == e_again || m_errno == e_intr;
  }
  bool is_blocked_prolonged() const {
    return m_errno == e_deadlk;
  }

  bool is_closed() const {
    return m_errno == e_connreset || m_errno == e_connaborted;
  }

  bool is_bad_path() const {
    return m_errno == e_noent || m_errno == e_notdir || m_errno == e_access;
  }

  static error_number current() {
    return errno;
  }
  static void clear_global() {
    errno = 0;
  }
  static void set_global(error_number err) {
    errno = err.m_errno;
  }

  bool operator==(const error_number& e) const {
    return m_errno == e.m_errno;
  }

private:
  int m_errno;
};

} // namespace rak

#endif
