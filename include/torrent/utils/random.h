#ifndef LIBTORRENT_TORRENT_UTILS_RANDOM_H
#define LIBTORRENT_TORRENT_UTILS_RANDOM_H

#include <cinttypes>
#include <limits>
#include <random>

template<typename T>
constexpr inline std::uniform_int_distribution<T>
uniform_dist() {
  return std::uniform_int_distribution<T>(std::numeric_limits<T>::min(),
                                          std::numeric_limits<T>::max());
}

template<typename T, typename G>
constexpr inline T
random_uniform_template(T min, T max, G& gen) {
  return std::uniform_int_distribution<T>(min, max)(gen);
}

namespace torrent {

static thread_local std::random_device rd;
static thread_local std::mt19937       mt32(rd());
static thread_local std::mt19937_64    mt64(rd());

static thread_local auto dist_int32 = uniform_dist<int32_t>();
static thread_local auto dist_int64 = uniform_dist<int64_t>();
static thread_local auto dist_uint8 =
  std::uniform_int_distribution<uint16_t>(std::numeric_limits<uint8_t>::min(),
                                          std::numeric_limits<uint8_t>::max());
static thread_local auto dist_uint32 = uniform_dist<uint32_t>();

inline int32_t
random_int32() {
  return dist_int32(mt32);
}

inline int64_t
random_int64() {
  return dist_int64(mt64);
}

inline uint8_t
random_uint8() {
  return dist_uint8(mt32);
}

inline uint32_t
random_uint32() {
  return dist_uint32(mt32);
}

inline char
random_char() {
  return (char)random_uint8();
}

inline char
random_uniform_char(char min, char max) {
  if (std::numeric_limits<char>::is_signed) {
    return (char)random_uniform_template<int16_t>(min, max, mt32);
  } else {
    return (char)random_uniform_template<uint16_t>(min, max, mt32);
  }
}

inline size_t
random_uniform_size(size_t min, size_t max) {
  return random_uniform_template(min, max, mt64);
}

inline uint16_t
random_uniform_uint16(uint16_t min, uint16_t max) {
  return random_uniform_template(min, max, mt32);
}

inline uint32_t
random_uniform_uint32(uint32_t min, uint32_t max) {
  return random_uniform_template(min, max, mt32);
}

} // namespace torrent

#endif
