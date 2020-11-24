"""Load dependencies needed to compile the libtorrent as a 3rd-party consumer."""

load("@bazel_tools//tools/build_defs/repo:git.bzl", "git_repository")
load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

def libtorrent_deps():
    if not native.existing_rule("boringssl"):
        git_repository(
            name = "boringssl",
            commit = "613fe9dbe74b58d6aaaf0d22fe57dccd964c7413",
            remote = "https://boringssl.googlesource.com/boringssl",
        )

    if not native.existing_rule("zlib"):
        http_archive(
            name = "zlib",
            build_file = "@libtorrent//:third_party/zlib.BUILD",
            sha256 = "629380c90a77b964d896ed37163f5c3a34f6e6d897311f1df2a7016355c45eff",
            strip_prefix = "zlib-1.2.11",
            urls = ["https://github.com/madler/zlib/archive/v1.2.11.tar.gz"],
        )
