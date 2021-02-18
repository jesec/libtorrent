// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2005-2011, Jari Sundell <jaris@ifi.uio.no>

#ifndef LIBTORRENT_TORRENT_ERROR_H
#define LIBTORRENT_TORRENT_ERROR_H

#include <torrent/common.h>

namespace torrent {

constexpr int e_none = 0;

// Handshake errors passed to signal handler
constexpr int e_handshake_not_bittorrent            = 1;
constexpr int e_handshake_not_accepting_connections = 2;
constexpr int e_handshake_duplicate                 = 3;
constexpr int e_handshake_unknown_download          = 4;
constexpr int e_handshake_inactive_download         = 5;
constexpr int e_handshake_unwanted_connection       = 6;
constexpr int e_handshake_is_self                   = 7;
constexpr int e_handshake_invalid_value             = 8;
constexpr int e_handshake_unencrypted_rejected      = 9;
constexpr int e_handshake_invalid_encryption        = 10;
constexpr int e_handshake_encryption_sync_failed    = 11;
constexpr int e_handshake_network_unreachable       = 13;
constexpr int e_handshake_network_timeout           = 14;
constexpr int e_handshake_invalid_order             = 15;
constexpr int e_handshake_toomanyfailed             = 16;
constexpr int e_handshake_no_peer_info              = 17;
constexpr int e_handshake_network_socket_error      = 18;
constexpr int e_handshake_network_read_error        = 19;
constexpr int e_handshake_network_write_error       = 20;

// constexpr int e_handshake_incoming                  = 13;
// constexpr int e_handshake_outgoing                  = 14;
// constexpr int e_handshake_outgoing_encrypted        = 15;
// constexpr int e_handshake_outgoing_proxy            = 16;
// constexpr int e_handshake_success                   = 17;
// constexpr int e_handshake_retry_plaintext           = 18;
// constexpr int e_handshake_retry_encrypted           = 19;

constexpr int e_last = 20;

const char*
strerror(int err) LIBTORRENT_EXPORT;

} // namespace torrent

#endif
