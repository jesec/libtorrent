// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2005-2011, Jari Sundell <jaris@ifi.uio.no>

#ifndef LIBTORRENT_HASH_COMPUTE_H
#define LIBTORRENT_HASH_COMPUTE_H

#include <cstring>
#include <openssl/sha.h>

namespace torrent {

class Sha1 {
public:
  void init();
  void update(const void* data, unsigned int length);

  void final_c(char* buffer);

private:
  SHA_CTX m_ctx;
};

#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
inline void
Sha1::init() {
  SHA1_Init(&m_ctx);
}

inline void
Sha1::update(const void* data, unsigned int length) {
  SHA1_Update(&m_ctx, (const void*)data, length);
}

inline void
Sha1::final_c(char* buffer) {
  SHA1_Final((unsigned char*)buffer, &m_ctx);
}

inline void
sha1_salt(const char*  salt,
          unsigned int saltLength,
          const char*  key,
          unsigned int keyLength,
          void*        out) {
  Sha1 sha1;

  sha1.init();
  sha1.update(salt, saltLength);
  sha1.update(key, keyLength);
  sha1.final_c((char*)out);
}

inline void
sha1_salt(const char*  salt,
          unsigned int saltLength,
          const char*  key1,
          unsigned int key1Length,
          const char*  key2,
          unsigned int key2Length,
          void*        out) {
  Sha1 sha1;

  sha1.init();
  sha1.update(salt, saltLength);
  sha1.update(key1, key1Length);
  sha1.update(key2, key2Length);
  sha1.final_c((char*)out);
}

} // namespace torrent

#endif
