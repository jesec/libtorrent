AC_INIT(libtorrent_test, 0.12.5, jaris@ifi.uio.no)

AM_INIT_AUTOMAKE
AM_CONFIG_HEADER(config.h)
AM_PATH_CPPUNIT(1.9.6)
AC_PROG_CXX
AC_PROG_CC
AC_PROG_INSTALL

TORRENT_CHECK_MADVISE()
TORRENT_CHECK_CACHELINE()
TORRENT_MINCORE()

CC_ATTRIBUTE_UNUSED(
	AC_DEFINE([__UNUSED], [__attribute__((unused))], [Wrapper around unused attribute]),
	AC_DEFINE([__UNUSED], [], [Null-wrapper if unused attribute is unsupported])
)

AC_OUTPUT([
	Makefile
])
