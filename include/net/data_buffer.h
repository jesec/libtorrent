// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2005-2011, Jari Sundell <jaris@ifi.uio.no>

#ifndef LIBTORRENT_NET_DATA_BUFFER_H
#define LIBTORRENT_NET_DATA_BUFFER_H

#include <cinttypes>
#include <memory>

namespace torrent {

// Recipient must call clear() when done with the buffer.
struct DataBuffer {
  DataBuffer() = default;
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
    set(nullptr, nullptr, false);
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
    return m_data == nullptr;
  }
  size_t length() const {
    return m_end - m_data;
  }

  void clear();
  void set(char* data, char* end, bool owned);

private:
  char* m_data{ nullptr };
  char* m_end{ nullptr };

  // Used to indicate if buffer held by PCB is its own and needs to be
  // deleted after transmission (false if shared with other connections).
  bool m_owned{ true };
};

inline void
DataBuffer::clear() {
  if (!empty() && m_owned)
    delete[] m_data;

  m_data = m_end = nullptr;
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
