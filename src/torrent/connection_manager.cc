// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2005-2011, Jari Sundell <jaris@ifi.uio.no>

#include <sys/types.h>

#include "manager.h"
#include "net/listen.h"
#include "torrent/connection_manager.h"
#include "torrent/error.h"
#include "torrent/exceptions.h"
#include "torrent/utils/address_info.h"
#include "torrent/utils/socket_address.h"

namespace torrent {

// Fix TrackerUdp, etc, if this is made async.
static ConnectionManager::slot_resolver_result_type*
resolve_host(const char*                                  host,
             int                                          family,
             int                                          socktype,
             ConnectionManager::slot_resolver_result_type slot) {
  if (manager->main_thread_main()->is_current())
    thread_base::release_global_lock();

  utils::address_info* ai;
  int                  err;

  if ((err = utils::address_info::get_address_info(
         host, family, socktype, &ai)) != 0) {
    if (manager->main_thread_main()->is_current())
      thread_base::acquire_global_lock();

    slot(NULL, err);
    return NULL;
  }

  utils::socket_address sa;
  sa.copy(*ai->address(), ai->length());
  utils::address_info::free_address_info(ai);

  if (manager->main_thread_main()->is_current())
    thread_base::acquire_global_lock();

  slot(sa.c_sockaddr(), 0);
  return NULL;
}

ConnectionManager::ConnectionManager()
  : m_size(0)
  , m_maxSize(0)
  ,

  m_priority(iptos_throughput)
  , m_sendBufferSize(0)
  , m_receiveBufferSize(0)
  , m_encryptionOptions(encryption_none)
  ,

  m_listen(new Listen)
  , m_listen_port(0)
  , m_listen_backlog(SOMAXCONN) {

  m_bindAddress  = (new utils::socket_address())->c_sockaddr();
  m_localAddress = (new utils::socket_address())->c_sockaddr();
  m_proxyAddress = (new utils::socket_address())->c_sockaddr();

  utils::socket_address::cast_from(m_bindAddress)->clear();
  utils::socket_address::cast_from(m_localAddress)->clear();
  utils::socket_address::cast_from(m_proxyAddress)->clear();

  m_slot_resolver = [](const char*               host,
                       int                       family,
                       int                       socktype,
                       slot_resolver_result_type slot) {
    return resolve_host(host, family, socktype, slot);
  };
}

ConnectionManager::~ConnectionManager() {
  delete m_listen;

  delete m_bindAddress;
  delete m_localAddress;
  delete m_proxyAddress;
}

bool
ConnectionManager::can_connect() const {
  return m_size < m_maxSize;
}

void
ConnectionManager::set_send_buffer_size(uint32_t s) {
  m_sendBufferSize = s;
}

void
ConnectionManager::set_receive_buffer_size(uint32_t s) {
  m_receiveBufferSize = s;
}

void
ConnectionManager::set_encryption_options(uint32_t options) {
  m_encryptionOptions = options;
}

void
ConnectionManager::set_bind_address(const sockaddr* sa) {
  const utils::socket_address* rsa = utils::socket_address::cast_from(sa);

  if (rsa->family() != utils::socket_address::af_inet)
    throw input_error(
      "Tried to set a bind address that is not an af_inet address.");

  utils::socket_address::cast_from(m_bindAddress)->copy(*rsa, rsa->length());
}

void
ConnectionManager::set_local_address(const sockaddr* sa) {
  const utils::socket_address* rsa = utils::socket_address::cast_from(sa);

  if (rsa->family() != utils::socket_address::af_inet)
    throw input_error(
      "Tried to set a local address that is not an af_inet address.");

  utils::socket_address::cast_from(m_localAddress)->copy(*rsa, rsa->length());
}

void
ConnectionManager::set_proxy_address(const sockaddr* sa) {
  const utils::socket_address* rsa = utils::socket_address::cast_from(sa);

  if (rsa->family() != utils::socket_address::af_inet)
    throw input_error(
      "Tried to set a proxy address that is not an af_inet address.");

  utils::socket_address::cast_from(m_proxyAddress)->copy(*rsa, rsa->length());
}

uint32_t
ConnectionManager::filter(const sockaddr* sa) {
  if (!m_slot_filter)
    return 1;
  else
    return m_slot_filter(sa);
}

bool
ConnectionManager::listen_open(port_type begin, port_type end) {
  if (!m_listen->open(begin,
                      end,
                      m_listen_backlog,
                      utils::socket_address::cast_from(m_bindAddress)))
    return false;

  m_listen_port = m_listen->port();

  return true;
}

void
ConnectionManager::listen_close() {
  m_listen->close();
}

void
ConnectionManager::set_listen_backlog(int v) {
  if (v < 1 || v >= (1 << 16))
    throw input_error("backlog value out of bounds");

  if (m_listen->is_open())
    throw input_error("backlog value must be set before listen port is opened");

  m_listen_backlog = v;
}

} // namespace torrent
