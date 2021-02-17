// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2005-2007, Jari Sundell <jaris@ifi.uio.no>

#ifndef LIBTORRENT_UTILS_ERROR_NUMBER_H
#define LIBTORRENT_UTILS_ERROR_NUMBER_H

#include <system_error>

namespace torrent {
namespace utils {

class error_number {
public:
  error_number() = default;
  error_number(std::errc e)
    : m_errno(e) {}

  bool is_valid() const {
    return static_cast<int>(m_errno) != 0;
  }

  std::errc value() const {
    return m_errno;
  }

  std::string message() const {
    return std::make_error_code(m_errno).message();
  }

  bool is_blocked_momentary() const {
    return m_errno == std::errc::resource_unavailable_try_again ||
           m_errno == std::errc::interrupted;
  }
  bool is_blocked_prolonged() const {
    return m_errno == std::errc::resource_deadlock_would_occur;
  }

  bool is_closed() const {
    return m_errno == std::errc::connection_reset ||
           m_errno == std::errc::connection_aborted;
  }

  bool is_bad_path() const {
    return m_errno == std::errc::no_such_file_or_directory ||
           m_errno == std::errc::not_a_directory ||
           m_errno == std::errc::permission_denied;
  }

  static error_number current() {
    return static_cast<std::errc>(errno);
  }
  static void clear_global() {
    errno = 0;
  }
  static void set_global(error_number err) {
    errno = static_cast<int>(err.m_errno);
  }

  bool operator==(const error_number& e) const {
    return m_errno == e.m_errno;
  }

private:
  std::errc m_errno{ 0 };
};

} // namespace utils
} // namespace torrent

#endif
