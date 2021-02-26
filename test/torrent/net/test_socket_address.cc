#include "test/torrent/net/test_socket_address.h"

#include "test/helpers/network.h"
#include "torrent/exceptions.h"
#include "torrent/net/socket_address.h"

TEST_F(test_socket_address, test_sa_is_any) {
  TEST_DEFAULT_SA;

  ASSERT_TRUE(torrent::sap_is_any(sin_any));
  ASSERT_TRUE(torrent::sap_is_any(sin_any_5000));
  ASSERT_TRUE(torrent::sap_is_any(sin6_v4_any));
  ASSERT_TRUE(torrent::sap_is_any(sin6_v4_any_5000));

  ASSERT_FALSE(torrent::sap_is_any(sin_bc));
  ASSERT_FALSE(torrent::sap_is_any(sin_1));
  ASSERT_FALSE(torrent::sap_is_any(sin6_1));
  ASSERT_FALSE(torrent::sap_is_any(sin_bc_5000));
  ASSERT_FALSE(torrent::sap_is_any(sin_1_5000));
  ASSERT_FALSE(torrent::sap_is_any(sin6_1_5000));

  ASSERT_FALSE(torrent::sap_is_any(c_sin_bc));
  ASSERT_FALSE(torrent::sap_is_any(c_sin_1));
  ASSERT_FALSE(torrent::sap_is_any(c_sin6_1));
  ASSERT_FALSE(torrent::sap_is_any(c_sin_bc_5000));
  ASSERT_FALSE(torrent::sap_is_any(c_sin_1_5000));
  ASSERT_FALSE(torrent::sap_is_any(c_sin6_1_5000));
}

TEST_F(test_socket_address, test_sa_is_broadcast) {
  TEST_DEFAULT_SA;

  ASSERT_TRUE(torrent::sap_is_broadcast(sin_bc));
  ASSERT_TRUE(torrent::sap_is_broadcast(sin_bc_5000));
  ASSERT_TRUE(torrent::sap_is_broadcast(sin6_v4_bc));
  ASSERT_TRUE(torrent::sap_is_broadcast(sin6_v4_bc_5000));

  ASSERT_FALSE(torrent::sap_is_broadcast(sin_any));
  ASSERT_FALSE(torrent::sap_is_broadcast(sin_1));
  ASSERT_FALSE(torrent::sap_is_broadcast(sin6_any));
  ASSERT_FALSE(torrent::sap_is_broadcast(sin6_1));
  ASSERT_FALSE(torrent::sap_is_broadcast(sin_any_5000));
  ASSERT_FALSE(torrent::sap_is_broadcast(sin_1_5000));
  ASSERT_FALSE(torrent::sap_is_broadcast(sin6_any_5000));
  ASSERT_FALSE(torrent::sap_is_broadcast(sin6_1_5000));

  ASSERT_FALSE(torrent::sap_is_broadcast(c_sin_any));
  ASSERT_FALSE(torrent::sap_is_broadcast(c_sin_1));
  ASSERT_FALSE(torrent::sap_is_broadcast(c_sin6_any));
  ASSERT_FALSE(torrent::sap_is_broadcast(c_sin6_1));
  ASSERT_FALSE(torrent::sap_is_broadcast(c_sin_any_5000));
  ASSERT_FALSE(torrent::sap_is_broadcast(c_sin_1_5000));
  ASSERT_FALSE(torrent::sap_is_broadcast(c_sin6_any_5000));
  ASSERT_FALSE(torrent::sap_is_broadcast(c_sin6_1_5000));
}

TEST_F(test_socket_address, test_make) {
  torrent::sa_unique_ptr sa_unspec = torrent::sa_make_unspec();
  ASSERT_NE(sa_unspec, nullptr);
  ASSERT_EQ(sa_unspec->sa_family, AF_UNSPEC);

  torrent::sa_unique_ptr sa_inet = torrent::sa_make_inet();
  ASSERT_NE(sa_inet, nullptr);
  ASSERT_EQ(sa_inet->sa_family, AF_INET);

  sockaddr_in* sin_inet = reinterpret_cast<sockaddr_in*>(sa_inet.get());
  ASSERT_EQ(sin_inet->sin_family, AF_INET);
  ASSERT_EQ(sin_inet->sin_port, 0);
  ASSERT_EQ(sin_inet->sin_addr.s_addr, in_addr().s_addr);

  torrent::sa_unique_ptr sa_inet6 = torrent::sa_make_inet6();
  ASSERT_NE(sa_inet6, nullptr);
  ASSERT_EQ(sa_inet6->sa_family, AF_INET6);

  sockaddr_in6* sin6_inet6 = reinterpret_cast<sockaddr_in6*>(sa_inet6.get());
  ASSERT_EQ(sin6_inet6->sin6_family, AF_INET6);
  ASSERT_EQ(sin6_inet6->sin6_port, 0);
  ASSERT_EQ(sin6_inet6->sin6_flowinfo, 0);
  ASSERT_TRUE(compare_sin6_addr(sin6_inet6->sin6_addr, in6_addr{ { { 0 } } }));
  ASSERT_EQ(sin6_inet6->sin6_scope_id, 0);

  torrent::sa_unique_ptr sa_unix = torrent::sa_make_unix("");
  ASSERT_NE(sa_unix, nullptr);
  ASSERT_EQ(sa_unix->sa_family, AF_UNIX);
}

TEST_F(test_socket_address, test_sin_from_sa) {
  torrent::sa_unique_ptr  sa_zero = wrap_ai_get_first_sa("0.0.0.0");
  torrent::sin_unique_ptr sin_zero;

  ASSERT_NE(sa_zero, nullptr);
  ASSERT_NO_THROW({ sin_zero = torrent::sin_from_sa(std::move(sa_zero)); });
  ASSERT_EQ(sa_zero, nullptr);
  ASSERT_NE(sin_zero, nullptr);

  ASSERT_EQ(sin_zero->sin_addr.s_addr, htonl(0x0));

  torrent::sa_unique_ptr  sa_inet = wrap_ai_get_first_sa("1.2.3.4");
  torrent::sin_unique_ptr sin_inet;

  ASSERT_NE(sa_inet, nullptr);
  ASSERT_NO_THROW({ sin_inet = torrent::sin_from_sa(std::move(sa_inet)); });
  ASSERT_EQ(sa_inet, nullptr);
  ASSERT_NE(sin_inet, nullptr);

  ASSERT_EQ(sin_inet->sin_addr.s_addr, htonl(0x01020304));

  ASSERT_THROW(torrent::sin_from_sa(torrent::sa_unique_ptr()),
               torrent::internal_error);
  ASSERT_THROW(torrent::sin_from_sa(torrent::sa_make_unspec()),
               torrent::internal_error);
  ASSERT_THROW(torrent::sin_from_sa(torrent::sa_make_inet6()),
               torrent::internal_error);
}

TEST_F(test_socket_address, test_sin6_from_sa) {
  torrent::sa_unique_ptr   sa_zero = wrap_ai_get_first_sa("::");
  torrent::sin6_unique_ptr sin6_zero;

  ASSERT_NE(sa_zero, nullptr);
  ASSERT_NO_THROW({ sin6_zero = torrent::sin6_from_sa(std::move(sa_zero)); });
  ASSERT_EQ(sa_zero, nullptr);
  ASSERT_NE(sin6_zero, nullptr);

  ASSERT_EQ(sin6_zero->sin6_addr.s6_addr[0], 0x0);
  ASSERT_EQ(sin6_zero->sin6_addr.s6_addr[1], 0x0);
  ASSERT_EQ(sin6_zero->sin6_addr.s6_addr[15], 0x0);

  torrent::sa_unique_ptr   sa_inet6 = wrap_ai_get_first_sa("ff01::1");
  torrent::sin6_unique_ptr sin6_inet6;

  ASSERT_NE(sa_inet6, nullptr);
  ASSERT_NO_THROW({ sin6_inet6 = torrent::sin6_from_sa(std::move(sa_inet6)); });
  ASSERT_EQ(sa_inet6, nullptr);
  ASSERT_NE(sin6_inet6, nullptr);

  ASSERT_EQ(sin6_inet6->sin6_addr.s6_addr[0], 0xff);
  ASSERT_EQ(sin6_inet6->sin6_addr.s6_addr[1], 0x01);
  ASSERT_EQ(sin6_inet6->sin6_addr.s6_addr[15], 0x01);

  ASSERT_THROW(torrent::sin6_from_sa(torrent::sa_unique_ptr()),
               torrent::internal_error);
  ASSERT_THROW(torrent::sin6_from_sa(torrent::sa_make_unspec()),
               torrent::internal_error);
  ASSERT_THROW(torrent::sin6_from_sa(torrent::sa_make_inet()),
               torrent::internal_error);
}

TEST_F(test_socket_address, test_sa_equal) {
  TEST_DEFAULT_SA;

  ASSERT_TRUE(
    torrent::sap_equal(torrent::sa_make_unspec(), torrent::sa_make_unspec()));
  ASSERT_TRUE(
    torrent::sap_equal(torrent::sa_make_inet(), torrent::sa_make_inet()));
  ASSERT_TRUE(
    torrent::sap_equal(torrent::sa_make_inet6(), torrent::sa_make_inet6()));

  ASSERT_FALSE(
    torrent::sap_equal(torrent::sa_make_unspec(), torrent::sa_make_inet()));
  ASSERT_FALSE(
    torrent::sap_equal(torrent::sa_make_unspec(), torrent::sa_make_inet6()));
  ASSERT_FALSE(
    torrent::sap_equal(torrent::sa_make_inet(), torrent::sa_make_inet6()));
  ASSERT_FALSE(
    torrent::sap_equal(torrent::sa_make_inet6(), torrent::sa_make_inet()));

  ASSERT_TRUE(torrent::sap_equal(sin_1, sin_1));
  ASSERT_TRUE(torrent::sap_equal(sin_1, c_sin_1));
  ASSERT_TRUE(torrent::sap_equal(c_sin_1, sin_1));
  ASSERT_TRUE(torrent::sap_equal(c_sin_1, c_sin_1));

  ASSERT_FALSE(torrent::sap_equal(sin_1, sin_2));
  ASSERT_FALSE(torrent::sap_equal(sin_1, c_sin_2));
  ASSERT_FALSE(torrent::sap_equal(c_sin_1, sin_2));
  ASSERT_FALSE(torrent::sap_equal(c_sin_1, c_sin_2));

  ASSERT_TRUE(torrent::sap_equal(sin6_1, sin6_1));
  ASSERT_TRUE(torrent::sap_equal(sin6_1, c_sin6_1));
  ASSERT_TRUE(torrent::sap_equal(c_sin6_1, sin6_1));
  ASSERT_TRUE(torrent::sap_equal(c_sin6_1, c_sin6_1));

  ASSERT_FALSE(torrent::sap_equal(sin6_1, sin6_2));
  ASSERT_FALSE(torrent::sap_equal(sin6_1, c_sin6_2));
  ASSERT_FALSE(torrent::sap_equal(c_sin6_1, sin6_2));
  ASSERT_FALSE(torrent::sap_equal(c_sin6_1, c_sin6_2));

  ASSERT_TRUE(torrent::sap_equal(sin_1_5000, sin_1_5000));
  ASSERT_TRUE(torrent::sap_equal(sin6_1_5000, sin6_1_5000));
  ASSERT_FALSE(torrent::sap_equal(sin_1_5000, sin_1_5100));
  ASSERT_FALSE(torrent::sap_equal(sin6_1_5000, sin6_1_5100));
  ASSERT_FALSE(torrent::sap_equal(sin_1_5000, sin_2_5000));
  ASSERT_FALSE(torrent::sap_equal(sin6_1_5000, sin6_2_5000));
  ASSERT_FALSE(torrent::sap_equal(sin_1_5000, sin_2_5100));
  ASSERT_FALSE(torrent::sap_equal(sin6_1_5000, sin6_2_5100));

  ASSERT_THROW(
    torrent::sap_equal(torrent::sa_make_unix(""), torrent::sa_make_unix("")),
    torrent::internal_error);
  ASSERT_THROW(torrent::sap_equal(torrent::sa_make_unix(""), sin6_1),
               torrent::internal_error);
  ASSERT_THROW(torrent::sap_equal(sin6_1, torrent::sa_make_unix("")),
               torrent::internal_error);
}

TEST_F(test_socket_address, test_sa_equal_addr) {
  TEST_DEFAULT_SA;

  ASSERT_TRUE(torrent::sap_equal_addr(torrent::sa_make_unspec(),
                                      torrent::sa_make_unspec()));
  ASSERT_TRUE(
    torrent::sap_equal_addr(torrent::sa_make_inet(), torrent::sa_make_inet()));
  ASSERT_TRUE(torrent::sap_equal_addr(torrent::sa_make_inet6(),
                                      torrent::sa_make_inet6()));

  ASSERT_FALSE(torrent::sap_equal_addr(torrent::sa_make_unspec(),
                                       torrent::sa_make_inet()));
  ASSERT_FALSE(torrent::sap_equal_addr(torrent::sa_make_unspec(),
                                       torrent::sa_make_inet6()));
  ASSERT_FALSE(
    torrent::sap_equal_addr(torrent::sa_make_inet(), torrent::sa_make_inet6()));
  ASSERT_FALSE(
    torrent::sap_equal_addr(torrent::sa_make_inet6(), torrent::sa_make_inet()));

  ASSERT_TRUE(torrent::sap_equal_addr(sin_1, sin_1));
  ASSERT_TRUE(torrent::sap_equal_addr(sin_1, c_sin_1));
  ASSERT_TRUE(torrent::sap_equal_addr(c_sin_1, sin_1));
  ASSERT_TRUE(torrent::sap_equal_addr(c_sin_1, c_sin_1));

  ASSERT_FALSE(torrent::sap_equal_addr(sin_1, sin_2));
  ASSERT_FALSE(torrent::sap_equal_addr(sin_1, c_sin_2));
  ASSERT_FALSE(torrent::sap_equal_addr(c_sin_1, sin_2));
  ASSERT_FALSE(torrent::sap_equal_addr(c_sin_1, c_sin_2));

  ASSERT_TRUE(torrent::sap_equal_addr(sin6_1, sin6_1));
  ASSERT_TRUE(torrent::sap_equal_addr(sin6_1, c_sin6_1));
  ASSERT_TRUE(torrent::sap_equal_addr(c_sin6_1, sin6_1));
  ASSERT_TRUE(torrent::sap_equal_addr(c_sin6_1, c_sin6_1));

  ASSERT_FALSE(torrent::sap_equal_addr(sin6_1, sin6_2));
  ASSERT_FALSE(torrent::sap_equal_addr(sin6_1, c_sin6_2));
  ASSERT_FALSE(torrent::sap_equal_addr(c_sin6_1, sin6_2));
  ASSERT_FALSE(torrent::sap_equal_addr(c_sin6_1, c_sin6_2));

  ASSERT_TRUE(torrent::sap_equal_addr(sin_1_5000, sin_1_5000));
  ASSERT_TRUE(torrent::sap_equal_addr(sin6_1_5000, sin6_1_5000));
  ASSERT_TRUE(torrent::sap_equal_addr(sin_1_5000, sin_1_5100));
  ASSERT_TRUE(torrent::sap_equal_addr(sin6_1_5000, sin6_1_5100));
  ASSERT_FALSE(torrent::sap_equal_addr(sin_1_5000, sin_2_5000));
  ASSERT_FALSE(torrent::sap_equal_addr(sin6_1_5000, sin6_2_5000));
  ASSERT_FALSE(torrent::sap_equal_addr(sin_1_5000, sin_2_5100));
  ASSERT_FALSE(torrent::sap_equal_addr(sin6_1_5000, sin6_2_5100));

  ASSERT_THROW(torrent::sap_equal_addr(torrent::sa_make_unix(""),
                                       torrent::sa_make_unix("")),
               torrent::internal_error);
  ASSERT_THROW(torrent::sap_equal_addr(torrent::sa_make_unix(""), sin6_1),
               torrent::internal_error);
  ASSERT_THROW(torrent::sap_equal_addr(sin6_1, torrent::sa_make_unix("")),
               torrent::internal_error);
}

TEST_F(test_socket_address, test_sa_copy) {
  TEST_DEFAULT_SA;

  ASSERT_TRUE(torrent::sap_equal(torrent::sap_copy(torrent::sa_make_unspec()),
                                 torrent::sa_make_unspec()));
  ASSERT_TRUE(
    torrent::sap_equal(torrent::sap_copy(torrent::sa_make_inet()), sin_any));
  ASSERT_TRUE(
    torrent::sap_equal(torrent::sap_copy(torrent::sa_make_inet6()), sin6_any));

  ASSERT_NE(torrent::sap_copy(sin_1).get(), sin_1.get());
  ASSERT_NE(torrent::sap_copy(c_sin_1).get(), c_sin_1.get());
  ASSERT_NE(torrent::sap_copy(sin6_1).get(), sin6_1.get());
  ASSERT_NE(torrent::sap_copy(c_sin6_1).get(), c_sin6_1.get());
  ASSERT_NE(torrent::sap_copy(sin_1_5000).get(), sin_1_5000.get());
  ASSERT_NE(torrent::sap_copy(sin6_1_5000).get(), sin6_1_5000.get());

  ASSERT_TRUE(torrent::sap_equal(torrent::sap_copy(sin_1), sin_1));
  ASSERT_TRUE(torrent::sap_equal(torrent::sap_copy(sin_1), c_sin_1));
  ASSERT_TRUE(torrent::sap_equal(torrent::sap_copy(c_sin_1), sin_1));
  ASSERT_TRUE(torrent::sap_equal(torrent::sap_copy(c_sin_1), c_sin_1));

  ASSERT_TRUE(torrent::sap_equal(torrent::sap_copy(sin6_1), sin6_1));
  ASSERT_TRUE(torrent::sap_equal(torrent::sap_copy(sin6_1), c_sin6_1));
  ASSERT_TRUE(torrent::sap_equal(torrent::sap_copy(c_sin6_1), sin6_1));
  ASSERT_TRUE(torrent::sap_equal(torrent::sap_copy(c_sin6_1), c_sin6_1));

  ASSERT_TRUE(torrent::sap_equal(torrent::sap_copy(sin_1_5000), sin_1_5000));
  ASSERT_TRUE(torrent::sap_equal(torrent::sap_copy(sin6_1_5000), sin6_1_5000));

  auto sin6_flags = torrent::sap_copy(sin6_1_5000);
  reinterpret_cast<sockaddr_in6*>(sin6_flags.get())->sin6_flowinfo = 0x12345678;
  reinterpret_cast<sockaddr_in6*>(sin6_flags.get())->sin6_scope_id = 0x12345678;

  // TODO: Need 'strict' equal test.
  ASSERT_TRUE(torrent::sap_equal(torrent::sap_copy(sin6_flags), sin6_flags));

  ASSERT_THROW(torrent::sap_copy(torrent::sa_unique_ptr()),
               torrent::internal_error);
  ASSERT_THROW(torrent::sap_copy(torrent::c_sa_unique_ptr()),
               torrent::internal_error);
}

TEST_F(test_socket_address, test_sa_copy_addr) {
  TEST_DEFAULT_SA;

  ASSERT_TRUE(
    torrent::sap_equal(torrent::sap_copy_addr(torrent::sa_make_unspec()),
                       torrent::sa_make_unspec()));
  ASSERT_TRUE(torrent::sap_equal(
    torrent::sap_copy_addr(torrent::sa_make_inet()), sin_any));
  ASSERT_TRUE(torrent::sap_equal(
    torrent::sap_copy_addr(torrent::sa_make_inet6()), sin6_any));
  ASSERT_TRUE(
    torrent::sap_equal(torrent::sap_copy_addr(torrent::sa_make_unspec(), 5000),
                       torrent::sa_make_unspec()));
  ASSERT_TRUE(torrent::sap_equal(
    torrent::sap_copy_addr(torrent::sa_make_inet(), 5000), sin_any_5000));
  ASSERT_TRUE(torrent::sap_equal(
    torrent::sap_copy_addr(torrent::sa_make_inet6(), 5000), sin6_any_5000));

  ASSERT_NE(torrent::sap_copy_addr(sin_1).get(), sin_1.get());
  ASSERT_NE(torrent::sap_copy_addr(c_sin_1).get(), c_sin_1.get());
  ASSERT_NE(torrent::sap_copy_addr(sin6_1).get(), sin6_1.get());
  ASSERT_NE(torrent::sap_copy_addr(c_sin6_1).get(), c_sin6_1.get());
  ASSERT_NE(torrent::sap_copy_addr(sin_1_5000).get(), sin_1_5000.get());
  ASSERT_NE(torrent::sap_copy_addr(sin6_1_5000).get(), sin6_1_5000.get());

  ASSERT_TRUE(torrent::sap_equal(torrent::sap_copy_addr(sin_1), sin_1));
  ASSERT_TRUE(torrent::sap_equal(torrent::sap_copy_addr(sin_1), c_sin_1));
  ASSERT_TRUE(torrent::sap_equal(torrent::sap_copy_addr(c_sin_1), sin_1));
  ASSERT_TRUE(torrent::sap_equal(torrent::sap_copy_addr(c_sin_1), c_sin_1));
  ASSERT_TRUE(
    torrent::sap_equal(torrent::sap_copy_addr(sin_1, 5000), sin_1_5000));
  ASSERT_TRUE(
    torrent::sap_equal(torrent::sap_copy_addr(sin_1, 5000), c_sin_1_5000));
  ASSERT_TRUE(
    torrent::sap_equal(torrent::sap_copy_addr(c_sin_1, 5000), sin_1_5000));
  ASSERT_TRUE(
    torrent::sap_equal(torrent::sap_copy_addr(c_sin_1, 5000), c_sin_1_5000));

  ASSERT_TRUE(torrent::sap_equal(torrent::sap_copy_addr(sin6_1), sin6_1));
  ASSERT_TRUE(torrent::sap_equal(torrent::sap_copy_addr(sin6_1), c_sin6_1));
  ASSERT_TRUE(torrent::sap_equal(torrent::sap_copy_addr(c_sin6_1), sin6_1));
  ASSERT_TRUE(torrent::sap_equal(torrent::sap_copy_addr(c_sin6_1), c_sin6_1));
  ASSERT_TRUE(
    torrent::sap_equal(torrent::sap_copy_addr(sin6_1, 5000), sin6_1_5000));
  ASSERT_TRUE(
    torrent::sap_equal(torrent::sap_copy_addr(sin6_1, 5000), c_sin6_1_5000));
  ASSERT_TRUE(
    torrent::sap_equal(torrent::sap_copy_addr(c_sin6_1, 5000), sin6_1_5000));
  ASSERT_TRUE(
    torrent::sap_equal(torrent::sap_copy_addr(c_sin6_1, 5000), c_sin6_1_5000));

  ASSERT_TRUE(torrent::sap_equal(torrent::sap_copy_addr(sin_1_5000), sin_1));
  ASSERT_TRUE(torrent::sap_equal(torrent::sap_copy_addr(sin6_1_5000), sin6_1));
  ASSERT_TRUE(
    torrent::sap_equal(torrent::sap_copy_addr(sin_1_5000, 5100), sin_1_5100));
  ASSERT_TRUE(
    torrent::sap_equal(torrent::sap_copy_addr(sin6_1_5000, 5100), sin6_1_5100));

  auto sin6_flags = wrap_ai_get_first_sa("ff01::1", "5555");
  reinterpret_cast<sockaddr_in6*>(sin6_flags.get())->sin6_flowinfo = 0x12345678;
  reinterpret_cast<sockaddr_in6*>(sin6_flags.get())->sin6_scope_id = 0x12345678;

  ASSERT_TRUE(torrent::sap_equal(torrent::sap_copy_addr(sin6_flags), sin6_1));

  ASSERT_THROW(torrent::sap_copy_addr(torrent::sa_unique_ptr()),
               torrent::internal_error);
  ASSERT_THROW(torrent::sap_copy_addr(torrent::c_sa_unique_ptr()),
               torrent::internal_error);
  ASSERT_THROW(torrent::sap_copy_addr(torrent::sa_unique_ptr(), 5000),
               torrent::internal_error);
  ASSERT_THROW(torrent::sap_copy_addr(torrent::c_sa_unique_ptr(), 5000),
               torrent::internal_error);
}

TEST_F(test_socket_address, test_sa_from_v4mapped) {
  TEST_DEFAULT_SA;

  ASSERT_TRUE(
    torrent::sap_equal_addr(torrent::sap_from_v4mapped(sin6_v4_any), sin_any));
  ASSERT_TRUE(
    torrent::sap_is_port_any(torrent::sap_from_v4mapped(sin6_v4_any)));

  ASSERT_TRUE(
    torrent::sap_equal_addr(torrent::sap_from_v4mapped(sin6_v4_1), sin_1));
  ASSERT_TRUE(torrent::sap_is_port_any(torrent::sap_from_v4mapped(sin6_v4_1)));

  ASSERT_TRUE(
    torrent::sap_equal_addr(torrent::sap_from_v4mapped(sin6_v4_bc), sin_bc));
  ASSERT_TRUE(torrent::sap_is_port_any(torrent::sap_from_v4mapped(sin6_v4_bc)));

  ASSERT_THROW(torrent::sap_from_v4mapped(torrent::sa_make_unspec()),
               torrent::internal_error);
  ASSERT_THROW(torrent::sap_from_v4mapped(torrent::sa_make_inet()),
               torrent::internal_error);
  ASSERT_THROW(torrent::sap_from_v4mapped(torrent::sa_make_unix("")),
               torrent::internal_error);
  ASSERT_THROW(torrent::sap_from_v4mapped(sin_any), torrent::internal_error);
  ASSERT_THROW(torrent::sap_from_v4mapped(sin_bc), torrent::internal_error);
  ASSERT_THROW(torrent::sap_from_v4mapped(sin_1), torrent::internal_error);
}

TEST_F(test_socket_address, test_sa_to_v4mapped) {
  TEST_DEFAULT_SA;

  ASSERT_TRUE(
    torrent::sap_equal_addr(torrent::sap_to_v4mapped(sin_any), sin6_v4_any));
  ASSERT_TRUE(torrent::sap_is_v4mapped(torrent::sap_to_v4mapped(sin_any)));
  ASSERT_TRUE(torrent::sap_is_port_any(torrent::sap_to_v4mapped(sin_any)));

  ASSERT_TRUE(
    torrent::sap_equal_addr(torrent::sap_to_v4mapped(sin_bc), sin6_v4_bc));
  ASSERT_TRUE(torrent::sap_is_v4mapped(torrent::sap_to_v4mapped(sin_bc)));
  ASSERT_TRUE(torrent::sap_is_port_any(torrent::sap_to_v4mapped(sin_bc)));

  ASSERT_TRUE(
    torrent::sap_equal_addr(torrent::sap_to_v4mapped(sin_1), sin6_v4_1));
  ASSERT_TRUE(torrent::sap_is_v4mapped(torrent::sap_to_v4mapped(sin_1)));
  ASSERT_TRUE(torrent::sap_is_port_any(torrent::sap_to_v4mapped(sin_1)));

  ASSERT_THROW(torrent::sap_to_v4mapped(torrent::sa_make_unspec()),
               torrent::internal_error);
  ASSERT_THROW(torrent::sap_to_v4mapped(torrent::sa_make_inet6()),
               torrent::internal_error);
  ASSERT_THROW(torrent::sap_to_v4mapped(torrent::sa_make_unix("")),
               torrent::internal_error);
  ASSERT_THROW(torrent::sap_to_v4mapped(sin6_any), torrent::internal_error);
  ASSERT_THROW(torrent::sap_to_v4mapped(sin6_1), torrent::internal_error);
  ASSERT_THROW(torrent::sap_to_v4mapped(sin6_v4_any), torrent::internal_error);
  ASSERT_THROW(torrent::sap_to_v4mapped(sin6_v4_bc), torrent::internal_error);
  ASSERT_THROW(torrent::sap_to_v4mapped(sin6_v4_1), torrent::internal_error);
}
