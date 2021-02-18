// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2005-2011, Jari Sundell <jaris@ifi.uio.no>

#ifndef LIBTORRENT_PROTOCOL_HANDSHAKE_ENCRYPTION_H
#define LIBTORRENT_PROTOCOL_HANDSHAKE_ENCRYPTION_H

#include <cstring>

#include "encryption_info.h"

namespace torrent {

class DiffieHellman;

class HandshakeEncryption {
public:
  using Retry = enum {
    RETRY_NONE,
    RETRY_PLAIN,
    RETRY_ENCRYPTED,
  };

  static constexpr int crypto_plain = 1;
  static constexpr int crypto_rc4   = 2;

  static const unsigned char    dh_prime[];
  static constexpr unsigned int dh_prime_length = 96;

  static const unsigned char    dh_generator[];
  static constexpr unsigned int dh_generator_length = 1;

  static const unsigned char    vc_data[];
  static constexpr unsigned int vc_length = 8;

  HandshakeEncryption(int options)
    : m_key(nullptr)
    , m_options(options)
    , m_crypto(0)
    , m_retry(RETRY_NONE)
    , m_syncLength(0)
    , m_lengthIA(0) {}

  bool has_crypto_plain() const {
    return m_crypto & crypto_plain;
  }
  bool has_crypto_rc4() const {
    return m_crypto & crypto_rc4;
  }

  DiffieHellman* key() {
    return m_key;
  }
  EncryptionInfo* info() {
    return &m_info;
  }

  int options() const {
    return m_options;
  }

  int crypto() const {
    return m_crypto;
  }
  void set_crypto(int val) {
    m_crypto = val;
  }

  Retry retry() const {
    return m_retry;
  }
  void set_retry(Retry val) {
    m_retry = val;
  }

  bool should_retry() const;

  const char* sync() const {
    return m_sync;
  }
  unsigned int sync_length() const {
    return m_syncLength;
  }

  void set_sync(const char* src, unsigned int len) {
    std::memcpy(m_sync, src, (m_syncLength = len));
  }
  char* modify_sync(unsigned int len) {
    m_syncLength = len;
    return m_sync;
  }

  unsigned int length_ia() const {
    return m_lengthIA;
  }
  void set_length_ia(unsigned int len) {
    m_lengthIA = len;
  }

  bool initialize();
  void cleanup();

  void initialize_decrypt(const char* origHash, bool incoming);
  void initialize_encrypt(const char* origHash, bool incoming);

  void deobfuscate_hash(char* src) const;

  void hash_req1_to_sync();
  void encrypt_vc_to_sync(const char* origHash);

  static void copy_vc(void* dest) {
    std::memset(dest, 0, vc_length);
  }
  static bool compare_vc(const void* buf);

private:
  DiffieHellman* m_key;

  // A pointer instead?
  EncryptionInfo m_info;

  int m_options;
  int m_crypto;

  Retry m_retry;

  char         m_sync[20];
  unsigned int m_syncLength;

  unsigned int m_lengthIA;
};

} // namespace torrent

#endif
