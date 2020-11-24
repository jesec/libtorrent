load("@rules_cc//cc:defs.bzl", "cc_library", "cc_test")

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
    cmd = "cmake -S $$(dirname $(location CMakeLists.txt)) -B $(RULEDIR) -DBUILDINFO_ONLY=ON",
)

filegroup(
    name = "included_headers",
    srcs = ["include/torrent/buildinfo.h"] + glob(["include/**/*.h"]),
)

filegroup(
    name = "exported_headers",
    srcs = ["include/torrent/buildinfo.h"] + glob(["include/torrent/**/*.h"]),
)

cc_library(
    name = "torrent",
    srcs = glob(["src/**/*.cc"]) + ["//:included_headers"],
    hdrs = ["//:exported_headers"],
    copts = COPTS + ["-DEXPORT_LIBTORRENT_SYMBOLS=1"],
    includes = ["include"],
    linkopts = LINKOPTS + [
        "-lcrypto",
        "-lpthread",
        "-lz",
    ],
    visibility = ["//visibility:public"],
)

cc_test(
    name = "libtorrent_test",
    srcs = glob(["test/**/*.cc"]) + ["//:included_headers"],
    copts = COPTS,
    includes = ["include"],
    linkopts = LINKOPTS + [
        "-lcppunit",
    ],
    deps = ["//:torrent"],
)
