workspace(name = "libtorrent")

load("//:libtorrent_repos.bzl", "libtorrent_repos")

libtorrent_repos()

load("//:libtorrent_deps.bzl", "libtorrent_deps")

libtorrent_deps()

load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

http_archive(
    name = "com_google_absl",
    sha256 = "00c3707bf9cd5eabd1ec6932cc65b97378c043f22573be3adf7d11bb7af17d06",
    strip_prefix = "abseil-cpp-f3f785ab59478dd0312bf1b5df65d380650bf0dc",
    urls = ["https://github.com/abseil/abseil-cpp/archive/f3f785ab59478dd0312bf1b5df65d380650bf0dc.zip"],
)

http_archive(
    name = "com_google_googletest",
    sha256 = "3519a051b20f0dd3a58e1dedd391a3cbd27eb70189afb1185dc4eaefe111211f",
    strip_prefix = "googletest-1de637fbdd4ab0051229707f855eee76f5a3d5da",
    urls = ["https://github.com/google/googletest/archive/1de637fbdd4ab0051229707f855eee76f5a3d5da.zip"],
)
