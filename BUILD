
cc_library(
    name = "remill-base",
    hdrs = glob(["include/**/*.h"]),
    strip_include_prefix = "include",
    visibility = ["//:__subpackages__"],
    deps = [
        "@llvm-project//llvm:Core",
        "@llvm-project//llvm:TransformUtils",
        "@llvm-project//llvm:IPO",
        "@com_github_gflags_gflags//:gflags",
        "@com_github_google_glog//:glog",
        "@com_github_intelxed_xed//:xed",
    ]
)
