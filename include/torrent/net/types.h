#ifndef LIBTORRENT_NET_TYPES_H
#define LIBTORRENT_NET_TYPES_H

#include <memory>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/un.h>

namespace std {
template<>
struct default_delete<sockaddr> {
  default_delete() = default;
  template<class U>
  constexpr default_delete(default_delete<U>) noexcept {}
  void operator()(sockaddr* ptr) const noexcept {
    if (ptr != nullptr) {
      switch (ptr->sa_family) {
        case AF_INET:
          delete reinterpret_cast<sockaddr_in*>(ptr);
          return;
        case AF_INET6:
          delete reinterpret_cast<sockaddr_in6*>(ptr);
          return;
        case AF_UNIX:
          delete reinterpret_cast<sockaddr_un*>(ptr);
          return;
        default:
          delete ptr;
          return;
      }
    }
  }
};

template<>
struct default_delete<const sockaddr> {
  default_delete() = default;
  template<class U>
  constexpr default_delete(default_delete<U>) noexcept {}
  void operator()(const sockaddr* ptr) const noexcept {
    if (ptr != nullptr) {
      switch (ptr->sa_family) {
        case AF_INET:
          delete reinterpret_cast<sockaddr_in*>(const_cast<sockaddr*>(ptr));
          return;
        case AF_INET6:
          delete reinterpret_cast<sockaddr_in6*>(const_cast<sockaddr*>(ptr));
          return;
        case AF_UNIX:
          delete reinterpret_cast<sockaddr_un*>(const_cast<sockaddr*>(ptr));
          return;
        default:
          delete const_cast<sockaddr*>(ptr);
          return;
      }
    }
  }
};
}

namespace torrent {

using sa_unique_ptr   = std::unique_ptr<sockaddr>;
using sin_unique_ptr  = std::unique_ptr<sockaddr_in>;
using sin6_unique_ptr = std::unique_ptr<sockaddr_in6>;
using sun_unique_ptr  = std::unique_ptr<sockaddr_un>;

using c_sa_unique_ptr   = std::unique_ptr<const sockaddr>;
using c_sin_unique_ptr  = std::unique_ptr<const sockaddr_in>;
using c_sin6_unique_ptr = std::unique_ptr<const sockaddr_in6>;
using c_sun_unique_ptr  = std::unique_ptr<const sockaddr_un>;

using fd_sap_tuple = std::tuple<int, sa_unique_ptr>;

struct listen_result_type {
  int           fd;
  sa_unique_ptr address;
};

} // namespace torrent

#endif
