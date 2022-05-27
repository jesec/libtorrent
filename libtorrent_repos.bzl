"""Repositories needed to compile the libtorrent as a 3rd-party consumer."""

load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")
load("@bazel_tools//tools/build_defs/repo:utils.bzl", "maybe")

def libtorrent_repos():
    maybe(
        http_archive,
        name = "boringssl",
        sha256 = "efeba07af99d2fc1908d54f6caad1bfec95fa3e235e145233d3e6bdb79379c51",
        strip_prefix = "boringssl-8f90ba425bdcd6a90b88baabfe58b1997f1893f3",
        urls = ["https://github.com/google/boringssl/archive/8f90ba425bdcd6a90b88baabfe58b1997f1893f3.zip"],
    )

    maybe(
        http_archive,
        name = "zlib",
        build_file = "@libtorrent//:third_party/zlib.BUILD",
        sha256 = "8258b75a72303b661a238047cb348203d88d9dddf85d480ed885f375916fcab6",
        strip_prefix = "zlib-ng-2.0.6",
        urls = ["https://github.com/zlib-ng/zlib-ng/archive/refs/tags/2.0.6.tar.gz"],
    )

    maybe(
        http_archive,
        name = "rules_foreign_cc",
        sha256 = "cac13e84d23c2f044c6aaeda4226323a0dead78c31a3e34defaaf03488b5f813",
        strip_prefix = "rules_foreign_cc-5a09829838662332171546ab685d494772b51523",
        url = "https://github.com/bazelbuild/rules_foreign_cc/archive/5a09829838662332171546ab685d494772b51523.zip",
    )
