#include <chrono>
#include <csignal>
#include <cstring>
#include <thread>

#include "torrent/exceptions.h"
#include "torrent/poll.h"
#include "torrent/utils/log.h"
#include "torrent/utils/thread_base.h"
#include "torrent/utils/thread_interrupt.h"
#include "utils/instrumentation.h"

namespace torrent {

thread_base::global_lock_type lt_cacheline_aligned
  thread_base::m_global{ 0, std::mutex() };

thread_base::thread_base()
  : m_instrumentation_index(INSTRUMENTATION_POLLING_DO_POLL_OTHERS -
                            INSTRUMENTATION_POLLING_DO_POLL) {
  thread_interrupt::pair_type interrupt_sockets =
    thread_interrupt::create_pair();

  m_interrupt_sender   = interrupt_sockets.first;
  m_interrupt_receiver = interrupt_sockets.second;
}

thread_base::~thread_base() {
  if (m_thread) {
    if (m_thread->joinable()) {
      m_thread->join();
    }
  }

  delete m_interrupt_sender;
  delete m_interrupt_receiver;
  delete m_poll;
}

void
thread_base::start_thread() {
  if (m_poll == nullptr) {
    throw internal_error("No poll object for thread defined.");
  }

  if (!is_initialized()) {
    throw internal_error("Wrong state.");
  }

  try {
    m_thread    = std::make_unique<std::thread>(thread_base::event_loop, this);
    m_thread_id = m_thread->get_id();
  } catch (std::system_error&) {
    throw internal_error("Failed to create thread.");
  }
}

void
thread_base::stop_thread() {
  m_flags |= flag_do_shutdown;
  interrupt();
}

void
thread_base::stop_thread_wait() {
  stop_thread();

  release_global_lock();

  while (!is_inactive()) {
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
  }

  acquire_global_lock();
}

// Fix interrupting when shutting down thread.
void
thread_base::interrupt() {
  // Only poke when polling, set no_timeout
  if (is_polling())
    m_interrupt_sender->poke();
}

bool
thread_base::should_handle_sigusr1() {
  return false;
}

void*
thread_base::event_loop(thread_base* thread) {
  thread->m_state = STATE_ACTIVE;

  lt_log_print(
    torrent::LOG_THREAD_NOTICE, "%s: Starting thread.", thread->name());

  try {
    thread->m_poll->insert_read(thread->m_interrupt_receiver);

    while (true) {
      if (thread->m_slot_do_work)
        thread->m_slot_do_work();

      thread->call_events();
      thread->signal_bitfield()->work();

      thread->m_flags |= flag_polling;

      // Call again after setting flag_polling to ensure we process
      // any events set while it was working.
      if (thread->m_slot_do_work)
        thread->m_slot_do_work();

      thread->call_events();
      thread->signal_bitfield()->work();

      uint64_t next_timeout = 0;

      if (!thread->has_no_timeout()) {
        next_timeout = thread->next_timeout_usec();

        if (thread->m_slot_next_timeout)
          next_timeout = std::min(next_timeout, thread->m_slot_next_timeout());
      }

      // Add the sleep call when testing interrupts, etc.
      // std::this_thread::sleep_for(std::chrono::microseconds(50));

      int poll_flags = 0;

      if (!(thread->flags() & flag_main_thread))
        poll_flags = torrent::Poll::poll_worker_thread;

      instrumentation_update(INSTRUMENTATION_POLLING_DO_POLL, 1);
      instrumentation_update(
        instrumentation_enum(INSTRUMENTATION_POLLING_DO_POLL +
                             thread->m_instrumentation_index),
        1);

      int event_count = thread->m_poll->do_poll(next_timeout, poll_flags);

      instrumentation_update(INSTRUMENTATION_POLLING_EVENTS, event_count);
      instrumentation_update(
        instrumentation_enum(INSTRUMENTATION_POLLING_EVENTS +
                             thread->m_instrumentation_index),
        event_count);

      thread->m_flags &= ~(flag_polling | flag_no_timeout);
    }

    thread->m_poll->remove_write(thread->m_interrupt_receiver);
  } catch (torrent::shutdown_exception& e) {
    lt_log_print(
      torrent::LOG_THREAD_NOTICE, "%s: Shutting down thread.", thread->name());
  }

  thread->m_state = STATE_INACTIVE;
  return nullptr;
}

} // namespace torrent
