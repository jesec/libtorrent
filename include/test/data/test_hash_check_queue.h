#include <map>
#include <vector>

#include "data/hash_check_queue.h"
#include "data/hash_queue_node.h"
#include "test/helpers/test_fixture.h"
#include "torrent/hash_string.h"

class test_hash_check_queue : public test_fixture {
  CPPUNIT_TEST_SUITE(test_hash_check_queue);

  CPPUNIT_TEST(test_single);
  CPPUNIT_TEST(test_multiple);
  CPPUNIT_TEST(test_erase);

  CPPUNIT_TEST(test_thread);

  CPPUNIT_TEST_SUITE_END();

public:
  void setUp();

  void test_single();
  void test_multiple();
  void test_erase();

  void test_thread();
};

typedef std::map<int, torrent::HashString> done_chunks_type;
typedef std::vector<torrent::ChunkHandle>  handle_list;

torrent::HashString
hash_for_index(uint32_t index);

bool
verify_hash(const done_chunks_type*    done_chunks,
            int                        index,
            const torrent::HashString& hash);
