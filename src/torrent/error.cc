// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2005-2011, Jari Sundell <jaris@ifi.uio.no>

#include "config.h"

#include "error.h"

namespace torrent {

static const char* errorStrings[e_last + 1] = {
  "unknown error", // e_none

  "not BitTorrent protocol",         // eh_not_bittorrent
  "not accepting connections",       // eh_not_accepting_connections
  "duplicate peer ID",               // eh_duplicate
  "unknown download",                // eh_unknown_download
  "download inactive",               // eh_inactive_download
  "seeder rejected",                 // eh_unwanted_connection
  "is self",                         // eh_is_self
  "invalid value received",          // eh_invalid_value
  "unencrypted connection rejected", // eh_unencrypted_rejected
  "invalid encryption method",       // eh_invalid_encryption
  "encryption sync failed",          // eh_encryption_sync_failed
  "<deprecated>",                    // eh_
  "network unreachable",             // eh_network_unreachable
  "network timeout",                 // eh_network_timeout
  "invalid message order",           // eh_invalid_order
  "too many failed chunks",          // eh_toomanyfailed
  "no peer info",                    // eh_no_peer_info
  "network socket error",            // eh_network_socket_error
  "network read error",              // eh_network_read_error
  "network write error",             // eh_network_write_error

  //   "", // e_handshake_incoming
  //   "", // e_handshake_outgoing
  //   "", // e_handshake_outgoing_encrypted
  //   "", // e_handshake_outgoing_proxy
  //   "", // e_handshake_success
  //   "", // e_handshake_retry_plaintext
  //   ""  // e_handshake_retry_encrypted
};

const char*
strerror(int err) {
  if (err < 0 || err > e_last)
    return "Unknown error";

  return errorStrings[err];
}

} // namespace torrent
