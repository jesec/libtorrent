load("@rules_foreign_cc//foreign_cc:defs.bzl", "cmake")

filegroup(
    name = "all",
    srcs = glob([
        "**/*",
    ]),
)

cmake(
    name = "zlib",
    cache_entries = {
        "BUILD_SHARED_LIBS": "off",
        "CMAKE_POSITION_INDEPENDENT_CODE": "on",
        "ZLIB_COMPAT": "on",
        "ZLIB_ENABLE_TESTS": "off",
    },
    lib_source = "//:all",
    out_static_libs = ["libz.a"],
    visibility = ["//visibility:public"],
)
