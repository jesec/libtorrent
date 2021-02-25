#include <functional>
#include <iostream>

#include "globals.h"
#include "torrent/utils/priority_queue_default.h"

#include "test/torrent/tracker_controller_features.h"
#include "test/torrent/tracker_list_test.h"

void
tracker_controller_features::SetUp() {
  ASSERT_TRUE(torrent::taskScheduler.empty());

  torrent::cachedTime = torrent::utils::timer::current();
}

void
tracker_controller_features::TearDown() {
  torrent::taskScheduler.clear();
}

TEST_F(tracker_controller_features, test_requesting_basic) {
  TEST_MULTI3_BEGIN();
  TEST_SEND_SINGLE_BEGIN(update);

  ASSERT_TRUE(tracker_0_0->trigger_success(8, 9));

  tracker_controller.start_requesting();
  ASSERT_TRUE(test_goto_next_timeout(&tracker_controller, 0));
  TEST_MULTI3_IS_BUSY("00111", "00111");

  ASSERT_TRUE(tracker_1_0->trigger_success());
  ASSERT_TRUE(tracker_2_0->trigger_success());
  ASSERT_TRUE(tracker_3_0->trigger_success());

  // TODO: Change this so that requesting state results in tracker
  // requests from many peers. Also, add a limit so we don't keep
  // requesting from spent trackers.

  // Next timeout should be soon...
  ASSERT_TRUE(test_goto_next_timeout(&tracker_controller, 30));
  TEST_MULTI3_IS_BUSY("00000", "00000");

  ASSERT_TRUE(test_goto_next_timeout(&tracker_controller,
                                     tracker_0_0->min_interval() - 30));
  TEST_MULTI3_IS_BUSY("10111", "10111");

  ASSERT_TRUE(tracker_0_0->trigger_success());
  ASSERT_TRUE(tracker_1_0->trigger_success());
  ASSERT_TRUE(tracker_2_0->trigger_success());
  ASSERT_TRUE(tracker_3_0->trigger_success());

  tracker_controller.stop_requesting();

  TEST_MULTIPLE_END(8, 0);
}

TEST_F(tracker_controller_features, test_requesting_timeout) {
  TEST_MULTI3_BEGIN();
  TEST_SEND_SINGLE_BEGIN(update);

  tracker_controller.start_requesting();
  ASSERT_TRUE(test_goto_next_timeout(&tracker_controller, 0));

  TEST_MULTI3_IS_BUSY("10111", "10111");

  ASSERT_TRUE(tracker_0_0->trigger_failure());
  ASSERT_TRUE(tracker_controller.seconds_to_next_timeout() == 5);
  // ASSERT_TRUE(tracker_0_1->trigger_failure());
  ASSERT_TRUE(tracker_1_0->trigger_failure());
  ASSERT_TRUE(tracker_2_0->trigger_failure());
  ASSERT_TRUE(tracker_3_0->trigger_failure());

  // ASSERT_TRUE(test_goto_next_timeout(&tracker_controller, 0));
  TEST_MULTI3_IS_BUSY("01000", "01000");

  ASSERT_TRUE(test_goto_next_timeout(&tracker_controller, 5));
  TEST_MULTI3_IS_BUSY("01111", "01111");

  ASSERT_TRUE(!tracker_controller.task_timeout()->is_queued());
  ASSERT_TRUE(tracker_0_1->trigger_success());
  ASSERT_TRUE(tracker_controller.seconds_to_next_timeout() == 30);

  TEST_MULTIPLE_END(1, 4);
}

TEST_F(tracker_controller_features, test_promiscious_timeout) {
  TEST_MULTI3_BEGIN();
  TEST_SEND_SINGLE_BEGIN(start);

  ASSERT_TRUE(test_goto_next_timeout(&tracker_controller, 3));

  TEST_MULTI3_IS_BUSY("10111", "10111");

  ASSERT_TRUE(!tracker_controller.task_timeout()->is_queued());

  ASSERT_TRUE(tracker_0_0->trigger_success());
  ASSERT_TRUE(!(tracker_controller.flags() &
                torrent::TrackerController::flag_promiscuous_mode));

  // ASSERT_TRUE(tracker_0_1->trigger_success());
  ASSERT_TRUE(tracker_1_0->trigger_success());
  ASSERT_TRUE(tracker_2_0->trigger_success());
  ASSERT_TRUE(!tracker_controller.task_timeout()->is_queued());

  ASSERT_TRUE(tracker_3_0->trigger_success());
  ASSERT_TRUE(test_goto_next_timeout(&tracker_controller,
                                     tracker_0_0->normal_interval()));

  TEST_MULTIPLE_END(4, 0);
}

// Fix failure event handler so that it properly handles multi-request
// situations. This includes fixing old tests.

TEST_F(tracker_controller_features, test_promiscious_failed) {
  TEST_MULTI3_BEGIN();
  TEST_SEND_SINGLE_BEGIN(start);

  ASSERT_TRUE(tracker_0_0->trigger_failure());
  ASSERT_TRUE((tracker_controller.flags() &
               torrent::TrackerController::flag_promiscuous_mode));

  TEST_MULTI3_IS_BUSY("01111", "01111");
  ASSERT_TRUE(tracker_controller.task_timeout()->is_queued());

  ASSERT_TRUE(tracker_3_0->trigger_failure());
  torrent::cachedTime += torrent::utils::timer::from_seconds(2);
  ASSERT_TRUE(tracker_2_0->trigger_failure());

  TEST_MULTI3_IS_BUSY("01100", "01100");
  ASSERT_TRUE(test_goto_next_timeout(&tracker_controller, 3));
  TEST_MULTI3_IS_BUSY("01101", "01101");

  ASSERT_TRUE(tracker_0_1->trigger_failure());
  ASSERT_TRUE(tracker_1_0->trigger_failure());
  ASSERT_TRUE(tracker_3_0->trigger_failure());

  ASSERT_TRUE(tracker_0_0->trigger_failure());

  ASSERT_TRUE(!tracker_list.has_active());
  ASSERT_TRUE(tracker_controller.task_timeout()->is_queued());

  TEST_MULTIPLE_END(0, 7);
}

TEST_F(tracker_controller_features, test_scrape_basic) {
  TEST_GROUP_BEGIN();
  tracker_controller.disable();

  ASSERT_TRUE(!tracker_controller.task_scrape()->is_queued());
  tracker_0_1->set_can_scrape();
  tracker_0_2->set_can_scrape();
  tracker_2_0->set_can_scrape();

  tracker_controller.scrape_request(0);

  TEST_GROUP_IS_BUSY("000000", "000000");
  ASSERT_TRUE(!tracker_controller.task_timeout()->is_queued());
  ASSERT_TRUE(tracker_controller.task_scrape()->is_queued());
  ASSERT_TRUE(tracker_0_1->latest_event() == torrent::Tracker::EVENT_NONE);
  ASSERT_TRUE(tracker_0_2->latest_event() == torrent::Tracker::EVENT_NONE);
  ASSERT_TRUE(tracker_2_0->latest_event() == torrent::Tracker::EVENT_NONE);

  TEST_GOTO_NEXT_SCRAPE(0);

  TEST_GROUP_IS_BUSY("010001", "010001");
  ASSERT_TRUE(!tracker_controller.task_timeout()->is_queued());
  ASSERT_TRUE(!tracker_controller.task_scrape()->is_queued());
  ASSERT_TRUE(tracker_0_1->latest_event() == torrent::Tracker::EVENT_SCRAPE);
  ASSERT_TRUE(tracker_0_2->latest_event() == torrent::Tracker::EVENT_NONE);
  ASSERT_TRUE(tracker_2_0->latest_event() == torrent::Tracker::EVENT_SCRAPE);

  ASSERT_TRUE(tracker_0_1->trigger_scrape());
  ASSERT_TRUE(tracker_2_0->trigger_scrape());

  TEST_GROUP_IS_BUSY("000000", "000000");
  ASSERT_TRUE(!tracker_controller.task_timeout()->is_queued());
  ASSERT_TRUE(!tracker_controller.task_scrape()->is_queued());

  ASSERT_TRUE(tracker_0_1->scrape_time_last() != 0);
  ASSERT_TRUE(tracker_0_2->scrape_time_last() == 0);
  ASSERT_TRUE(tracker_2_0->scrape_time_last() != 0);

  TEST_MULTIPLE_END(0, 0);
}

TEST_F(tracker_controller_features, test_scrape_priority) {
  TEST_SINGLE_BEGIN();
  ASSERT_TRUE(test_goto_next_timeout(&tracker_controller, 0));
  tracker_0_0->trigger_success();
  tracker_0_0->set_can_scrape();

  tracker_controller.scrape_request(0);

  TEST_GOTO_NEXT_SCRAPE(0);
  ASSERT_TRUE(tracker_0_0->is_busy());
  ASSERT_TRUE(tracker_0_0->latest_event() == torrent::Tracker::EVENT_SCRAPE);

  // Check the other event types too?
  tracker_controller.send_update_event();

  ASSERT_TRUE(tracker_0_0->is_busy());
  ASSERT_TRUE(tracker_0_0->latest_event() == torrent::Tracker::EVENT_NONE);

  ASSERT_TRUE(tracker_controller.task_timeout()->is_queued());
  ASSERT_TRUE(!tracker_controller.task_scrape()->is_queued());

  tracker_0_0->trigger_success();

  ASSERT_TRUE(tracker_controller.seconds_to_next_timeout() > 1);

  torrent::cachedTime += torrent::utils::timer::from_seconds(
    tracker_controller.seconds_to_next_timeout() - 1);
  torrent::utils::priority_queue_perform(&torrent::taskScheduler,
                                         torrent::cachedTime);

  tracker_controller.scrape_request(0);
  TEST_GOTO_NEXT_SCRAPE(0);

  ASSERT_TRUE(tracker_0_0->is_busy());
  ASSERT_TRUE(tracker_0_0->latest_event() == torrent::Tracker::EVENT_SCRAPE);

  ASSERT_TRUE(test_goto_next_timeout(&tracker_controller, 1));

  ASSERT_TRUE(tracker_0_0->is_busy());
  ASSERT_TRUE(tracker_0_0->latest_event() == torrent::Tracker::EVENT_NONE);

  TEST_SINGLE_END(2, 0);
}

TEST_F(tracker_controller_features, test_groups_requesting) {
  TEST_GROUP_BEGIN();
  TEST_SEND_SINGLE_BEGIN(start);

  // ASSERT_TRUE(tracker_0_0->trigger_success(10, 20));

  tracker_controller.start_requesting();

  ASSERT_TRUE(test_goto_next_timeout(&tracker_controller, 0));
  TEST_GROUP_IS_BUSY("100101", "100101");

  ASSERT_TRUE(tracker_0_0->trigger_success());
  ASSERT_TRUE(tracker_1_0->trigger_success());
  ASSERT_TRUE(tracker_2_0->trigger_success());

  // TODO: Change this so that requesting state results in tracker
  // requests from many peers. Also, add a limit so we don't keep
  // requesting from spent trackers.

  // Next timeout should be soon...
  ASSERT_TRUE(test_goto_next_timeout(&tracker_controller, 30));
  TEST_GROUP_IS_BUSY("000000", "000000");
  ASSERT_TRUE(test_goto_next_timeout(&tracker_controller,
                                     tracker_0_0->min_interval() - 30));
  TEST_GROUP_IS_BUSY("100101", "100101");

  ASSERT_TRUE(tracker_0_0->trigger_success());
  ASSERT_TRUE(tracker_1_0->trigger_success());
  ASSERT_TRUE(tracker_2_0->trigger_success());

  // Once we've requested twice, it should stop requesting from that tier.
  ASSERT_TRUE(test_goto_next_timeout(&tracker_controller, 30));
  TEST_GROUP_IS_BUSY("000000", "000000");

  tracker_controller.stop_requesting();

  TEST_MULTIPLE_END(6, 0);
}

TEST_F(tracker_controller_features, test_groups_scrape) {
  TEST_GROUP_BEGIN();
  tracker_controller.disable();

  tracker_0_0->set_can_scrape();
  tracker_0_1->set_can_scrape();
  tracker_0_2->set_can_scrape();
  tracker_1_0->set_can_scrape();
  tracker_1_1->set_can_scrape();
  tracker_2_0->set_can_scrape();

  ASSERT_TRUE(!tracker_controller.task_scrape()->is_queued());

  tracker_controller.scrape_request(0);

  TEST_GROUP_IS_BUSY("000000", "000000");
  TEST_GOTO_NEXT_SCRAPE(0);
  ASSERT_TRUE(tracker_0_0->latest_event() == torrent::Tracker::EVENT_SCRAPE);
  ASSERT_TRUE(tracker_0_1->latest_event() == torrent::Tracker::EVENT_NONE);
  ASSERT_TRUE(tracker_0_2->latest_event() == torrent::Tracker::EVENT_NONE);
  ASSERT_TRUE(tracker_1_0->latest_event() == torrent::Tracker::EVENT_SCRAPE);
  ASSERT_TRUE(tracker_1_1->latest_event() == torrent::Tracker::EVENT_NONE);
  ASSERT_TRUE(tracker_2_0->latest_event() == torrent::Tracker::EVENT_SCRAPE);

  TEST_GROUP_IS_BUSY("100101", "100101");
  ASSERT_TRUE(tracker_0_0->trigger_scrape());
  ASSERT_TRUE(tracker_1_0->trigger_scrape());
  ASSERT_TRUE(tracker_2_0->trigger_scrape());

  // Test with a non-can_scrape !busy tracker?

  // TEST_GROUP_IS_BUSY("100101", "100101");
  // ASSERT_TRUE(tracker_0_0->trigger_scrape());
  // ASSERT_TRUE(tracker_0_1->trigger_scrape());
  // ASSERT_TRUE(tracker_0_2->trigger_scrape());
  // ASSERT_TRUE(tracker_1_0->trigger_scrape());
  // ASSERT_TRUE(tracker_1_1->trigger_scrape());
  // ASSERT_TRUE(tracker_2_0->trigger_scrape());

  TEST_GROUP_IS_BUSY("000000", "000000");

  TEST_MULTIPLE_END(0, 0);
}
