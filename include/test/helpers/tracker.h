#include "torrent/tracker_controller.h"
#include "torrent/tracker_list.h"

class TrackerTest : public torrent::Tracker {
public:
  static constexpr int flag_close_on_done     = max_flag_size << 0;
  static constexpr int flag_scrape_on_success = max_flag_size << 1;

  // TODO: Clean up tracker related enums.
  TrackerTest(torrent::TrackerList* parent,
              const std::string&    url,
              int                   flags = torrent::Tracker::flag_enabled)
    : torrent::Tracker(parent, url, flags)
    , m_busy(false)
    , m_open(false)
    , m_requesting_state(-1) {
    m_flags |= flag_close_on_done;
  }

  virtual bool is_busy() const {
    return m_busy;
  }
  bool is_open() const {
    return m_open;
  }

  virtual Type type() const {
    return (Type)(TRACKER_DHT + 1);
  }

  int requesting_state() const {
    return m_requesting_state;
  }

  bool trigger_success(uint32_t new_peers = 0, uint32_t sum_peers = 0);
  bool trigger_success(torrent::TrackerList::address_list* address_list,
                       uint32_t                            new_peers = 0);
  bool trigger_failure();
  bool trigger_scrape();

  void set_close_on_done(bool state) {
    if (state)
      m_flags |= flag_close_on_done;
    else
      m_flags &= ~flag_close_on_done;
  }
  void set_scrape_on_success(bool state) {
    if (state)
      m_flags |= flag_scrape_on_success;
    else
      m_flags &= ~flag_scrape_on_success;
  }
  void set_can_scrape() {
    m_flags |= flag_can_scrape;
  }

  void set_success(uint32_t counter, uint32_t time_last) {
    m_success_counter   = counter;
    m_success_time_last = time_last;
    m_normal_interval = default_normal_interval;
    m_min_interval = default_min_interval;
  }
  void set_failed(uint32_t counter, uint32_t time_last) {
    m_failed_counter   = counter;
    m_failed_time_last = time_last;
    m_normal_interval = 0;
    m_min_interval = 0;
  }
  void set_latest_new_peers(uint32_t peers) {
    m_latest_new_peers = peers;
  }
  void set_latest_sum_peers(uint32_t peers) {
    m_latest_sum_peers = peers;
  }

  void set_new_normal_interval(uint32_t timeout) {
    set_normal_interval(timeout);
  }
  void set_new_min_interval(uint32_t timeout) {
    set_min_interval(timeout);
  }

  virtual void send_state(int state) {
    m_busy             = true;
    m_open             = true;
    m_requesting_state = m_latest_event = state;
  }
  virtual void send_scrape() {
    m_busy             = true;
    m_open             = true;
    m_requesting_state = m_latest_event = torrent::Tracker::EVENT_SCRAPE;
  }
  virtual void close() {
    m_busy             = false;
    m_open             = false;
    m_requesting_state = -1;
  }
  virtual void disown() {
    m_busy             = false;
    m_open             = false;
    m_requesting_state = -1;
  }

private:
  bool m_busy;
  bool m_open;
  int  m_requesting_state;
};

extern uint32_t return_new_peers;
inline uint32_t
increment_value(int* value) {
  (*value)++;
  return return_new_peers;
}

inline void
increment_value_void(int* value) {
  (*value)++;
}
inline unsigned int
increment_value_uint(int* value) {
  (*value)++;
  return return_new_peers;
}

bool
check_has_active_in_group(const torrent::TrackerList* tracker_list,
                          const char*                 states,
                          bool                        scrape);

inline bool
check_tracker_is_busy(torrent::Tracker* tracker, char state) {
  return state == '0' || tracker->is_busy();
}

inline bool
check_tracker_is_not_busy(torrent::Tracker* tracker, char state) {
  return state == '1' || !tracker->is_busy();
}

bool
test_goto_next_timeout(torrent::TrackerController* tracker_controller,
                       uint32_t                    assumed_timeout,
                       bool                        is_scrape = false);

#define TRACKER_SETUP()                                                        \
  torrent::TrackerList tracker_list;                                           \
  int                  success_counter        = 0;                             \
  int                  failure_counter        = 0;                             \
  int                  scrape_success_counter = 0;                             \
  int                  scrape_failure_counter = 0;                             \
  tracker_list.slot_success() =                                                \
    std::bind(&increment_value_uint, &success_counter);                        \
  tracker_list.slot_failure() =                                                \
    std::bind(&increment_value_void, &failure_counter);                        \
  tracker_list.slot_scrape_success() =                                         \
    std::bind(&increment_value_void, &scrape_success_counter);                 \
  tracker_list.slot_scrape_failure() =                                         \
    std::bind(&increment_value_void, &scrape_failure_counter)

#define TRACKER_TEARDOWN() tracker_list.clear()

#define TRACKER_INSERT(group, name)                                            \
  TrackerTest* name = new TrackerTest(&tracker_list, "");                      \
  tracker_list.insert(group, name)

#define TEST_TRACKER_IS_BUSY(tracker, state)                                   \
  ASSERT_PRED2(check_tracker_is_busy, tracker, state);                         \
  ASSERT_PRED2(check_tracker_is_busy, tracker, state)

#define TEST_MULTI3_IS_BUSY(original, rearranged)                              \
  TEST_TRACKER_IS_BUSY(tracker_0_0, original[0]);                              \
  TEST_TRACKER_IS_BUSY(tracker_0_1, original[1]);                              \
  TEST_TRACKER_IS_BUSY(tracker_1_0, original[2]);                              \
  TEST_TRACKER_IS_BUSY(tracker_2_0, original[3]);                              \
  TEST_TRACKER_IS_BUSY(tracker_3_0, original[4]);                              \
  TEST_TRACKER_IS_BUSY(tracker_list[0], rearranged[0]);                        \
  TEST_TRACKER_IS_BUSY(tracker_list[1], rearranged[1]);                        \
  TEST_TRACKER_IS_BUSY(tracker_list[2], rearranged[2]);                        \
  TEST_TRACKER_IS_BUSY(tracker_list[3], rearranged[3]);                        \
  TEST_TRACKER_IS_BUSY(tracker_list[4], rearranged[4])

#define TEST_GROUP_IS_BUSY(original, rearranged)                               \
  TEST_TRACKER_IS_BUSY(tracker_0_0, original[0]);                              \
  TEST_TRACKER_IS_BUSY(tracker_0_1, original[1]);                              \
  TEST_TRACKER_IS_BUSY(tracker_0_2, original[2]);                              \
  TEST_TRACKER_IS_BUSY(tracker_1_0, original[3]);                              \
  TEST_TRACKER_IS_BUSY(tracker_1_1, original[4]);                              \
  TEST_TRACKER_IS_BUSY(tracker_2_0, original[5]);                              \
  TEST_TRACKER_IS_BUSY(tracker_list[0], rearranged[0]);                        \
  TEST_TRACKER_IS_BUSY(tracker_list[1], rearranged[1]);                        \
  TEST_TRACKER_IS_BUSY(tracker_list[2], rearranged[2]);                        \
  TEST_TRACKER_IS_BUSY(tracker_list[3], rearranged[3]);                        \
  TEST_TRACKER_IS_BUSY(tracker_list[4], rearranged[4]);                        \
  TEST_TRACKER_IS_BUSY(tracker_list[5], rearranged[5])

#define TEST_TRACKERS_IS_BUSY_5(original, rearranged)                          \
  TEST_TRACKER_IS_BUSY(tracker_0, original[0]);                                \
  TEST_TRACKER_IS_BUSY(tracker_1, original[1]);                                \
  TEST_TRACKER_IS_BUSY(tracker_2, original[2]);                                \
  TEST_TRACKER_IS_BUSY(tracker_3, original[3]);                                \
  TEST_TRACKER_IS_BUSY(tracker_4, original[4]);                                \
  TEST_TRACKER_IS_BUSY(tracker_list[0], rearranged[0]);                        \
  TEST_TRACKER_IS_BUSY(tracker_list[1], rearranged[1]);                        \
  TEST_TRACKER_IS_BUSY(tracker_list[2], rearranged[2]);                        \
  TEST_TRACKER_IS_BUSY(tracker_list[3], rearranged[3]);                        \
  TEST_TRACKER_IS_BUSY(tracker_list[4], rearranged[4])

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
              std::placeholders::_1)

#define TRACKER_CONTROLLER_TEARDOWN() tracker_list.clear()

#define TEST_SINGLE_BEGIN()                                                    \
  TRACKER_CONTROLLER_SETUP();                                                  \
  TRACKER_INSERT(0, tracker_0_0);                                              \
                                                                               \
  tracker_controller.enable();                                                 \
  ASSERT_FALSE(tracker_controller.flags() &                                    \
               torrent::TrackerController::mask_send)

#define TEST_SINGLE_END(succeeded, failed)                                     \
  tracker_controller.disable();                                                \
  ASSERT_FALSE(tracker_list.has_active());                                     \
  ASSERT_EQ(success_counter, succeeded);                                       \
  ASSERT_EQ(failure_counter, failure_counter);                                 \
  TRACKER_CONTROLLER_TEARDOWN()

#define TEST_SEND_SINGLE_BEGIN(event_name)                                     \
  tracker_controller.send_##event_name##_event();                              \
  ASSERT_EQ(tracker_controller.flags() &                                       \
              torrent::TrackerController::mask_send,                           \
            torrent::TrackerController::flag_send_##event_name);               \
                                                                               \
  ASSERT_TRUE(tracker_controller.is_active());                                 \
  ASSERT_EQ(tracker_controller.tracker_list()->count_active(), 1)

#define TEST_SEND_SINGLE_END(succeeded, failed)                                \
  TEST_SINGLE_END(succeeded, failed);                                          \
  ASSERT_EQ(tracker_controller.seconds_to_next_timeout(), 0)

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
               torrent::TrackerController::mask_send)

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
               torrent::TrackerController::mask_send)

#define TEST_MULTIPLE_END(succeeded, failed)                                   \
  tracker_controller.disable();                                                \
  ASSERT_FALSE(tracker_list.has_active());                                     \
  ASSERT_EQ(success_counter, succeeded);                                       \
  ASSERT_EQ(failure_counter, failed);                                          \
  TRACKER_CONTROLLER_TEARDOWN()

#define TEST_GOTO_NEXT_SCRAPE(assumed_scrape)                                  \
  ASSERT_TRUE(tracker_controller.task_scrape()->is_queued());                  \
  ASSERT_EQ(assumed_scrape, tracker_controller.seconds_to_next_scrape());      \
  ASSERT_TRUE(test_goto_next_timeout(&tracker_controller, assumed_scrape, true))
