#include "torrent/download.h"
#include "torrent/utils/log.h"
#include "torrent/utils/option_strings.h"

#include "test/helpers/fixture.h"

class test_option_strings : public test_fixture {};

#define TEST_ENTRY(group, name, value)                                         \
  {                                                                            \
    lt_log_print(torrent::LOG_MOCK_CALLS, "option_string: %s", name);          \
    std::string result(torrent::option_as_string(torrent::group, value));      \
    ASSERT_EQ(result, name) << "Not found '" + result + "'";                   \
    ASSERT_EQ(torrent::option_find_string(torrent::group, name), value);       \
  }

TEST_F(test_option_strings, test_entries) {
  TEST_ENTRY(
    OPTION_CONNECTION_TYPE, "leech", torrent::Download::CONNECTION_LEECH);
  TEST_ENTRY(
    OPTION_CONNECTION_TYPE, "seed", torrent::Download::CONNECTION_SEED);
  TEST_ENTRY(OPTION_CONNECTION_TYPE,
             "initial_seed",
             torrent::Download::CONNECTION_INITIAL_SEED);
  TEST_ENTRY(
    OPTION_CONNECTION_TYPE, "metadata", torrent::Download::CONNECTION_METADATA);

  TEST_ENTRY(OPTION_LOG_GROUP, "critical", torrent::LOG_CRITICAL);
  TEST_ENTRY(OPTION_LOG_GROUP, "storage_notice", torrent::LOG_STORAGE_NOTICE);
  TEST_ENTRY(OPTION_LOG_GROUP, "torrent_debug", torrent::LOG_TORRENT_DEBUG);
}
