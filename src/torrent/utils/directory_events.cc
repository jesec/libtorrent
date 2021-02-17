// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2005-2011, Jari Sundell <jaris@ifi.uio.no>

#include "torrent/buildinfo.h"

#include <cerrno>
#include <string>
#include <unistd.h>

#ifdef LT_HAVE_INOTIFY
#include <sys/inotify.h>
#endif

#include "manager.h"
#include "net/socket_fd.h"
#include "torrent/exceptions.h"
#include "torrent/poll.h"
#include "torrent/utils/directory_events.h"
#include "torrent/utils/error_number.h"

namespace torrent {

bool
directory_events::open() {
  if (m_fileDesc != -1)
    return true;

  utils::error_number::current().clear_global();

#ifdef LT_HAVE_INOTIFY
  m_fileDesc = inotify_init();

  if (!SocketFd(m_fileDesc).set_nonblock()) {
    SocketFd(m_fileDesc).close();
    m_fileDesc = -1;
  }
#else
  utils::error_number::set_global(utils::error_number::e_nodev);
#endif

  if (m_fileDesc == -1)
    return false;

  manager->poll()->open(this);
  manager->poll()->insert_read(this);

  return true;
}

void
directory_events::close() {
  if (m_fileDesc == -1)
    return;

  manager->poll()->remove_read(this);
  manager->poll()->close(this);

  ::close(m_fileDesc);
  m_fileDesc = -1;
  m_wd_list.clear();
}

#ifdef LT_HAVE_INOTIFY
void
directory_events::notify_on(const std::string& path,
                            int                flags,
                            const slot_string& slot) {
  if (path.empty())
    throw input_error("Cannot add notification event for empty paths.");

  int in_flags = IN_MASK_ADD;

#ifdef IN_EXCL_UNLINK
  in_flags |= IN_EXCL_UNLINK;
#endif

#ifdef IN_ONLYDIR
  in_flags |= IN_ONLYDIR;
#endif

  if ((flags & flag_on_added))
    in_flags |= (IN_CREATE | IN_MOVED_TO);

  if ((flags & flag_on_updated))
    in_flags |= IN_CLOSE_WRITE;

  if ((flags & flag_on_removed))
    in_flags |= (IN_DELETE | IN_MOVED_FROM);

  int result = inotify_add_watch(m_fileDesc, path.c_str(), in_flags);

  if (result == -1)
    throw input_error("Call to inotify_add_watch(...) failed: " +
                      std::string(utils::error_number::current().c_str()));

  wd_list::iterator itr = m_wd_list.insert(m_wd_list.end(), watch_descriptor());
  itr->descriptor       = result;
  itr->path             = path + (*path.rbegin() != '/' ? "/" : "");
  itr->slot             = slot;
}
#else
void
directory_events::notify_on(const std::string&, int, const slot_string&) {
  throw input_error("No support for inotify.");
}
#endif

void
directory_events::event_read() {
#ifdef LT_HAVE_INOTIFY
  char    buffer[2048];
  ssize_t result = ::read(m_fileDesc, buffer, 2048);

  if (result < 0)
    return;

  if ((size_t)result < sizeof(struct inotify_event))
    return;

  struct inotify_event* event = (struct inotify_event*)buffer;

  while (event + 1 <= (struct inotify_event*)(buffer + result)) {
    char* next_event = (char*)event + sizeof(struct inotify_event) + event->len;

    if (event->len == 0 || next_event > buffer + 2048)
      return;

    wd_list::const_iterator itr =
      std::find_if(m_wd_list.begin(),
                   m_wd_list.end(),
                   [ewd = event->wd](const watch_descriptor& wd) {
                     return wd.compare_desc(ewd);
                   });

    if (itr != m_wd_list.end()) {
      std::string sname(event->name);
      if ((sname.substr(sname.find_last_of('.')) == ".torrent"))
        itr->slot(itr->path + event->name);
    }

    event = (struct inotify_event*)(next_event);
  }
#endif
}

void
directory_events::event_write() {}

void
directory_events::event_error() {}

} // namespace torrent
