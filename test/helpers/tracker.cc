#include <iostream>

#include "globals.h"
#include "net/address_list.h"

#include "test/helpers/tracker.h"

uint32_t return_new_peers = 0xdeadbeef;

bool
TrackerTest::trigger_success(uint32_t new_peers, uint32_t sum_peers) {
  torrent::TrackerList::address_list address_list;

  for (unsigned int i = 0; i != sum_peers; i++) {
    torrent::utils::socket_address sa;
    sa.sa_inet()->clear();
    sa.sa_inet()->set_port(0x100 + i);
    address_list.push_back(sa);
  }

  return trigger_success(&address_list, new_peers);
}

bool
TrackerTest::trigger_success(torrent::TrackerList::address_list* address_list,
                             uint32_t                            new_peers) {
  if (parent() == nullptr || !is_busy() || !is_open())
    return false;

  m_busy           = false;
  m_open           = !(m_flags & flag_close_on_done);
  return_new_peers = new_peers;

  if (m_latest_event == EVENT_SCRAPE) {
    parent()->receive_scrape_success(this);
  } else {
    set_normal_interval(default_normal_interval);
    set_min_interval(default_min_interval);
    parent()->receive_success(this, address_list);
  }

  m_requesting_state = -1;
  return true;
}

bool
TrackerTest::trigger_failure() {
  if (parent() == nullptr || !is_busy() || !is_open())
    return false;

  m_busy           = false;
  m_open           = !(m_flags & flag_close_on_done);
  return_new_peers = 0;

  if (m_latest_event == EVENT_SCRAPE) {
    parent()->receive_scrape_failed(this, "failed");
  } else {
    set_normal_interval(0);
    set_min_interval(0);
    parent()->receive_failed(this, "failed");
  }

  m_requesting_state = -1;
  return true;
}

bool
TrackerTest::trigger_scrape() {
  if (parent() == nullptr || !is_busy() || !is_open())
    return false;

  if (m_latest_event != EVENT_SCRAPE)
    return false;

  return trigger_success();
}

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
