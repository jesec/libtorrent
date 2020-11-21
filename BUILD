load("@rules_cc//cc:defs.bzl", "cc_library", "cc_test")

package(default_visibility = ["//visibility:public"])

COPTS = [
    "-std=c++11",
    "-Ofast",
    "-Wall",
    "-Wextra",
    "-Werror",
    "-faligned-new",
    "-flto",
]

LINKOPTS = [
    "-O3",
    "-flto",
    "-s",
]

genrule(
    name = "buildinfo",
    srcs = [
        "CMakeLists.txt",
    ] + glob([
        "cmake/**/*",
        "**/*.in",
    ]),
    outs = ["include/torrent/buildinfo.h"],
    cmd = "cmake -S . -B $(RULEDIR) -DBUILDINFO_ONLY=ON",
)

filegroup(
    name = "exported_headers",
    srcs = ["include/torrent/buildinfo.h"] + glob(["include/torrent/**/*.h"]),
)

cc_library(
    name = "libtorrent",
    srcs = glob(["src/**/*.cc"]) + ["include/torrent/buildinfo.h"] + glob(["include/**/*.h"]),
    hdrs = ["//:exported_headers"],
    copts = COPTS + ["-DEXPORT_LIBTORRENT_SYMBOLS=1"],
    includes = ["include"],
    linkopts = LINKOPTS,
)

cc_test(
    name = "libtorrent_test",
    srcs = glob(["test/**/*.cc"]) + ["include/torrent/buildinfo.h"] + glob(["include/**/*.h"]),
    copts = COPTS,
    includes = ["include"],
    linkopts = LINKOPTS + [
        "-lcppunit",
        "-lcrypto",
        "-lpthread",
        "-lz",
    ],
    deps = ["//:libtorrent"],
)
