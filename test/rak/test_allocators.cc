#include <cstdint>
#include <vector>

#include <gtest/gtest.h>

#include "torrent/utils/allocators.h"

class AllocatorsTest : public ::testing::Test {
public:
  typedef std::vector<char, torrent::utils::cacheline_allocator<char>>
    aligned_vector_type;
};

template<typename T>
bool
is_aligned(const T& t) {
  return t.empty() ||
         (reinterpret_cast<intptr_t>(&t[0]) & (LT_SMP_CACHE_BYTES - 1)) == 0x0;
}

TEST_F(AllocatorsTest, testAlignment) {
  aligned_vector_type v1;
  aligned_vector_type v2(1, 'a');
  aligned_vector_type v3(16, 'a');
  aligned_vector_type v4(LT_SMP_CACHE_BYTES, 'b');
  aligned_vector_type v5(1, 'a');

  ASSERT_TRUE(is_aligned(v1));
  ASSERT_TRUE(is_aligned(v2));
  ASSERT_TRUE(is_aligned(v3));
  ASSERT_TRUE(is_aligned(v4));
  ASSERT_TRUE(is_aligned(v5));
}
