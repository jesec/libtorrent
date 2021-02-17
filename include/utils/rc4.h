// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2005-2011, Jari Sundell <jaris@ifi.uio.no>

#ifndef LIBTORRENT_RC4_H
#define LIBTORRENT_RC4_H

#include <openssl/rc4.h>

namespace torrent {

class RC4 {
public:
  RC4() = default;

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
};

} // namespace torrent

#endif
