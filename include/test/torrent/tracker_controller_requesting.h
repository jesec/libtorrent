#include <gtest/gtest.h>

#include "test/torrent/tracker_controller_test.h"

class tracker_controller_requesting : public ::testing::Test {
public:
  void SetUp() override;
  void TearDown() override;
};
