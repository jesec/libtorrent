load("@rules_cc//cc:defs.bzl", "cc_library", "cc_test")

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
        "-lpthread",
    ],
    visibility = ["//visibility:public"],
    deps = [
        "@boringssl//:crypto",
        "@zlib",
    ],
)

cc_test(
    name = "libtorrent_test",
    srcs = glob(["test/**/*.cc"]) + ["//:included_headers"],
    copts = COPTS,
    includes = ["include"],
    linkopts = LINKOPTS + select({
        "//:macos": [],
        "//conditions:default": ["-lcppunit"],
    }),
    deps = ["//:torrent"] + select({
        "//:macos": [
            "@cppunit",
        ],
        "//conditions:default": [],
    }),
)
