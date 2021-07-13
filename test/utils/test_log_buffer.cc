#include "globals.h"
#include "torrent/utils/log_buffer.h"

#include "test/helpers/fixture.h"

class test_log_buffer : public test_fixture {};

TEST_F(test_log_buffer, test_basic) {
  torrent::log_buffer log;
  torrent::cachedTime = torrent::utils::timer::from_seconds(1000);

  log.lock();
  ASSERT_TRUE(log.empty());
  ASSERT_EQ(log.find_older(0), log.end());
  log.unlock();

  log.lock_and_push_log("foobar", 6, -1);
  ASSERT_TRUE(log.empty());

  log.lock_and_push_log("foobar", 6, 0);
  ASSERT_EQ(log.size(), 1);
  ASSERT_EQ(log.back().timestamp, 1000);
  ASSERT_EQ(log.back().group, 0);
  ASSERT_EQ(log.back().message, "foobar");

  torrent::cachedTime += torrent::utils::timer::from_milliseconds(1000);

  log.lock_and_push_log("barbaz", 6, 0);
  ASSERT_EQ(log.size(), 2);
  ASSERT_EQ(log.back().timestamp, 1001);
  ASSERT_EQ(log.back().group, 0);
  ASSERT_EQ(log.back().message, "barbaz");
}

TEST_F(test_log_buffer, test_timestamps) {
  torrent::log_buffer log;
  torrent::cachedTime = torrent::utils::timer::from_seconds(1000);

  log.lock_and_push_log("foobar", 6, 0);
  ASSERT_EQ(log.back().timestamp, 1000);
  ASSERT_EQ(log.find_older(1000 - 1), log.begin());
  ASSERT_EQ(log.find_older(1000), log.end());
  ASSERT_EQ(log.find_older(1000 + 1), log.end());

  torrent::cachedTime += torrent::utils::timer::from_milliseconds(10 * 1000);

  log.lock_and_push_log("foobar", 6, 0);
  ASSERT_EQ(log.back().timestamp, 1010);
  ASSERT_EQ(log.find_older(1010 - 10), log.begin());
  ASSERT_EQ(log.find_older(1010 - 1), log.begin() + 1);
  ASSERT_EQ(log.find_older(1010), log.end());
  ASSERT_EQ(log.find_older(1010 + 1), log.end());
}
