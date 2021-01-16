"""Load dependencies needed to compile the libtorrent as a 3rd-party consumer."""

load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

def libtorrent_deps():
    if not native.existing_rule("boringssl"):
        http_archive(
            name = "boringssl",
            sha256 = "bcab08a22c28f5322316542aa2c3a9ef0a9f9fde9be22d489cee574867b24675",
            strip_prefix = "boringssl-613fe9dbe74b58d6aaaf0d22fe57dccd964c7413",
            urls = ["https://github.com/google/boringssl/archive/613fe9dbe74b58d6aaaf0d22fe57dccd964c7413.zip"],
        )

    if not native.existing_rule("zlib"):
        http_archive(
            name = "zlib",
            build_file = "@libtorrent//:third_party/zlib.BUILD",
            sha256 = "629380c90a77b964d896ed37163f5c3a34f6e6d897311f1df2a7016355c45eff",
            strip_prefix = "zlib-1.2.11",
            urls = ["https://github.com/madler/zlib/archive/v1.2.11.tar.gz"],
        )
