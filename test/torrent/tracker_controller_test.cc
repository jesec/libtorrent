#include <functional>
#include <iostream>

#include "globals.h"
#include "torrent/utils/priority_queue_default.h"

#include "test/torrent/tracker_controller_test.h"
#include "test/torrent/tracker_list_test.h"

bool
test_goto_next_timeout(torrent::TrackerController* tracker_controller,
                       uint32_t                    assumed_timeout,
                       bool                        is_scrape) {
  uint32_t next_timeout = tracker_controller->task_timeout()->is_queued()
                            ? tracker_controller->seconds_to_next_timeout()
                            : ~uint32_t();
  uint32_t next_scrape  = tracker_controller->task_scrape()->is_queued()
                            ? tracker_controller->seconds_to_next_scrape()
                            : ~uint32_t();

  if (next_timeout == next_scrape && next_timeout == ~uint32_t()) {
    std::cout << "(nq)";
    return false;
  }

  if (next_timeout < next_scrape) {
    if (is_scrape) {
      std::cout << "(t" << next_timeout << "<s" << next_scrape << ")";
      return false;
    }

    if (assumed_timeout != next_timeout) {
      std::cout << '(' << assumed_timeout << "!=t" << next_timeout << ')';
      return false;
    }
  } else if (next_scrape < next_timeout) {
    if (!is_scrape) {
      std::cout << "(s" << next_scrape << "<t" << next_timeout << ")";
      return false;
    }

    if (assumed_timeout != next_scrape) {
      std::cout << '(' << assumed_timeout << "!=s" << next_scrape << ')';
      return false;
    }
  }

  torrent::cachedTime +=
    torrent::utils::timer::from_seconds(is_scrape ? next_scrape : next_timeout);
  torrent::utils::priority_queue_perform(&torrent::taskScheduler,
                                         torrent::cachedTime);
  return true;
}

void
tracker_controller_test::SetUp() {
  ASSERT_TRUE(torrent::taskScheduler.empty());
  torrent::cachedTime = torrent::utils::timer::current();
}

void
tracker_controller_test::TearDown() {
  torrent::taskScheduler.clear();
  torrent::cachedTime = 0;
}

TEST_F(tracker_controller_test, test_basic) {
  torrent::TrackerController tracker_controller(NULL);

  ASSERT_TRUE(tracker_controller.flags() == 0);
  ASSERT_TRUE(!tracker_controller.is_active());
  ASSERT_TRUE(!tracker_controller.is_requesting());
}

TEST_F(tracker_controller_test, test_enable) {
  torrent::TrackerList       tracker_list;
  torrent::TrackerController tracker_controller(&tracker_list);

  tracker_controller.enable();

  ASSERT_TRUE(tracker_controller.is_active());
  ASSERT_TRUE(!tracker_controller.is_requesting());
}

TEST_F(tracker_controller_test, test_requesting) {
  torrent::TrackerList       tracker_list;
  torrent::TrackerController tracker_controller(&tracker_list);

  tracker_controller.enable();
  tracker_controller.start_requesting();

  ASSERT_TRUE(tracker_controller.is_active());
  ASSERT_TRUE(tracker_controller.is_requesting());

  tracker_controller.stop_requesting();

  ASSERT_TRUE(tracker_controller.is_active());
  ASSERT_TRUE(!tracker_controller.is_requesting());
}

TEST_F(tracker_controller_test, test_timeout) {
  TRACKER_CONTROLLER_SETUP();
  TRACKER_INSERT(0, tracker_0_0);

  ASSERT_TRUE(enabled_counter == 1);
  ASSERT_TRUE(!tracker_0_0->is_busy());
  ASSERT_TRUE(!tracker_controller.task_timeout()->is_queued());

  tracker_controller.enable();

  torrent::utils::priority_queue_perform(&torrent::taskScheduler,
                                         torrent::cachedTime);

  ASSERT_TRUE(timeout_counter == 1);
  TEST_SINGLE_END(0, 0);
}

TEST_F(tracker_controller_test, test_single_success) {
  TEST_SINGLE_BEGIN();

  tracker_list.send_state_idx(0, 1);
  ASSERT_TRUE(tracker_0_0->trigger_success());

  ASSERT_TRUE(success_counter == 1 && failure_counter == 0);

  tracker_list.send_state_idx(0, 2);
  ASSERT_TRUE(tracker_0_0->trigger_failure());

  TEST_SINGLE_END(1, 1);
}

#define TEST_SINGLE_FAILURE_TIMEOUT(test_interval)                             \
  torrent::utils::priority_queue_perform(&torrent::taskScheduler,              \
                                         torrent::cachedTime);                 \
  ASSERT_TRUE(tracker_0_0->trigger_failure());                                 \
  ASSERT_EQ(tracker_controller.seconds_to_next_timeout(), test_interval);      \
  torrent::cachedTime += torrent::utils::timer::from_seconds(test_interval);

TEST_F(tracker_controller_test, test_single_failure) {
  torrent::cachedTime = torrent::utils::timer::from_seconds(1 << 20);
  TEST_SINGLE_BEGIN();

  TEST_SINGLE_FAILURE_TIMEOUT(5);
  TEST_SINGLE_FAILURE_TIMEOUT(10);
  TEST_SINGLE_FAILURE_TIMEOUT(20);
  TEST_SINGLE_FAILURE_TIMEOUT(40);
  TEST_SINGLE_FAILURE_TIMEOUT(80);
  TEST_SINGLE_FAILURE_TIMEOUT(160);
  TEST_SINGLE_FAILURE_TIMEOUT(320);
  TEST_SINGLE_FAILURE_TIMEOUT(320);

  // TODO: Test with cachedTime not rounded to second.

  // Test also with a low timer value...
  // torrent::cachedTime = torrent::utils::timer::from_seconds(1000);

  TEST_SINGLE_END(0, 4);
}

TEST_F(tracker_controller_test, test_single_disable) {
  TEST_SINGLE_BEGIN();
  tracker_list.send_state_idx(0, 0);
  TEST_SINGLE_END(0, 0);
}

TEST_F(tracker_controller_test, test_send_start) {
  TEST_SINGLE_BEGIN();
  TEST_SEND_SINGLE_BEGIN(start);

  ASSERT_TRUE(!tracker_controller.task_timeout()->is_queued());

  ASSERT_TRUE(tracker_0_0->is_busy());
  ASSERT_TRUE(tracker_0_0->requesting_state() ==
              torrent::Tracker::EVENT_STARTED);

  ASSERT_TRUE(tracker_0_0->trigger_success());
  ASSERT_TRUE(
    !(tracker_controller.flags() & torrent::TrackerController::mask_send));

  ASSERT_TRUE(tracker_controller.seconds_to_next_timeout() != 0);

  tracker_controller.send_start_event();
  tracker_controller.disable();

  ASSERT_TRUE(!tracker_0_0->is_busy());

  TEST_SEND_SINGLE_END(1, 0);
}

TEST_F(tracker_controller_test, test_send_stop_normal) {
  TEST_SINGLE_BEGIN();
  TEST_SEND_SINGLE_BEGIN(update);

  ASSERT_TRUE(tracker_controller.task_timeout()->is_queued());
  ASSERT_TRUE(tracker_0_0->trigger_success());

  tracker_controller.send_stop_event();
  ASSERT_TRUE(
    (tracker_controller.flags() & torrent::TrackerController::mask_send) ==
    torrent::TrackerController::flag_send_stop);

  ASSERT_TRUE(tracker_controller.seconds_to_next_timeout() == 0);

  ASSERT_TRUE(tracker_0_0->trigger_success());
  ASSERT_TRUE(
    (tracker_controller.flags() & torrent::TrackerController::mask_send) == 0);

  tracker_controller.send_stop_event();
  tracker_controller.disable();

  ASSERT_TRUE(tracker_0_0->is_busy());
  tracker_0_0->trigger_success();

  TEST_SEND_SINGLE_END(3, 0);
}

// send_stop during request and right after start, send stop failed.

TEST_F(tracker_controller_test, test_send_completed_normal) {
  TEST_SINGLE_BEGIN();
  TEST_SEND_SINGLE_BEGIN(update);

  ASSERT_TRUE(tracker_controller.task_timeout()->is_queued());
  ASSERT_TRUE(tracker_0_0->trigger_success());

  tracker_controller.send_completed_event();
  ASSERT_TRUE(
    (tracker_controller.flags() & torrent::TrackerController::mask_send) ==
    torrent::TrackerController::flag_send_completed);

  ASSERT_TRUE(tracker_controller.seconds_to_next_timeout() == 0);

  ASSERT_TRUE(tracker_0_0->trigger_success());
  ASSERT_TRUE(
    (tracker_controller.flags() & torrent::TrackerController::mask_send) == 0);

  tracker_controller.send_completed_event();
  tracker_controller.disable();

  ASSERT_TRUE(tracker_0_0->is_busy());
  tracker_0_0->trigger_success();

  TEST_SEND_SINGLE_END(3, 0);
}

TEST_F(tracker_controller_test, test_send_update_normal) {
  TEST_SINGLE_BEGIN();
  TEST_SEND_SINGLE_BEGIN(update);

  ASSERT_TRUE(
    (tracker_controller.flags() & torrent::TrackerController::mask_send) ==
    torrent::TrackerController::flag_send_update);

  ASSERT_TRUE(tracker_controller.task_timeout()->is_queued());
  ASSERT_TRUE(tracker_0_0->latest_event() == torrent::Tracker::EVENT_NONE);

  ASSERT_TRUE(tracker_0_0->trigger_success());

  TEST_SEND_SINGLE_END(1, 0);
}

TEST_F(tracker_controller_test, test_send_update_failure) {
  torrent::cachedTime = torrent::utils::timer::from_seconds(1 << 20);
  TEST_SINGLE_BEGIN();

  tracker_controller.send_update_event();

  TEST_SINGLE_FAILURE_TIMEOUT(5);
  TEST_SINGLE_FAILURE_TIMEOUT(10);

  TEST_SINGLE_END(0, 2);
}

TEST_F(tracker_controller_test, test_send_task_timeout) {
  TEST_SINGLE_BEGIN();
  TEST_SEND_SINGLE_BEGIN(update);

  ASSERT_TRUE(tracker_controller.task_timeout()->is_queued());

  TEST_SEND_SINGLE_END(0, 0);
}

TEST_F(tracker_controller_test, test_send_close_on_enable) {
  TRACKER_CONTROLLER_SETUP();
  TRACKER_INSERT(0, tracker_0);
  TRACKER_INSERT(0, tracker_1);
  TRACKER_INSERT(0, tracker_2);
  TRACKER_INSERT(0, tracker_3);

  tracker_list.send_state_idx(0, torrent::Tracker::EVENT_NONE);
  tracker_list.send_state_idx(1, torrent::Tracker::EVENT_STARTED);
  tracker_list.send_state_idx(2, torrent::Tracker::EVENT_STOPPED);
  tracker_list.send_state_idx(3, torrent::Tracker::EVENT_COMPLETED);

  tracker_controller.enable();

  ASSERT_TRUE(!tracker_0->is_busy());
  ASSERT_TRUE(!tracker_1->is_busy());
  ASSERT_TRUE(!tracker_2->is_busy());
  ASSERT_TRUE(tracker_3->is_busy());
}

TEST_F(tracker_controller_test, test_multiple_success) {
  TEST_MULTI3_BEGIN();
  TEST_SEND_SINGLE_BEGIN(update);

  ASSERT_TRUE(tracker_0_0->trigger_success());

  ASSERT_PRED3(test_goto_next_timeout,
               &tracker_controller,
               tracker_0_0->normal_interval(),
               false);
  TEST_MULTI3_IS_BUSY("10000", "10000");

  ASSERT_TRUE(tracker_0_0->trigger_success());

  ASSERT_PRED3(test_goto_next_timeout,
               &tracker_controller,
               tracker_0_0->normal_interval(),
               false);
  TEST_MULTI3_IS_BUSY("10000", "10000");

  ASSERT_TRUE(tracker_0_0->trigger_success());

  ASSERT_TRUE(!tracker_list.has_active());
  ASSERT_TRUE(tracker_0_0->success_counter() == 3);

  TEST_MULTIPLE_END(3, 0);
}

TEST_F(tracker_controller_test, test_multiple_failure) {
  TEST_MULTI3_BEGIN();
  TEST_SEND_SINGLE_BEGIN(update);

  ASSERT_TRUE(tracker_0_0->trigger_failure());
  ASSERT_TRUE(!tracker_controller.is_failure_mode());

  TEST_MULTI3_IS_BUSY("01000", "01000");
  ASSERT_TRUE(tracker_0_1->trigger_failure());
  TEST_MULTI3_IS_BUSY("00100", "00100");
  ASSERT_TRUE(tracker_1_0->trigger_success());

  ASSERT_PRED3(test_goto_next_timeout,
               &tracker_controller,
               tracker_0_0->normal_interval(),
               false);
  TEST_MULTI3_IS_BUSY("10000", "10000");
  ASSERT_TRUE(tracker_0_0->trigger_failure());
  TEST_MULTI3_IS_BUSY("01000", "01000");
  ASSERT_TRUE(tracker_0_1->trigger_failure());

  ASSERT_TRUE(!tracker_controller.is_failure_mode());
  TEST_MULTI3_IS_BUSY("00100", "00100");
  ASSERT_TRUE(tracker_1_0->trigger_failure());
  ASSERT_TRUE(tracker_controller.is_failure_mode());

  TEST_MULTI3_IS_BUSY("00010", "00010");
  ASSERT_TRUE(tracker_2_0->trigger_failure());
  TEST_MULTI3_IS_BUSY("00001", "00001");
  ASSERT_TRUE(tracker_3_0->trigger_failure());

  // Properly tests that we're going into timeouts... Disable remaining
  // trackers.

  // for (int i = 0; i < 5; i++)
  //   std::cout << std::endl << i << ": "
  //             << tracker_list[i]->activity_time_last() << ' '
  //             << tracker_list[i]->success_time_last() << ' '
  //             << tracker_list[i]->failed_time_last() << std::endl;

  // Try inserting some delays in order to test the timers.

  ASSERT_PRED3(test_goto_next_timeout, &tracker_controller, 5, false);
  TEST_MULTI3_IS_BUSY("00100", "00100");
  ASSERT_TRUE(tracker_1_0->trigger_success());
  ASSERT_TRUE(!tracker_controller.is_failure_mode());

  // ASSERT_PRED3(test_goto_next_timeout,
  //              &tracker_controller,
  //              tracker_list[0]->normal_interval(), false);
  // TEST_MULTI3_IS_BUSY("01000", "10000");
  // ASSERT_TRUE(tracker_0_1->trigger_success());

  ASSERT_TRUE(tracker_list.count_active() == 0);
  TEST_MULTIPLE_END(2, 7);
}

TEST_F(tracker_controller_test, test_multiple_cycle) {
  TEST_MULTI3_BEGIN();
  TEST_SEND_SINGLE_BEGIN(update);

  ASSERT_TRUE(tracker_0_0->trigger_failure());
  ASSERT_TRUE(tracker_0_1->trigger_success());
  ASSERT_TRUE(tracker_list.front() == tracker_0_1);

  ASSERT_PRED3(test_goto_next_timeout,
               &tracker_controller,
               tracker_0_1->normal_interval(),
               false);

  TEST_MULTI3_IS_BUSY("01000", "10000");
  ASSERT_TRUE(tracker_0_1->trigger_success());

  ASSERT_TRUE(tracker_list.count_active() == 0);
  TEST_MULTIPLE_END(2, 1);
}

TEST_F(tracker_controller_test, test_multiple_cycle_second_group) {
  TEST_MULTI3_BEGIN();
  TEST_SEND_SINGLE_BEGIN(update);

  ASSERT_TRUE(tracker_0_0->trigger_failure());
  ASSERT_TRUE(tracker_0_1->trigger_failure());
  ASSERT_TRUE(tracker_1_0->trigger_success());

  ASSERT_TRUE(tracker_list[0] == tracker_0_0);
  ASSERT_TRUE(tracker_list[1] == tracker_0_1);
  ASSERT_TRUE(tracker_list[2] == tracker_1_0);
  ASSERT_TRUE(tracker_list[3] == tracker_2_0);
  ASSERT_TRUE(tracker_list[4] == tracker_3_0);

  TEST_MULTIPLE_END(1, 2);
}

TEST_F(tracker_controller_test, test_multiple_send_stop) {
  TEST_MULTI3_BEGIN();

  tracker_list.send_state_idx(1, torrent::Tracker::EVENT_NONE);
  tracker_list.send_state_idx(3, torrent::Tracker::EVENT_NONE);
  tracker_list.send_state_idx(4, torrent::Tracker::EVENT_NONE);

  ASSERT_TRUE(tracker_0_1->trigger_success());
  ASSERT_TRUE(tracker_2_0->trigger_success());
  ASSERT_TRUE(tracker_3_0->trigger_success());
  ASSERT_TRUE(tracker_list.count_active() == 0);

  tracker_controller.send_stop_event();
  ASSERT_TRUE(tracker_list.count_active() == 3);

  TEST_MULTI3_IS_BUSY("01011", "10011");
  ASSERT_TRUE(tracker_0_1->trigger_success());
  ASSERT_TRUE(
    (tracker_controller.flags() & torrent::TrackerController::mask_send) !=
    torrent::TrackerController::flag_send_stop);
  ASSERT_TRUE(tracker_2_0->trigger_success());
  ASSERT_TRUE(tracker_3_0->trigger_success());

  ASSERT_TRUE(tracker_list.count_active() == 0);

  tracker_controller.send_stop_event();
  tracker_controller.disable();
  ASSERT_TRUE(
    (tracker_controller.flags() & torrent::TrackerController::mask_send) ==
    torrent::TrackerController::flag_send_stop);
  TEST_MULTI3_IS_BUSY("01011", "10011");

  tracker_controller.enable(
    torrent::TrackerController::enable_dont_reset_stats);
  TEST_MULTI3_IS_BUSY("00000", "00000");
  ASSERT_TRUE(
    (tracker_controller.flags() & torrent::TrackerController::mask_send) == 0);

  tracker_controller.send_stop_event();
  TEST_MULTI3_IS_BUSY("01011", "10011");

  tracker_controller.disable();
  tracker_controller.enable();
  TEST_MULTI3_IS_BUSY("00000", "00000");
  ASSERT_TRUE(
    (tracker_controller.flags() & torrent::TrackerController::mask_send) == 0);

  tracker_controller.send_stop_event();
  TEST_MULTI3_IS_BUSY("00000", "00000");

  // Test also that after closing the tracker controller, and
  // reopening it a 'send stop' event causes no tracker to be busy.

  TEST_MULTIPLE_END(6, 0);
}

#if 0
TEST_F(tracker_controller_test, test_multiple_send_update) {
  TEST_MULTI3_BEGIN();

  tracker_controller.send_update_event();
  ASSERT_PRED3(test_goto_next_timeout, &tracker_controller, 0, false);
  TEST_MULTI3_IS_BUSY("10000", "10000");

  ASSERT_TRUE(tracker_0_0->trigger_success());

  tracker_0_0->set_failed(1, torrent::cachedTime.seconds());

  tracker_controller.send_update_event();
  ASSERT_PRED3(test_goto_next_timeout, &tracker_controller, 0, false);
  TEST_MULTI3_IS_BUSY("01000", "01000");

  ASSERT_TRUE(tracker_0_1->trigger_failure());

  ASSERT_PRED3(test_goto_next_timeout, &tracker_controller, 0, false);
  TEST_MULTI3_IS_BUSY("00100", "00100");

  TEST_MULTIPLE_END(2, 0);
}
#endif

// Test timeout called, no usable trackers at all which leads to
// disabling the timeout.
//
// The enable/disable tracker functions will poke the tracker
// controller, ensuring the tast timeout gets re-started.
TEST_F(tracker_controller_test, test_timeout_lacking_usable) {
  TEST_MULTI3_BEGIN();

  std::for_each(tracker_list.begin(),
                tracker_list.end(),
                std::mem_fn(&torrent::Tracker::disable));
  ASSERT_TRUE(tracker_controller.task_timeout()->is_queued());

  ASSERT_PRED3(test_goto_next_timeout, &tracker_controller, 0, false);

  TEST_MULTI3_IS_BUSY("00000", "00000");
  ASSERT_TRUE(!tracker_controller.task_timeout()->is_queued());

  tracker_1_0->enable();

  ASSERT_TRUE(tracker_controller.task_timeout()->is_queued());
  ASSERT_TRUE(tracker_controller.seconds_to_next_timeout() == 0);

  ASSERT_PRED3(test_goto_next_timeout, &tracker_controller, 0, false);

  TEST_MULTI3_IS_BUSY("00100", "00100");

  ASSERT_TRUE(!tracker_controller.task_timeout()->is_queued());
  tracker_3_0->enable();
  ASSERT_TRUE(!tracker_controller.task_timeout()->is_queued());

  ASSERT_TRUE(enabled_counter == 5 + 2 && disabled_counter == 5);
  TEST_MULTIPLE_END(0, 0);
}

TEST_F(tracker_controller_test, test_disable_tracker) {
  TEST_SINGLE_BEGIN();
  TEST_SEND_SINGLE_BEGIN(update);

  ASSERT_TRUE(tracker_controller.task_timeout()->is_queued());
  ASSERT_TRUE(tracker_0_0->is_busy());

  tracker_0_0->disable();

  ASSERT_TRUE(!tracker_0_0->is_busy());
  ASSERT_TRUE(tracker_controller.task_timeout()->is_queued());

  TEST_SINGLE_END(0, 0);
}

TEST_F(tracker_controller_test, test_new_peers) {
  TRACKER_CONTROLLER_SETUP();
  TRACKER_INSERT(0, tracker_0);

  tracker_list.send_state_idx(0, torrent::Tracker::EVENT_NONE);

  ASSERT_TRUE(tracker_0->trigger_success(10));
  ASSERT_TRUE(tracker_0->latest_new_peers() == 10);

  tracker_controller.enable();

  ASSERT_PRED3(test_goto_next_timeout, &tracker_controller, 0, false);
  ASSERT_TRUE(tracker_0->trigger_success(20));
  ASSERT_TRUE(tracker_0->latest_new_peers() == 20);
}

// Add new function for finding the first tracker that will time out,
// e.g. both with failure mode and normal rerequesting.

// Make sure that adding a new tracker to a tracker list with no
// usable tracker restarts the tracker requests.

// Test timeout called, usable trackers exist but could not be called
// at this moment, thus leading to no active trackers. (Rather, clean
// up the 'is_usable()' functions, and make it a new function that is
// independent of enabled, or is itself manipulating/dependent on
// enabled)

// Add checks to ensure we don't prematurely do timeout on a tracker
// after disabling the lead one.

// Make sure that we always remain with timeout, even if we send a
// request that somehow doesn't actually activate any
// trackers. e.g. check all send_tracker_itr uses.

// Add test for promiscious mode while with a single tracker?

// Test send_* with controller not enabled.

// Test cleanup after disable.
// Test cleanup of timers at disable (and empty timers at enable).

// Test trying to send_start twice, etc.

// Test send_start promiscious...
//   - Make sure we check that no timer is inserted while still having active
//   trackers.
//   - Calculate the next timeout according to a list of in-use trackers, with
//   the first timeout as the interval.

// Test clearing of recv/failed counter on trackers.
