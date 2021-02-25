#include <functional>
#include <unistd.h>

#include "torrent/exceptions.h"
#include "torrent/poll_select.h"
#include "torrent/utils/log.h"
#include "torrent/utils/thread_base.h"

#include "test/torrent/utils/test_thread_base.h"

#include "test/helpers/test_thread.h"
#include "test/helpers/test_utils.h"

#define TEST_BEGIN(name)                                                       \
  lt_log_print(torrent::LOG_MOCK_CALLS, "thread_base: %s", name);

void
throw_shutdown_exception() {
  throw torrent::shutdown_exception();
}

void
test_thread_base::TearDown() {
  EXPECT_TRUE(torrent::thread_base::trylock_global_lock());
  torrent::thread_base::release_global_lock();
  test_fixture::TearDown();
}

TEST_F(test_thread_base, test_basic) {
  test_thread* thread = new test_thread;

  ASSERT_TRUE(thread->flags() == 0);

  ASSERT_TRUE(!thread->is_active());
  ASSERT_TRUE(thread->global_queue_size() == 0);
  ASSERT_TRUE(thread->poll() == NULL);

  // Check active...

  delete thread;
}

TEST_F(test_thread_base, test_lifecycle) {
  test_thread* thread = new test_thread;

  ASSERT_TRUE(thread->state() == torrent::thread_base::STATE_UNKNOWN);
  ASSERT_TRUE(thread->test_state() == test_thread::TEST_NONE);

  thread->init_thread();
  ASSERT_TRUE(thread->state() == torrent::thread_base::STATE_INITIALIZED);
  ASSERT_TRUE(thread->is_initialized());
  ASSERT_TRUE(thread->test_state() == test_thread::TEST_PRE_START);

  thread->set_pre_stop();
  ASSERT_TRUE(!wait_for_true(std::bind(
    &test_thread::is_test_state, thread, test_thread::TEST_PRE_STOP)));

  thread->start_thread();
  ASSERT_TRUE(wait_for_true(
    std::bind(&test_thread::is_state, thread, test_thread::STATE_ACTIVE)));
  ASSERT_TRUE(thread->is_active());
  ASSERT_TRUE(wait_for_true(std::bind(
    &test_thread::is_test_state, thread, test_thread::TEST_PRE_STOP)));

  thread->stop_thread();
  ASSERT_TRUE(wait_for_true(
    std::bind(&test_thread::is_state, thread, test_thread::STATE_INACTIVE)));
  ASSERT_TRUE(thread->is_inactive());

  delete thread;
}

TEST_F(test_thread_base, test_global_lock_basic) {
  test_thread* thread = new test_thread;

  thread->init_thread();
  thread->start_thread();

  ASSERT_TRUE(torrent::thread_base::global_queue_size() == 0);

  // Acquire main thread...
  EXPECT_TRUE(torrent::thread_base::trylock_global_lock());
  EXPECT_TRUE(!torrent::thread_base::trylock_global_lock());

  torrent::thread_base::release_global_lock();
  EXPECT_TRUE(torrent::thread_base::trylock_global_lock());
  EXPECT_TRUE(!torrent::thread_base::trylock_global_lock());

  torrent::thread_base::release_global_lock();
  torrent::thread_base::acquire_global_lock();
  EXPECT_TRUE(!torrent::thread_base::trylock_global_lock());

  thread->set_acquire_global();
  ASSERT_TRUE(!wait_for_true(std::bind(
    &test_thread::is_test_flags, thread, test_thread::test_flag_has_global)));

  torrent::thread_base::release_global_lock();
  ASSERT_TRUE(wait_for_true(std::bind(
    &test_thread::is_test_flags, thread, test_thread::test_flag_has_global)));

  ASSERT_TRUE(!torrent::thread_base::trylock_global_lock());
  torrent::thread_base::release_global_lock();
  EXPECT_TRUE(torrent::thread_base::trylock_global_lock());

  // Test waive (loop).

  ASSERT_TRUE(torrent::thread_base::global_queue_size() == 0);

  torrent::thread_base::release_global_lock();
  thread->stop_thread();
  ASSERT_TRUE(wait_for_true(
    std::bind(&test_thread::is_state, thread, test_thread::STATE_INACTIVE)));

  delete thread;
}

TEST_F(test_thread_base, test_interrupt) {
  test_thread* thread = new test_thread;
  thread->set_test_flag(test_thread::test_flag_long_timeout);

  thread->init_thread();
  thread->start_thread();

  // Vary the various timeouts.

  for (int i = 0; i < 100; i++) {
    thread->interrupt();
    usleep(0);

    thread->set_test_flag(test_thread::test_flag_do_work);
    thread->interrupt();

    // Wait for flag to clear.
    ASSERT_TRUE(wait_for_true(std::bind(&test_thread::is_not_test_flags,
                                        thread,
                                        test_thread::test_flag_do_work)));
  }

  thread->stop_thread();
  ASSERT_TRUE(wait_for_true(
    std::bind(&test_thread::is_state, thread, test_thread::STATE_INACTIVE)));

  delete thread;
}

TEST_F(test_thread_base, test_stop) {
  {
    TEST_BEGIN("trylock global lock");
    EXPECT_TRUE(torrent::thread_base::trylock_global_lock());
    // torrent::thread_base::acquire_global_lock();
  };

  for (int i = 0; i < 20; i++) {
    EXPECT_TRUE(!torrent::thread_base::trylock_global_lock());

    test_thread* thread = new test_thread;
    thread->set_test_flag(test_thread::test_flag_do_work);

    {
      TEST_BEGIN("init and start thread");
      thread->init_thread();
      thread->start_thread();
    };

    {
      TEST_BEGIN("stop and delete thread");
      thread->stop_thread_wait();
      ASSERT_TRUE(thread->is_inactive());

      delete thread;
    }
  }

  {
    TEST_BEGIN("release global lock");
    torrent::thread_base::release_global_lock();
  };
}
