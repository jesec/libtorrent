// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2005-2011, Jari Sundell <jaris@ifi.uio.no>

#ifndef LIBTORRENT_DOWNLOAD_CHUNK_STATISTICS_H
#define LIBTORRENT_DOWNLOAD_CHUNK_STATISTICS_H

#include <cinttypes>
#include <vector>

namespace torrent {

class PeerChunks;

class ChunkStatistics : public std::vector<uint8_t> {
public:
  typedef std::vector<uint8_t> base_type;
  typedef uint32_t             size_type;

  typedef base_type::value_type       value_type;
  typedef base_type::reference        reference;
  typedef base_type::const_reference  const_reference;
  typedef base_type::iterator         iterator;
  typedef base_type::const_iterator   const_iterator;
  typedef base_type::reverse_iterator reverse_iterator;

  using base_type::empty;
  using base_type::size;

  static const size_type max_accounted = 255;

  ChunkStatistics()
    : m_complete(0)
    , m_accounted(0) {}
  ~ChunkStatistics() {}

  size_type complete() const {
    return m_complete;
  }
  // size_type           incomplete() const;

  // Number of non-complete peers whom's bitfield is added to the
  // statistics.
  size_type accounted() const {
    return m_accounted;
  }

  void initialize(size_type s);
  void clear();

  // When a peer connects and sends a non-empty bitfield and is not a
  // seeder, we can be fairly sure it won't just disconnect
  // immediately. Thus it should be resonable to possibly spend the
  // effort adding it to the statistics if nessesary.

  // Where do we decide on policy? On whetever we count the chunks,
  // the type of connection shouldn't matter? As f.ex PCSeed will only
  // make sense when seeding, it won't be counted.

  // Might want to prefer to add peers we are interested in, but which
  // arn't in us.

  void received_connect(PeerChunks* pc);
  void received_disconnect(PeerChunks* pc);

  // The caller must ensure that the chunk index is valid and has not
  // been set already.
  void received_have_chunk(PeerChunks* pc, uint32_t index, uint32_t length);

  const_iterator begin() const {
    return base_type::begin();
  }
  const_iterator end() const {
    return base_type::end();
  }

  const_reference rarity(size_type n) const {
    return base_type::operator[](n);
  }

  const_reference operator[](size_type n) const {
    return base_type::operator[](n);
  }

private:
  inline bool should_add(PeerChunks* pc);

  ChunkStatistics(const ChunkStatistics&);
  void operator=(const ChunkStatistics&);

  size_type m_complete;
  size_type m_accounted;
};

} // namespace torrent

#endif
