#include <vector>

#include <gtest/gtest.h>

#include "torrent/utils/allocators.h"

class AllocatorsTest : public ::testing::Test {
public:
  typedef std::vector<char, torrent::utils::cacheline_allocator<char>>
    aligned_vector_type;
};
