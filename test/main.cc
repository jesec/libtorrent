#include <gtest/gtest.h>

#include "torrent/utils/log.h"
#include "torrent/utils/log_buffer.h"

class progress_listener : public ::testing::EmptyTestEventListener {
private:
  torrent::log_buffer_ptr m_current_log_buffer;

  void OnTestStart(const ::testing::TestInfo&) override {
    torrent::log_cleanup();
    m_current_log_buffer = torrent::log_open_log_buffer("test_output");
  }

  void OnTestPartResult(
    const ::testing::TestPartResult& test_part_result) override {
    if (test_part_result.failed()) {
      for (const auto& entry : *m_current_log_buffer) {
        std::cout << entry.timestamp << ' ' << entry.message << '\n';
      }
    }
  }

  void OnTestEnd(const ::testing::TestInfo&) override {
    m_current_log_buffer.reset();
    torrent::log_cleanup();
  }
};

int
main(int argc, char* argv[]) {
  ::testing::InitGoogleTest(&argc, argv);

  ::testing::UnitTest::GetInstance()->listeners().Append(new progress_listener);

  return RUN_ALL_TESTS();
}
