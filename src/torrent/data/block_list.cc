// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2005-2011, Jari Sundell <jaris@ifi.uio.no>

#include <algorithm>
#include <functional>

#include "torrent/data/block_list.h"
#include "torrent/data/block_transfer.h"
#include "torrent/exceptions.h"

namespace torrent {

BlockList::BlockList(const Piece& piece, uint32_t blockLength) noexcept(false)
  : m_piece(piece)
  , m_priority(PRIORITY_OFF)
  , m_finished(0)
  ,

  m_failed(0)
  , m_attempt(0)
  ,

  m_bySeeder(false) {

  if (piece.length() == 0)
    throw internal_error(
      "BlockList::BlockList(...) received zero length piece.");

  // Look into optimizing this by using input iterators in the ctor.
  base_type::resize((m_piece.length() + blockLength - 1) / blockLength);

  // ATM assume offset of 0.
  //   uint32_t offset = m_piece.offset();
  uint32_t offset = 0;

  for (iterator itr = begin(), last = end() - 1; itr != last;
       ++itr, offset += blockLength) {
    itr->set_parent(this);
    itr->set_piece(Piece(m_piece.index(), offset, blockLength));
  }

  base_type::back().set_parent(this);
  base_type::back().set_piece(Piece(m_piece.index(),
                                    offset,
                                    (m_piece.length() % blockLength)
                                      ? m_piece.length() % blockLength
                                      : blockLength));
}

BlockList::~BlockList() {
  // The default dtor's handles cleaning up the blocks and block transfers.
}

void
BlockList::do_all_failed() {
  clear_finished();
  set_attempt(0);

  // Clear leaders when we want to redownload the chunk.
  std::for_each(begin(), end(), [](Block& p) { return p.failed_leader(); });
  std::for_each(begin(), end(), [](Block& p) { return p.retry_transfer(); });
}

} // namespace torrent
