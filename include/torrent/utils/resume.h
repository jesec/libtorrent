// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2005-2011, Jari Sundell <jaris@ifi.uio.no>

// Various functions for loading and saving various states of a
// download to an Object.
//
// These functions use only the public interface, and thus the client
// may choose to replace these with their own resume code.

// Should propably move this into a sub-directory.

#ifndef LIBTORRENT_UTILS_RESUME_H
#define LIBTORRENT_UTILS_RESUME_H

#include <torrent/common.h>

namespace torrent {

// When saving resume data for a torrent that is currently active, set
// 'onlyCompleted' to ensure that a crash, etc, will cause incomplete
// files to be hashed.

void
resume_load_progress(Download download, const Object& object) LIBTORRENT_EXPORT;
void
resume_save_progress(Download download, Object& object) LIBTORRENT_EXPORT;
void
resume_clear_progress(Download download, Object& object) LIBTORRENT_EXPORT;

bool
resume_load_bitfield(Download download, const Object& object) LIBTORRENT_EXPORT;
void
resume_save_bitfield(Download download, Object& object) LIBTORRENT_EXPORT;

// Do not call 'resume_load_uncertain_pieces' directly.
void
resume_load_uncertain_pieces(Download      download,
                             const Object& object) LIBTORRENT_EXPORT;
void
resume_save_uncertain_pieces(Download download,
                             Object&  object) LIBTORRENT_EXPORT;

bool
resume_check_target_files(Download      download,
                          const Object& object) LIBTORRENT_EXPORT;

void
resume_load_file_priorities(Download      download,
                            const Object& object) LIBTORRENT_EXPORT;
void
resume_save_file_priorities(Download download,
                            Object&  object) LIBTORRENT_EXPORT;

void
resume_load_addresses(Download      download,
                      const Object& object) LIBTORRENT_EXPORT;
void
resume_save_addresses(Download download, Object& object) LIBTORRENT_EXPORT;

void
resume_load_tracker_settings(Download      download,
                             const Object& object) LIBTORRENT_EXPORT;
void
resume_save_tracker_settings(Download download,
                             Object&  object) LIBTORRENT_EXPORT;

} // namespace torrent

#endif
