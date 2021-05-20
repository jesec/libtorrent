load("@rules_foreign_cc//tools/build_defs:cmake.bzl", "cmake_external")

filegroup(
    name = "all",
    srcs = glob([
        "**/*",
    ]),
)

cmake_external(
    name = "zlib",
    cache_entries = {
        "ZLIB_COMPAT": "on",
        "ZLIB_ENABLE_TESTS": "off",
    },
    lib_source = "//:all",
    static_libraries = ["libz.a"],
    visibility = ["//visibility:public"],
)
