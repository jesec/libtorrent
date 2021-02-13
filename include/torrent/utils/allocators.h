// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2005-2007, Jari Sundell <jaris@ifi.uio.no>

// Some allocators for cacheline aligned chunks of memory, etc.

#ifndef LIBTORRENT_UTILS_ALLOCATORS_H
#define LIBTORRENT_UTILS_ALLOCATORS_H

#include <cstddef>
#include <limits>
#include <stdlib.h>
#include <sys/types.h>

#include <torrent/utils/cacheline.h>

namespace torrent {
namespace utils {

template<class T = void*>
class cacheline_allocator {
public:
  typedef size_t      size_type;
  typedef ptrdiff_t   difference_type;
  typedef T*          pointer;
  typedef const T*    const_pointer;
  typedef const void* const_void_pointer;
  typedef T&          reference;
  typedef const T&    const_reference;
  typedef T           value_type;

  cacheline_allocator() noexcept {}
  cacheline_allocator(const cacheline_allocator&) noexcept {}
  template<class U>
  cacheline_allocator(const cacheline_allocator<U>&) noexcept {}
  ~cacheline_allocator() noexcept {}

  template<class U>
  struct rebind {
    typedef cacheline_allocator<U> other;
  };

  // return address of values
  pointer address(reference value) const {
    return &value;
  }
  const_pointer address(const_reference value) const {
    return &value;
  }

  size_type max_size() const noexcept {
    return std::numeric_limits<size_t>::max() / sizeof(T);
  }

  pointer allocate(size_type num, const_void_pointer = 0) {
    return alloc_size(num * sizeof(T));
  }

  static pointer alloc_size(size_type size) {
    pointer ptr = NULL;

    int result [[maybe_unused]] =
      posix_memalign((void**)&ptr, LT_SMP_CACHE_BYTES, size);

    return ptr;
  }

  void construct(pointer p, const T& value) {
    new ((void*)p) T(value);
  }
  void destroy(pointer p) {
    p->~T();
  }
  void deallocate(pointer p, size_type) {
    free((void*)p);
  }
};

template<class T1, class T2>
bool
operator==(const cacheline_allocator<T1>&,
           const cacheline_allocator<T2>&) noexcept {
  return true;
}

template<class T1, class T2>
bool
operator!=(const cacheline_allocator<T1>&,
           const cacheline_allocator<T2>&) noexcept {
  return false;
}

} // namespace utils
} // namespace torrent

//
// Operator new with custom allocators:
//

template<typename T>
void*
operator new(size_t s, torrent::utils::cacheline_allocator<T> a) {
  return a.alloc_size(s);
}

#endif
