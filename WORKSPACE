workspace(name = "libtorrent")

load("//:libtorrent_repos.bzl", "libtorrent_repos")

libtorrent_repos()

load("//:libtorrent_deps.bzl", "libtorrent_deps")

libtorrent_deps()

load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

http_archive(
    name = "gtest",
    sha256 = "3519a051b20f0dd3a58e1dedd391a3cbd27eb70189afb1185dc4eaefe111211f",
    strip_prefix = "googletest-1de637fbdd4ab0051229707f855eee76f5a3d5da",
    urls = ["https://github.com/google/googletest/archive/1de637fbdd4ab0051229707f855eee76f5a3d5da.zip"],
)
