// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2005-2011, Jari Sundell <jaris@ifi.uio.no>

#ifndef LIBTORRENT_PEER_RESOURCE_MANAGER_H
#define LIBTORRENT_PEER_RESOURCE_MANAGER_H

#include <cinttypes>
#include <string>
#include <torrent/common.h>
#include <vector>

namespace torrent {

// This class will handle the division of various resources like
// uploads. For now the weight is equal to the value of the priority.
//
// Although the ConnectionManager class keeps a tally of open sockets,
// we still need to balance them across the different downloads so
// ResourceManager will also keep track of those.
//
// Add unlimited handling later.

class choke_group;
class DownloadMain;
class Rate;
class ResourceManager;

class LIBTORRENT_EXPORT resource_manager_entry {
public:
  friend class ResourceManager;

  resource_manager_entry(DownloadMain* d   = NULL,
                         uint16_t      pri = 0,
                         uint16_t      grp = 0)
    : m_download(d)
    , m_priority(pri)
    , m_group(grp) {}

  DownloadMain* download() {
    return m_download;
  }
  const DownloadMain* c_download() const {
    return m_download;
  }

  uint16_t priority() const {
    return m_priority;
  }
  uint16_t group() const {
    return m_group;
  }

  const Rate* up_rate() const;
  const Rate* down_rate() const;

protected:
  void set_priority(uint16_t pri) {
    m_priority = pri;
  }
  void set_group(uint16_t grp) {
    m_group = grp;
  }

private:
  DownloadMain* m_download;

  uint16_t m_priority;
  uint16_t m_group;
};

class LIBTORRENT_EXPORT ResourceManager
  : private std::vector<resource_manager_entry>
  , private std::vector<choke_group*> {
public:
  typedef std::vector<resource_manager_entry> base_type;
  typedef std::vector<choke_group*>           choke_base_type;
  typedef base_type::value_type               value_type;
  typedef base_type::iterator                 iterator;

  typedef choke_base_type::iterator group_iterator;

  using base_type::begin;
  using base_type::capacity;
  using base_type::end;
  using base_type::size;

  ResourceManager();
  ~ResourceManager() noexcept(false);

  void insert(DownloadMain* d, uint16_t priority) {
    insert(value_type(d, priority));
  }
  void erase(DownloadMain* d);

  void push_group(const std::string& name);

  iterator find(DownloadMain* d);
  iterator find_throw(DownloadMain* d);
  iterator find_group_end(uint16_t group);

  unsigned int group_size() const {
    return choke_base_type::size();
  }
  choke_group* group_back() {
    return choke_base_type::back();
  }

  choke_group* group_at(uint16_t grp);
  choke_group* group_at_name(const std::string& name);

  int group_index_of(const std::string& name);

  group_iterator group_begin() {
    return choke_base_type::begin();
  }
  group_iterator group_end() {
    return choke_base_type::end();
  }

  resource_manager_entry& entry_at(DownloadMain* d) {
    return *find_throw(d);
  }

  void set_priority(iterator itr, uint16_t pri);
  void set_group(iterator itr, uint16_t grp);

  // When setting this, make sure you choke peers, else change
  // receive_can_unchoke.
  unsigned int currently_upload_unchoked() const {
    return m_currentlyUploadUnchoked;
  }
  unsigned int currently_download_unchoked() const {
    return m_currentlyDownloadUnchoked;
  }

  unsigned int max_upload_unchoked() const {
    return m_maxUploadUnchoked;
  }
  unsigned int max_download_unchoked() const {
    return m_maxDownloadUnchoked;
  }

  void set_max_upload_unchoked(unsigned int m);
  void set_max_download_unchoked(unsigned int m);

  void receive_upload_unchoke(int num);
  void receive_download_unchoke(int num);

  int retrieve_upload_can_unchoke();
  int retrieve_download_can_unchoke();

  void receive_tick();

private:
  iterator insert(const resource_manager_entry& entry);

  void update_group_iterators();
  void validate_group_iterators();

  unsigned int total_weight() const;

  int balance_unchoked(unsigned int weight,
                       unsigned int max_unchoked,
                       bool         is_up);

  unsigned int m_currentlyUploadUnchoked;
  unsigned int m_currentlyDownloadUnchoked;

  unsigned int m_maxUploadUnchoked;
  unsigned int m_maxDownloadUnchoked;
};

} // namespace torrent

#endif
