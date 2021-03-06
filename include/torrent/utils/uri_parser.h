// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2005-2011, Jari Sundell <jaris@ifi.uio.no>

#ifndef LIBTORRENT_UTILS_URI_PARSER_H
#define LIBTORRENT_UTILS_URI_PARSER_H

#include <string>
#include <torrent/common.h>
#include <torrent/exceptions.h>
#include <vector>

namespace torrent {
namespace utils {

using uri_resource_list = std::vector<std::string>;
using uri_query_list    = std::vector<std::string>;

struct uri_base_state {
  static constexpr int state_empty   = 0;
  static constexpr int state_valid   = 1;
  static constexpr int state_invalid = 2;

  uri_base_state()
    : state(state_empty) {}

  int state;
};

struct uri_state : uri_base_state {
  std::string uri;
  std::string scheme;
  std::string resource;
  std::string query;
  std::string fragment;
};

struct uri_resource_state : public uri_base_state {
  std::string       resource;
  uri_resource_list path;
};

struct uri_query_state : public uri_base_state {
  std::string    query;
  uri_query_list elements;
};

void
uri_parse_str(std::string uri, uri_state& state) LIBTORRENT_EXPORT;
void
uri_parse_c_str(const char* uri, uri_state& state) LIBTORRENT_EXPORT;

void
uri_parse_resource(std::string query, uri_query_state& state) LIBTORRENT_EXPORT;
void
uri_parse_resource_authority(std::string      query,
                             uri_query_state& state) LIBTORRENT_EXPORT;
void
uri_parse_resource_path(std::string      query,
                        uri_query_state& state) LIBTORRENT_EXPORT;

void
uri_parse_query_str(std::string      query,
                    uri_query_state& state) LIBTORRENT_EXPORT;
void
uri_parse_query_c_str(const char*      query,
                      uri_query_state& state) LIBTORRENT_EXPORT;

class LIBTORRENT_EXPORT uri_error : public ::torrent::input_error {
public:
  uri_error(const char* msg)
    : ::torrent::input_error(msg) {}
  uri_error(const std::string& msg)
    : ::torrent::input_error(msg) {}
};

} // namespace utils
} // namespace torrent

#endif
