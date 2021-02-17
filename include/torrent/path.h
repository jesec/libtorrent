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
  using base_type = std::vector<std::string>;

  using value_type      = base_type::value_type;
  using pointer         = base_type::pointer;
  using const_pointer   = base_type::const_pointer;
  using reference       = base_type::reference;
  using const_reference = base_type::const_reference;
  using size_type       = base_type::size_type;
  using difference_type = base_type::difference_type;
  using allocator_type  = base_type::allocator_type;

  using iterator               = base_type::iterator;
  using reverse_iterator       = base_type::reverse_iterator;
  using const_iterator         = base_type::const_iterator;
  using const_reverse_iterator = base_type::const_reverse_iterator;

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
