#include <sstream>

#include <gtest/gtest.h>

#include "torrent/object_stream.h"

#include "test/helpers/bencode.h"

torrent::Object
create_bencode(const char* str) {
  torrent::Object   obj;
  std::stringstream stream(str);

  stream >> obj;

  EXPECT_EQ(stream.fail(), false);
  return obj;
}

torrent::Object
create_bencode_c(const char* str) {
  torrent::Object obj;
  const char*     last = str + strlen(str);

  EXPECT_EQ(object_read_bencode_c(str, last, &obj), last);
  return obj;
}

bool
validate_bencode(const char* first, const char* last) {
  torrent::Object obj;
  return object_read_bencode_c(first, last, &obj) == last;
}

bool
compare_bencode(const torrent::Object& obj,
                const char*            str,
                uint32_t               skip_mask) {
  char buffer[256];
  std::memset(buffer, 0, 256);
  torrent::object_write_bencode(buffer, buffer + 256, &obj, skip_mask);

  return strcmp(buffer, str) == 0;
}
