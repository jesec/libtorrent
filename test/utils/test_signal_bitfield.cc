#include "torrent/exceptions.h"
#include "torrent/utils/signal_bitfield.h"
#include "torrent/utils/thread_base.h"

#include "test/helpers/fixture.h"
#include "test/helpers/thread.h"
#include "test/helpers/utils.h"

class test_signal_bitfield : public test_fixture {
public:
  void TearDown() {
    EXPECT_TRUE(torrent::thread_base::trylock_global_lock());
    torrent::thread_base::release_global_lock();
    test_fixture::TearDown();
  };
};

static void
mark_index(std::atomic<uint32_t>* bitfield, unsigned int index) {
  bitfield->fetch_or(1 << index);
}

static bool
check_index(std::atomic<uint32_t>* bitfield, unsigned int index) {
  return *bitfield & (1 << index);
}

static bool
verify_did_internal_error(const std::function<unsigned int()>& func,
                          bool                                 should_throw) {
  bool did_throw = false;

  try {
    func();
  } catch (torrent::internal_error& e) {
    did_throw = true;
  }

  return should_throw == did_throw;
}

#define SETUP_SIGNAL_BITFIELD()                                                \
  std::atomic<uint32_t>    marked_bitfield = 0;                                \
  torrent::signal_bitfield signal_bitfield;

#define SIGNAL_BITFIELD_DID_INTERNAL_ERROR(verify_slot, did_throw)             \
  ASSERT_TRUE(verify_did_internal_error(                                       \
    std::bind(&torrent::signal_bitfield::add_signal,                           \
              &signal_bitfield,                                                \
              torrent::signal_bitfield::slot_type(verify_slot)),               \
    did_throw));

TEST_F(test_signal_bitfield, test_basic) {
  SETUP_SIGNAL_BITFIELD();

  ASSERT_EQ(torrent::signal_bitfield::max_size,
            sizeof(torrent::signal_bitfield::bitfield_type) * 8);

  SIGNAL_BITFIELD_DID_INTERNAL_ERROR(torrent::signal_bitfield::slot_type(),
                                     true);

  for (unsigned int i = 0; i < torrent::signal_bitfield::max_size; i++)
    ASSERT_TRUE(signal_bitfield.add_signal(
                  std::bind(&mark_index, &marked_bitfield, i)) == i);

  SIGNAL_BITFIELD_DID_INTERNAL_ERROR(
    std::bind(
      &mark_index, &marked_bitfield, torrent::signal_bitfield::max_size),
    true);
}

TEST_F(test_signal_bitfield, test_single) {
  SETUP_SIGNAL_BITFIELD();

  ASSERT_TRUE(signal_bitfield.add_signal(
                std::bind(&mark_index, &marked_bitfield, 0)) == 0);

  signal_bitfield.signal(0);
  ASSERT_EQ(marked_bitfield, 0x0);

  signal_bitfield.work();
  ASSERT_EQ(marked_bitfield, 0x1);

  marked_bitfield = 0;

  signal_bitfield.work();
  ASSERT_EQ(marked_bitfield, 0x0);
}

TEST_F(test_signal_bitfield, test_multiple) {
  SETUP_SIGNAL_BITFIELD();

  for (unsigned int i = 0; i < torrent::signal_bitfield::max_size; i++)
    ASSERT_TRUE(signal_bitfield.add_signal(
                  std::bind(&mark_index, &marked_bitfield, i)) == i);

  signal_bitfield.signal(2);
  signal_bitfield.signal(31);
  ASSERT_EQ(marked_bitfield, 0x0);

  signal_bitfield.work();
  ASSERT_EQ(marked_bitfield,
            (((unsigned int)1 << 2) | ((unsigned int)1 << 31)));

  marked_bitfield = 0;

  signal_bitfield.work();
  ASSERT_EQ(marked_bitfield, 0x0);
}

TEST_F(test_signal_bitfield, test_threaded) {
  std::atomic<uint32_t> marked_bitfield = 0;
  test_thread*          thread          = new test_thread;
  // thread->set_test_flag(test_thread::test_flag_long_timeout);

  for (unsigned int i = 0; i < torrent::signal_bitfield::max_size; i++)
    ASSERT_TRUE(thread->signal_bitfield()->add_signal(
                  std::bind(&mark_index, &marked_bitfield, i)) == i);

  thread->init_thread();
  thread->start_thread();

  // Vary the various timeouts.

  for (int i = 0; i < 100; i++) {
    // thread->interrupt();
    // usleep(0);

    thread->signal_bitfield()->signal(i % 20);
    // thread->interrupt();

    ASSERT_TRUE(
      wait_for_true(std::bind(&check_index, &marked_bitfield, i % 20)));
    marked_bitfield &= ~uint32_t();
  }

  thread->stop_thread();
  ASSERT_TRUE(wait_for_true(
    std::bind(&test_thread::is_state, thread, test_thread::STATE_INACTIVE)));

  delete thread;
}

// Test invalid signal added.
// Test overflow signals added.
// Test multiple signals triggered.

// Stresstest with real thread/polling.
