"""Load dependencies needed to compile the libtorrent as a 3rd-party consumer."""

load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

def libtorrent_deps():
    if not native.existing_rule("boringssl"):
        http_archive(
            name = "boringssl",
            sha256 = "b2a7d159741008e61a1387ec6d93879539e8d7db055c769e4fefe9a371582e44",
            strip_prefix = "boringssl-bdbe37905216bea8dd4d0fdee93f6ee415d3aa15",
            urls = ["https://github.com/google/boringssl/archive/bdbe37905216bea8dd4d0fdee93f6ee415d3aa15.zip"],
            patches = ["@libtorrent//:third_party/boringssl.patch"],
        )

    if not native.existing_rule("zlib"):
        http_archive(
            name = "zlib",
            build_file = "@libtorrent//:third_party/zlib.BUILD",
            sha256 = "629380c90a77b964d896ed37163f5c3a34f6e6d897311f1df2a7016355c45eff",
            strip_prefix = "zlib-1.2.11",
            urls = ["https://github.com/madler/zlib/archive/v1.2.11.tar.gz"],
        )
