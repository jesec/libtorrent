#include <iostream>
#include <unordered_map>
#include <unordered_set>

#include <gtest/gtest.h>

#include "torrent/hash_string.h"

class HashStringTest : public ::testing::Test {};

TEST_F(HashStringTest, test_basic) {
  torrent::HashString hash;

  std::unordered_map<torrent::HashString, int> m;
  std::unordered_set<torrent::HashString>      s;

  torrent::hash_string_from_hex_c_str(
    "A94A8FE5CCB19BA61C4C0873D391E987982FBBD3", hash);

  ASSERT_EQ(m.emplace(hash, 1).second, true);
  ASSERT_EQ(m.find(hash)->second, 1);
  ASSERT_EQ(m.emplace(hash, 1).second, false);

  ASSERT_EQ(s.insert(hash).second, true);
  ASSERT_NE(s.find(hash), s.end());
  ASSERT_EQ(s.insert(hash).second, false);

  torrent::hash_string_from_hex_c_str(
    "a94a8fe5ccb19ba61c4c0873d391e987982fbbd3", hash);

  ASSERT_EQ(m.find(hash)->second, 1);
  ASSERT_NE(s.find(hash), s.end());

  ASSERT_EQ(m.emplace(hash, 1).second, false);
  ASSERT_EQ(s.insert(hash).second, false);
}
