// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2005-2007, Jari Sundell <jaris@ifi.uio.no>

// Some allocators for cacheline aligned chunks of memory, etc.

#ifndef RAK_ALLOCATORS_H
#define RAK_ALLOCATORS_H

#include <cstddef>
#include <limits>
#include <stdlib.h>
#include <sys/types.h>

namespace rak {

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

  cacheline_allocator() throw() {}
  cacheline_allocator(const cacheline_allocator&) throw() {}
  template<class U>
  cacheline_allocator(const cacheline_allocator<U>&) throw() {}
  ~cacheline_allocator() throw() {}

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

  size_type max_size() const throw() {
    return std::numeric_limits<size_t>::max() / sizeof(T);
  }

  pointer allocate(size_type num, const_void_pointer = 0) {
    return alloc_size(num * sizeof(T));
  }

  static pointer alloc_size(size_type size) {
    pointer      ptr = NULL;
    int __UNUSED result =
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
           const cacheline_allocator<T2>&) throw() {
  return true;
}

template<class T1, class T2>
bool
operator!=(const cacheline_allocator<T1>&,
           const cacheline_allocator<T2>&) throw() {
  return false;
}

} // namespace rak

//
// Operator new with custom allocators:
//

template<typename T>
void*
operator new(size_t s, rak::cacheline_allocator<T> a) {
  return a.alloc_size(s);
}

#endif // namespace rak
