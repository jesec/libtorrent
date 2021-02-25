load("@rules_cc//cc:defs.bzl", "cc_library", "cc_test")
load("@rules_foreign_cc//tools/build_defs:cmake.bzl", "cmake_external")

config_setting(
    name = "macos",
    values = {"cpu": "darwin"},
    visibility = ["//visibility:private"],
)

config_setting(
    name = "opt",
    values = {"compilation_mode": "opt"},
    visibility = ["//visibility:private"],
)

COPTS = [
    "-std=c++17",
    "-Wall",
    "-Wextra",
    "-Wpedantic",
    "-Wdeprecated",
    "-Werror",
] + select({
    "//:opt": [
        "-Ofast",
        "-flto",
    ],
    "//conditions:default": [],
})

LINKOPTS = select({
    "//:opt": [
        "-O3",
        "-flto",
    ],
    "//conditions:default": [],
})

filegroup(
    name = "cmake_rules",
    srcs = [
        "CMakeLists.txt",
    ] + glob([
        "cmake/**/*",
    ]),
)

cmake_external(
    name = "buildinfo",
    cache_entries = {
        "BUILDINFO_ONLY": "ON",
    },
    headers_only = True,
    lib_source = "//:cmake_rules",
)

filegroup(
    name = "included_headers",
    srcs = glob(
        ["include/**/*.h"],
        exclude = ["include/buildinfo.h"],
    ),
)

filegroup(
    name = "exported_headers",
    srcs = glob(["include/torrent/**/*.h"]),
)

cc_library(
    name = "torrent",
    srcs = glob(["src/**/*.cc"]) + ["//:included_headers"],
    hdrs = ["//:exported_headers"],
    copts = COPTS + ["-DEXPORT_LIBTORRENT_SYMBOLS=1"],
    includes = ["include"],
    linkopts = LINKOPTS + [
        "-lpthread",
    ],
    visibility = ["//visibility:public"],
    deps = [
        "//:buildinfo",
        "@boringssl//:crypto",
        "@zlib",
    ],
)

cc_test(
    name = "libtorrent_test",
    srcs = glob(["test/**/*.cc"]) + ["//:included_headers"],
    copts = COPTS,
    includes = ["include"],
    linkopts = LINKOPTS,
    deps = [
        "//:torrent",
        "@gtest",
    ],
)
