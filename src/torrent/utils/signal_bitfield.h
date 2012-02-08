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

#ifndef LIBTORRENT_UTILS_SIGNAL_BITFIELD_H
#define LIBTORRENT_UTILS_SIGNAL_BITFIELD_H

#include <tr1/functional>
#include <torrent/common.h>

namespace torrent {

class LIBTORRENT_EXPORT signal_bitfield {
public:
  typedef uint32_t                    bitfield_type;
  typedef std::tr1::function<void ()> slot_type;
  
  static const unsigned int max_size = 32;

  signal_bitfield() : m_bitfield(0), m_size(0) {}
  
  void          signal(unsigned int index, bool interrupt = true);
  
  unsigned int  add_signal(slot_type slot);

private:
  bitfield_type m_bitfield lt_cacheline_aligned;
  unsigned int  m_size     lt_cacheline_aligned;
  slot_type     m_slots[max_size];
};

}

#endif
