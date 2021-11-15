
workspace(
    name = "remill"
)

load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")
load("@bazel_tools//tools/build_defs/repo:git.bzl", "git_repository")

# =====
# setup the toolchain and common bazel libs
# =====

SKYLIB_VERSION = "1.0.3"
http_archive(
    name = "bazel_skylib",
    sha256 = "1c531376ac7e5a180e0237938a2536de0c54d93f5c278634818e0efc952dd56c",
    urls = [
        "https://mirror.bazel.build/github.com/bazelbuild/bazel-skylib/releases/download/{version}/bazel-skylib-{version}.tar.gz".format(version=SKYLIB_VERSION),
        "https://github.com/bazelbuild/bazel-skylib/releases/download/{version}/bazel-skylib-{version}.tar.gz".format(version=SKYLIB_VERSION),
    ],
)

git_repository(
    name = "io_bazel_stardoc",
    commit = "8f6d22452d088b49b13ba2c224af69ccc8ccbc90",
    remote = "https://github.com/bazelbuild/stardoc.git",
    shallow_since = "1620849756 -0400"
)

git_repository(
    name = "rules_cc_toolchain",
    commit = "fc7d3aed91501a6f7871cb567a1ee516df0dbed9",
    remote = "https://github.com/DCNick3/bazel_rules_cc_toolchain",
    shallow_since = "1636803112 +0300"
)

load(
    "@rules_cc_toolchain//config:rules_cc_toolchain_config_repository.bzl",
    "rules_cc_toolchain_config",
)

rules_cc_toolchain_config(
    name = "rules_cc_toolchain_config"
)

load("@bazel_skylib//:workspace.bzl", "bazel_skylib_workspace")

bazel_skylib_workspace()

load("@rules_cc_toolchain//:rules_cc_toolchain_deps.bzl", "rules_cc_toolchain_deps")

rules_cc_toolchain_deps()

load("@rules_cc_toolchain//cc_toolchain:cc_toolchain.bzl", "register_cc_toolchains")

register_cc_toolchains()



load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")
http_archive(
    name = "rules_python",
    url = "https://github.com/bazelbuild/rules_python/releases/download/0.5.0/rules_python-0.5.0.tar.gz",
    sha256 = "cd6730ed53a002c56ce4e2f396ba3b3be262fd7cb68339f0377a45e8227fe332",
)


# =====
# setup the actual remill dependencies
# =====


load("//third_party/llvm:workspace.bzl", llvm_repo = "repo")
llvm_repo("llvm-raw") # gets the llvm-raw repo

load("//third_party/llvm:setup.bzl", "llvm_setup")
llvm_setup("llvm-project") # gets the llvm-project repo

load("//third_party/sysroots:workspace.bzl", sysroots_repo = "repo")
sysroots_repo() # gets the sysroots repos


http_archive(
    name = "com_github_gflags_gflags",
    sha256 = "34af2f15cf7367513b352bdcd2493ab14ce43692d2dcd9dfc499492966c64dcf",
    strip_prefix = "gflags-2.2.2",
    urls = ["https://github.com/gflags/gflags/archive/v2.2.2.tar.gz"],
)

http_archive(
    name = "com_github_google_glog",
    sha256 = "21bc744fb7f2fa701ee8db339ded7dce4f975d0d55837a97be7d46e8382dea5a",
    strip_prefix = "glog-0.5.0",
    urls = ["https://github.com/google/glog/archive/v0.5.0.zip"],
)


load("//third_party/xed:workspace.bzl", xed_repo = "repo")

xed_repo()

