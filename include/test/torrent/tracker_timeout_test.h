#include <gtest/gtest.h>

#include "torrent/tracker_controller.h"

class tracker_timeout_test : public ::testing::Test {
public:
  void SetUp() override;
  void TearDown() override;
};
