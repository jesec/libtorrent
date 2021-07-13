#include <iostream>

#include <gtest/gtest.h>

#include "torrent/utils/ranges.h"

class RangesTest : public ::testing::Test {};

template<typename Type>
bool
verify_ranges(const torrent::ranges<Type>& ranges) {
  typename torrent::ranges<Type>::const_iterator first = ranges.begin();
  typename torrent::ranges<Type>::const_iterator last  = ranges.begin();

  if (first == last)
    return true;

  if (first->second <= first->first)
    return false;

  Type boundary = first++->second;

  do {
    if (first->first <= boundary)
      return false;

    if (first->second <= first->first)
      return false;

  } while (++first != last);

  return true;
}

TEST_F(RangesTest, test_basic) {}

TEST_F(RangesTest, test_intersect) {
  torrent::ranges<int> range;

  ASSERT_TRUE(verify_ranges(range));

  ASSERT_EQ(range.intersect_distance(0, 0), 0);
  ASSERT_EQ(range.intersect_distance(0, 10), 0);

  range.insert(0, 5);

  ASSERT_TRUE(verify_ranges(range));

  ASSERT_EQ(range.intersect_distance(0, 5), 5);
  ASSERT_EQ(range.intersect_distance(0, 10), 5);
  ASSERT_EQ(range.intersect_distance(-5, 5), 5);
  ASSERT_EQ(range.intersect_distance(-5, 10), 5);

  ASSERT_EQ(range.intersect_distance(2, 2), 0);
  ASSERT_EQ(range.intersect_distance(1, 4), 3);
  ASSERT_EQ(range.intersect_distance(1, 10), 4);
  ASSERT_EQ(range.intersect_distance(-5, 4), 4);

  range.insert(10, 15);

  ASSERT_TRUE(verify_ranges(range));

  ASSERT_EQ(range.intersect_distance(0, 15), 10);
  ASSERT_EQ(range.intersect_distance(0, 20), 10);
  ASSERT_EQ(range.intersect_distance(-5, 15), 10);
  ASSERT_EQ(range.intersect_distance(-5, 20), 10);

  ASSERT_EQ(range.intersect_distance(2, 12), 5);
  ASSERT_EQ(range.intersect_distance(1, 14), 8);
  ASSERT_EQ(range.intersect_distance(1, 20), 9);
  ASSERT_EQ(range.intersect_distance(-5, 14), 9);
}

TEST_F(RangesTest, test_create_union) {
  torrent::ranges<int> range_1l;
  torrent::ranges<int> range_1r;

  torrent::ranges<int> range_1u;

  // Empty:
  range_1u = torrent::ranges<int>::create_union(range_1l, range_1r);

  ASSERT_TRUE(verify_ranges(range_1u));
  ASSERT_EQ(range_1u.intersect_distance(-5, 20), 0);

  range_1l.insert(0, 5);

  // Left one entry:
  range_1u = torrent::ranges<int>::create_union(range_1l, range_1r);

  ASSERT_TRUE(verify_ranges(range_1u));
  ASSERT_EQ(range_1u.intersect_distance(-5, 20), 5);

  // Right one entry:
  range_1u = torrent::ranges<int>::create_union(range_1r, range_1l);

  ASSERT_TRUE(verify_ranges(range_1u));
  ASSERT_EQ(range_1u.intersect_distance(-5, 20), 5);

  range_1r.insert(10, 15);

  // Both one entry:
  range_1u = torrent::ranges<int>::create_union(range_1l, range_1r);

  //  std::cout << "range_1u.intersect_distance(-5, 20) = " <<
  //  range_1u.intersect_distance(-5, 20) << std::endl;

  ASSERT_TRUE(verify_ranges(range_1u));
  ASSERT_EQ(range_1u.intersect_distance(-5, 20), 10);

  range_1r.insert(4, 6);

  // Overlap:
  range_1u = torrent::ranges<int>::create_union(range_1l, range_1r);

  //  std::cout << "range_1u.intersect_distance(-5, 20) = " <<
  //  range_1u.intersect_distance(-5, 20) << std::endl;

  ASSERT_TRUE(verify_ranges(range_1u));
  ASSERT_EQ(range_1u.intersect_distance(-5, 20), 11);

  range_1r.insert(9, 10);

  // Boundary:
  range_1u = torrent::ranges<int>::create_union(range_1l, range_1r);

  ASSERT_TRUE(verify_ranges(range_1u));
  ASSERT_EQ(range_1u.intersect_distance(-5, 20), 12);

  // Trailing ranges left.
  range_1l.insert(25, 30);
  range_1l.insert(35, 40);

  range_1u = torrent::ranges<int>::create_union(range_1l, range_1r);

  ASSERT_TRUE(verify_ranges(range_1u));
  ASSERT_EQ(range_1u.intersect_distance(-5, 50), 22);

  // Trailing ranges right.
  range_1r.insert(37, 45);
  range_1r.insert(50, 55);

  range_1u = torrent::ranges<int>::create_union(range_1l, range_1r);

  ASSERT_TRUE(verify_ranges(range_1u));
  ASSERT_EQ(range_1u.intersect_distance(-5, 60), 32);
}
