#include "test/helpers/test_fixture.h"

class test_thread_base : public test_fixture {
public:
  void TearDown() override;
};
