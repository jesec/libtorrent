#include "globals.h"
#include "torrent/utils/random.h"

#include "test/helpers/fixture.h"

class test_random : public test_fixture {};

TEST_F(test_random, test_basic) {
  for (int i = 0; i < 100; ++i) {
    ASSERT_NO_THROW(torrent::random_int32());
    ASSERT_NO_THROW(torrent::random_int64());
    ASSERT_NO_THROW(torrent::random_uint8());
    ASSERT_NO_THROW(torrent::random_uint32());
    ASSERT_NO_THROW(torrent::random_char());

    auto rand_char = torrent::random_uniform_char('0', '9');
    ASSERT_GE(rand_char, '0');
    ASSERT_LE(rand_char, '9');
  }
}

TEST_F(test_random, test_handshake_key) {
  auto length = torrent::random_uniform_uint32(0, 512 - 1);
  auto pad    = static_cast<char*>(malloc(length * sizeof(char)));

  uint32_t cur = 0;
  while (cur < length) {
    auto remaining  = length - cur;
    auto chunk_size = remaining > 8 ? 8 : remaining;

    auto rand_int64 = torrent::random_int64();
    std::memcpy(pad + cur, &rand_int64, chunk_size);

    cur += chunk_size;
  }

  ASSERT_EQ(cur, length);

  free(pad);
}
