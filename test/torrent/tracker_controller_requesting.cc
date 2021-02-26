#include <functional>
#include <iostream>

#include "globals.h"
#include "torrent/utils/priority_queue_default.h"

#include "test/torrent/tracker_controller_requesting.h"
#include "test/torrent/tracker_list_test.h"

void
tracker_controller_requesting::SetUp() {
  ASSERT_TRUE(torrent::taskScheduler.empty());
  torrent::cachedTime = torrent::utils::timer::current();
}

void
tracker_controller_requesting::TearDown() {
  torrent::taskScheduler.clear();
  torrent::cachedTime = 0;
}

void
do_test_hammering_basic(bool     success1,
                        bool     success2,
                        bool     success3,
                        uint32_t min_interval = 0) {
  TEST_SINGLE_BEGIN();
  TEST_SEND_SINGLE_BEGIN(start);

  if (min_interval != 0)
    tracker_0_0->set_new_min_interval(min_interval);

  ASSERT_TRUE(tracker_0_0->is_busy());
  if (success1) {
    ASSERT_TRUE(tracker_0_0->trigger_success());
  } else {
    ASSERT_TRUE(tracker_0_0->trigger_failure());
  }

  ASSERT_EQ(tracker_controller.seconds_to_next_timeout(),
            tracker_0_0->normal_interval());
  ASSERT_FALSE((tracker_controller.flags() &
                torrent::TrackerController::flag_promiscuous_mode));

  tracker_controller.start_requesting();

  ASSERT_FALSE(tracker_0_0->is_busy());
  ASSERT_TRUE(test_goto_next_timeout(&tracker_controller, 0));
  ASSERT_FALSE(tracker_0_0->is_busy());

  ASSERT_TRUE(tracker_controller.flags() &
              torrent::TrackerController::flag_requesting);
  ASSERT_TRUE(
    test_goto_next_timeout(&tracker_controller, tracker_0_0->min_interval()));

  ASSERT_TRUE(tracker_0_0->is_busy());

  if (success2) {
    ASSERT_TRUE(tracker_0_0->trigger_success());

    ASSERT_TRUE(test_goto_next_timeout(&tracker_controller, 30));
    ASSERT_TRUE(test_goto_next_timeout(&tracker_controller,
                                       tracker_0_0->min_interval() - 30));
  } else {
    ASSERT_TRUE(tracker_0_0->trigger_failure());

    ASSERT_TRUE(test_goto_next_timeout(&tracker_controller, 5));
    ASSERT_TRUE(tracker_0_0->is_busy());
  }

  tracker_controller.stop_requesting();

  ASSERT_TRUE(tracker_0_0->is_busy());
  if (success3) {
    ASSERT_TRUE(tracker_0_0->trigger_success());
  } else {
    ASSERT_TRUE(tracker_0_0->trigger_failure());
  }

  TEST_SINGLE_END(success1 + success2 + success3,
                  !success1 + !success2 + !success3);
}

TEST_F(tracker_controller_requesting, test_hammering_basic_success) {
  do_test_hammering_basic(true, true, true);
}

TEST_F(tracker_controller_requesting,
       test_hammering_basic_success_long_timeout) {
  do_test_hammering_basic(true, true, true, 1000);
}

TEST_F(tracker_controller_requesting,
       test_hammering_basic_success_short_timeout) {
  do_test_hammering_basic(true, true, true, 300);
}

TEST_F(tracker_controller_requesting, test_hammering_basic_failure) {
  do_test_hammering_basic(true, false, false);
}

TEST_F(tracker_controller_requesting,
       test_hammering_basic_failure_long_timeout) {
  do_test_hammering_basic(true, false, false, 1000);
}

TEST_F(tracker_controller_requesting,
       test_hammering_basic_failure_short_timeout) {
  do_test_hammering_basic(true, false, false, 300);
}

// Differentiate between failure connection / http error and tracker returned
// error.

void
do_test_hammering_multi3(bool     success1,
                         bool     success2,
                         bool     success3,
                         uint32_t min_interval = 0) {
  TEST_MULTI3_BEGIN();
  TEST_SEND_SINGLE_BEGIN(start);

  if (min_interval != 0)
    tracker_0_0->set_new_min_interval(min_interval);

  TEST_MULTI3_IS_BUSY("10000", "10000");
  if (success1) {
    ASSERT_TRUE(tracker_0_0->trigger_success());
  } else {
    ASSERT_TRUE(tracker_0_0->trigger_failure());
  }

  ASSERT_EQ(tracker_controller.seconds_to_next_timeout(),
            tracker_0_0->normal_interval());
  ASSERT_FALSE((tracker_controller.flags() &
                torrent::TrackerController::flag_promiscuous_mode));

  tracker_controller.start_requesting();

  TEST_MULTI3_IS_BUSY("00000", "00000");
  ASSERT_TRUE(test_goto_next_timeout(&tracker_controller, 0));
  TEST_MULTI3_IS_BUSY("00111", "00111");

  if (success2) {
    ASSERT_TRUE(tracker_2_0->trigger_success());
  } else {
    ASSERT_TRUE(tracker_2_0->trigger_failure());
  }

  ASSERT_TRUE(test_goto_next_timeout(&tracker_controller, 30));
  TEST_MULTI3_IS_BUSY("00101", "00101");

  TrackerTest* next_tracker = tracker_0_0;
  unsigned int next_timeout = next_tracker->min_interval();
  const char*  next_is_busy = "10111";

  if (tracker_0_0->min_interval() < tracker_2_0->min_interval()) {
    ASSERT_TRUE(test_goto_next_timeout(&tracker_controller,
                                       next_tracker->min_interval() - 30));
    TEST_MULTI3_IS_BUSY("10101", "10101");
  } else if (tracker_0_0->min_interval() > tracker_2_0->min_interval()) {
    next_tracker = tracker_2_0;
    next_timeout = tracker_0_0->min_interval() - tracker_2_0->min_interval();
    next_is_busy = "10101";
    ASSERT_TRUE(test_goto_next_timeout(&tracker_controller,
                                       next_tracker->min_interval() - 30));
    TEST_MULTI3_IS_BUSY("00111", "00111");
  } else {
    ASSERT_TRUE(test_goto_next_timeout(&tracker_controller,
                                       next_tracker->min_interval() - 30));
    TEST_MULTI3_IS_BUSY("10111", "10111");
  }

  if (success2) {
    ASSERT_TRUE(next_tracker->trigger_success());

    ASSERT_TRUE(test_goto_next_timeout(&tracker_controller, 30));
    ASSERT_TRUE(test_goto_next_timeout(&tracker_controller, next_timeout - 30));

    TEST_MULTI3_IS_BUSY(next_is_busy, next_is_busy);
  } else {
    ASSERT_TRUE(next_tracker->trigger_failure());

    ASSERT_TRUE(test_goto_next_timeout(&tracker_controller, 5));
    TEST_MULTI3_IS_BUSY("10000", "10000");
  }

  tracker_controller.stop_requesting();

  TEST_MULTI3_IS_BUSY(next_is_busy, next_is_busy);
  if (success3) {
    ASSERT_TRUE(tracker_0_0->trigger_success());
  } else {
    ASSERT_TRUE(tracker_0_0->trigger_failure());
  }

  TEST_MULTIPLE_END(success1 + 2 * success2 + success3,
                    !success1 + 2 * !success2 + !success3);
}

TEST_F(tracker_controller_requesting, test_hammering_multi_success) {
  do_test_hammering_multi3(true, true, true);
}

TEST_F(tracker_controller_requesting,
       test_hammering_multi_success_long_timeout) {
  do_test_hammering_multi3(true, true, true, 1000);
}

TEST_F(tracker_controller_requesting,
       test_hammering_multi_success_short_timeout) {
  do_test_hammering_multi3(true, true, true, 300);
}

TEST_F(tracker_controller_requesting, test_hammering_multi_failure) {}
