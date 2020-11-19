// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2005-2011, Jari Sundell <jaris@ifi.uio.no>

#ifndef LIBTORRENT_RC4_H
#define LIBTORRENT_RC4_H

#include "config.h"

#ifdef USE_CYRUS_RC4
extern "C" {
#include <rc4.h>
}
#else
#ifdef USE_OPENSSL
#include <openssl/rc4.h>
#endif
#endif

namespace torrent {

class RC4 {
public:
  RC4() {}

#ifdef USE_CYRUS_RC4
  RC4(const unsigned char key[], int len) {
    rc4_init(&m_key, key, len);
  }

  void crypt(const void* indata, void* outdata, unsigned int length) {
    rc4_encrypt(&m_key, (const char*)indata, (char*)outdata, length);
  }
  void crypt(void* data, unsigned int length) {
    rc4_encrypt(&m_key, (const char*)data, (char*)data, length);
  }

private:
  rc4_context_t m_key;

#else
#ifdef USE_OPENSSL
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
  RC4(const unsigned char key[], int len) {
    RC4_set_key(&m_key, len, key);
  }

  void crypt(const void* indata, void* outdata, unsigned int length) {
    ::RC4(
      &m_key, length, (const unsigned char*)indata, (unsigned char*)outdata);
  }
  void crypt(void* data, unsigned int length) {
    ::RC4(&m_key, length, (unsigned char*)data, (unsigned char*)data);
  }

private:
  RC4_KEY m_key;

#else
  RC4(const unsigned char key[], int len) {}

  void crypt(const void* indata, void* outdata, unsigned int length) {}
  void crypt(void* data, unsigned int length) {}
#endif
#endif
};

}; // namespace torrent

#endif
