// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2005-2011, Jari Sundell <jaris@ifi.uio.no>

#ifndef LIBTORRENT_DOWNLOAD_CHUNK_SELECTOR_H
#define LIBTORRENT_DOWNLOAD_CHUNK_SELECTOR_H

#include <cinttypes>

#include "torrent/bitfield.h"
#include "torrent/data/download_data.h"
#include "torrent/utils/partial_queue.h"
#include "torrent/utils/ranges.h"

namespace torrent {

// This class is responsible for deciding on which chunk index to
// download next based on the peer's bitfield. It keeps its own
// bitfield which starts out as a copy of Content::bitfield but sets
// chunks that are being downloaded.
//
// When updating Content::bitfield, make sure you update this bitfield
// and unmark any chunks in Delegator.

class ChunkStatistics;
class PeerChunks;

class ChunkSelector {
public:
  static constexpr uint32_t invalid_chunk = ~(uint32_t)0;

  ChunkSelector(download_data* data)
    : m_data(data) {}

  bool empty() const {
    return size() == 0;
  }
  uint32_t size() const {
    return m_data->untouched_bitfield()->size_bits();
  }

  //  const Bitfield*     bitfield()                    { return
  //  m_data->untouched_bitfield(); }

  // priority_ranges*    high_priority()               { return &m_highPriority;
  // } priority_ranges*    normal_priority()             { return
  // &m_normalPriority; }

  // Initialize doesn't update the priority cache, so it is as if it
  // has empty priority ranges.
  void initialize(ChunkStatistics* cs);
  void cleanup();

  // Sequential chunk selection
  bool is_sequential_enabled() {
    return m_sequential;
  };
  void set_sequential_enabled(bool enabled) {
    m_sequential = enabled;
  };

  // Call this once you've modified the bitfield or priorities to
  // update cached information. This must be called once before using
  // find.
  void update_priorities();

  uint32_t find(PeerChunks* pc, bool highPriority);

  bool is_wanted(uint32_t index) const;

  // Call this to set the index as being downloaded, finished etc,
  // thus ignored. Propably should find a better name for this.
  void using_index(uint32_t index);
  void not_using_index(uint32_t index);

  // The caller must ensure that the chunk index is valid and has not
  // been set already.
  //
  // The user only needs to call this when it needs to know whetever
  // it should become interested, or if it is in the process of
  // downloading.
  //
  // Returns whetever we're interested in that piece.
  bool received_have_chunk(PeerChunks* pc, uint32_t index);

private:
  bool        search_linear(const Bitfield*                       bf,
                            utils::partial_queue*                 pq,
                            const download_data::priority_ranges* ranges,
                            uint32_t                              first,
                            uint32_t                              last);
  inline bool search_linear_range(const Bitfield*       bf,
                                  utils::partial_queue* pq,
                                  uint32_t              first,
                                  uint32_t              last);
  inline bool search_linear_byte(utils::partial_queue* pq,
                                 uint32_t              index,
                                 Bitfield::value_type  wanted);

  //   inline uint32_t     search_rarest(const Bitfield* bf, priority_ranges*
  //   ranges, uint32_t first, uint32_t last); inline uint32_t
  //   search_rarest_range(const Bitfield* bf, uint32_t first, uint32_t last);
  //   inline uint32_t     search_rarest_byte(uint8_t wanted);

  void advance_position();

  download_data* m_data;

  ChunkStatistics* m_statistics;

  utils::partial_queue m_sharedQueue;

  uint32_t m_position;

  bool m_sequential = false;
};

} // namespace torrent

#endif
