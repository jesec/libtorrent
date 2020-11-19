// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2005-2011, Jari Sundell <jaris@ifi.uio.no>

#ifndef LIBTORRENT_PATH_H
#define LIBTORRENT_PATH_H

#include <string>
#include <torrent/common.h>
#include <vector>

namespace torrent {

// Use a blank first path to get root and "." to get current dir.

class LIBTORRENT_EXPORT Path : private std::vector<std::string> {
public:
  typedef std::vector<std::string> base_type;

  typedef base_type::value_type      value_type;
  typedef base_type::pointer         pointer;
  typedef base_type::const_pointer   const_pointer;
  typedef base_type::reference       reference;
  typedef base_type::const_reference const_reference;
  typedef base_type::size_type       size_type;
  typedef base_type::difference_type difference_type;
  typedef base_type::allocator_type  allocator_type;

  typedef base_type::iterator               iterator;
  typedef base_type::reverse_iterator       reverse_iterator;
  typedef base_type::const_iterator         const_iterator;
  typedef base_type::const_reverse_iterator const_reverse_iterator;

  using base_type::clear;
  using base_type::empty;
  using base_type::reserve;
  using base_type::size;

  using base_type::back;
  using base_type::begin;
  using base_type::end;
  using base_type::front;
  using base_type::rbegin;
  using base_type::rend;

  using base_type::push_back;

  using base_type::at;
  using base_type::operator[];

  void insert_path(iterator pos, const std::string& path);

  // Return the path as a string with '/' deliminator. The deliminator
  // is only inserted between path elements.
  std::string as_string() const;

  const std::string encoding() const {
    return m_encoding;
  }
  void set_encoding(const std::string& enc) {
    m_encoding = enc;
  }

  base_type* base() {
    return this;
  }
  const base_type* base() const {
    return this;
  }

private:
  std::string m_encoding;
};

} // namespace torrent

#endif
