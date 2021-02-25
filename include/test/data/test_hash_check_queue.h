#include <map>
#include <vector>

#include "data/hash_check_queue.h"
#include "data/hash_queue_node.h"
#include "test/helpers/test_fixture.h"
#include "torrent/hash_string.h"

class test_hash_check_queue : public test_fixture {
public:
  void SetUp() override;
};

typedef std::map<int, torrent::HashString> done_chunks_type;
typedef std::vector<torrent::ChunkHandle>  handle_list;

torrent::HashString
hash_for_index(uint32_t index);

bool
verify_hash(const done_chunks_type*    done_chunks,
            int                        index,
            const torrent::HashString& hash);
