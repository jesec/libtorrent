// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2005-2011, Jari Sundell <jaris@ifi.uio.no>

#include "torrent/data/file.h"
#include "torrent/exceptions.h"

#include "torrent/data/file_list_iterator.h"

namespace torrent {

bool
FileListIterator::is_file() const {
  return m_depth >= 0 && m_depth + 1 == (int32_t)(*m_position)->path()->size();
}

bool
FileListIterator::is_empty() const {
  return (*m_position)->path()->size() == 0;
}

bool
FileListIterator::is_entering() const {
  return m_depth >= 0 && m_depth + 1 != (int32_t)(*m_position)->path()->size();
}

FileListIterator&
FileListIterator::operator++() {
  int32_t size = (*m_position)->path()->size();

  if (size == 0) {
    m_position++;
    return *this;
  }

  m_depth++;

  if (m_depth > size)
    throw internal_error("FileListIterator::operator ++() m_depth > size.");

  if (m_depth == size)
    m_depth = -(size - 1);

  if (m_depth == -(int32_t)(*m_position)->match_depth_next()) {
    m_depth = (*m_position)->match_depth_next();
    m_position++;
  }

  return *this;
}

FileListIterator&
FileListIterator::operator--() {
  // We're guaranteed that if m_depth != 0 then so is the path size,
  // so there's no need to check for it.
  if (m_depth == 0) {
    m_position--;

    // This ensures we properly start iterating the paths in a tree
    // without failing badly when size == 0.
    if ((*m_position)->path()->size() > 1)
      m_depth = -1;

  } else if (m_depth == (int32_t)(*m_position)->match_depth_prev()) {
    m_position--;

    // If only the last element differs, then we don't switch to
    // negative depth. Also make sure we skip the negative of the
    // current depth, as we index by the depth we're exiting from.
    if (m_depth + 1 != (int32_t)(*m_position)->path()->size())
      m_depth = -(m_depth + 1);

  } else {
    auto size = (int32_t)(*m_position)->path()->size();
    m_depth--;

    if (m_depth < -size)
      throw internal_error("FileListIterator::operator --() m_depth < -size.");

    if (m_depth == -size)
      m_depth = size - 1;
  }

  return *this;
}

FileListIterator&
FileListIterator::forward_current_depth() {
  uint32_t baseDepth = depth();

  if (!is_entering())
    return ++(*this);

  // If the above test was false then we know there must be a
  // 'leaving' at baseDepth before the end of the list.
  do {
    ++(*this);
  } while (depth() > baseDepth);

  return *this;
}

FileListIterator&
FileListIterator::backward_current_depth() {
  --(*this);

  if (is_entering() || is_file() || is_empty())
    return *this;

  if (depth() == 0)
    throw internal_error(
      "FileListIterator::backward_current_depth() depth() == 0.");

  uint32_t baseDepth = depth();

  while (depth() >= baseDepth)
    --(*this);

  return *this;
}

} // namespace torrent
