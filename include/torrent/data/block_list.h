// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2005-2011, Jari Sundell <jaris@ifi.uio.no>

#ifndef LIBTORRENT_BLOCK_LIST_H
#define LIBTORRENT_BLOCK_LIST_H

#include <torrent/common.h>
#include <torrent/data/block.h>
#include <torrent/data/piece.h>
#include <vector>

namespace torrent {

// Temporary workaround until we can use C++11's std::vector::emblace_back.
template<typename Type>
class no_copy_vector {
public:
  using value_type      = Type;
  using size_type       = size_t;
  using reference       = value_type&;
  using difference_type = ptrdiff_t;

  using iterator       = value_type*;
  using const_iterator = const value_type*;

  no_copy_vector() = default;
  ~no_copy_vector() {
    clear();
  }

  size_type size() const {
    return m_size;
  }
  bool empty() const {
    return m_size == 0;
  }

  void resize(size_type s) {
    clear();
    m_size   = s;
    m_values = new value_type[s];
  }

  void clear() {
    delete[] m_values;
    m_values = nullptr;
    m_size   = 0;
  }

  iterator begin() {
    return m_values;
  }
  const_iterator begin() const {
    return m_values;
  }

  iterator end() {
    return m_values + m_size;
  }
  const_iterator end() const {
    return m_values + m_size;
  }

  value_type& back() {
    return *(m_values + m_size - 1);
  }

  value_type& operator[](size_type idx) {
    return m_values[idx];
  }

private:
  no_copy_vector(const no_copy_vector&) = delete;
  void operator=(const no_copy_vector&) = delete;

  size_type m_size{ 0 };
  Block*    m_values{ nullptr };
};

class LIBTORRENT_EXPORT BlockList : public no_copy_vector<Block> {
public:
  using base_type = no_copy_vector<Block>;
  using size_type = uint32_t;

  using base_type::difference_type;
  using base_type::reference;
  using base_type::value_type;

  using base_type::iterator;
  // using base_type::reverse_iterator;
  using base_type::empty;
  using base_type::size;

  using base_type::begin;
  using base_type::end;
  // using base_type::rbegin;
  // using base_type::rend;

  using base_type::operator[];

  BlockList(const Piece& piece, uint32_t blockLength);
  ~BlockList();

  bool is_all_finished() const {
    return m_finished == size();
  }

  const Piece& piece() const {
    return m_piece;
  }

  uint32_t index() const {
    return m_piece.index();
  }

  priority_t priority() const {
    return m_priority;
  }
  void set_priority(priority_t p) {
    m_priority = p;
  }

  size_type finished() const {
    return m_finished;
  }
  void inc_finished() {
    m_finished++;
  }
  void clear_finished() {
    m_finished = 0;
  }

  uint32_t failed() const {
    return m_failed;
  }

  // Temporary, just increment for now.
  void inc_failed() {
    m_failed++;
  }

  uint32_t attempt() const {
    return m_attempt;
  }
  void set_attempt(uint32_t a) {
    m_attempt = a;
  }

  // Set when the chunk was initially requested from a seeder. This
  // allows us to quickly determine if it is a suitable chunk to
  // request from another seeder, e.g by already knowing it is a rare
  // piece.
  bool by_seeder() const {
    return m_bySeeder;
  }
  void set_by_seeder(bool state) {
    m_bySeeder = state;
  }

  void do_all_failed();

private:
  BlockList(const BlockList&) = delete;
  void operator=(const BlockList&) = delete;

  Piece      m_piece;
  priority_t m_priority;

  size_type m_finished;
  uint32_t  m_failed;
  uint32_t  m_attempt;

  bool m_bySeeder;
};

} // namespace torrent

#endif
