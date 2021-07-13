#include <gtest/gtest.h>

#include "globals.h"
#include "torrent/http.h"

#include "test/helpers/tracker.h"

class tracker_list_test : public ::testing::Test {
public:
  void SetUp() {
    torrent::cachedTime = torrent::utils::timer::current();
  };

  void TearDown() {
    torrent::cachedTime = 0;
  };
};

class http_get : public torrent::Http {
public:
  ~http_get() {}

  // Start must never throw on bad input. Calling start/stop on an
  // object in the wrong state should throw a torrent::internal_error.
  void start() {}
  void close() {}
};

torrent::Http*
http_factory() {
  return new http_get;
}

TEST_F(tracker_list_test, test_basic) {
  TRACKER_SETUP();
  TRACKER_INSERT(0, tracker_0);

  ASSERT_EQ(tracker_0, tracker_list[0]);

  ASSERT_EQ(tracker_list[0]->parent(), &tracker_list);
  ASSERT_EQ(
    std::distance(tracker_list.begin_group(0), tracker_list.end_group(0)), 1);
  ASSERT_NE(tracker_list.find_usable(tracker_list.begin()), tracker_list.end());
}

TEST_F(tracker_list_test, test_enable) {
  TRACKER_SETUP();
  int enabled_counter  = 0;
  int disabled_counter = 0;

  tracker_list.slot_tracker_enabled() =
    std::bind(&increment_value_void, &enabled_counter);
  tracker_list.slot_tracker_disabled() =
    std::bind(&increment_value_void, &disabled_counter);

  TRACKER_INSERT(0, tracker_0);
  TRACKER_INSERT(1, tracker_1);
  ASSERT_EQ(enabled_counter, 2);
  ASSERT_EQ(disabled_counter, 0);

  tracker_0->enable();
  tracker_1->enable();
  ASSERT_EQ(enabled_counter, 2);
  ASSERT_EQ(disabled_counter, 0);

  tracker_0->disable();
  tracker_1->enable();
  ASSERT_EQ(enabled_counter, 2);
  ASSERT_EQ(disabled_counter, 1);

  tracker_1->disable();
  tracker_0->disable();
  ASSERT_EQ(enabled_counter, 2);
  ASSERT_EQ(disabled_counter, 2);

  tracker_0->enable();
  tracker_1->enable();
  tracker_0->enable();
  tracker_1->enable();
  ASSERT_EQ(enabled_counter, 4);
  ASSERT_EQ(disabled_counter, 2);
}

TEST_F(tracker_list_test, test_close) {
  TRACKER_SETUP();
  TRACKER_INSERT(0, tracker_0);
  TRACKER_INSERT(0, tracker_1);
  TRACKER_INSERT(0, tracker_2);
  TRACKER_INSERT(0, tracker_3);

  tracker_list.send_state_idx(0, torrent::Tracker::EVENT_NONE);
  tracker_list.send_state_idx(1, torrent::Tracker::EVENT_STARTED);
  tracker_list.send_state_idx(2, torrent::Tracker::EVENT_STOPPED);
  tracker_list.send_state_idx(3, torrent::Tracker::EVENT_COMPLETED);

  ASSERT_TRUE(tracker_0->is_busy());
  ASSERT_TRUE(tracker_1->is_busy());
  ASSERT_TRUE(tracker_2->is_busy());
  ASSERT_TRUE(tracker_3->is_busy());

  tracker_list.close_all_excluding((1 << torrent::Tracker::EVENT_STARTED) |
                                   (1 << torrent::Tracker::EVENT_STOPPED));

  ASSERT_FALSE(tracker_0->is_busy());
  ASSERT_TRUE(tracker_1->is_busy());
  ASSERT_TRUE(tracker_2->is_busy());
  ASSERT_FALSE(tracker_3->is_busy());

  tracker_list.close_all();

  ASSERT_FALSE(tracker_0->is_busy());
  ASSERT_FALSE(tracker_1->is_busy());
  ASSERT_FALSE(tracker_2->is_busy());
  ASSERT_FALSE(tracker_3->is_busy());

  tracker_list.send_state_idx(0, torrent::Tracker::EVENT_NONE);
  tracker_list.send_state_idx(1, torrent::Tracker::EVENT_STARTED);
  tracker_list.send_state_idx(2, torrent::Tracker::EVENT_STOPPED);
  tracker_list.send_state_idx(3, torrent::Tracker::EVENT_COMPLETED);

  tracker_list.close_all_excluding((1 << torrent::Tracker::EVENT_NONE) |
                                   (1 << torrent::Tracker::EVENT_COMPLETED));

  ASSERT_TRUE(tracker_0->is_busy());
  ASSERT_FALSE(tracker_1->is_busy());
  ASSERT_FALSE(tracker_2->is_busy());
  ASSERT_TRUE(tracker_3->is_busy());
}

// Test clear.

TEST_F(tracker_list_test, test_tracker_flags) {
  TRACKER_SETUP();
  tracker_list.insert(0, new TrackerTest(&tracker_list, ""));
  tracker_list.insert(0, new TrackerTest(&tracker_list, "", 0));
  tracker_list.insert(
    0, new TrackerTest(&tracker_list, "", torrent::Tracker::flag_enabled));
  tracker_list.insert(
    0,
    new TrackerTest(&tracker_list, "", torrent::Tracker::flag_extra_tracker));
  tracker_list.insert(0,
                      new TrackerTest(&tracker_list,
                                      "",
                                      torrent::Tracker::flag_enabled |
                                        torrent::Tracker::flag_extra_tracker));

  ASSERT_EQ((tracker_list[0]->flags() & torrent::Tracker::mask_base_flags),
            torrent::Tracker::flag_enabled);
  ASSERT_EQ((tracker_list[1]->flags() & torrent::Tracker::mask_base_flags), 0);
  ASSERT_EQ((tracker_list[2]->flags() & torrent::Tracker::mask_base_flags),
            torrent::Tracker::flag_enabled);
  ASSERT_EQ((tracker_list[3]->flags() & torrent::Tracker::mask_base_flags),
            torrent::Tracker::flag_extra_tracker);
  ASSERT_EQ(tracker_list[4]->flags() & torrent::Tracker::mask_base_flags,
            torrent::Tracker::flag_enabled |
              torrent::Tracker::flag_extra_tracker);
}

TEST_F(tracker_list_test, test_find_url) {
  TRACKER_SETUP();

  tracker_list.insert(0, new TrackerTest(&tracker_list, "http://1"));
  tracker_list.insert(0, new TrackerTest(&tracker_list, "http://2"));
  tracker_list.insert(1, new TrackerTest(&tracker_list, "http://3"));

  ASSERT_EQ(tracker_list.find_url("http://"), tracker_list.end());

  ASSERT_NE(tracker_list.find_url("http://1"), tracker_list.end());
  ASSERT_EQ(*tracker_list.find_url("http://1"), tracker_list[0]);

  ASSERT_NE(tracker_list.find_url("http://2"), tracker_list.end());
  ASSERT_EQ(*tracker_list.find_url("http://2"), tracker_list[1]);

  ASSERT_NE(tracker_list.find_url("http://3"), tracker_list.end());
  ASSERT_EQ(*tracker_list.find_url("http://3"), tracker_list[2]);
}

TEST_F(tracker_list_test, test_can_scrape) {
  TRACKER_SETUP();
  torrent::Http::slot_factory() = std::bind(&http_factory);

  tracker_list.insert_url(0, "http://example.com/announce");
  ASSERT_TRUE(tracker_list.back()->flags() & torrent::Tracker::flag_can_scrape);
  ASSERT_EQ(torrent::Tracker::scrape_url_from(tracker_list.back()->url()),
            "http://example.com/scrape");

  tracker_list.insert_url(0, "http://example.com/x/announce");
  ASSERT_TRUE(tracker_list.back()->flags() & torrent::Tracker::flag_can_scrape);
  ASSERT_EQ(torrent::Tracker::scrape_url_from(tracker_list.back()->url()),
            "http://example.com/x/scrape");

  tracker_list.insert_url(0, "http://example.com/announce.php");
  ASSERT_TRUE(tracker_list.back()->flags() & torrent::Tracker::flag_can_scrape);
  ASSERT_EQ(torrent::Tracker::scrape_url_from(tracker_list.back()->url()),
            "http://example.com/scrape.php");

  tracker_list.insert_url(0, "http://example.com/a");
  ASSERT_FALSE(tracker_list.back()->flags() &
               torrent::Tracker::flag_can_scrape);

  tracker_list.insert_url(0, "http://example.com/announce?x2%0644");
  ASSERT_TRUE(tracker_list.back()->flags() & torrent::Tracker::flag_can_scrape);
  ASSERT_EQ(torrent::Tracker::scrape_url_from(tracker_list.back()->url()),
            "http://example.com/scrape?x2%0644");

  tracker_list.insert_url(0, "http://example.com/announce?x=2/4");
  ASSERT_FALSE(tracker_list.back()->flags() &
               torrent::Tracker::flag_can_scrape);

  tracker_list.insert_url(0, "http://example.com/x%064announce");
  ASSERT_FALSE(tracker_list.back()->flags() &
               torrent::Tracker::flag_can_scrape);
}

TEST_F(tracker_list_test, test_single_success) {
  TRACKER_SETUP();
  TRACKER_INSERT(0, tracker_0);

  ASSERT_FALSE(tracker_0->is_busy());
  ASSERT_FALSE(tracker_0->is_busy_not_scrape());
  ASSERT_FALSE(tracker_0->is_open());
  ASSERT_EQ(tracker_0->requesting_state(), -1);
  ASSERT_EQ(tracker_0->latest_event(), torrent::Tracker::EVENT_NONE);

  tracker_list.send_state_idx(0, torrent::Tracker::EVENT_STARTED);

  ASSERT_TRUE(tracker_0->is_busy());
  ASSERT_TRUE(tracker_0->is_busy_not_scrape());
  ASSERT_TRUE(tracker_0->is_open());
  ASSERT_EQ(tracker_0->requesting_state(), torrent::Tracker::EVENT_STARTED);
  ASSERT_EQ(tracker_0->latest_event(), torrent::Tracker::EVENT_STARTED);

  ASSERT_TRUE(tracker_0->trigger_success());

  ASSERT_FALSE(tracker_0->is_busy());
  ASSERT_FALSE(tracker_0->is_busy_not_scrape());
  ASSERT_FALSE(tracker_0->is_open());
  ASSERT_EQ(tracker_0->requesting_state(), -1);
  ASSERT_EQ(tracker_0->latest_event(), torrent::Tracker::EVENT_STARTED);

  ASSERT_EQ(success_counter, 1);
  ASSERT_EQ(failure_counter, 0);
  ASSERT_EQ(tracker_0->success_counter(), 1);
  ASSERT_EQ(tracker_0->failed_counter(), 0);
}

TEST_F(tracker_list_test, test_single_failure) {
  TRACKER_SETUP();
  TRACKER_INSERT(0, tracker_0);

  tracker_list.send_state_idx(0, 1);
  ASSERT_TRUE(tracker_0->trigger_failure());

  ASSERT_FALSE(tracker_0->is_busy());
  ASSERT_FALSE(tracker_0->is_open());
  ASSERT_EQ(tracker_0->requesting_state(), -1);

  ASSERT_EQ(success_counter, 0);
  ASSERT_EQ(failure_counter, 1);
  ASSERT_EQ(tracker_0->success_counter(), 0);
  ASSERT_EQ(tracker_0->failed_counter(), 1);

  tracker_list.send_state_idx(0, 1);
  ASSERT_TRUE(tracker_0->trigger_success());

  ASSERT_EQ(success_counter, 1);
  ASSERT_EQ(failure_counter, 1);
  ASSERT_EQ(tracker_0->success_counter(), 1);
  ASSERT_EQ(tracker_0->failed_counter(), 0);
}

TEST_F(tracker_list_test, test_single_closing) {
  TRACKER_SETUP();
  TRACKER_INSERT(0, tracker_0);

  ASSERT_FALSE(tracker_0->is_open());

  tracker_0->set_close_on_done(false);
  tracker_list.send_state_idx(0, 1);

  ASSERT_TRUE(tracker_0->is_open());
  ASSERT_TRUE(tracker_0->trigger_success());

  ASSERT_FALSE(tracker_0->is_busy());
  ASSERT_TRUE(tracker_0->is_open());

  tracker_list.close_all();
  tracker_list.clear_stats();

  ASSERT_FALSE(tracker_0->is_open());
  ASSERT_EQ(tracker_0->success_counter(), 0);
  ASSERT_EQ(tracker_0->failed_counter(), 0);
}

TEST_F(tracker_list_test, test_multiple_success) {
  TRACKER_SETUP();
  TRACKER_INSERT(0, tracker_0_0);
  TRACKER_INSERT(0, tracker_0_1);
  TRACKER_INSERT(1, tracker_1_0);
  TRACKER_INSERT(1, tracker_1_1);

  ASSERT_FALSE(tracker_0_0->is_busy());
  ASSERT_FALSE(tracker_0_1->is_busy());
  ASSERT_FALSE(tracker_1_0->is_busy());
  ASSERT_FALSE(tracker_1_1->is_busy());

  tracker_list.send_state_idx(0, 1);

  ASSERT_TRUE(tracker_0_0->is_busy());
  ASSERT_FALSE(tracker_0_1->is_busy());
  ASSERT_FALSE(tracker_1_0->is_busy());
  ASSERT_FALSE(tracker_1_1->is_busy());

  ASSERT_TRUE(tracker_0_0->trigger_success());

  ASSERT_FALSE(tracker_0_0->is_busy());
  ASSERT_FALSE(tracker_0_1->is_busy());
  ASSERT_FALSE(tracker_1_0->is_busy());
  ASSERT_FALSE(tracker_1_1->is_busy());

  tracker_list.send_state_idx(1, 1);
  tracker_list.send_state_idx(3, 1);

  ASSERT_FALSE(tracker_0_0->is_busy());
  ASSERT_TRUE(tracker_0_1->is_busy());
  ASSERT_FALSE(tracker_1_0->is_busy());
  ASSERT_TRUE(tracker_1_1->is_busy());

  ASSERT_TRUE(tracker_1_1->trigger_success());

  ASSERT_FALSE(tracker_0_0->is_busy());
  ASSERT_TRUE(tracker_0_1->is_busy());
  ASSERT_FALSE(tracker_1_0->is_busy());
  ASSERT_FALSE(tracker_1_1->is_busy());

  ASSERT_TRUE(tracker_0_1->trigger_success());

  ASSERT_FALSE(tracker_0_0->is_busy());
  ASSERT_FALSE(tracker_0_1->is_busy());
  ASSERT_FALSE(tracker_1_0->is_busy());
  ASSERT_FALSE(tracker_1_1->is_busy());

  ASSERT_EQ(success_counter, 3);
  ASSERT_EQ(failure_counter, 0);
}

TEST_F(tracker_list_test, test_scrape_success) {
  TRACKER_SETUP();
  TRACKER_INSERT(0, tracker_0);

  tracker_0->set_can_scrape();
  tracker_list.send_scrape(tracker_0);

  ASSERT_TRUE(tracker_0->is_busy());
  ASSERT_FALSE(tracker_0->is_busy_not_scrape());
  ASSERT_TRUE(tracker_0->is_open());
  ASSERT_EQ(tracker_0->requesting_state(), torrent::Tracker::EVENT_SCRAPE);
  ASSERT_EQ(tracker_0->latest_event(), torrent::Tracker::EVENT_SCRAPE);

  ASSERT_TRUE(tracker_0->trigger_scrape());

  ASSERT_FALSE(tracker_0->is_busy());
  ASSERT_FALSE(tracker_0->is_busy_not_scrape());
  ASSERT_FALSE(tracker_0->is_open());
  ASSERT_EQ(tracker_0->requesting_state(), -1);
  ASSERT_EQ(tracker_0->latest_event(), torrent::Tracker::EVENT_SCRAPE);

  ASSERT_EQ(success_counter, 0);
  ASSERT_EQ(failure_counter, 0);
  ASSERT_EQ(scrape_success_counter, 1);
  ASSERT_EQ(scrape_failure_counter, 0);
  ASSERT_EQ(tracker_0->success_counter(), 0);
  ASSERT_EQ(tracker_0->failed_counter(), 0);
  ASSERT_EQ(tracker_0->scrape_counter(), 1);
}

TEST_F(tracker_list_test, test_scrape_failure) {
  TRACKER_SETUP();
  TRACKER_INSERT(0, tracker_0);

  tracker_0->set_can_scrape();
  tracker_list.send_scrape(tracker_0);

  ASSERT_TRUE(tracker_0->trigger_failure());

  ASSERT_FALSE(tracker_0->is_busy());
  ASSERT_FALSE(tracker_0->is_open());
  ASSERT_EQ(tracker_0->requesting_state(), -1);
  ASSERT_EQ(tracker_0->latest_event(), torrent::Tracker::EVENT_SCRAPE);

  ASSERT_EQ(success_counter, 0);
  ASSERT_EQ(failure_counter, 0);
  ASSERT_EQ(scrape_success_counter, 0);
  ASSERT_EQ(scrape_failure_counter, 1);
  ASSERT_EQ(tracker_0->success_counter(), 0);
  ASSERT_EQ(tracker_0->failed_counter(), 0);
  ASSERT_EQ(tracker_0->scrape_counter(), 0);
}

bool
check_has_active_in_group(const torrent::TrackerList* tracker_list,
                          const char*                 states,
                          bool                        scrape) {
  int group = 0;

  while (*states != '\0') {
    bool result = scrape
                    ? tracker_list->has_active_in_group(group++)
                    : tracker_list->has_active_not_scrape_in_group(group++);

    if ((*states == '1' && !result) || (*states == '0' && result))
      return false;

    states++;
  }

  return true;
}

TEST_F(tracker_list_test, test_has_active) {
  TRACKER_SETUP();
  TRACKER_INSERT(0, tracker_0);
  TRACKER_INSERT(0, tracker_1);
  TRACKER_INSERT(1, tracker_2);
  TRACKER_INSERT(3, tracker_3);
  TRACKER_INSERT(4, tracker_4);

  // TODO: Test scrape...

  TEST_TRACKERS_IS_BUSY_5("00000", "00000");
  ASSERT_FALSE(tracker_list.has_active());
  ASSERT_FALSE(tracker_list.has_active_not_scrape());
  ASSERT_TRUE(check_has_active_in_group(&tracker_list, "000000", false));
  ASSERT_TRUE(check_has_active_in_group(&tracker_list, "000000", true));

  tracker_list.send_state_idx(0, 1);
  TEST_TRACKERS_IS_BUSY_5("10000", "10000");
  ASSERT_TRUE(tracker_list.has_active());
  ASSERT_TRUE(tracker_list.has_active_not_scrape());
  ASSERT_TRUE(check_has_active_in_group(&tracker_list, "100000", false));
  ASSERT_TRUE(check_has_active_in_group(&tracker_list, "100000", true));

  ASSERT_TRUE(tracker_0->trigger_success());
  TEST_TRACKERS_IS_BUSY_5("00000", "00000");
  ASSERT_FALSE(tracker_list.has_active());
  ASSERT_FALSE(tracker_list.has_active_not_scrape());
  ASSERT_TRUE(check_has_active_in_group(&tracker_list, "000000", false));
  ASSERT_TRUE(check_has_active_in_group(&tracker_list, "000000", true));

  tracker_list.send_state_idx(1, 1);
  tracker_list.send_state_idx(3, 1);
  TEST_TRACKERS_IS_BUSY_5("01010", "01010");
  ASSERT_TRUE(tracker_list.has_active());
  ASSERT_TRUE(tracker_list.has_active_not_scrape());
  ASSERT_TRUE(check_has_active_in_group(&tracker_list, "100100", false));
  ASSERT_TRUE(check_has_active_in_group(&tracker_list, "100100", true));

  tracker_2->set_can_scrape();
  tracker_list.send_scrape(tracker_2);

  tracker_list.send_state_idx(1, 1);
  tracker_list.send_state_idx(3, 1);
  TEST_TRACKERS_IS_BUSY_5("01110", "01110");
  ASSERT_TRUE(tracker_list.has_active());
  ASSERT_TRUE(tracker_list.has_active_not_scrape());
  ASSERT_TRUE(check_has_active_in_group(&tracker_list, "100100", false));
  ASSERT_TRUE(check_has_active_in_group(&tracker_list, "110100", true));

  ASSERT_TRUE(tracker_1->trigger_success());
  TEST_TRACKERS_IS_BUSY_5("00110", "00110");
  ASSERT_TRUE(tracker_list.has_active());
  ASSERT_TRUE(tracker_list.has_active_not_scrape());
  ASSERT_TRUE(check_has_active_in_group(&tracker_list, "000100", false));
  ASSERT_TRUE(check_has_active_in_group(&tracker_list, "010100", true));

  ASSERT_TRUE(tracker_2->trigger_scrape());
  ASSERT_TRUE(tracker_3->trigger_success());
  TEST_TRACKERS_IS_BUSY_5("00000", "00000");
  ASSERT_FALSE(tracker_list.has_active());
  ASSERT_FALSE(tracker_list.has_active_not_scrape());
  ASSERT_TRUE(check_has_active_in_group(&tracker_list, "000000", false));
  ASSERT_TRUE(check_has_active_in_group(&tracker_list, "000000", true));
}
