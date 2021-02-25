#include <csignal>
#include <functional>

#include "data/hash_queue.h"
#include "data/hash_queue_node.h"
#include "globals.h"
#include "thread_disk.h"
#include "torrent/chunk_manager.h"
#include "torrent/exceptions.h"
#include "torrent/hash_string.h"
#include "torrent/poll_select.h"

#include "test/data/test_chunk_list.h"
#include "test/data/test_hash_check_queue.h"
#include "test/data/test_hash_queue.h"

#include "test/helpers/test_thread.h"
#include "test/helpers/test_utils.h"

typedef std::map<int, torrent::HashString> done_chunks_type;

static void
chunk_done(torrent::ChunkList*  chunk_list,
           done_chunks_type*    done_chunks,
           torrent::ChunkHandle handle,
           const char*          hash_value) {
  if (hash_value != NULL)
    (*done_chunks)[handle.index()] =
      *torrent::HashString::cast_from(hash_value);

  chunk_list->release(&handle);
}

bool
check_for_chunk_done(torrent::HashQueue* hash_queue,
                     done_chunks_type*   done_chunks,
                     int                 index) {
  hash_queue->work();
  return done_chunks->find(index) != done_chunks->end();
}

static torrent::Poll*
create_select_poll() {
  return torrent::PollSelect::create(256);
}

static void
do_nothing() {}

void
test_hash_queue::SetUp() {
  test_fixture::SetUp();

  ASSERT_TRUE(torrent::taskScheduler.empty());

  torrent::Poll::slot_create_poll() = std::bind(&create_select_poll);
  signal(SIGUSR1, (sig_t)&do_nothing);
}

void
test_hash_queue::TearDown() {
  torrent::taskScheduler.clear();
  test_fixture::TearDown();
}

static void
fill_queue() {}

TEST_F(test_hash_queue, test_single) {
  SETUP_CHUNK_LIST();
  SETUP_THREAD();
  thread_disk->start_thread();

  done_chunks_type    done_chunks;
  torrent::HashQueue* hash_queue = new torrent::HashQueue(thread_disk);
  hash_queue->slot_has_work()    = std::bind(&fill_queue);

  torrent::ChunkHandle handle_0 =
    chunk_list->get(0, torrent::ChunkList::get_blocking);
  hash_queue->push_back(handle_0,
                        NULL,
                        std::bind(&chunk_done,
                                  chunk_list,
                                  &done_chunks,
                                  std::placeholders::_1,
                                  std::placeholders::_2));

  ASSERT_TRUE(hash_queue->size() == 1);
  ASSERT_TRUE(hash_queue->front().handle().is_blocking());
  ASSERT_TRUE(hash_queue->front().handle().object() == &((*chunk_list)[0]));

  hash_queue->work();

  ASSERT_TRUE(wait_for_true(
    std::bind(&check_for_chunk_done, hash_queue, &done_chunks, 0)));
  ASSERT_TRUE(done_chunks[0] == hash_for_index(0));

  // chunk_list->release(&handle_0);

  ASSERT_TRUE(thread_disk->hash_queue()->empty());
  delete hash_queue;

  thread_disk->stop_thread();
  CLEANUP_THREAD();
  CLEANUP_CHUNK_LIST();
}

TEST_F(test_hash_queue, test_multiple) {
  SETUP_CHUNK_LIST();
  SETUP_THREAD();
  thread_disk->start_thread();

  done_chunks_type    done_chunks;
  torrent::HashQueue* hash_queue = new torrent::HashQueue(thread_disk);
  hash_queue->slot_has_work()    = std::bind(&fill_queue);

  for (unsigned int i = 0; i < 20; i++) {
    hash_queue->push_back(chunk_list->get(i, torrent::ChunkList::get_blocking),
                          NULL,
                          std::bind(&chunk_done,
                                    chunk_list,
                                    &done_chunks,
                                    std::placeholders::_1,
                                    std::placeholders::_2));

    ASSERT_TRUE(hash_queue->size() == i + 1);
    ASSERT_TRUE(hash_queue->back().handle().is_blocking());
    ASSERT_TRUE(hash_queue->back().handle().object() == &((*chunk_list)[i]));
  }

  for (unsigned int i = 0; i < 20; i++) {
    ASSERT_TRUE(wait_for_true(
      std::bind(&check_for_chunk_done, hash_queue, &done_chunks, i)));
    ASSERT_TRUE(done_chunks[i] == hash_for_index(i));
  }

  ASSERT_TRUE(thread_disk->hash_queue()->empty());
  delete hash_queue;

  thread_disk->stop_thread();
  CLEANUP_THREAD();
  CLEANUP_CHUNK_LIST();
}

TEST_F(test_hash_queue, test_erase) {
  SETUP_CHUNK_LIST();
  SETUP_THREAD();

  torrent::HashQueue* hash_queue = new torrent::HashQueue(thread_disk);
  hash_queue->slot_has_work()    = std::bind(&fill_queue);

  done_chunks_type done_chunks;

  for (unsigned int i = 0; i < 20; i++) {
    hash_queue->push_back(chunk_list->get(i, torrent::ChunkList::get_blocking),
                          NULL,
                          std::bind(&chunk_done,
                                    chunk_list,
                                    &done_chunks,
                                    std::placeholders::_1,
                                    std::placeholders::_2));

    ASSERT_TRUE(hash_queue->size() == i + 1);
  }

  hash_queue->remove(NULL);
  ASSERT_TRUE(hash_queue->empty());

  ASSERT_TRUE(thread_disk->hash_queue()->empty());
  delete hash_queue;
  delete thread_disk;

  CLEANUP_CHUNK_LIST();
}

TEST_F(test_hash_queue, test_erase_stress) {
  SETUP_CHUNK_LIST();
  SETUP_THREAD();
  thread_disk->start_thread();

  torrent::HashQueue* hash_queue = new torrent::HashQueue(thread_disk);
  hash_queue->slot_has_work()    = std::bind(&fill_queue);

  done_chunks_type done_chunks;

  for (unsigned int i = 0; i < 1000; i++) {
    for (unsigned int i = 0; i < 20; i++) {
      hash_queue->push_back(
        chunk_list->get(i, torrent::ChunkList::get_blocking),
        NULL,
        std::bind(&chunk_done,
                  chunk_list,
                  &done_chunks,
                  std::placeholders::_1,
                  std::placeholders::_2));

      ASSERT_TRUE(hash_queue->size() == i + 1);
    }

    hash_queue->remove(NULL);
    ASSERT_TRUE(hash_queue->empty());
  }

  ASSERT_TRUE(thread_disk->hash_queue()->empty());
  delete hash_queue;

  thread_disk->stop_thread();
  CLEANUP_THREAD();
  CLEANUP_CHUNK_LIST();
}

// Test erase of different id's.

// Current code doesn't work well if we remove a hash...
