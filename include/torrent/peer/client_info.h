// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2005-2011, Jari Sundell <jaris@ifi.uio.no>

#ifndef LIBTORRENT_PEER_CLIENT_INFO_H
#define LIBTORRENT_PEER_CLIENT_INFO_H

#include <torrent/common.h>

namespace torrent {

class LIBTORRENT_EXPORT ClientInfo {
public:
  friend class ClientList;

  typedef enum {
    TYPE_UNKNOWN,
    TYPE_AZUREUS,
    TYPE_COMPACT,
    TYPE_MAINLINE,
    TYPE_MAX_SIZE
  } id_type;

  struct info_type {
    const char* m_shortDescription;
  };

  static const uint32_t max_key_size     = 2;
  static const uint32_t max_version_size = 4;

  id_type type() const {
    return m_type;
  }

  const char* key() const {
    return m_key;
  }
  const char* version() const {
    return m_version;
  }
  const char* upper_version() const {
    return m_upperVersion;
  }

  const char* short_description() const {
    return m_info->m_shortDescription;
  }
  void set_short_description(const char* str) {
    m_info->m_shortDescription = str;
  }

  static unsigned int key_size(id_type id);
  static unsigned int version_size(id_type id);

  // The intersect/disjoint postfix indicates what kind of equivalence
  // comparison we get when using !less && !greater.
  static bool less_intersects(const ClientInfo& left, const ClientInfo& right);
  static bool less_disjoint(const ClientInfo& left, const ClientInfo& right);

  static bool greater_intersects(const ClientInfo& left,
                                 const ClientInfo& right);
  static bool greater_disjoint(const ClientInfo& left, const ClientInfo& right);

  static bool intersects(const ClientInfo& left, const ClientInfo& right);
  static bool equal_to(const ClientInfo& left, const ClientInfo& right);

protected:
  void set_type(id_type t) {
    m_type = t;
  }

  info_type* info() const {
    return m_info;
  }
  void set_info(info_type* ptr) {
    m_info = ptr;
  }

  char* mutable_key() {
    return m_key;
  }
  char* mutable_version() {
    return m_version;
  }
  char* mutable_upper_version() {
    return m_upperVersion;
  }

private:
  id_type m_type;
  char    m_key[max_key_size];

  // The client version. The ClientInfo object in ClientList bounds
  // the versions that this object applies to. When the user searches
  // the ClientList for a client id, m_version will be set to the
  // actual client version.
  char m_version[max_version_size];
  char m_upperVersion[max_version_size];

  // We don't really care about cleaning up this as deleting an entry
  // form ClientList shouldn't happen.
  info_type* m_info;
};

} // namespace torrent

#endif
