// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2021, Contributors to the libTorrent project

#pragma once

#include <cstddef>
#include <new>

#if defined(__cpp_lib_hardware_interference_size) && !defined(_LIBCPP_VERSION)

constexpr std::size_t LT_SMP_CACHE_BYTES =
  std::hardware_constructive_interference_size;

#else

#if defined(__x86_64__) || defined(__i386__)
constexpr std::size_t LT_SMP_CACHE_BYTES = 64;
#else
constexpr std::size_t LT_SMP_CACHE_BYTES = 128;
#endif

#endif

static_assert(LT_SMP_CACHE_BYTES >= alignof(std::max_align_t));

#define lt_cacheline_aligned __attribute__((__aligned__(LT_SMP_CACHE_BYTES)))
