// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2005-2011, Jari Sundell <jaris@ifi.uio.no>

#include <sys/socket.h>

#include "net/socket_fd.h"
#include "torrent/exceptions.h"
#include "torrent/utils/error_number.h"
#include "torrent/utils/thread_interrupt.h"
#include "utils/instrumentation.h"

namespace torrent {

thread_interrupt::thread_interrupt(int fd)
  : m_poking(false) {
  m_fileDesc = fd;
  get_fd().set_nonblock();
}

thread_interrupt::~thread_interrupt() {
  if (m_fileDesc == -1)
    return;

  ::close(m_fileDesc);
  m_fileDesc = -1;
}

bool
thread_interrupt::poke() {
  if (!__sync_bool_compare_and_swap(&m_other->m_poking, false, true))
    return true;

  int result = ::send(m_fileDesc, "a", 1, 0);

  if (result == 0 ||
      (result == -1 && !utils::error_number::current().is_blocked_momentary()))
    throw internal_error("Invalid result writing to thread_interrupt socket.");

  instrumentation_update(INSTRUMENTATION_POLLING_INTERRUPT_POKE, 1);

  return true;
}

thread_interrupt::pair_type
thread_interrupt::create_pair() {
  int fd1, fd2;

  if (!SocketFd::open_socket_pair(fd1, fd2))
    throw internal_error("Could not create socket pair for thread_interrupt: " +
                         utils::error_number::current().message() + ".");

  thread_interrupt* t1 = new thread_interrupt(fd1);
  thread_interrupt* t2 = new thread_interrupt(fd2);

  t1->m_other = t2;
  t2->m_other = t1;

  return pair_type(t1, t2);
}

void
thread_interrupt::event_read() {
  char buffer[256];
  int  result = ::recv(m_fileDesc, buffer, 256, 0);

  if (result == 0 ||
      (result == -1 && !utils::error_number::current().is_blocked_momentary()))
    throw internal_error(
      "Invalid result reading from thread_interrupt socket.");

  instrumentation_update(INSTRUMENTATION_POLLING_INTERRUPT_READ_EVENT, 1);

  __sync_bool_compare_and_swap(&m_poking, true, false);
}

} // namespace torrent
