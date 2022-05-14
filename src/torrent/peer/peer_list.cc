// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2005-2011, Jari Sundell <jaris@ifi.uio.no>

#include <algorithm>
#include <functional>

#include "download/available_list.h"
#include "globals.h"
#include "manager.h"
#include "torrent/download_info.h"
#include "torrent/exceptions.h"
#include "torrent/peer/client_list.h"
#include "torrent/peer/peer_info.h"
#include "torrent/peer/peer_list.h"
#include "torrent/utils/log.h"
#include "torrent/utils/socket_address.h"

#define LT_LOG_EVENTS(log_fmt, ...)                                            \
  lt_log_print_info(                                                           \
    LOG_PEER_LIST_EVENTS, m_info, "peer_list", log_fmt, __VA_ARGS__);
#define LT_LOG_ADDRESS(log_fmt, ...)                                           \
  lt_log_print_info(                                                           \
    LOG_PEER_LIST_ADDRESS, m_info, "peer_list", log_fmt, __VA_ARGS__);
#define LT_LOG_SA_FMT "'%s:%" PRIu16 "'"

namespace torrent {

// TODO: Clean up...
bool
socket_address_less(const sockaddr* s1, const sockaddr* s2) {
  const utils::socket_address* sa1 = utils::socket_address::cast_from(s1);
  const utils::socket_address* sa2 = utils::socket_address::cast_from(s2);

  if (sa1->family() != sa2->family()) {
    return sa1->family() < sa2->family();

  } else if (sa1->family() == utils::socket_address::af_inet) {
    // Sort by hardware byte order to ensure proper ordering for
    // humans.
    return sa1->sa_inet()->address_h() < sa2->sa_inet()->address_h();

  } else if (sa1->family() == utils::socket_address::af_inet6) {
    const in6_addr addr1 = sa1->sa_inet6()->address();
    const in6_addr addr2 = sa2->sa_inet6()->address();

    return memcmp(&addr1, &addr2, sizeof(in6_addr)) < 0;

  } else {
    throw internal_error(
      "socket_address_key(...) tried to compare an invalid family type.");
  }
}

struct peer_list_equal_port
  : public std::binary_function<PeerList::reference, uint16_t, bool> {
  bool operator()(PeerList::reference p, uint16_t port) {
    return utils::socket_address::cast_from(p.second->socket_address())
             ->port() == port;
  }
};

//
// PeerList:
//

PeerList::PeerList()
  : m_available_list(new AvailableList) {}

PeerList::~PeerList() {
  LT_LOG_EVENTS("deleting list total:%" PRIuPTR " available:%" PRIuPTR,
                size(),
                m_available_list->size());

  for (const auto& [address, peerInfo] : *this) {
    delete peerInfo;
  }

  base_type::clear();

  m_info = nullptr;
  delete m_available_list;
}

void
PeerList::set_info(DownloadInfo* info) {
  m_info = info;

  LT_LOG_EVENTS("creating list", 0);
}

PeerInfo*
PeerList::insert_address(const sockaddr* sa, int flags) {
  socket_address_key sock_key = socket_address_key::from_sockaddr(sa);

  if (sock_key.is_valid() && !socket_address_key::is_comparable_sockaddr(sa)) {
    LT_LOG_EVENTS("address not comparable", 0);
    return nullptr;
  }

  const utils::socket_address* address = utils::socket_address::cast_from(sa);

  range_type range = base_type::equal_range(sock_key);

  // Do some special handling if we got a new port number but the
  // address was present.
  //
  // What we do depends on the flags, but for now just allow one
  // PeerInfo per address key and do nothing.
  if (range.first != range.second) {
    LT_LOG_EVENTS("address already exists " LT_LOG_SA_FMT,
                  address->address_str().c_str(),
                  address->port());
    return nullptr;
  }

  auto peerInfo = new PeerInfo(sa);
  peerInfo->set_listen_port(address->port());

  manager->client_list()->retrieve_unknown(&peerInfo->mutable_client_info());

  base_type::insert(range.second, value_type(sock_key, peerInfo));

  if ((flags & address_available) && peerInfo->listen_port() != 0) {
    m_available_list->push_back(address);
    LT_LOG_EVENTS("added available address " LT_LOG_SA_FMT,
                  address->address_str().c_str(),
                  address->port());
  } else {
    LT_LOG_EVENTS("added unavailable address " LT_LOG_SA_FMT,
                  address->address_str().c_str(),
                  address->port());
  }

  return peerInfo;
}

inline bool
socket_address_less_rak(const utils::socket_address& s1,
                        const utils::socket_address& s2) {
  return socket_address_less(s1.c_sockaddr(), s2.c_sockaddr());
}

uint32_t
PeerList::insert_available(const void* al) {
  auto addressList = static_cast<const AddressList*>(al);

  uint32_t inserted = 0;
  uint32_t invalid  = 0;
  uint32_t unneeded = 0;
  uint32_t updated  = 0;

  if (m_available_list->size() + addressList->size() >
      m_available_list->capacity())
    m_available_list->reserve(m_available_list->size() + addressList->size() +
                              128);

  // Optimize this so that we don't traverse the tree for every
  // insert, since we know 'al' is sorted.

  auto itr       = addressList->begin();
  auto last      = addressList->end();
  auto availItr  = m_available_list->begin();
  auto availLast = m_available_list->end();

  for (; itr != last; itr++) {
    if (!socket_address_key::is_comparable_sockaddr(itr->c_sockaddr()) ||
        itr->port() == 0) {
      invalid++;
      LT_LOG_ADDRESS("skipped invalid address " LT_LOG_SA_FMT,
                     itr->address_str().c_str(),
                     itr->port());
      continue;
    }

    availItr = std::find_if(
      availItr, availLast, [&itr](const utils::socket_address& sa) {
        return socket_address_less_rak(sa, *itr);
      });

    if (availItr != availLast &&
        !socket_address_less(availItr->c_sockaddr(), itr->c_sockaddr())) {
      // The address is already in m_available_list, so don't bother
      // going further.
      unneeded++;
      continue;
    }

    socket_address_key sock_key =
      socket_address_key::from_sockaddr(itr->c_sockaddr());

    // Check if the peerinfo exists, if it does, check if we would
    // ever want to connect. Just update the timer for the last
    // availability notice if the peer isn't really ideal, but might
    // be used in an emergency.
    range_type range = base_type::equal_range(sock_key);

    if (range.first != range.second) {
      // Add some logic here to select the best PeerInfo, but for now
      // just assume the first one is the only one that exists.
      PeerInfo* peerInfo = range.first->second;

      if (peerInfo->listen_port() == 0)
        peerInfo->set_port(itr->port());

      if (peerInfo->connection() != nullptr ||
          peerInfo->last_handshake() + 600 > (uint32_t)cachedTime.seconds()) {
        updated++;
        continue;
      }

      // If the peer has sent us bad chunks or we just connected or
      // tried to do so a few minutes ago, only update its
      // availability timer.
    }

    // Should we perhaps add to available list even though we don't
    // want the peer, just to ensure we don't need to search for the
    // PeerInfo every time it gets reported. Though I'd assume it
    // won't happen often enough to be worth it.

    inserted++;
    m_available_list->push_back(&*itr);

    LT_LOG_ADDRESS("added available address " LT_LOG_SA_FMT,
                   itr->address_str().c_str(),
                   itr->port());
  }

  LT_LOG_EVENTS("inserted peers"
                " inserted:%" PRIu32 " invalid:%" PRIu32 " unneeded:%" PRIu32
                " updated:%" PRIu32 " total:%" PRIuPTR " available:%" PRIuPTR,
                inserted,
                invalid,
                unneeded,
                updated,
                size(),
                m_available_list->size());

  return inserted;
}

uint32_t
PeerList::available_list_size() const {
  return m_available_list->size();
}

PeerInfo*
PeerList::connected(const sockaddr* sa, int flags) {
  const utils::socket_address* address  = utils::socket_address::cast_from(sa);
  socket_address_key           sock_key = socket_address_key::from_sockaddr(sa);

  if (!sock_key.is_valid() || !socket_address_key::is_comparable_sockaddr(sa))
    return nullptr;

  PeerInfo*  peerInfo;
  range_type range = base_type::equal_range(sock_key);

  if (range.first == range.second) {
    // Create a new entry.
    peerInfo = new PeerInfo(sa);
    base_type::insert(range.second, value_type(sock_key, peerInfo));
  } else if (!range.first->second->is_connected()) {
    // Use an old entry.
    peerInfo = range.first->second;
    peerInfo->set_port(address->port());
  } else {
    // Make sure we don't end up throwing away the port the host is
    // actually listening on, when there may be several simultaneous
    // connection attempts to/from different ports.
    //
    // This also ensure we can connect to peers running on the same
    // host as the tracker.
    if (flags & connect_keep_handshakes &&
        range.first->second->is_handshake() &&
        utils::socket_address::cast_from(range.first->second->socket_address())
            ->port() != address->port())
      m_available_list->buffer()->push_back(*address);

    return nullptr;
  }

  if (flags & connect_filter_recent &&
      peerInfo->last_handshake() + 600 > (uint32_t)cachedTime.seconds())
    return nullptr;

  if (!(flags & connect_incoming))
    peerInfo->set_listen_port(address->port());

  if (flags & connect_incoming)
    peerInfo->set_flags(PeerInfo::flag_incoming);
  else
    peerInfo->unset_flags(PeerInfo::flag_incoming);

  peerInfo->set_flags(PeerInfo::flag_connected);
  peerInfo->set_last_handshake(cachedTime.seconds());

  return peerInfo;
}

// Make sure we properly clear port when disconnecting.

void
PeerList::disconnected(PeerInfo* p, int flags) {
  socket_address_key sock_key =
    socket_address_key::from_sockaddr(p->socket_address());

  range_type range = base_type::equal_range(sock_key);

  auto itr = std::find_if(
    range.first, range.second, [p](value_type& v) { return p == v.second; });

  if (itr == range.second) {
    if (std::find_if(base_type::begin(), base_type::end(), [p](value_type& v) {
          return p == v.second;
        }) == base_type::end())
      throw internal_error(
        "PeerList::disconnected(...) itr == range.second, doesn't exist.");
    else
      throw internal_error(
        "PeerList::disconnected(...) itr == range.second, not in the range.");
  }

  disconnected(itr, flags);
}

PeerList::iterator
PeerList::disconnected(iterator itr, int flags) {
  if (itr == base_type::end())
    throw internal_error("PeerList::disconnected(...) itr == end().");

  if (!itr->second->is_connected())
    throw internal_error("PeerList::disconnected(...) !itr->is_connected().");

  if (itr->second->transfer_counter() != 0) {
    // Currently we only log these as it only affects the culling of
    // peers.
    LT_LOG_EVENTS("disconnected with non-zero transfer counter (%" PRIu32
                  ") for peer %40s",
                  itr->second->transfer_counter(),
                  itr->second->id_hex());
  }

  itr->second->unset_flags(PeerInfo::flag_connected);

  // Replace the socket address port with the listening port so that
  // future outgoing connections will connect to the right port.
  itr->second->set_port(0);

  if (flags & disconnect_set_time)
    itr->second->set_last_connection(cachedTime.seconds());

  if (flags & disconnect_available && itr->second->listen_port() != 0)
    m_available_list->push_back(
      utils::socket_address::cast_from(itr->second->socket_address()));

  // Do magic to get rid of unneeded entries.
  return ++itr;
}

uint32_t
PeerList::cull_peers(int flags) {
  uint32_t counter = 0;
  uint32_t timer;

  if (flags & cull_old)
    timer = cachedTime.seconds() - 24 * 60 * 60;
  else
    timer = 0;

  for (auto itr = base_type::begin(); itr != base_type::end();) {
    if (itr->second->is_connected() ||
        itr->second->transfer_counter() !=
          0 || // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
        itr->second->last_connection() >= timer ||

        (flags & cull_keep_interesting &&
         (itr->second->failed_counter() != 0 || itr->second->is_blocked()))) {
      itr++;
      continue;
    }

    // ##################### TODO: LOG CULLING OF PEERS ######################
    //   *** AND STATS OF DISCONNECTING PEERS (the peer info...)...

    // The key is a pointer to a member in the value, although the key
    // shouldn't actually be used in erase (I think), just ot be safe
    // we delete it after erase.
    auto      tmp      = itr++;
    PeerInfo* peerInfo = tmp->second;

    base_type::erase(tmp);
    delete peerInfo;

    counter++;
  }

  return counter;
}

} // namespace torrent
