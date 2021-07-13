#include <cinttypes>
#include <iostream>
#include <sstream>

#include <gtest/gtest.h>

#include "torrent/object.h"
#include "torrent/object_stream.h"

#include "test/helpers/bencode.h"

class ObjectStreamTest : public ::testing::Test {};

static const char* ordered_bencode =
  "d1:ei0e4:ipv44:XXXX4:ipv616:XXXXXXXXXXXXXXXX1:md11:upload_onlyi3e12:ut_"
  "holepunchi4e11:ut_metadatai2e6:ut_pexi1ee13:metadata_sizei15408e1:"
  "pi16033e4:reqqi255e1:v15:uuTorrent 1.8.46:yourip4:XXXXe";
static const char* unordered_bencode =
  "d1:ei0e1:md11:upload_onlyi3e12:ut_holepunchi4e11:ut_metadatai2e6:ut_"
  "pexi1ee4:ipv44:XXXX4:ipv616:XXXXXXXXXXXXXXXX13:metadata_sizei15408e1:"
  "pi16033e4:reqqi255e1:v15:uuTorrent 1.8.46:yourip4:XXXXe";

static const char* string_bencode = "32:aaaaaaaabbbbbbbbccccccccdddddddd";

TEST_F(ObjectStreamTest, testInputOrdered) {
  torrent::Object orderedObj   = create_bencode(ordered_bencode);
  torrent::Object unorderedObj = create_bencode(unordered_bencode);

  ASSERT_FALSE((orderedObj.flags() & torrent::Object::flag_unordered));
  ASSERT_TRUE(unorderedObj.flags() & torrent::Object::flag_unordered);
}

TEST_F(ObjectStreamTest, testInputNullKey) {
  torrent::Object obj = create_bencode("d0:i1e5:filesi2ee");

  ASSERT_FALSE((obj.flags() & torrent::Object::flag_unordered));
}

TEST_F(ObjectStreamTest, testOutputMask) {
  torrent::Object normalObj = create_bencode("d1:ai1e1:bi2e1:ci3ee");

  ASSERT_TRUE(compare_bencode(normalObj, "d1:ai1e1:bi2e1:ci3ee"));

  normalObj.get_key("b").set_flags(torrent::Object::flag_session_data);
  normalObj.get_key("c").set_flags(torrent::Object::flag_static_data);

  ASSERT_TRUE(compare_bencode(
    normalObj, "d1:ai1e1:ci3ee", torrent::Object::flag_session_data));
}

//
// Testing for bugs in bencode write.
//

// Dummy function that invalidates the buffer once called.

torrent::object_buffer_t
object_write_to_invalidate(void*, torrent::object_buffer_t buffer) {
  return torrent::object_buffer_t(buffer.second, buffer.second);
}

TEST_F(ObjectStreamTest, testBuffer) {
  char                     raw_buffer[16];
  torrent::object_buffer_t buffer(raw_buffer, raw_buffer + 16);

  torrent::Object obj = create_bencode(string_bencode);

  object_write_bencode_c(&object_write_to_invalidate, nullptr, buffer, &obj);
}

static const char* single_level_bencode = "d1:ai1e1:bi2e1:cl1:ai1e1:bi2eee";

TEST_F(ObjectStreamTest, testReadBencodeC) {
  torrent::Object orderedObj   = create_bencode_c(ordered_bencode);
  torrent::Object unorderedObj = create_bencode_c(unordered_bencode);

  ASSERT_FALSE((orderedObj.flags() & torrent::Object::flag_unordered));
  ASSERT_TRUE(unorderedObj.flags() & torrent::Object::flag_unordered);
  ASSERT_TRUE(compare_bencode(orderedObj, ordered_bencode));

  //  torrent::Object single_level = create_bencode_c(single_level_bencode);
  torrent::Object single_level = create_bencode_c(single_level_bencode);

  ASSERT_TRUE(compare_bencode(single_level, single_level_bencode));
}

bool
object_write_bencode(const torrent::Object& obj, const char* original) {
  try {
    char  buffer[1025];
    char* last =
      torrent::object_write_bencode(buffer, buffer + 1024, &obj).first;
    return std::strncmp(buffer, original, std::distance(buffer, last)) == 0;

  } catch (torrent::bencode_error& e) {
    return false;
  }
}

bool
object_stream_read_skip(const char* input) {
  try {
    torrent::Object tmp;
    return torrent::object_read_bencode_c(input, input + strlen(input), &tmp) ==
             input + strlen(input) &&
           torrent::object_read_bencode_skip_c(input, input + strlen(input)) ==
             input + strlen(input);
  } catch (torrent::bencode_error& e) {
    return false;
  }
}

bool
object_stream_read_skip_catch(const char* input) {
  try {
    torrent::Object tmp;
    torrent::object_read_bencode_c(input, input + strlen(input), &tmp);
    std::cout << "(N)";
    return false;
  } catch (torrent::bencode_error& e) {
  }

  try {
    torrent::object_read_bencode_skip_c(input, input + strlen(input));
    std::cout << "(S)";
    return false;
  } catch (torrent::bencode_error& e) {
    return true;
  }
}

TEST_F(ObjectStreamTest, test_read_skip) {
  ASSERT_TRUE(object_stream_read_skip("i0e"));
  ASSERT_TRUE(object_stream_read_skip("i9999e"));
  ASSERT_TRUE(object_stream_read_skip("i-1e"));
  ASSERT_TRUE(object_stream_read_skip("i-9999e"));

  ASSERT_TRUE(object_stream_read_skip("0:"));
  ASSERT_TRUE(object_stream_read_skip("4:test"));

  ASSERT_TRUE(object_stream_read_skip("le"));
  ASSERT_TRUE(object_stream_read_skip("li1ee"));
  ASSERT_TRUE(object_stream_read_skip("llee"));
  ASSERT_TRUE(object_stream_read_skip("ll1:a1:bel1:c1:dee"));

  ASSERT_TRUE(object_stream_read_skip("de"));
  ASSERT_TRUE(object_stream_read_skip("d1:ai1e1:b1:xe"));
  ASSERT_TRUE(object_stream_read_skip("d1:ali1eee"));
  ASSERT_TRUE(object_stream_read_skip("d1:ad1:bi1eee"));

  ASSERT_TRUE(object_stream_read_skip("d1:md6:ut_pexi0eee"));
}

TEST_F(ObjectStreamTest, test_read_skip_invalid) {
  ASSERT_TRUE(object_stream_read_skip_catch(""));
  ASSERT_TRUE(object_stream_read_skip_catch("i"));
  ASSERT_TRUE(object_stream_read_skip_catch("1"));
  ASSERT_TRUE(object_stream_read_skip_catch("d"));

  ASSERT_TRUE(object_stream_read_skip_catch("i-0e"));
  ASSERT_TRUE(object_stream_read_skip_catch("i--1e"));
  ASSERT_TRUE(object_stream_read_skip_catch("-1"));
  ASSERT_TRUE(object_stream_read_skip_catch("-1a"));

  ASSERT_TRUE(object_stream_read_skip_catch("llllllll"
                                            "llllllll"
                                            "llllllll"
                                            "llllllll"
                                            "llllllll"
                                            "llllllll"
                                            "llllllll"
                                            "llllllll"
                                            "llllllll"
                                            "llllllll"
                                            "llllllll"
                                            "llllllll"
                                            "llllllll"
                                            "llllllll"
                                            "llllllll"
                                            "llllllll"));
}

TEST_F(ObjectStreamTest, test_write) {
  torrent::Object obj;

  ASSERT_TRUE(object_write_bencode(torrent::Object(), ""));
  ASSERT_TRUE(object_write_bencode(torrent::Object((int64_t)0), "i0e"));
  ASSERT_TRUE(object_write_bencode(torrent::Object((int64_t)1), "i1e"));
  ASSERT_TRUE(object_write_bencode(torrent::Object((int64_t)-1), "i-1e"));
  ASSERT_TRUE(object_write_bencode(torrent::Object(INT64_C(123456789012345)),
                                   "i123456789012345e"));
  ASSERT_TRUE(object_write_bencode(torrent::Object(INT64_C(-123456789012345)),
                                   "i-123456789012345e"));

  ASSERT_TRUE(object_write_bencode(torrent::Object("test"), "4:test"));
  ASSERT_TRUE(object_write_bencode(torrent::Object::create_list(), "le"));
  ASSERT_TRUE(object_write_bencode(torrent::Object::create_map(), "de"));

  obj               = torrent::Object::create_map();
  obj.as_map()["a"] = (int64_t)1;
  ASSERT_TRUE(object_write_bencode(obj, "d1:ai1ee"));

  obj.as_map()["b"] = "test";
  ASSERT_TRUE(object_write_bencode(obj, "d1:ai1e1:b4:teste"));

  obj.as_map()["c"] = torrent::Object::create_list();
  obj.as_map()["c"].as_list().push_back("foo");
  ASSERT_TRUE(object_write_bencode(obj, "d1:ai1e1:b4:test1:cl3:fooee"));

  obj.as_map()["c"].as_list().push_back(torrent::Object());
  obj.as_map()["d"] = torrent::Object();
  ASSERT_TRUE(object_write_bencode(obj, "d1:ai1e1:b4:test1:cl3:fooee"));
}
