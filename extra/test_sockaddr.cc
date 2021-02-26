#include <iostream>

#include "torrent/object.h"
#include "torrent/utils/address_info.h"

void
print_addr(const char* name, const torrent::utils::socket_address_inet& sa) {
  std::cout << name << ": " << sa.family() << ' ' << sa.address_str() << ':'
            << sa.port() << std::endl;
}

bool
lookup_address(const char* name) {
  torrent::utils::address_info* result;

  std::cout << "Lookup: " << name << std::endl;

  //   int errcode = torrent::utils::address_info::get_address_info(name, 0, 0,
  //   &result);
  int errcode =
    torrent::utils::address_info::get_address_info(name, PF_INET6, 0, &result);

  if (errcode != 0) {
    std::cout << "Failed: " << torrent::utils::address_info::strerror(errcode)
              << std::endl;
    return false;
  }

  for (torrent::utils::address_info* itr = result; itr != nullptr;
       itr                               = itr->next()) {
    std::cout << "Flags: " << itr->flags() << std::endl;
    std::cout << "Family: " << itr->family() << std::endl;
    std::cout << "Socket Type: " << itr->socket_type() << std::endl;
    std::cout << "Protocol: " << itr->protocol() << std::endl;
    std::cout << "Length: " << itr->length() << std::endl;

    std::cout << "Address: " << itr->address()->family() << ' '
              << itr->address()->address_str() << ':' << itr->address()->port()
              << std::endl;
  }

  // Release.
  freeaddrinfo(reinterpret_cast<addrinfo*>(result));

  return true;
}

int
main(int argc, char** argv) {
  std::cout << "sizeof(sockaddr_in): " << sizeof(sockaddr_in) << std::endl;
  std::cout << "sizeof(sockaddr_in6): " << sizeof(sockaddr_in6) << std::endl;

  torrent::utils::socket_address saNone;
  saNone.set_family();
  print_addr("none", *saNone.sa_inet());

  torrent::utils::socket_address_inet sa1;
  sa1.set_family();
  sa1.set_port(0);
  sa1.set_address_any();

  print_addr("sa1", sa1);

  torrent::utils::socket_address_inet sa2;
  sa2.set_family();
  sa2.set_port(12345);

  if (!sa2.set_address_str("123.45.67.255"))
    return -1;

  print_addr("sa2", sa2);

  torrent::utils::socket_address sa3;
  sa3.sa_inet()->set_family();
  sa3.sa_inet()->set_port(6999);
  sa3.sa_inet()->set_address_str("127.0.0.2");

  print_addr("sa3", *sa3.sa_inet());

  lookup_address("www.uio.no");
  lookup_address("www.ipv6.org");
  lookup_address("lampedusa");

  return 0;
}
