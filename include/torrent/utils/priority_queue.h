// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2005-2007, Jari Sundell <jaris@ifi.uio.no>

// priority_queue is a priority queue implemented using a binary
// heap. It can contain multiple instances of a value.

#ifndef LIBTORRENT_UTILS_PRIORITY_QUEUE_H
#define LIBTORRENT_UTILS_PRIORITY_QUEUE_H

#include <algorithm>
#include <functional>
#include <vector>

namespace torrent {
namespace utils {

template<typename Value,
         typename Compare,
         typename Equal,
         typename Alloc = std::allocator<Value>>
class priority_queue : public std::vector<Value, Alloc> {
public:
  using base_type       = std::vector<Value, Alloc>;
  using reference       = typename base_type::reference;
  using const_reference = typename base_type::const_reference;
  using iterator        = typename base_type::iterator;
  using const_iterator  = typename base_type::const_iterator;
  using value_type      = typename base_type::value_type;

  using base_type::begin;
  using base_type::clear;
  using base_type::empty;
  using base_type::end;
  using base_type::size;

  priority_queue(Compare l = Compare(), Equal e = Equal())
    : m_compare(l)
    , m_equal(e) {}

  const_reference top() const {
    return base_type::front();
  }

  void pop() {
    std::pop_heap(begin(), end(), m_compare);
    base_type::pop_back();
  }

  void push(const value_type& value) {
    base_type::push_back(value);
    std::push_heap(begin(), end(), m_compare);
  }

  template<typename Key>
  iterator find(const Key& key) {
    return std::find_if(begin(), end(), [this, key](Key& curKey) {
      return m_equal(curKey, key);
    });
  }

  template<typename Key>
  bool erase(const Key& key) {
    iterator itr = find(key);

    if (itr == end())
      return false;

    erase(itr);
    return true;
  }

  // Removes 'itr' from the queue. This assumes 'itr' has been
  // modified such that it has a higher priority than any other
  // element in the queue.
  void erase(iterator itr) {
    //     std::push_heap(begin(), ++itr, m_compare);
    //     pop();
    base_type::erase(itr);
    std::make_heap(begin(), end(), m_compare);
  }

private:
  Compare m_compare;
  Equal   m_equal;
};

// Iterate while the top node has higher priority, as 'Compare'
// returns false.
template<typename Queue, typename Compare>
class queue_pop_iterator
  : public std::iterator<std::forward_iterator_tag, void, void, void, void> {
public:
  using container_type = Queue;

  queue_pop_iterator()
    : m_queue(nullptr) {}
  queue_pop_iterator(Queue* q, Compare c)
    : m_queue(q)
    , m_compare(c) {}

  queue_pop_iterator& operator++() {
    m_queue->pop();
    return *this;
  }
  queue_pop_iterator& operator++(int) {
    m_queue->pop();
    return *this;
  }

  typename container_type::const_reference operator*() {
    return m_queue->top();
  }

  bool operator!=(const queue_pop_iterator&) {
    return !m_queue->empty() && !m_compare(m_queue->top());
  }
  bool operator==(const queue_pop_iterator&) {
    return m_queue->empty() || m_compare(m_queue->top());
  }

private:
  Queue*  m_queue;
  Compare m_compare;
};

template<typename Queue, typename Compare>
inline queue_pop_iterator<Queue, Compare>
queue_popper(Queue& queue, Compare comp) {
  return queue_pop_iterator<Queue, Compare>(&queue, comp);
}

} // namespace utils
} // namespace torrent

#endif
