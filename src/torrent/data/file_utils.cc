// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2005-2011, Jari Sundell <jaris@ifi.uio.no>

#include <cstring>

#include "torrent/data/file.h"
#include "torrent/data/file_utils.h"
#include "torrent/exceptions.h"

namespace torrent {

FileList::iterator
file_split(FileList*          fileList,
           FileList::iterator position,
           uint64_t           maxSize,
           const std::string& suffix) {
  const Path* srcPath   = (*position)->path();
  uint64_t    splitSize = ((*position)->size_bytes() + maxSize - 1) / maxSize;

  if (srcPath->empty() || (*position)->size_bytes() == 0)
    throw input_error(
      "Tried to split a file with an empty path or zero length file.");

  if (splitSize > 1000)
    throw input_error("Tried to split a file into more than 1000 parts.");

  // Also replace dwnlctor's vector.
  FileList::split_type* splitList = new FileList::split_type[splitSize];
  FileList::split_type* splitItr  = splitList;

  unsigned int nameSize = srcPath->back().size() + suffix.size();
  char         name[nameSize + 4];

  std::memcpy(name, srcPath->back().c_str(), srcPath->back().size());
  std::memcpy(name + srcPath->back().size(), suffix.c_str(), suffix.size());

  for (unsigned int i = 0; i != splitSize; ++i, ++splitItr) {
    if (i == splitSize - 1 && (*position)->size_bytes() % maxSize != 0)
      splitItr->first = (*position)->size_bytes() % maxSize;
    else
      splitItr->first = maxSize;

    name[nameSize + 0] = '0' + (i / 100) % 10;
    name[nameSize + 1] = '0' + (i / 10) % 10;
    name[nameSize + 2] = '0' + (i / 1) % 10;
    name[nameSize + 3] = '\0';

    splitItr->second        = *srcPath;
    splitItr->second.back() = name;
  }

  return fileList->split(position, splitList, splitItr).second;
}

void
file_split_all(FileList*          fileList,
               uint64_t           maxSize,
               const std::string& suffix) {
  if (maxSize == 0)
    throw input_error("Tried to split torrent files into zero sized chunks.");

  FileList::iterator itr = fileList->begin();

  while (itr != fileList->end())
    if ((*itr)->size_bytes() > maxSize && !(*itr)->path()->empty())
      itr = file_split(fileList, itr, maxSize, suffix);
    else
      itr++;
}

} // namespace torrent
