"""Load dependencies needed to compile the libtorrent as a 3rd-party consumer."""

load("@rules_foreign_cc//foreign_cc:repositories.bzl", "rules_foreign_cc_dependencies")

def libtorrent_deps():
    rules_foreign_cc_dependencies()
