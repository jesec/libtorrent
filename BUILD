load("@rules_cc//cc:defs.bzl", "cc_library")

package(default_visibility = ["//visibility:public"])

COPTS = [
    "-std=c++11",
    "-O3",
    "-Wall",
    "-Wextra",
    "-Werror",
    "-faligned-new",
]

filegroup(
    name = "exported_headers",
    srcs = glob(["include/torrent/**/*.h"]),
)

genrule(
    name = "config_h",
    srcs = [
        "autogen.sh",
        "INSTALL",
        "NEWS",
        "README",
        "AUTHORS",
        "ChangeLog",
    ] + glob([
        "**/*.ac",
        "**/*.am",
        "**/*.in",
        "scripts/*.m4",
    ]),
    outs = ["config.h"],
    cmd = "./$(location autogen.sh) " +
          "&& ./`dirname $(location autogen.sh)`/configure " +
          "&& cp config.h $(location config.h)",
)

cc_library(
    name = "libtorrent",
    srcs = glob(["src/**/*.cc"]) + ["config.h"] + glob(["include/**/*.h"]),
    hdrs = ["//:exported_headers"],
    copts = COPTS,
    includes = ["include"],
)
