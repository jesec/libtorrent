// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2005-2011, Jari Sundell <jaris@ifi.uio.no>

#include "torrent/buildinfo.h"

#include <cerrno>
#include <cstring>
#include <iostream>
#include <netdb.h>
#include <sstream>
#include <unistd.h>

#ifdef LT_HAVE_BACKTRACE
#include <execinfo.h>
#endif

#include "torrent/exceptions.h"

namespace torrent {

// Use actual functions, instead of inlined, for the ctor of
// exceptions. This allows us to create breakpoints at throws. This is
// limited to rarely thrown exceptions.

void
communication_error::initialize(const std::string& msg) {
  m_msg = msg;
}
void
storage_error::initialize(const std::string& msg) {
  m_msg = msg;
}
void
resource_error::initialize(const std::string& msg) {
  m_msg = msg;
}
void
input_error::initialize(const std::string& msg) {
  m_msg = msg;
}

const char*
connection_error::what() const noexcept {
  return std::strerror(m_errno);
}

const char*
address_info_error::what() const noexcept {
  return ::gai_strerror(m_errno);
}

void
internal_error::initialize(const std::string& msg, const bool print) {
  m_msg = msg;
  std::stringstream output;

#ifdef LT_HAVE_BACKTRACE
  void* stackPtrs[20];

  // Print the stack and exit.
  int    stackSize    = ::backtrace(stackPtrs, 20);
  char** stackStrings = backtrace_symbols(stackPtrs, stackSize);

  for (int i = 0; i < stackSize; ++i)
    output << stackStrings[i] << std::endl;

  m_backtrace = output.str();
  free(stackStrings);
#else
  output << "Stack dump not enabled." << std::endl;
  m_backtrace = output.str();
#endif

  if (print) {
    std::cout << m_msg << std::endl;
    std::cout << m_backtrace << std::endl;
  }
}

} // namespace torrent
