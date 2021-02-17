// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2005-2011, Jari Sundell <jaris@ifi.uio.no>

#include "dht/dht_router.h"
#include "manager.h"
#include "torrent/exceptions.h"
#include "torrent/throttle.h"
#include "torrent/utils/log.h"

#include "torrent/dht_manager.h"

#define LT_LOG_THIS(log_fmt, ...)                                              \
  lt_log_print_subsystem(                                                      \
    torrent::LOG_DHT_MANAGER, "dht_manager", log_fmt, __VA_ARGS__);

namespace torrent {

DhtManager::~DhtManager() {
  stop();
  delete m_router;
}

void
DhtManager::initialize(const Object& dhtCache) {
  auto bind_address = utils::socket_address::cast_from(
    manager->connection_manager()->bind_address());

  LT_LOG_THIS("initializing (bind_address:%s)",
              bind_address->pretty_address_str().c_str());

  if (m_router != nullptr)
    throw internal_error(
      "DhtManager::initialize called with DHT already active.");

  try {
    m_router = new DhtRouter(dhtCache, bind_address);
  } catch (torrent::local_error& e) {
    LT_LOG_THIS("initialization failed (error:%s)", e.what());
  }
}

bool
DhtManager::start(port_type port) {
  LT_LOG_THIS("starting (port:%d)", port);

  if (m_router == nullptr)
    throw internal_error(
      "DhtManager::start called without initializing first.");

  m_port = port;

  try {
    m_router->start(port);
  } catch (torrent::local_error& e) {
    LT_LOG_THIS("start failed (error:%s)", e.what());
    return false;
  }

  return true;
}

void
DhtManager::stop() {
  if (m_router == nullptr)
    return;

  LT_LOG_THIS("stopping", 0);
  m_router->stop();
}

bool
DhtManager::is_active() const {
  return m_router != nullptr && m_router->is_active();
}

void
DhtManager::add_node(const sockaddr* addr, int port) {
  if (m_router != nullptr)
    m_router->contact(utils::socket_address::cast_from(addr), port);
}

void
DhtManager::add_node(const std::string& host, int port) {
  if (m_router != nullptr)
    m_router->add_contact(host, port);
}

Object*
DhtManager::store_cache(Object* container) const {
  if (m_router == nullptr)
    throw internal_error(
      "DhtManager::store_cache called but DHT not initialized.");

  return m_router->store_cache(container);
}

DhtManager::statistics_type
DhtManager::get_statistics() const {
  return m_router->get_statistics();
}

void
DhtManager::reset_statistics() {
  m_router->reset_statistics();
}

void
DhtManager::set_upload_throttle(Throttle* t) {
  if (m_router->is_active())
    throw internal_error(
      "DhtManager::set_upload_throttle() called while DHT server active.");

  m_router->set_upload_throttle(t->throttle_list());
}

void
DhtManager::set_download_throttle(Throttle* t) {
  if (m_router->is_active())
    throw internal_error(
      "DhtManager::set_download_throttle() called while DHT server active.");

  m_router->set_download_throttle(t->throttle_list());
}

} // namespace torrent
