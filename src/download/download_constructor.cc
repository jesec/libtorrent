// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2005-2011, Jari Sundell <jaris@ifi.uio.no>

#include <cstdio>
#include <cstring>

#include "download/download_constructor.h"
#include "download/download_wrapper.h"
#include "manager.h"
#include "torrent/data/file.h"
#include "torrent/data/file_list.h"
#include "torrent/dht_manager.h"
#include "torrent/exceptions.h"
#include "torrent/object.h"
#include "torrent/tracker_controller.h"
#include "torrent/tracker_list.h"
#include "torrent/utils/string_manip.h"

namespace torrent {

struct download_constructor_is_single_path {
  bool operator()(Object::map_type::const_reference v) const {
    return std::strncmp(v.first.c_str(), "name.", sizeof("name.") - 1) == 0 &&
           v.second.is_string();
  }
};

struct download_constructor_is_multi_path {
  bool operator()(Object::map_type::const_reference v) const {
    return std::strncmp(v.first.c_str(), "path.", sizeof("path.") - 1) == 0 &&
           v.second.is_list();
  }
};

struct download_constructor_encoding_match {
  bool operator()(const Path& p, const char* enc) {
    return strcasecmp(p.encoding().c_str(), enc) == 0;
  }
};

void
DownloadConstructor::initialize(Object& b) {
  if (!b.has_key_map("info") && b.has_key_string("magnet-uri"))
    parse_magnet_uri(b, b.get_key_string("magnet-uri"));

  if (b.has_key_string("encoding"))
    m_defaultEncoding = b.get_key_string("encoding");

  if (b.has_key_value("creation date"))
    m_download->info()->set_creation_date(b.get_key_value("creation date"));

  if (b.get_key("info").has_key_value("private") &&
      b.get_key("info").get_key_value("private") == 1)
    m_download->info()->set_private();

  parse_name(b.get_key("info"));
  parse_info(b.get_key("info"));
}

// Currently using a hack of the path thingie to extract the correct
// torrent name.
void
DownloadConstructor::parse_name(const Object& b) {
  if (is_invalid_path_element(b.get_key("name")))
    throw input_error("Bad torrent file, \"name\" is an invalid path name.");

  std::list<Path> pathList;

  pathList.emplace_back();
  pathList.back().set_encoding(m_defaultEncoding);
  pathList.back().push_back(b.get_key_string("name"));

  for (auto itr = b.as_map().begin();
       (itr = std::find_if(
          itr, b.as_map().end(), download_constructor_is_single_path())) !=
       b.as_map().end();
       ++itr) {
    // "name.[encoding]": name
    const auto& [key, value] = *itr;
    pathList.emplace_back();
    pathList.back().set_encoding(key.substr(sizeof("name.") - 1));
    pathList.back().push_back(value.as_string());
  }

  if (pathList.empty())
    throw input_error("Bad torrent file, an entry has no valid name.");

  Path name = choose_path(&pathList);

  if (name.empty())
    throw internal_error(
      "DownloadConstructor::parse_name(...) Ended up with an empty Path.");

  m_download->info()->set_name(name.front());
}

void
DownloadConstructor::parse_info(const Object& b) {
  FileList* fileList = m_download->main()->file_list();

  if (!fileList->empty())
    throw internal_error(
      "parse_info received an already initialized Content object.");

  if (b.flags() & Object::flag_unordered)
    throw input_error("Download has unordered info dictionary.");

  uint32_t chunkSize;

  if (b.has_key_value("meta_download") && b.get_key_value("meta_download"))
    m_download->info()->set_flags(DownloadInfo::flag_meta_download);

  if (m_download->info()->is_meta_download()) {
    if (b.get_key_string("pieces").length() != HashString::size_data)
      throw input_error("Meta-download has invalid piece data.");

    chunkSize = 1;
    parse_single_file(b, chunkSize);

  } else {
    chunkSize = b.get_key_value("piece length");

    if (chunkSize <= (1 << 10) || chunkSize > (512 << 20))
      throw input_error("Torrent has an invalid \"piece length\".");
  }

  if (b.has_key("length")) {
    parse_single_file(b, chunkSize);

  } else if (b.has_key("files")) {
    parse_multi_files(b.get_key("files"), chunkSize);
    fileList->set_root_dir("./" + m_download->info()->name());

  } else if (!m_download->info()->is_meta_download()) {
    throw input_error("Torrent must have either length or files entry.");
  }

  if (fileList->size_bytes() == 0 && !m_download->info()->is_meta_download())
    throw input_error("Torrent has zero length.");

  // Set chunksize before adding files to make sure the index range is
  // correct.
  m_download->set_complete_hash(b.get_key_string("pieces"));

  if (m_download->complete_hash().size() / 20 < fileList->size_chunks())
    throw bencode_error(
      "Torrent size and 'info:pieces' length does not match.");
}

void
DownloadConstructor::parse_tracker(const Object& b) {
  const Object::list_type* announce_list = nullptr;

  if (b.has_key_list("announce-list") &&
      // Some torrent makers create empty/invalid 'announce-list'
      // entries while still having valid 'announce'.
      !(announce_list = &b.get_key_list("announce-list"))->empty() &&
      std::find_if(announce_list->begin(),
                   announce_list->end(),
                   std::mem_fn(&Object::is_list)) != announce_list->end()) {
    for (const auto& group : *announce_list) {
      add_tracker_group(group);
    }
  } else if (b.has_key("announce")) {
    add_tracker_single(b.get_key("announce"), 0);
  } else if (!manager->dht_manager()->is_valid() ||
             m_download->info()->is_private()) {
    throw bencode_error("Could not find any trackers");
  }

  if (manager->dht_manager()->is_valid() && !m_download->info()->is_private()) {
    m_download->main()->tracker_list()->insert_url(
      m_download->main()->tracker_list()->size_group(), "dht://");
  }

  if (manager->dht_manager()->is_valid() && b.has_key_list("nodes")) {
    for (const auto& node : b.get_key_list("nodes")) {
      add_dht_node(node);
    }
  }

  m_download->main()->tracker_list()->randomize_group_entries();
}

void
DownloadConstructor::add_tracker_group(const Object& group) {
  if (!group.is_list())
    throw bencode_error("Tracker group list not a list");

  for (const auto& tracker : group.as_list()) {
    add_tracker_single(tracker,
                       m_download->main()->tracker_list()->size_group());
  }
}

void
DownloadConstructor::add_tracker_single(const Object& b, int group) {
  if (!b.is_string())
    throw bencode_error("Tracker entry not a string");

  m_download->main()->tracker_list()->insert_url(
    group, utils::trim_classic(b.as_string()));
}

void
DownloadConstructor::add_dht_node(const Object& b) {
  if (!b.is_list() || b.as_list().size() < 2)
    return;

  auto el = b.as_list().begin();

  if (!el->is_string())
    return;

  const std::string& host = el->as_string();

  if (!(++el)->is_value())
    return;

  manager->dht_manager()->add_node(host, el->as_value());
}

bool
DownloadConstructor::is_valid_path_element(const Object& b) {
  return b.is_string() && b.as_string() != "." && b.as_string() != ".." &&
         std::find(b.as_string().begin(), b.as_string().end(), '/') ==
           b.as_string().end() &&
         std::find(b.as_string().begin(), b.as_string().end(), '\0') ==
           b.as_string().end();
}

void
DownloadConstructor::parse_single_file(const Object& b, uint32_t chunkSize) {
  if (is_invalid_path_element(b.get_key("name")))
    throw input_error("Bad torrent file, \"name\" is an invalid path name.");

  FileList* fileList = m_download->main()->file_list();
  fileList->initialize(chunkSize == 1 ? 1 : b.get_key_value("length"),
                       chunkSize);
  fileList->set_multi_file(false);

  std::list<Path> pathList;

  pathList.emplace_back();
  pathList.back().set_encoding(m_defaultEncoding);
  pathList.back().push_back(b.get_key_string("name"));

  for (auto itr = b.as_map().begin();
       (itr = std::find_if(
          itr, b.as_map().end(), download_constructor_is_single_path())) !=
       b.as_map().end();
       ++itr) {
    pathList.emplace_back();
    pathList.back().set_encoding(itr->first.substr(sizeof("name.") - 1));
    pathList.back().push_back(itr->second.as_string());
  }

  if (pathList.empty())
    throw input_error("Bad torrent file, an entry has no valid filename.");

  *fileList->front()->mutable_path() = choose_path(&pathList);
  fileList->update_paths(fileList->begin(), fileList->end());
}

void
DownloadConstructor::parse_multi_files(const Object& b, uint32_t chunkSize) {
  const Object::list_type& objectList = b.as_list();

  // Multi file torrent
  if (objectList.empty())
    throw input_error("Bad torrent file, entry has no files.");

  int64_t                           torrentSize = 0;
  std::vector<FileList::split_type> splitList(objectList.size());
  auto                              splitItr = splitList.begin();

  for (auto listItr = objectList.begin(), listLast = objectList.end();
       listItr != listLast;
       ++listItr, ++splitItr) {
    std::list<Path> pathList;

    if (listItr->has_key_list("path"))
      pathList.push_back(
        create_path(listItr->get_key_list("path"), m_defaultEncoding));

    auto itr  = listItr->as_map().begin();
    auto last = listItr->as_map().end();

    while ((itr = std::find_if(
              itr, last, download_constructor_is_multi_path())) != last) {
      pathList.push_back(create_path(itr->second.as_list(),
                                     itr->first.substr(sizeof("path.") - 1)));
      ++itr;
    }

    if (pathList.empty())
      throw input_error("Bad torrent file, an entry has no valid filename.");

    int64_t length = listItr->get_key_value("length");

    if (length < 0 || torrentSize + length < 0)
      throw input_error("Bad torrent file, invalid length for file.");

    torrentSize += length;
    *splitItr = FileList::split_type(length, choose_path(&pathList));
  }

  FileList* fileList = m_download->main()->file_list();
  fileList->set_multi_file(true);

  fileList->initialize(torrentSize, chunkSize);
  fileList->split(fileList->begin(), &*splitList.begin(), &*splitList.end());
  fileList->update_paths(fileList->begin(), fileList->end());
}

inline Path
DownloadConstructor::create_path(const Object::list_type& plist,
                                 const std::string&       enc) {
  // Make sure we are given a proper file path.
  if (plist.empty())
    throw input_error("Bad torrent file, \"path\" has zero entries.");

  if (std::find_if(plist.begin(), plist.end(), [](const Object& b) {
        return is_invalid_path_element(b);
      }) != plist.end())
    throw input_error(
      "Bad torrent file, \"path\" has zero entries or a zero length entry.");

  Path p;
  p.set_encoding(enc);

  std::transform(plist.begin(),
                 plist.end(),
                 std::back_inserter(p),
                 [](const Object& elem) { return elem.as_string(); });

  return p;
}

inline Path
DownloadConstructor::choose_path(std::list<Path>* pathList) {
  auto pathFirst     = pathList->begin();
  auto pathLast      = pathList->end();
  auto encodingFirst = m_encodingList->begin();
  auto encodingLast  = m_encodingList->end();

  for (; encodingFirst != encodingLast; ++encodingFirst) {
    auto itr =
      std::find_if(pathFirst, pathLast, [encodingFirst](const Path& p) {
        return download_constructor_encoding_match()(p, encodingFirst->c_str());
      });

    if (itr != pathLast)
      pathList->splice(pathFirst, *pathList, itr);
  }

  return pathList->front();
}

static const char*
parse_base32_sha1(const char* pos, HashString& hash) {
  HashString::iterator hashItr = hash.begin();

  static constexpr int base_shift = 8 + 8 - 5;
  int                  shift      = base_shift;
  uint16_t             decoded    = 0;

  while (*pos) {
    char     c = *pos++;
    uint16_t value;

    if (c >= 'A' && c <= 'Z')
      value = c - 'A';
    else if (c >= 'a' && c <= 'z')
      value = c - 'a';
    else if (c >= '2' && c <= '7')
      value = 26 + c - '2';
    else if (c == '&')
      break;
    else
      return nullptr;

    decoded |= (value << shift);
    if (shift <= 8) {
      // Too many characters for a base32 SHA1.
      if (hashItr == hash.end())
        return nullptr;

      *hashItr++ = (decoded >> 8);
      decoded <<= 8;
      shift += 3;
    } else {
      shift -= 5;
    }
  }

  return hashItr != hash.end() || shift != base_shift ? nullptr : pos;
}

void
DownloadConstructor::parse_magnet_uri(Object& b, const std::string& uri) {
  if (std::strncmp(uri.c_str(), "magnet:?", 8))
    throw input_error("Invalid magnet URI.");

  const char* pos = uri.c_str() + 8;

  Object     trackers(Object::create_list());
  HashString hash;
  bool       hashValid = false;

  while (*pos) {
    const char* tagStart = pos;
    while (*pos != '=')
      if (!*pos++)
        break;

    raw_string tag(tagStart, pos - tagStart);
    pos++;

    // hash may be base32 encoded (optional in BEP 0009 and common practice)
    if (raw_bencode_equal_c_str(tag, "xt")) {
      if (strncmp(pos, "urn:btih:", 9))
        throw input_error("Invalid magnet URI.");

      pos += 9;

      const char* nextPos = parse_base32_sha1(pos, hash);
      if (nextPos != nullptr) {
        pos       = nextPos;
        hashValid = true;
        continue;
      }
    }

    // everything else, including sometimes the hash, is url encoded.
    std::string decoded;
    while (*pos) {
      char c = *pos++;
      if (c == '%') {
        if (sscanf(pos, "%02hhx", (unsigned char*)&c) != 1)
          throw input_error("Invalid magnet URI.");

        pos += 2;

      } else if (c == '&') {
        break;
      }

      decoded.push_back(c);
    }

    if (raw_bencode_equal_c_str(tag, "xt")) {
      // url-encoded hash as per magnet URN specs
      if (decoded.length() == hash.size_data) {
        hash      = *HashString::cast_from(decoded);
        hashValid = true;

        // hex-encoded hash as per BEP 0009
      } else if (decoded.length() == hash.size_data * 2) {
        std::string::iterator hexItr = decoded.begin();
        for (HashString::iterator itr = hash.begin(), last = hash.end();
             itr != last;
             itr++, hexItr += 2)
          *itr = (utils::hexchar_to_value(*hexItr) << 4) +
                 utils::hexchar_to_value(*(hexItr + 1));
        hashValid = true;

      } else {
        throw input_error("Invalid magnet URI.");
      }

    } else if (raw_bencode_equal_c_str(tag, "tr")) {
      trackers.insert_back(Object::create_list()).insert_back(decoded);
    }
    // could also handle "dn" = display name (torrent name), but we can't really
    // use that
  }

  if (!hashValid)
    throw input_error("Invalid magnet URI.");

  Object& info = b.insert_key("info", Object::create_map());
  info.insert_key("pieces", hash.str());
  info.insert_key("name", utils::transform_hex(hash.str()) + ".meta");
  info.insert_key("meta_download", (int64_t)1);

  if (!trackers.as_list().empty()) {
    b.insert_preserve_copy(
      "announce", trackers.as_list().begin()->as_list().begin()->as_string());
    b.insert_preserve_type("announce-list", trackers);
  }
}

} // namespace torrent
