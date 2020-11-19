// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2005-2011, Jari Sundell <jaris@ifi.uio.no>

#ifndef LIBTORRENT_NET_DATA_BUFFER_H
#define LIBTORRENT_NET_DATA_BUFFER_H

#include <cinttypes>
#include <memory>

namespace torrent {

// Recipient must call clear() when done with the buffer.
struct DataBuffer {
  DataBuffer()
    : m_data(NULL)
    , m_end(NULL)
    , m_owned(true) {}
  DataBuffer(char* data, char* end)
    : m_data(data)
    , m_end(end)
    , m_owned(true) {}

  DataBuffer clone() const {
    DataBuffer d = *this;
    d.m_owned    = false;
    return d;
  }
  DataBuffer release() {
    DataBuffer d = *this;
    set(NULL, NULL, false);
    return d;
  }

  char* data() const {
    return m_data;
  }
  char* end() const {
    return m_end;
  }

  bool owned() const {
    return m_owned;
  }
  bool empty() const {
    return m_data == NULL;
  }
  size_t length() const {
    return m_end - m_data;
  }

  void clear();
  void set(char* data, char* end, bool owned);

private:
  char* m_data;
  char* m_end;

  // Used to indicate if buffer held by PCB is its own and needs to be
  // deleted after transmission (false if shared with other connections).
  bool m_owned;
};

inline void
DataBuffer::clear() {
  if (!empty() && m_owned)
    delete[] m_data;

  m_data = m_end = NULL;
  m_owned        = false;
}

inline void
DataBuffer::set(char* data, char* end, bool owned) {
  m_data  = data;
  m_end   = end;
  m_owned = owned;
}

} // namespace torrent

#endif
