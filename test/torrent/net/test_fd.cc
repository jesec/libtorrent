#include "test/torrent/net/test_fd.h"

#include "torrent/net/fd.h"

TEST_F(test_fd, test_valid_flags) {
  ASSERT_TRUE(torrent::fd_valid_flags(torrent::fd_flag_stream));
  ASSERT_TRUE(torrent::fd_valid_flags(torrent::fd_flag_stream |
                                      torrent::fd_flag_nonblock));
  ASSERT_TRUE(torrent::fd_valid_flags(torrent::fd_flag_stream |
                                      torrent::fd_flag_reuse_address));
  ASSERT_TRUE(
    torrent::fd_valid_flags(torrent::fd_flag_stream | torrent::fd_flag_v4only));
  ASSERT_TRUE(
    torrent::fd_valid_flags(torrent::fd_flag_stream | torrent::fd_flag_v6only));

  ASSERT_FALSE(
    torrent::fd_valid_flags(torrent::fd_flag_v4only | torrent::fd_flag_v6only));
  ASSERT_FALSE(torrent::fd_valid_flags(torrent::fd_flag_stream |
                                       torrent::fd_flag_v4only |
                                       torrent::fd_flag_v6only));

  ASSERT_FALSE(torrent::fd_valid_flags(torrent::fd_flags()));
  ASSERT_TRUE(
    !torrent::fd_valid_flags(torrent::fd_flags(~torrent::fd_flag_all)));
  ASSERT_FALSE(torrent::fd_valid_flags(
    torrent::fd_flags(torrent::fd_flag_stream | ~torrent::fd_flag_all)));
  ASSERT_FALSE(torrent::fd_valid_flags(torrent::fd_flags(0x3245132)));
}
