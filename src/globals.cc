// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2005-2011, Jari Sundell <jaris@ifi.uio.no>

#include "config.h"

#include "globals.h"
#include "manager.h"
#include "torrent/connection_manager.h"
#include "torrent/event.h"
#include "torrent/poll.h"

namespace torrent {

LIBTORRENT_EXPORT rak::priority_queue_default taskScheduler;
LIBTORRENT_EXPORT rak::timer cachedTime;

void
poll_event_open(Event* event) {
  manager->poll()->open(event);
  manager->connection_manager()->inc_socket_count();
}
void
poll_event_close(Event* event) {
  manager->poll()->close(event);
  manager->connection_manager()->dec_socket_count();
}
void
poll_event_closed(Event* event) {
  manager->poll()->closed(event);
  manager->connection_manager()->dec_socket_count();
}
void
poll_event_insert_read(Event* event) {
  manager->poll()->insert_read(event);
}
void
poll_event_insert_write(Event* event) {
  manager->poll()->insert_write(event);
}
void
poll_event_insert_error(Event* event) {
  manager->poll()->insert_error(event);
}
void
poll_event_remove_read(Event* event) {
  manager->poll()->remove_read(event);
}
void
poll_event_remove_write(Event* event) {
  manager->poll()->remove_write(event);
}
void
poll_event_remove_error(Event* event) {
  manager->poll()->remove_error(event);
}

} // namespace torrent
