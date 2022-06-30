#include <algorithm>
#include <cstring>
#include <fstream>
#include <functional>
#include <iostream>

#include "torrent/exceptions.h"
#include "torrent/utils/log.h"

#include "test/helpers/fixture.h"

class test_log : public test_fixture {
public:
  void SetUp() {
    // Don't initialize since this creates the group->child connections.
    //  torrent::log_initialize();
    torrent::log_cleanup();
  };

  void TearDown() {
    torrent::log_cleanup();
  };
};

namespace torrent {
typedef std::vector<std::pair<std::string, log_slot>> log_output_list;
extern log_output_list                                log_outputs;
} // namespace torrent

const char*  expected_output = NULL;
unsigned int output_mask;

static void
test_output(const char* output, unsigned int length, unsigned int mask) {
  ASSERT_EQ(std::strcmp(output, expected_output), 0)
    << "'" + std::string(output) + "' != '" + std::string(expected_output) +
         "'";
  ASSERT_EQ(std::strlen(output), length) << "'" + std::string(output) + "'";
  output_mask |= mask;
}

#define LTUNIT_ASSERT_OUTPUT(group, mask, expected, ...)                       \
  output_mask     = 0;                                                         \
  expected_output = expected;                                                  \
  lt_log_print(group, __VA_ARGS__);                                            \
  ASSERT_TRUE(output_mask == (mask));

TEST_F(test_log, test_basic) {
  ASSERT_TRUE(!torrent::log_groups.empty());
  ASSERT_TRUE(torrent::log_groups.size() == torrent::LOG_GROUP_MAX_SIZE);

  ASSERT_TRUE(std::find_if(
                torrent::log_groups.begin(),
                torrent::log_groups.end(),
                std::bind(&torrent::log_group::valid, std::placeholders::_1)) ==
              torrent::log_groups.end());
}

inline void
open_output(const char* name, int mask = 0) {
  torrent::log_open_output(
    name,
    std::bind(
      &::test_output, std::placeholders::_1, std::placeholders::_2, mask));
}

TEST_F(test_log, test_output_open) {
  ASSERT_TRUE(torrent::log_groups[0].size_outputs() == 0);

  // Add test for unknown output names.

  open_output("test_output_1", 0x0);
  torrent::log_add_group_output(0, "test_output_1");

  ASSERT_TRUE(torrent::log_outputs.size() == 1);
  ASSERT_TRUE(torrent::log_outputs[0].first == "test_output_1");
  ASSERT_TRUE(torrent::log_groups[0].outputs() == 0x1);
  ASSERT_TRUE(torrent::log_groups[0].size_outputs() == 1);

  // Test inserting duplicate names, should catch.
  // ASSERT_THROW(torrent::log_open_output("test_output_1",
  // torrent::log_slot());, torrent::input_error);

  // try {
  //   torrent::log_open_output("test_output_1", torrent::log_slot());
  // } catch (torrent::input_error& e) {
  //   return;
  // }

  // ASSERT_TRUE(false);

  // Test more than 64 entries.
}

// Test to make sure we don't call functions when using lt_log_print
// on unused log items.

TEST_F(test_log, test_print) {
  open_output("test_print_1", 0x1);
  open_output("test_print_2", 0x2);
  torrent::log_add_group_output(0, "test_print_1");

  LTUNIT_ASSERT_OUTPUT(0, 0x1, "foo_bar", "foo_bar");
  LTUNIT_ASSERT_OUTPUT(0, 0x1, "foo 123 bar", "foo %i %s", 123, "bar");

  torrent::log_add_group_output(0, "test_print_2");

  LTUNIT_ASSERT_OUTPUT(0, 0x1 | 0x2, "test_multiple", "test_multiple");
}

enum { GROUP_PARENT_1, GROUP_PARENT_2, GROUP_CHILD_1, GROUP_CHILD_1_1 };

TEST_F(test_log, test_children) {
  open_output("test_children_1", 0x1);
  open_output("test_children_2", 0x2);
  torrent::log_add_group_output(GROUP_PARENT_1, "test_children_1");
  torrent::log_add_group_output(GROUP_PARENT_2, "test_children_2");

  torrent::log_add_child(GROUP_PARENT_1, GROUP_CHILD_1);
  torrent::log_add_child(GROUP_CHILD_1, GROUP_CHILD_1_1);

  // std::cout << "cached_output(" <<
  // torrent::log_groups[GROUP_PARENT_1].cached_outputs() << ')';

  LTUNIT_ASSERT_OUTPUT(GROUP_PARENT_1, 0x1, "parent_1", "parent_1");
  LTUNIT_ASSERT_OUTPUT(GROUP_CHILD_1, 0x1, "child_1", "child_1");
  LTUNIT_ASSERT_OUTPUT(GROUP_CHILD_1_1, 0x1, "child_1", "child_1");

  torrent::log_add_child(GROUP_PARENT_2, GROUP_CHILD_1);

  LTUNIT_ASSERT_OUTPUT(GROUP_PARENT_2, 0x2, "parent_2", "parent_2");
  LTUNIT_ASSERT_OUTPUT(GROUP_CHILD_1, 0x3, "child_1", "child_1");
  LTUNIT_ASSERT_OUTPUT(GROUP_CHILD_1_1, 0x3, "child_1", "child_1");
}

TEST_F(test_log, test_file_output) {
  std::string filename = "test_log.XXXXXX";

  auto tmp_fd = -1;
  if ((tmp_fd = mkstemp(&*filename.begin())) < 0) {
    ASSERT_TRUE(false);
  }

  torrent::log_open_file_output("test_file", filename.c_str());
  torrent::log_add_group_output(GROUP_PARENT_1, "test_file");

  lt_log_print(GROUP_PARENT_1, "test_file");

  torrent::log_cleanup(); // To ensure we flush the buffers.

  std::ifstream temp_file(filename.c_str());

  ASSERT_TRUE(temp_file.good());

  char buffer[256];
  temp_file.getline(buffer, 256);

  ASSERT_NE(std::string(buffer).find("test_file"), std::string::npos) << buffer;

  close(tmp_fd);
}

TEST_F(test_log, test_file_output_append) {
  std::string filename = "test_log.XXXXXX";

  auto tmp_fd = -1;
  if ((tmp_fd = mkstemp(&*filename.begin())) < 0) {
    ASSERT_TRUE(false);
  }

  torrent::log_open_file_output("test_file", filename.c_str(), false);
  torrent::log_add_group_output(GROUP_PARENT_1, "test_file");

  lt_log_print(GROUP_PARENT_1, "test_line_1");

  torrent::log_cleanup(); // To ensure we flush the buffers.

  // re-open and write 2nd line
  torrent::log_open_file_output("test_file", filename.c_str(), true);
  torrent::log_add_group_output(GROUP_PARENT_1, "test_file");

  lt_log_print(GROUP_PARENT_1, "test_line_2");

  torrent::log_cleanup(); // To ensure we flush the buffers.

  std::ifstream temp_file(filename.c_str());

  ASSERT_TRUE(temp_file.good());

  char buffer_line1[256];
  temp_file.getline(buffer_line1, 256);

  char buffer_line2[256];
  temp_file.getline(buffer_line2, 256);

  ASSERT_NE(std::string(buffer_line1).find("test_line_1"), std::string::npos)
    << buffer_line1;
  ASSERT_NE(std::string(buffer_line2).find("test_line_2"), std::string::npos)
    << buffer_line2;

  close(tmp_fd);
}
