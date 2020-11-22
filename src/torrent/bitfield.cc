// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2005-2011, Jari Sundell <jaris@ifi.uio.no>

#include <algorithm>

#include "torrent/bitfield.h"
#include "torrent/exceptions.h"
#include "torrent/utils/algorithm.h"
#include "utils/instrumentation.h"

namespace torrent {

void
Bitfield::set_size_bits(size_type s) noexcept(false) {
  if (m_data != NULL)
    throw internal_error(
      "Bitfield::set_size_bits(size_type s) m_data != NULL.");

  m_size = s;
}

void
Bitfield::set_size_set(size_type s) noexcept(false) {
  if (s > m_size || m_data != NULL)
    throw internal_error("Bitfield::set_size_set(size_type s) s > m_size.");

  m_set = s;
}

void
Bitfield::allocate() {
  if (m_data != NULL)
    return;

  m_data = new value_type[size_bytes()];

  instrumentation_update(INSTRUMENTATION_MEMORY_BITFIELDS,
                         (int64_t)size_bytes());
}

void
Bitfield::unallocate() {
  if (m_data == NULL)
    return;

  delete[] m_data;
  m_data = NULL;

  instrumentation_update(INSTRUMENTATION_MEMORY_BITFIELDS,
                         -(int64_t)size_bytes());
}

void
Bitfield::update() {
  // Clears the unused bits.
  clear_tail();

  m_set = 0;

  iterator itr  = m_data;
  iterator last = end();

  while (itr + sizeof(unsigned int) <= last) {
    m_set += utils::popcount_wrapper(*reinterpret_cast<unsigned int*>(itr));
    itr += sizeof(unsigned int);
  }

  while (itr != last) {
    m_set += utils::popcount_wrapper(*itr++);
  }
}

void
Bitfield::copy(const Bitfield& bf) {
  unallocate();

  m_size = bf.m_size;
  m_set  = bf.m_set;

  if (bf.m_data == NULL) {
    m_data = NULL;
  } else {
    allocate();
    std::memcpy(m_data, bf.m_data, size_bytes());
  }
}

void
Bitfield::swap(Bitfield& bf) {
  std::swap(m_size, bf.m_size);
  std::swap(m_set, bf.m_set);
  std::swap(m_data, bf.m_data);
}

void
Bitfield::set_all() {
  m_set = m_size;

  std::memset(m_data, ~value_type(), size_bytes());
  clear_tail();
}

void
Bitfield::unset_all() {
  m_set = 0;

  std::memset(m_data, value_type(), size_bytes());
}

// Quick hack. Speed improvements would require that m_set is kept
// up-to-date.
void
Bitfield::set_range(size_type first, size_type last) {
  while (first != last)
    set(first++);
}

void
Bitfield::unset_range(size_type first, size_type last) {
  while (first != last)
    unset(first++);
}

// size_type
// Bitfield::count_range(size_type first, size_type last) {
//   size_type count = 0;

//   // Some archs have bitcounting instructions, look into writing a
//   // wrapper for those.
//   for (iterator itr = m_data, last = end(); itr != last; ++itr)
//     m_set += bit_count_256[*itr];
// }

} // namespace torrent
