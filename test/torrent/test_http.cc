#include <sstream>

#include "torrent/http.h"

#include "test/helpers/fixture.h"

class test_http : public test_fixture {};

#define HTTP_SETUP()                                                           \
  bool http_destroyed   = false;                                               \
  bool stream_destroyed = false;                                               \
                                                                               \
  TestHttp*          test_http   = new TestHttp(&http_destroyed);              \
  torrent::Http*     http        = test_http;                                  \
  std::stringstream* http_stream = new StringStream(&stream_destroyed);        \
                                                                               \
  int done_counter   = 0;                                                      \
  int failed_counter = 0;                                                      \
                                                                               \
  http->set_stream(http_stream);                                               \
  http->signal_done().push_back(std::bind(&increment_value, &done_counter));   \
  http->signal_failed().push_back(std::bind(&increment_value, &failed_counter))

#define HTTP_TEARDOWN()                                                        \
  delete test_http;                                                            \
  delete http_stream

class StringStream : public std::stringstream {
public:
  StringStream(bool* destroyed)
    : m_destroyed(destroyed) {}
  ~StringStream() {
    *m_destroyed = true;
  }

private:
  bool* m_destroyed;
};

class TestHttp : public torrent::Http {
public:
  static constexpr int flag_active = 0x1;

  TestHttp(bool* destroyed = nullptr)
    : m_flags(0)
    , m_destroyed(destroyed) {}
  virtual ~TestHttp() {
    if (m_destroyed)
      *m_destroyed = true;
  }

  virtual void start() {
    m_flags |= flag_active;
  }
  virtual void close() {
    m_flags &= ~flag_active;
  }

  bool trigger_signal_done();
  bool trigger_signal_failed();

private:
  int   m_flags;
  bool* m_destroyed;
};

bool
TestHttp::trigger_signal_done() {
  if (!(m_flags & flag_active))
    return false;

  m_flags &= ~flag_active;
  trigger_done();
  return true;
}

bool
TestHttp::trigger_signal_failed() {
  if (!(m_flags & flag_active))
    return false;

  m_flags &= ~flag_active;
  trigger_failed("We Fail.");
  return true;
}

TestHttp*
create_test_http() {
  return new TestHttp;
}

static void
increment_value(int* value) {
  (*value)++;
}

TEST_F(test_http, test_basic) {
  torrent::Http::slot_factory() = std::bind(&create_test_http);

  auto http        = torrent::Http::slot_factory()();
  auto http_stream = new std::stringstream;

  http->set_url("http://example.com");
  ASSERT_EQ(http->url(), "http://example.com");

  ASSERT_EQ(http->stream(), nullptr);
  http->set_stream(http_stream);
  ASSERT_EQ(http->stream(), http_stream);

  ASSERT_EQ(http->timeout(), 0);
  http->set_timeout(666);
  ASSERT_EQ(http->timeout(), 666);

  delete http;
  delete http_stream;
}

TEST_F(test_http, test_done) {
  HTTP_SETUP();

  http->start();

  ASSERT_TRUE(test_http->trigger_signal_done());

  // Check that we didn't delete...

  ASSERT_EQ(done_counter, 1);
  ASSERT_EQ(failed_counter, 0);

  HTTP_TEARDOWN();
}

TEST_F(test_http, test_failure) {
  HTTP_SETUP();

  http->start();

  ASSERT_TRUE(test_http->trigger_signal_failed());

  // Check that we didn't delete...

  ASSERT_EQ(done_counter, 0);
  ASSERT_EQ(failed_counter, 1);

  HTTP_TEARDOWN();
}

TEST_F(test_http, test_delete_on_done) {
  HTTP_SETUP();
  http->start();
  http->set_delete_stream();

  ASSERT_FALSE(stream_destroyed);
  ASSERT_FALSE(http_destroyed);
  ASSERT_TRUE(test_http->trigger_signal_done());
  ASSERT_TRUE(stream_destroyed);
  ASSERT_FALSE(http_destroyed);
  ASSERT_EQ(http->stream(), nullptr);

  stream_destroyed = false;
  http_stream      = new StringStream(&stream_destroyed);
  http->set_stream(http_stream);

  http->start();
  http->set_delete_self();

  ASSERT_FALSE(stream_destroyed);
  ASSERT_FALSE(http_destroyed);
  ASSERT_TRUE(test_http->trigger_signal_done());
  ASSERT_TRUE(stream_destroyed);
  ASSERT_TRUE(http_destroyed);
}

TEST_F(test_http, test_delete_on_failure) {
  HTTP_SETUP();
  http->start();
  http->set_delete_stream();

  ASSERT_FALSE(stream_destroyed);
  ASSERT_FALSE(http_destroyed);
  ASSERT_TRUE(test_http->trigger_signal_failed());
  ASSERT_TRUE(stream_destroyed);
  ASSERT_FALSE(http_destroyed);
  ASSERT_EQ(http->stream(), nullptr);

  stream_destroyed = false;
  http_stream      = new StringStream(&stream_destroyed);
  http->set_stream(http_stream);

  http->start();
  http->set_delete_self();

  ASSERT_FALSE(stream_destroyed);
  ASSERT_FALSE(http_destroyed);
  ASSERT_TRUE(test_http->trigger_signal_failed());
  ASSERT_TRUE(stream_destroyed);
  ASSERT_TRUE(http_destroyed);
}
