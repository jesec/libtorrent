#include "torrent/chunk_manager.h"
#include "torrent/exceptions.h"
#include "torrent/utils/error_number.h"

#include "test/helpers/chunk.h"
#include "test/helpers/fixture.h"

class test_chunk_list : public test_fixture {};

TEST_F(test_chunk_list, test_basic) {
  torrent::ChunkManager chunk_manager;
  torrent::ChunkList    chunk_list;

  ASSERT_EQ(chunk_list.flags(), 0);
  ASSERT_EQ(chunk_list.chunk_size(), 0);

  chunk_list.set_chunk_size(1 << 16);
  chunk_list.set_manager(&chunk_manager);
  chunk_list.resize(32);

  ASSERT_EQ(chunk_list.size(), 32);
  ASSERT_EQ(chunk_list.chunk_size(), (1 << 16));

  for (unsigned int i = 0; i < 32; i++)
    ASSERT_EQ(chunk_list[i].index(), i);
}

TEST_F(test_chunk_list, test_get_release) {
  SETUP_CHUNK_LIST();

  ASSERT_FALSE((*chunk_list)[0].is_valid());

  torrent::ChunkHandle handle_0 = chunk_list->get(0);

  ASSERT_NE(handle_0.object(), nullptr);
  ASSERT_EQ(handle_0.object()->index(), 0);
  ASSERT_EQ(handle_0.index(), 0);
  ASSERT_FALSE(handle_0.is_writable());
  ASSERT_FALSE(handle_0.is_blocking());

  ASSERT_TRUE((*chunk_list)[0].is_valid());
  ASSERT_EQ((*chunk_list)[0].references(), 1);
  ASSERT_EQ((*chunk_list)[0].writable(), 0);
  ASSERT_EQ((*chunk_list)[0].blocking(), 0);

  chunk_list->release(&handle_0);

  torrent::ChunkHandle handle_1 =
    chunk_list->get(1, torrent::ChunkList::get_writable);

  ASSERT_NE(handle_1.object(), nullptr);
  ASSERT_EQ(handle_1.object()->index(), 1);
  ASSERT_EQ(handle_1.index(), 1);
  ASSERT_TRUE(handle_1.is_writable());
  ASSERT_FALSE(handle_1.is_blocking());

  ASSERT_TRUE((*chunk_list)[1].is_valid());
  ASSERT_EQ((*chunk_list)[1].references(), 1);
  ASSERT_EQ((*chunk_list)[1].writable(), 1);
  ASSERT_EQ((*chunk_list)[1].blocking(), 0);

  chunk_list->release(&handle_1);

  torrent::ChunkHandle handle_2 =
    chunk_list->get(2, torrent::ChunkList::get_blocking);

  ASSERT_NE(handle_2.object(), nullptr);
  ASSERT_EQ(handle_2.object()->index(), 2);
  ASSERT_EQ(handle_2.index(), 2);
  ASSERT_FALSE(handle_2.is_writable());
  ASSERT_TRUE(handle_2.is_blocking());

  ASSERT_TRUE((*chunk_list)[2].is_valid());
  ASSERT_EQ((*chunk_list)[2].references(), 1);
  ASSERT_EQ((*chunk_list)[2].writable(), 0);
  ASSERT_EQ((*chunk_list)[2].blocking(), 1);

  chunk_list->release(&handle_2);

  // Test ro->wr, etc.

  CLEANUP_CHUNK_LIST();
}

// Make sure we can't go into writable when blocking, etc.
TEST_F(test_chunk_list, test_blocking) {
  SETUP_CHUNK_LIST();

  torrent::ChunkHandle handle_0_ro =
    chunk_list->get(0, torrent::ChunkList::get_blocking);
  ASSERT_TRUE(handle_0_ro.is_valid());

  // Test writable, etc, on blocking without get_nonblock using a
  // timer on other thread.
  // torrent::ChunkHandle handle_1 = chunk_list->get(0,
  // torrent::ChunkList::get_writable);

  torrent::ChunkHandle handle_0_rw = chunk_list->get(
    0, torrent::ChunkList::get_writable | torrent::ChunkList::get_nonblock);
  ASSERT_FALSE(handle_0_rw.is_valid());
  ASSERT_EQ(handle_0_rw.error_number(),
            std::errc::resource_unavailable_try_again);

  chunk_list->release(&handle_0_ro);

  handle_0_rw = chunk_list->get(0, torrent::ChunkList::get_writable);
  ASSERT_TRUE(handle_0_rw.is_valid());

  chunk_list->release(&handle_0_rw);

  CLEANUP_CHUNK_LIST();
}
