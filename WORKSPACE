workspace(name = "libtorrent")

load("//:libtorrent_deps.bzl", "libtorrent_deps")

libtorrent_deps()

# MacOS workaround
_MACOS_CPPUNIT = """
load("@rules_cc//cc:defs.bzl", "cc_library")

cc_library(
  name = "cppunit",
  srcs = ["lib/libcppunit.dylib"],
  hdrs = glob(["include/cppunit/**"]),
  includes = ["include"],
  visibility = ["//visibility:public"],
)
"""

new_local_repository(
    name = "cppunit",
    build_file_content = _MACOS_CPPUNIT,
    path = "/usr/local/opt/cppunit",
)
