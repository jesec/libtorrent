"""Repositories needed to compile the libtorrent as a 3rd-party consumer."""

load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")
load("@bazel_tools//tools/build_defs/repo:utils.bzl", "maybe")

def libtorrent_repos():
    maybe(
        http_archive,
        name = "boringssl",
        sha256 = "46b103d0a84ecabf1f8e0443ef73c93df3fb7fe67bae932afa79e853abf63ead",
        strip_prefix = "boringssl-688fc5cf5428868679d2ae1072cad81055752068",
        urls = ["https://github.com/google/boringssl/archive/688fc5cf5428868679d2ae1072cad81055752068.zip"],
        patches = ["@libtorrent//:third_party/boringssl.patch"],
    )

    maybe(
        http_archive,
        name = "zlib",
        build_file = "@libtorrent//:third_party/zlib.BUILD",
        sha256 = "30305bd1551e3454bddf574f9863caf7137dde0fdbd4dcd7094eacfbb23955a0",
        strip_prefix = "zlib-ng-2.0.3",
        urls = ["https://github.com/zlib-ng/zlib-ng/archive/refs/tags/2.0.3.tar.gz"],
    )

    maybe(
        http_archive,
        name = "rules_foreign_cc",
        sha256 = "cac13e84d23c2f044c6aaeda4226323a0dead78c31a3e34defaaf03488b5f813",
        strip_prefix = "rules_foreign_cc-5a09829838662332171546ab685d494772b51523",
        url = "https://github.com/bazelbuild/rules_foreign_cc/archive/5a09829838662332171546ab685d494772b51523.zip",
    )
