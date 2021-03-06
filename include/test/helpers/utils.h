#ifndef LIBTORRENT_TEST_HELPERS_UTILS_H
#define LIBTORRENT_TEST_HELPERS_UTILS_H

#include <functional>
#include <unistd.h>

inline bool
wait_for_true(const std::function<bool()>& test_function) {
  int i = 100;

  do {
    if (test_function())
      return true;

    usleep(10 * 1000);
  } while (--i);

  return false;
}

#endif
