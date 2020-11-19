// libTorrent - BitTorrent library
// Copyright (C) 2005-2011, Jari Sundell
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
// In addition, as a special exception, the copyright holders give
// permission to link the code of portions of this program with the
// OpenSSL library under certain conditions as described in each
// individual source file, and distribute linked combinations
// including the two.
//
// You must obey the GNU General Public License in all respects for
// all of the code used other than OpenSSL.  If you modify file(s)
// with this exception, you may extend this exception to your version
// of the file(s), but you are not obligated to do so.  If you do not
// wish to do so, delete this exception statement from your version.
// If you delete this exception statement from all source files in the
// program, then also delete it here.
//
// Contact:  Jari Sundell <jaris@ifi.uio.no>
//
//           Skomakerveien 33
//           3185 Skoppum, NORWAY

#ifndef LIBTORRENT_DATA_CHUNK_HANDLE_H
#define LIBTORRENT_DATA_CHUNK_HANDLE_H

#include <rak/error_number.h>

#include "chunk_list_node.h"

namespace torrent {

class ChunkListNode;

class ChunkHandle {
public:
  ChunkHandle(ChunkListNode* c = NULL, bool wr = false, bool blk = false)
    : m_node(c)
    , m_writable(wr)
    , m_blocking(blk) {}

  bool is_valid() const {
    return m_node != NULL;
  }
  bool is_loaded() const {
    return m_node != NULL && m_node->is_valid();
  }
  bool is_writable() const {
    return m_writable;
  }
  bool is_blocking() const {
    return m_blocking;
  }

  void clear() {
    m_node     = NULL;
    m_writable = false;
    m_blocking = false;
  }

  rak::error_number error_number() const {
    return m_errorNumber;
  }
  void set_error_number(rak::error_number e) {
    m_errorNumber = e;
  }

  ChunkListNode* object() const {
    return m_node;
  }
  Chunk* chunk() const {
    return m_node->chunk();
  }

  uint32_t index() const {
    return m_node->index();
  }

  static ChunkHandle from_error(rak::error_number e) {
    ChunkHandle h;
    h.set_error_number(e);
    return h;
  }

private:
  ChunkListNode* m_node;
  bool           m_writable;
  bool           m_blocking;

  rak::error_number m_errorNumber;
};

} // namespace torrent

#endif
