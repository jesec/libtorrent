// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2005-2011, Jari Sundell <jaris@ifi.uio.no>

// Note that some of the exception classes below are strictly speaking
// _NOT DOING THE RIGHT THING_. One is really not supposed to use any
// calls to malloc/new in an exception's ctor.
//
// Will be fixed next API update.

#ifndef LIBTORRENT_EXCEPTIONS_H
#define LIBTORRENT_EXCEPTIONS_H

#include <exception>
#include <string>
#include <torrent/common.h>
#include <torrent/hash_string.h>
#include <torrent/utils/error_number.h>

namespace torrent {

// All exceptions inherit from std::exception to make catching
// everything libtorrent related at the root easier.
class LIBTORRENT_EXPORT base_error : public std::exception {
public:
  ~base_error() noexcept override = default;
};

// The library or application did some borking it shouldn't have, bug
// tracking time!
class LIBTORRENT_EXPORT internal_error : public base_error {
public:
  internal_error(const char* msg) {
    initialize(msg);
  }
  internal_error(const char* msg, const std::string& context) {
    initialize(std::string(msg) + " [" + context + "]");
  }
  internal_error(const char* msg, const HashString& hash) {
    initialize(std::string(msg) + " [#" + hash_string_to_hex_str(hash) + "]");
  }
  internal_error(const std::string& msg) {
    initialize(msg);
  }
  ~internal_error() noexcept override = default;

  const char* what() const noexcept override {
    return m_msg.c_str();
  }
  const std::string& backtrace() const noexcept {
    return m_backtrace;
  }

protected:
  internal_error() = default;
  // Use this function for breaking on throws.
  void initialize(const std::string& msg, const bool print = false);

private:
  std::string m_msg;
  std::string m_backtrace;
};

class LIBTORRENT_EXPORT destruct_error : public internal_error {
public:
  destruct_error(const char* msg) {
    initialize(msg, true);
  }
  destruct_error(const std::string& msg) {
    initialize(msg, true);
  }
};

// For some reason we couldn't talk with a protocol/tracker, might be a
// library bug, connection problem or bad input.
class LIBTORRENT_EXPORT network_error : public base_error {
public:
  ~network_error() noexcept override = default;
};

class LIBTORRENT_EXPORT communication_error : public network_error {
public:
  communication_error(const char* msg) {
    initialize(msg);
  }
  communication_error(const std::string& msg) {
    initialize(msg);
  }
  ~communication_error() noexcept override = default;

  const char* what() const noexcept override {
    return m_msg.c_str();
  }

private:
  // Use this function for breaking on throws.
  void initialize(const std::string& msg);

  std::string m_msg;
};

class LIBTORRENT_EXPORT connection_error : public network_error {
public:
  connection_error(std::errc err)
    : m_errno(err) {
    m_msg = std::string(utils::error_number(m_errno).message());
  }
  ~connection_error() noexcept override = default;

  const char* what() const noexcept override {
    return m_msg.c_str();
  };

  std::errc get_errno() const {
    return m_errno;
  }

private:
  std::errc   m_errno;
  std::string m_msg;
};

class LIBTORRENT_EXPORT address_info_error : public network_error {
public:
  address_info_error(std::errc err)
    : m_errno(err) {}
  ~address_info_error() noexcept override = default;

  const char* what() const noexcept override;

  std::errc get_errno() const {
    return m_errno;
  }

private:
  std::errc m_errno;
};

class LIBTORRENT_EXPORT close_connection : public network_error {
public:
  ~close_connection() noexcept override = default;
};

class LIBTORRENT_EXPORT blocked_connection : public network_error {
public:
  ~blocked_connection() noexcept override = default;
};

// Stuff like bad torrent file, disk space and permissions.
class LIBTORRENT_EXPORT local_error : public base_error {
public:
  ~local_error() noexcept override = default;
};

class LIBTORRENT_EXPORT storage_error : public local_error {
public:
  storage_error(const char* msg) {
    initialize(msg);
  }
  storage_error(const std::string& msg) {
    initialize(msg);
  }
  ~storage_error() noexcept override = default;

  const char* what() const noexcept override {
    return m_msg.c_str();
  }

private:
  // Use this function for breaking on throws.
  void initialize(const std::string& msg);

  std::string m_msg;
};

class LIBTORRENT_EXPORT resource_error : public local_error {
public:
  resource_error(const char* msg) {
    initialize(msg);
  }
  resource_error(const std::string& msg) {
    initialize(msg);
  }
  ~resource_error() noexcept override = default;

  const char* what() const noexcept override {
    return m_msg.c_str();
  }

private:
  // Use this function for breaking on throws.
  void initialize(const std::string& msg);

  std::string m_msg;
};

class LIBTORRENT_EXPORT input_error : public local_error {
public:
  input_error(const char* msg) {
    initialize(msg);
  }
  input_error(const std::string& msg) {
    initialize(msg);
  }
  ~input_error() noexcept override = default;

  const char* what() const noexcept override {
    return m_msg.c_str();
  }

private:
  // Use this function for breaking on throws.
  void initialize(const std::string& msg);

  std::string m_msg;
};

class LIBTORRENT_EXPORT bencode_error : public input_error {
public:
  bencode_error(const char* msg)
    : input_error(msg) {}
  bencode_error(const std::string& msg)
    : input_error(msg) {}

  ~bencode_error() noexcept override = default;
};

class LIBTORRENT_EXPORT shutdown_exception : public base_error {
public:
  ~shutdown_exception() noexcept override = default;
};

} // namespace torrent

#endif
