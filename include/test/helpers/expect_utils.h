#ifndef LIBTORRENT_TEST_HELPERS_EXPECT_UTILS_H
#define LIBTORRENT_TEST_HELPERS_EXPECT_UTILS_H

#include "test/helpers/mock_function.h"
#include "torrent/utils/random.h"

inline void
expect_random_uniform_uint16(uint16_t result, uint16_t first, uint16_t last) {
  mock_expect(&torrent::random_uniform_uint16, result, first, last);
}

#endif
