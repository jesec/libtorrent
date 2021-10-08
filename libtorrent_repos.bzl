"""Repositories needed to compile the libtorrent as a 3rd-party consumer."""

load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")
load("@bazel_tools//tools/build_defs/repo:utils.bzl", "maybe")

def libtorrent_repos():
    maybe(
        http_archive,
        name = "boringssl",
        sha256 = "7160f2f15c880b0da2dac61fb6496a26a9903881f35e77131cafdebad14163ac",
        strip_prefix = "boringssl-1285d5305ad69ceb519de76cd74e743aed1efd89",
        urls = ["https://github.com/google/boringssl/archive/1285d5305ad69ceb519de76cd74e743aed1efd89.zip"],
        patches = ["@libtorrent//:third_party/boringssl.patch"],
    )

    maybe(
        http_archive,
        name = "zlib",
        build_file = "@libtorrent//:third_party/zlib.BUILD",
        sha256 = "eca3fe72aea7036c31d00ca120493923c4d5b99fe02e6d3322f7c88dbdcd0085",
        strip_prefix = "zlib-ng-2.0.5",
        urls = ["https://github.com/zlib-ng/zlib-ng/archive/refs/tags/2.0.5.tar.gz"],
    )

    maybe(
        http_archive,
        name = "rules_foreign_cc",
        sha256 = "cac13e84d23c2f044c6aaeda4226323a0dead78c31a3e34defaaf03488b5f813",
        strip_prefix = "rules_foreign_cc-5a09829838662332171546ab685d494772b51523",
        url = "https://github.com/bazelbuild/rules_foreign_cc/archive/5a09829838662332171546ab685d494772b51523.zip",
    )
