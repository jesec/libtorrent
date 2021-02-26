#include <iostream>

#include "globals.h"
#include "torrent/tracker_controller.h"
#include "torrent/utils/priority_queue_default.h"

#include "test/torrent/tracker_list_test.h"
#include "test/torrent/tracker_timeout_test.h"

void
tracker_timeout_test::SetUp() {
  torrent::cachedTime = torrent::utils::timer::current();
  //  torrent::cachedTime = torrent::utils::timer::current().round_seconds();
}

void
tracker_timeout_test::TearDown() {}

TEST_F(tracker_timeout_test, test_set_timeout) {
  TrackerTest tracker(nullptr, "");

  ASSERT_EQ(tracker.normal_interval(), 1800);

  tracker.set_new_normal_interval(100);
  ASSERT_EQ(tracker.normal_interval(), 600);
  tracker.set_new_normal_interval(8 * 4000);
  ASSERT_EQ(tracker.normal_interval(), 8 * 3600);

  tracker.set_new_min_interval(100);
  ASSERT_EQ(tracker.min_interval(), 300);
  tracker.set_new_min_interval(4 * 4000);
  ASSERT_EQ(tracker.min_interval(), 4 * 3600);
}

TEST_F(tracker_timeout_test, test_timeout_tracker) {
  TrackerTest tracker(nullptr, "");
  int         flags = 0;

  ASSERT_EQ(torrent::tracker_next_timeout(&tracker, flags), 0);
  torrent::cachedTime += torrent::utils::timer::from_seconds(3);
  ASSERT_EQ(torrent::tracker_next_timeout(&tracker, flags), 0);

  flags = torrent::TrackerController::flag_active;

  ASSERT_EQ(torrent::tracker_next_timeout(&tracker, flags), 0);
  tracker.send_state(torrent::Tracker::EVENT_NONE);
  ASSERT_EQ(torrent::tracker_next_timeout(&tracker, flags), ~uint32_t());
  tracker.send_state(torrent::Tracker::EVENT_SCRAPE);
  ASSERT_EQ(torrent::tracker_next_timeout(&tracker, flags), 0);

  tracker.close();
  tracker.set_success(1, torrent::cachedTime.seconds());

  // Check also failed...

  ASSERT_EQ(torrent::tracker_next_timeout(&tracker, flags), 1800);
  tracker.send_state(torrent::Tracker::EVENT_NONE);
  ASSERT_EQ(torrent::tracker_next_timeout(&tracker, flags), ~uint32_t());
  tracker.send_state(torrent::Tracker::EVENT_SCRAPE);
  ASSERT_EQ(torrent::tracker_next_timeout(&tracker, flags), 1800);

  tracker.close();

  tracker.set_success(1, torrent::cachedTime.seconds() - 3);
  ASSERT_EQ(torrent::tracker_next_timeout(&tracker, flags), 1800 - 3);
  tracker.set_success(1, torrent::cachedTime.seconds() + 3);
  ASSERT_EQ(torrent::tracker_next_timeout(&tracker, flags), 1800 + 3);

  tracker.close();
  flags = torrent::TrackerController::flag_active |
          torrent::TrackerController::flag_promiscuous_mode;

  ASSERT_EQ(torrent::tracker_next_timeout(&tracker, flags), 0);
  tracker.send_state(torrent::Tracker::EVENT_NONE);
  ASSERT_EQ(torrent::tracker_next_timeout(&tracker, flags), ~uint32_t());
  tracker.send_state(torrent::Tracker::EVENT_SCRAPE);
  ASSERT_EQ(torrent::tracker_next_timeout(&tracker, flags), 0);
}

TEST_F(tracker_timeout_test, test_timeout_update) {
  TrackerTest tracker(nullptr, "");
  int         flags = 0;

  flags = torrent::TrackerController::flag_active |
          torrent::TrackerController::flag_send_update;

  ASSERT_EQ(torrent::tracker_next_timeout(&tracker, flags), 0);
  tracker.send_state(torrent::Tracker::EVENT_SCRAPE);
  ASSERT_EQ(torrent::tracker_next_timeout(&tracker, flags), 0);
  tracker.send_state(torrent::Tracker::EVENT_NONE);
  ASSERT_EQ(torrent::tracker_next_timeout(&tracker, flags), ~uint32_t());

  tracker.close();
  tracker.set_failed(1, torrent::cachedTime.seconds());
  ASSERT_EQ(torrent::tracker_next_timeout(&tracker, flags), 0);

  tracker.set_failed(0, torrent::cachedTime.seconds());
  tracker.set_success(0, torrent::cachedTime.seconds());
  ASSERT_EQ(torrent::tracker_next_timeout(&tracker, flags), 0);
}

TEST_F(tracker_timeout_test, test_timeout_requesting) {
  TrackerTest tracker(nullptr, "");
  int         flags = 0;

  flags = torrent::TrackerController::flag_active |
          torrent::TrackerController::flag_requesting;

  ASSERT_EQ(torrent::tracker_next_timeout(&tracker, flags), 0);
  tracker.send_state(torrent::Tracker::EVENT_SCRAPE);
  ASSERT_EQ(torrent::tracker_next_timeout(&tracker, flags), 0);
  tracker.send_state(torrent::Tracker::EVENT_NONE);
  ASSERT_EQ(torrent::tracker_next_timeout(&tracker, flags), ~uint32_t());

  // tracker.set_latest_new_peers(10 - 1);

  tracker.close();
  tracker.set_failed(1, torrent::cachedTime.seconds());
  ASSERT_EQ(torrent::tracker_next_timeout(&tracker, flags), 5);
  tracker.set_failed(2, torrent::cachedTime.seconds());
  ASSERT_EQ(torrent::tracker_next_timeout(&tracker, flags), 10);
  tracker.set_failed(6 + 1, torrent::cachedTime.seconds());
  ASSERT_EQ(torrent::tracker_next_timeout(&tracker, flags), 320);
  tracker.set_failed(7 + 1, torrent::cachedTime.seconds());
  ASSERT_EQ(torrent::tracker_next_timeout(&tracker, flags), 320);

  // std::cout << "timeout:" << torrent::tracker_next_timeout(&tracker, flags)
  // << std::endl;

  tracker.set_failed(0, torrent::cachedTime.seconds());
  tracker.set_success(0, torrent::cachedTime.seconds());
  // ASSERT_EQ(torrent::tracker_next_timeout(&tracker, flags), 10);
  // tracker.set_success(1, torrent::cachedTime.seconds());
  // ASSERT_EQ(torrent::tracker_next_timeout(&tracker, flags), 20);
  // tracker.set_success(2, torrent::cachedTime.seconds());
  ASSERT_EQ(torrent::tracker_next_timeout(&tracker, flags), 600);
  tracker.set_success(6, torrent::cachedTime.seconds());
  ASSERT_EQ(torrent::tracker_next_timeout(&tracker, flags), 600);

  tracker.set_success(1, torrent::cachedTime.seconds());
  // tracker.set_latest_sum_peers(9);
  // ASSERT_EQ(torrent::tracker_next_timeout(&tracker, flags), 20);
  tracker.set_latest_sum_peers(10);
  ASSERT_EQ(torrent::tracker_next_timeout(&tracker, flags), 600);

  tracker.set_latest_sum_peers(10);
  tracker.set_latest_new_peers(10);
  tracker.set_success(1, torrent::cachedTime.seconds());
  ASSERT_EQ(torrent::tracker_next_timeout(&tracker, flags), 600);
}
