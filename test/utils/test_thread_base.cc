#include <functional>
#include <unistd.h>

#include "torrent/exceptions.h"
#include "torrent/poll_select.h"
#include "torrent/utils/log.h"
#include "torrent/utils/thread_base.h"

#include "test/helpers/fixture.h"
#include "test/helpers/thread.h"
#include "test/helpers/utils.h"

class test_thread_base : public test_fixture {
public:
  void TearDown() override {
    EXPECT_TRUE(torrent::thread_base::trylock_global_lock());
    torrent::thread_base::release_global_lock();
    test_fixture::TearDown();
  };
};

#define TEST_BEGIN(name)                                                       \
  lt_log_print(torrent::LOG_MOCK_CALLS, "thread_base: %s", name);

void
throw_shutdown_exception() {
  throw torrent::shutdown_exception();
}

TEST_F(test_thread_base, test_basic) {
  test_thread thread;

  ASSERT_EQ(thread.flags(), 0);

  ASSERT_FALSE(thread.is_active());
  ASSERT_EQ(thread.global_queue_size(), 0);
  ASSERT_EQ(thread.poll(), nullptr);
}

TEST_F(test_thread_base, test_lifecycle) {
  test_thread thread;

  ASSERT_EQ(thread.state(), torrent::thread_base::STATE_UNKNOWN);
  ASSERT_EQ(thread.test_state(), test_thread::TEST_NONE);

  thread.init_thread();
  ASSERT_EQ(thread.state(), torrent::thread_base::STATE_INITIALIZED);
  ASSERT_TRUE(thread.is_initialized());
  ASSERT_EQ(thread.test_state(), test_thread::TEST_PRE_START);

  thread.set_pre_stop();
  ASSERT_FALSE(wait_for_true(
    [&thread] { return thread.is_test_state(test_thread::TEST_PRE_STOP); }));

  thread.start_thread();
  ASSERT_TRUE(wait_for_true(
    [&thread] { return thread.is_state(test_thread::STATE_ACTIVE); }));
  ASSERT_TRUE(thread.is_active());
  ASSERT_TRUE(wait_for_true(
    [&thread] { return thread.is_test_state(test_thread::TEST_PRE_STOP); }));

  thread.stop_thread();
  ASSERT_TRUE(wait_for_true(
    [&thread] { return thread.is_state(test_thread::STATE_INACTIVE); }));
  ASSERT_TRUE(thread.is_inactive());
}

TEST_F(test_thread_base, test_global_lock_basic) {
  test_thread thread;

  thread.init_thread();
  thread.start_thread();

  ASSERT_EQ(torrent::thread_base::global_queue_size(), 0);

  // Acquire main thread...
  ASSERT_TRUE(torrent::thread_base::trylock_global_lock());
  ASSERT_FALSE(torrent::thread_base::trylock_global_lock());

  torrent::thread_base::release_global_lock();
  ASSERT_TRUE(torrent::thread_base::trylock_global_lock());
  ASSERT_FALSE(torrent::thread_base::trylock_global_lock());

  torrent::thread_base::release_global_lock();
  torrent::thread_base::acquire_global_lock();
  ASSERT_FALSE(torrent::thread_base::trylock_global_lock());

  thread.set_acquire_global();
  ASSERT_FALSE(wait_for_true([&thread] {
    return thread.is_test_flags(test_thread::test_flag_has_global);
  }));

  torrent::thread_base::release_global_lock();
  ASSERT_TRUE(wait_for_true([&thread] {
    return thread.is_test_flags(test_thread::test_flag_has_global);
  }));

  ASSERT_FALSE(torrent::thread_base::trylock_global_lock());
  torrent::thread_base::release_global_lock();
  ASSERT_TRUE(torrent::thread_base::trylock_global_lock());

  // Test waive (loop).

  ASSERT_EQ(torrent::thread_base::global_queue_size(), 0);

  torrent::thread_base::release_global_lock();
  thread.stop_thread();
  ASSERT_TRUE(wait_for_true(
    [&thread] { return thread.is_state(test_thread::STATE_INACTIVE); }));
}

TEST_F(test_thread_base, test_interrupt) {
  test_thread thread;
  thread.set_test_flag(test_thread::test_flag_long_timeout);

  thread.init_thread();
  thread.start_thread();

  // Vary the various timeouts.

  for (int i = 0; i < 100; i++) {
    thread.interrupt();
    usleep(0);

    thread.set_test_flag(test_thread::test_flag_do_work);
    thread.interrupt();

    // Wait for flag to clear.
    ASSERT_TRUE(wait_for_true([&thread] {
      return thread.is_not_test_flags(test_thread::test_flag_do_work);
    }));
  }

  thread.stop_thread();
  ASSERT_TRUE(wait_for_true(
    [&thread] { return thread.is_state(test_thread::STATE_INACTIVE); }));
}

TEST_F(test_thread_base, test_stop) {
  {
    TEST_BEGIN("trylock global lock");
    EXPECT_TRUE(torrent::thread_base::trylock_global_lock());
    // torrent::thread_base::acquire_global_lock();
  };

  for (int i = 0; i < 20; i++) {
    EXPECT_TRUE(!torrent::thread_base::trylock_global_lock());

    test_thread thread;
    thread.set_test_flag(test_thread::test_flag_do_work);

    {
      TEST_BEGIN("init and start thread");
      thread.init_thread();
      thread.start_thread();
    };

    {
      TEST_BEGIN("stop and delete thread");
      thread.stop_thread_wait();
      ASSERT_TRUE(thread.is_inactive());
    }
  }

  {
    TEST_BEGIN("release global lock");
    torrent::thread_base::release_global_lock();
  };
}
