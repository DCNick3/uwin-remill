
cc_library(
    name = "aarch64",
    srcs = glob(["*.cpp", "*.h"]),
    visibility = [ "//lib:__subpackages__" ],
    deps = [
        "//:include-god",
        "@com_github_gflags_gflags//:gflags",
        "@com_github_google_glog//:glog",
        "@com_github_intelxed_xed//:xed",
        "@llvm-project//llvm:Core",
    ]
)
 
