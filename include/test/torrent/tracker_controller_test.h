#include <gtest/gtest.h>

#include "torrent/tracker_controller.h"

class tracker_controller_test : public ::testing::Test {
public:
  void SetUp() override;
  void TearDown() override;
};

#define TRACKER_CONTROLLER_SETUP()                                             \
  torrent::TrackerList       tracker_list;                                     \
  torrent::TrackerController tracker_controller(&tracker_list);                \
                                                                               \
  int success_counter  = 0;                                                    \
  int failure_counter  = 0;                                                    \
  int timeout_counter  = 0;                                                    \
  int enabled_counter  = 0;                                                    \
  int disabled_counter = 0;                                                    \
                                                                               \
  tracker_controller.slot_success() =                                          \
    std::bind(&increment_value_uint, &success_counter);                        \
  tracker_controller.slot_failure() =                                          \
    std::bind(&increment_value_void, &failure_counter);                        \
  tracker_controller.slot_timeout() =                                          \
    std::bind(&increment_value_void, &timeout_counter);                        \
  tracker_controller.slot_tracker_enabled() =                                  \
    std::bind(&increment_value_void, &enabled_counter);                        \
  tracker_controller.slot_tracker_disabled() =                                 \
    std::bind(&increment_value_void, &disabled_counter);                       \
                                                                               \
  tracker_list.slot_success() =                                                \
    std::bind(&torrent::TrackerController::receive_success,                    \
              &tracker_controller,                                             \
              std::placeholders::_1,                                           \
              std::placeholders::_2);                                          \
  tracker_list.slot_failure() =                                                \
    std::bind(&torrent::TrackerController::receive_failure,                    \
              &tracker_controller,                                             \
              std::placeholders::_1,                                           \
              std::placeholders::_2);                                          \
  tracker_list.slot_tracker_enabled() =                                        \
    std::bind(&torrent::TrackerController::receive_tracker_enabled,            \
              &tracker_controller,                                             \
              std::placeholders::_1);                                          \
  tracker_list.slot_tracker_disabled() =                                       \
    std::bind(&torrent::TrackerController::receive_tracker_disabled,           \
              &tracker_controller,                                             \
              std::placeholders::_1);

#define TEST_SINGLE_BEGIN()                                                    \
  TRACKER_CONTROLLER_SETUP();                                                  \
  TRACKER_INSERT(0, tracker_0_0);                                              \
                                                                               \
  tracker_controller.enable();                                                 \
  ASSERT_FALSE(tracker_controller.flags() &                                    \
               torrent::TrackerController::mask_send);

#define TEST_SINGLE_END(succeeded, failed)                                     \
  tracker_controller.disable();                                                \
  ASSERT_FALSE(tracker_list.has_active());                                     \
  ASSERT_EQ(success_counter, succeeded);                                       \
  ASSERT_EQ(failure_counter, failure_counter);

#define TEST_SEND_SINGLE_BEGIN(event_name)                                     \
  tracker_controller.send_##event_name##_event();                              \
  ASSERT_EQ(tracker_controller.flags() &                                       \
              torrent::TrackerController::mask_send,                           \
            torrent::TrackerController::flag_send_##event_name);               \
                                                                               \
  ASSERT_TRUE(tracker_controller.is_active());                                 \
  ASSERT_EQ(tracker_controller.tracker_list()->count_active(), 1);

#define TEST_SEND_SINGLE_END(succeeded, failed)                                \
  TEST_SINGLE_END(succeeded, failed)                                           \
  ASSERT_EQ(tracker_controller.seconds_to_next_timeout(), 0);                  \
  // ASSERT_NE(tracker_controller.seconds_to_promicious_mode(), 0);

#define TEST_MULTI3_BEGIN()                                                    \
  TRACKER_CONTROLLER_SETUP();                                                  \
  TRACKER_INSERT(0, tracker_0_0);                                              \
  TRACKER_INSERT(0, tracker_0_1);                                              \
  TRACKER_INSERT(1, tracker_1_0);                                              \
  TRACKER_INSERT(2, tracker_2_0);                                              \
  TRACKER_INSERT(3, tracker_3_0);                                              \
                                                                               \
  tracker_controller.enable();                                                 \
  ASSERT_FALSE(tracker_controller.flags() &                                    \
               torrent::TrackerController::mask_send);

#define TEST_GROUP_BEGIN()                                                     \
  TRACKER_CONTROLLER_SETUP();                                                  \
  TRACKER_INSERT(0, tracker_0_0);                                              \
  TRACKER_INSERT(0, tracker_0_1);                                              \
  TRACKER_INSERT(0, tracker_0_2);                                              \
  TRACKER_INSERT(1, tracker_1_0);                                              \
  TRACKER_INSERT(1, tracker_1_1);                                              \
  TRACKER_INSERT(2, tracker_2_0);                                              \
                                                                               \
  tracker_controller.enable();                                                 \
  ASSERT_FALSE(tracker_controller.flags() &                                    \
               torrent::TrackerController::mask_send);

#define TEST_MULTIPLE_END(succeeded, failed)                                   \
  tracker_controller.disable();                                                \
  ASSERT_FALSE(tracker_list.has_active());                                     \
  ASSERT_EQ(success_counter, succeeded);                                       \
  ASSERT_EQ(failure_counter, failed);

#define TEST_GOTO_NEXT_SCRAPE(assumed_scrape)                                  \
  ASSERT_TRUE(tracker_controller.task_scrape()->is_queued());                  \
  ASSERT_EQ(assumed_scrape, tracker_controller.seconds_to_next_scrape());      \
  ASSERT_TRUE(                                                                 \
    test_goto_next_timeout(&tracker_controller, assumed_scrape, true));

bool
test_goto_next_timeout(torrent::TrackerController* tracker_controller,
                       uint32_t                    assumed_timeout,
                       bool                        is_scrape = false);
