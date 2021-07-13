// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2005-2007, Jari Sundell <jaris@ifi.uio.no>

#ifndef LIBTORRENT_UTILS_STRING_MANIP_H
#define LIBTORRENT_UTILS_STRING_MANIP_H

#include <algorithm>
#include <cctype>
#include <climits>
#include <functional>
#include <iterator>
#include <locale>
#include <random>

namespace torrent {
namespace utils {

// Use these trim functions until n1872 is widely supported.

template<typename Sequence>
Sequence
trim_begin(const Sequence& seq) {
  if (seq.empty() || !std::isspace(*seq.begin()))
    return seq;

  typename Sequence::size_type pos = 0;

  while (pos != seq.length() && std::isspace(seq[pos]))
    pos++;

  return seq.substr(pos, seq.length() - pos);
}

template<typename Sequence>
Sequence
trim_end(const Sequence& seq) {
  if (seq.empty() || !std::isspace(*(--seq.end())))
    return seq;

  typename Sequence::size_type pos = seq.size();

  while (pos != 0 && std::isspace(seq[pos - 1]))
    pos--;

  return seq.substr(0, pos);
}

template<typename Sequence>
Sequence
trim(const Sequence& seq) {
  return trim_begin(trim_end(seq));
}

template<typename Sequence>
Sequence
trim_begin_classic(const Sequence& seq) {
  if (seq.empty() || !std::isspace(*seq.begin(), std::locale::classic()))
    return seq;

  typename Sequence::size_type pos = 0;

  while (pos != seq.length() && std::isspace(seq[pos], std::locale::classic()))
    pos++;

  return seq.substr(pos, seq.length() - pos);
}

template<typename Sequence>
Sequence
trim_end_classic(const Sequence& seq) {
  if (seq.empty() || !std::isspace(*(--seq.end()), std::locale::classic()))
    return seq;

  typename Sequence::size_type pos = seq.size();

  while (pos != 0 && std::isspace(seq[pos - 1], std::locale::classic()))
    pos--;

  return seq.substr(0, pos);
}

template<typename Sequence>
Sequence
trim_classic(const Sequence& seq) {
  return trim_begin_classic(trim_end_classic(seq));
}

// Consider rewritting such that m_seq is replaced by first/last.
template<typename Sequence>
class split_iterator_t {
public:
  using const_iterator = typename Sequence::const_iterator;
  using value_type     = typename Sequence::value_type;

  split_iterator_t() = default;

  split_iterator_t(const Sequence& seq, value_type delim)
    : m_seq(&seq)
    , m_delim(delim)
    , m_pos(seq.begin())
    , m_next(std::find(seq.begin(), seq.end(), delim)) {}

  Sequence operator*() {
    return Sequence(m_pos, m_next);
  }

  split_iterator_t& operator++() {
    m_pos = m_next;

    if (m_pos == m_seq->end())
      return *this;

    m_pos++;
    m_next = std::find(m_pos, m_seq->end(), m_delim);

    return *this;
  }

  bool operator==(const split_iterator_t&) const {
    return m_pos == m_seq->end();
  }
  bool operator!=(const split_iterator_t&) const {
    return m_pos != m_seq->end();
  }

private:
  const Sequence* m_seq;
  value_type      m_delim;
  const_iterator  m_pos;
  const_iterator  m_next;
};

template<typename Sequence>
inline split_iterator_t<Sequence>
split_iterator(const Sequence& seq, typename Sequence::value_type delim) {
  return split_iterator_t<Sequence>(seq, delim);
}

template<typename Sequence>
inline split_iterator_t<Sequence>
split_iterator(const Sequence&) {
  return split_iterator_t<Sequence>();
}

// Could optimize this abit.
inline char
hexchar_to_value(char c) {
  if (c >= '0' && c <= '9')
    return c - '0';

  else if (c >= 'A' && c <= 'Z')
    return 10 + c - 'A';

  else
    return 10 + c - 'a';
}

template<int pos, typename Value>
inline char
value_to_hexchar(Value v) {
  v >>= pos * 4;
  v &= 0xf;

  if (v < 0xA)
    return '0' + v;
  else
    return 'A' + v - 0xA;
}

template<typename InputIterator, typename OutputIterator>
OutputIterator
copy_escape_html(InputIterator first, InputIterator last, OutputIterator dest) {
  while (first != last) {
    if (std::isalpha(*first, std::locale::classic()) ||
        std::isdigit(*first, std::locale::classic()) || *first == '-') {
      *(dest++) = *first;

    } else {
      *(dest++) = '%';
      *(dest++) = value_to_hexchar<1>(*first);
      *(dest++) = value_to_hexchar<0>(*first);
    }

    ++first;
  }

  return dest;
}

template<typename InputIterator, typename OutputIterator>
OutputIterator
copy_escape_html(InputIterator  first1,
                 InputIterator  last1,
                 OutputIterator first2,
                 OutputIterator last2) {
  while (first1 != last1) {
    if (std::isalpha(*first1, std::locale::classic()) ||
        std::isdigit(*first1, std::locale::classic()) || *first1 == '-') {
      if (first2 == last2)
        break;
      else
        *(first2++) = *first1;

    } else {
      if (first2 == last2)
        break;
      else
        *(first2++) = '%';
      if (first2 == last2)
        break;
      else
        *(first2++) = value_to_hexchar<1>(*first1);
      if (first2 == last2)
        break;
      else
        *(first2++) = value_to_hexchar<0>(*first1);
    }

    ++first1;
  }

  return first2;
}

template<typename Iterator>
inline std::string
copy_escape_html(Iterator first, Iterator last) {
  std::string dest;
  copy_escape_html(first, last, std::back_inserter(dest));

  return dest;
}

template<typename Sequence>
inline Sequence
copy_escape_html(const Sequence& src) {
  Sequence dest;
  copy_escape_html(src.begin(), src.end(), std::back_inserter(dest));

  return dest;
}

template<typename Sequence>
inline std::string
copy_escape_html_str(const Sequence& src) {
  std::string dest;
  copy_escape_html(src.begin(), src.end(), std::back_inserter(dest));

  return dest;
}

// Consider support for larger than char type.
template<typename InputIterator, typename OutputIterator>
OutputIterator
transform_hex(InputIterator first, InputIterator last, OutputIterator dest) {
  while (first != last) {
    *(dest++) = value_to_hexchar<1>(*first);
    *(dest++) = value_to_hexchar<0>(*first);

    ++first;
  }

  return dest;
}

template<typename InputIterator, typename OutputIterator>
OutputIterator
transform_hex(InputIterator  first1,
              InputIterator  last1,
              OutputIterator first2,
              OutputIterator last2) {
  while (first1 != last1) {
    if (first2 == last2)
      break;
    else
      *(first2++) = value_to_hexchar<1>(*first1);
    if (first2 == last2)
      break;
    else
      *(first2++) = value_to_hexchar<0>(*first1);

    ++first1;
  }

  return first2;
}

template<typename Sequence>
inline Sequence
transform_hex(const Sequence& src) {
  Sequence dest;
  transform_hex(src.begin(), src.end(), std::back_inserter(dest));

  return dest;
}

template<typename Iterator>
inline std::string
transform_hex(Iterator first, Iterator last) {
  std::string dest;
  transform_hex(first, last, std::back_inserter(dest));

  return dest;
}

template<typename Sequence>
inline std::string
transform_hex_str(const Sequence& seq) {
  std::string dest;
  transform_hex(seq.begin(), seq.end(), std::back_inserter(dest));

  return dest;
}

template<typename Sequence>
Sequence
generate_random(size_t length) {
  std::random_device rd;
  std::mt19937       mt(rd());
  using bytes_randomizer =
    std::independent_bits_engine<std::mt19937, CHAR_BIT, uint8_t>;
  bytes_randomizer bytes(mt);
  Sequence         s;
  s.reserve(length);
  std::generate_n(std::back_inserter(s), length, std::ref(bytes));
  return s;
}

template<typename Iterator>
inline bool
is_all_alpha(Iterator first, Iterator last) {
  while (first != last)
    if (!std::isalpha(*first++, std::locale::classic()))
      return false;

  return true;
}

template<typename Sequence>
inline bool
is_all_alpha(const Sequence& src) {
  return is_all_alpha(src.begin(), src.end());
}

template<typename Iterator>
inline bool
is_all_alnum(Iterator first, Iterator last) {
  while (first != last)
    if (!std::isalnum(*first++, std::locale::classic()))
      return false;

  return true;
}

template<typename Sequence>
inline bool
is_all_alnum(const Sequence& src) {
  return is_all_alnum(src.begin(), src.end());
}

template<typename Iterator>
inline bool
is_all_name(Iterator first, Iterator last) {
  while (first != last) {
    if (!std::isalnum(*first, std::locale::classic()) && *first != '_')
      return false;

    first++;
  }

  return true;
}

template<typename Sequence>
inline bool
is_all_name(const Sequence& src) {
  return is_all_name(src.begin(), src.end());
}

template<typename Iterator>
std::string
sanitize(Iterator first, Iterator last) {
  std::string dest;
  for (; first != last; ++first) {
    if (std::isprint(*first) && *first != '\r' && *first != '\n' &&
        *first != '\t')
      dest += *first;
    else
      dest += " ";
  }

  return dest;
}

template<typename Sequence>
std::string
sanitize(const Sequence& src) {
  return trim(sanitize(src.begin(), src.end()));
}

template<typename Iterator>
std::string
striptags(Iterator first, Iterator last) {
  bool        copychar = true;
  std::string dest;

  for (; first != last; ++first) {
    if (std::isprint(*first) && *first == '<') {
      copychar = false;
    } else if (std::isprint(*first) && *first == '>') {
      copychar = true;
      continue;
    }

    if (copychar)
      dest += *first;
  }

  return dest;
}

template<typename Sequence>
std::string
striptags(const Sequence& src) {
  return striptags(src.begin(), src.end());
}

} // namespace utils
} // namespace torrent

#endif
