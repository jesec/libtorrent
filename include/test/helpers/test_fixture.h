#ifndef LIBTORRENT_HELPER_TEST_FIXTURE_H
#define LIBTORRENT_HELPER_TEST_FIXTURE_H

#include <gtest/gtest.h>

#include "test/helpers/mock_function.h"

class test_fixture : public ::testing::Test {
public:
  void SetUp() override;
  void TearDown() override;
};

#endif
