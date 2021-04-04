"""Repositories needed to compile the libtorrent as a 3rd-party consumer."""

load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")
load("@bazel_tools//tools/build_defs/repo:utils.bzl", "maybe")

def libtorrent_repos():
    maybe(
        http_archive,
        name = "boringssl",
        sha256 = "7d1225e63715769f70ae9634875d2c977eb64cdf9298b6bceb499eea46b1c4ae",
        strip_prefix = "boringssl-56410b82ceb888de9f137b54bb0bf11746505cf6",
        urls = ["https://github.com/google/boringssl/archive/56410b82ceb888de9f137b54bb0bf11746505cf6.zip"],
        patches = ["@libtorrent//:third_party/boringssl.patch"],
    )

    maybe(
        http_archive,
        name = "zlib",
        build_file = "@libtorrent//:third_party/zlib.BUILD",
        sha256 = "629380c90a77b964d896ed37163f5c3a34f6e6d897311f1df2a7016355c45eff",
        strip_prefix = "zlib-1.2.11",
        urls = ["https://github.com/madler/zlib/archive/v1.2.11.tar.gz"],
    )

    maybe(
        http_archive,
        name = "rules_foreign_cc",
        sha256 = "cac13e84d23c2f044c6aaeda4226323a0dead78c31a3e34defaaf03488b5f813",
        strip_prefix = "rules_foreign_cc-5a09829838662332171546ab685d494772b51523",
        url = "https://github.com/bazelbuild/rules_foreign_cc/archive/5a09829838662332171546ab685d494772b51523.zip",
    )
