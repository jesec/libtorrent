// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2005-2011, Jari Sundell <jaris@ifi.uio.no>

#include "torrent/buildinfo.h"

#include "dht/dht_bucket.h"
#include "dht/dht_node.h"
#include "torrent/exceptions.h"
#include "torrent/utils/random.h"

namespace torrent {

DhtBucket::DhtBucket(const HashString& begin, const HashString& end)
  : m_parent(nullptr)
  , m_child(nullptr)
  ,

  m_lastChanged(cachedTime.seconds())
  ,

  m_good(0)
  , m_bad(0)
  ,

  m_fullCacheLength(0)
  ,

  m_begin(begin)
  , m_end(end) {

  reserve(num_nodes);
}

void
DhtBucket::add_node(DhtNode* n) {
  push_back(n);
  touch();

  if (n->is_good())
    m_good++;
  else if (n->is_bad())
    m_bad++;

  m_fullCacheLength = 0;
}

void
DhtBucket::remove_node(DhtNode* n) {
  auto itr = std::find_if(
    begin(), end(), [n](DhtNode* curNode) { return curNode == n; });

  if (itr == end())
    throw internal_error(
      "DhtBucket::remove_node called for node not in bucket.");

  erase(itr);

  if (n->is_good())
    m_good--;
  else if (n->is_bad())
    m_bad--;

  m_fullCacheLength = 0;
}

void
DhtBucket::count() {
  m_good = std::count_if(begin(), end(), std::mem_fn(&DhtNode::is_good));
  m_bad  = std::count_if(begin(), end(), std::mem_fn(&DhtNode::is_bad));
}

// Called every 15 minutes for housekeeping.
void
DhtBucket::update() {
  count();

  // In case adjacent buckets whose nodes we borrowed have changed,
  // we force an update of the cache.
  m_fullCacheLength = 0;
}

DhtBucket::iterator
DhtBucket::find_replacement_candidate(bool onlyOldest) {
  auto         oldest     = end();
  unsigned int oldestTime = std::numeric_limits<unsigned int>::max();

  for (auto itr = begin(); itr != end(); ++itr) {
    if ((*itr)->is_bad() && !onlyOldest)
      return itr;

    if ((*itr)->last_seen() < oldestTime) {
      oldestTime = (*itr)->last_seen();
      oldest     = itr;
    }
  }

  return oldest;
}

void
DhtBucket::get_mid_point(HashString* middle) const {
  *middle = m_end;

  for (unsigned int i = 0; i < m_begin.size(); i++)
    if (m_begin[i] != m_end[i]) {
      (*middle)[i] = ((uint8_t)m_begin[i] + (uint8_t)m_end[i]) / 2;
      break;
    }
}

void
DhtBucket::get_random_id(HashString* rand_id) const {
  // Generate a random ID between m_begin and m_end.
  uint64_t begin64 = 0;
  uint64_t end64   = 0;

  // 0-7
  std::memcpy(&begin64, m_begin.data(), 8);
  std::memcpy(&end64, m_end.data(), 8);

  if (begin64 == end64) {
    std::memcpy(rand_id->data(), &begin64, 8);
  } else {
    auto n = random_uniform_uint64(begin64 + 1, end64 - 1);
    std::memcpy(rand_id->data(), &n, 8);
    return;
  }

  // 8-15
  std::memcpy(&begin64, m_begin.data() + 8, 8);
  std::memcpy(&end64, m_end.data() + 8, 8);

  if (begin64 == end64) {
    std::memcpy(rand_id->data() + 8, &begin64, 8);
  } else {
    auto n = random_uniform_uint64(begin64 + 1, end64 - 1);
    std::memcpy(rand_id->data() + 8, &n, 8);
    return;
  }

  uint32_t begin32 = 0;
  uint32_t end32   = 0;

  // 16-19
  std::memcpy(&begin32, m_begin.data() + 16, 4);
  std::memcpy(&end32, m_end.data() + 16, 4);

  if (begin32 == end32) {
    throw internal_error("DhtBucket::get_random_id can't generate.");
  } else {
    auto n = random_uniform_uint32(begin32 + 1, end32 - 1);
    std::memcpy(rand_id->data() + 16, &n, 4);
    return;
  }

#ifdef LT_USE_EXTRA_DEBUG
  if (!is_in_range(*rand_id))
    throw internal_error(
      "DhtBucket::get_random_id generated an out-of-range ID.");
#endif
}

DhtBucket*
DhtBucket::split(const HashString& id) {
  HashString mid_range;
  get_mid_point(&mid_range);

  auto other = new DhtBucket(m_begin, mid_range);

  // Set m_begin = mid_range + 1
  int carry = 1;
  for (unsigned int i = mid_range.size(); i > 0; i--) {
    unsigned int sum = (uint8_t)mid_range[i - 1] + carry;
    m_begin[i - 1]   = (uint8_t)sum;
    carry            = sum >> 8;
  }

  // Move nodes over to other bucket if they fall in its range, then
  // delete them from this one.
  auto split = std::partition(
    begin(), end(), [this](DhtNode* n) { return n->is_in_range(this); });

  other->insert(other->end(), split, end());

  for (const auto& node : *other) {
    node->set_bucket(other);
  }

  erase(split, end());

  other->set_time(m_lastChanged);
  other->count();

  count();

  // Maintain child (adjacent narrower bucket) and parent (adjacent wider
  // bucket) so that given router ID is in child.
  if (other->is_in_range(id)) {
    // Make other become our new child.
    m_child         = other;
    other->m_parent = this;

  } else {
    // We become other's child, other becomes our parent's child.
    if (parent() != nullptr) {
      parent()->m_child = other;
      other->m_parent   = parent();
    }

    m_parent       = other;
    other->m_child = this;
  }

  return other;
}

void
DhtBucket::build_full_cache() {
  DhtBucketChain chain(this);

  char* pos = m_fullCache;

  do {
    for (const auto& node : *this) {
      if (pos >= m_fullCache + sizeof(m_fullCache)) {
        break;
      }

      if (!node->is_bad()) {
        pos = node->store_compact(pos);

        if (pos > m_fullCache + sizeof(m_fullCache)) {
          throw internal_error(
            "DhtRouter::store_closest_nodes wrote past buffer end.");
        }
      }
    }
  } while (pos < m_fullCache + sizeof(m_fullCache) && chain.next() != nullptr);

  m_fullCacheLength = pos - m_fullCache;
}

} // namespace torrent
