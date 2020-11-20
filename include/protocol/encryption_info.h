// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2005-2011, Jari Sundell <jaris@ifi.uio.no>

#ifndef LIBTORRENT_PROTOCOL_ENCRYPTION_H
#define LIBTORRENT_PROTOCOL_ENCRYPTION_H

#include "utils/rc4.h"

namespace torrent {

class EncryptionInfo {
public:
  EncryptionInfo()
    : m_encrypted(false)
    , m_obfuscated(false)
    , m_decryptValid(false){};

  void encrypt(const void* indata, void* outdata, unsigned int length) {
    m_encrypt.crypt(indata, outdata, length);
  }
  void encrypt(void* data, unsigned int length) {
    m_encrypt.crypt(data, length);
  }
  void decrypt(const void* indata, void* outdata, unsigned int length) {
    m_decrypt.crypt(indata, outdata, length);
  }
  void decrypt(void* data, unsigned int length) {
    m_decrypt.crypt(data, length);
  }

  bool is_encrypted() const {
    return m_encrypted;
  }
  bool is_obfuscated() const {
    return m_obfuscated;
  }
  bool decrypt_valid() const {
    return m_decryptValid;
  }

  void set_obfuscated() {
    m_obfuscated = true;
    m_encrypted = m_decryptValid = false;
  }
  void set_encrypt(const RC4& encrypt) {
    m_encrypt   = encrypt;
    m_encrypted = m_obfuscated = true;
  }
  void set_decrypt(const RC4& decrypt) {
    m_decrypt      = decrypt;
    m_decryptValid = true;
  }

private:
  bool m_encrypted;
  bool m_obfuscated;
  bool m_decryptValid;

  RC4 m_encrypt;
  RC4 m_decrypt;
};

} // namespace torrent

#endif
