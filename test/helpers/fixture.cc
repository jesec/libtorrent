#include "torrent/utils/log.h"

#include "test/helpers/fixture.h"

void
test_fixture::SetUp() {
  mock_init();

  log_add_group_output(torrent::LOG_CONNECTION_BIND, "test_output");
  log_add_group_output(torrent::LOG_CONNECTION_FD, "test_output");
}

void
test_fixture::TearDown() {
  mock_cleanup();
}
