
cc_library(
    name = "bc",
    srcs = glob(["*.cpp", "*.h"]),
    deps = [
        "//:include-god",
        "@com_github_gflags_gflags//:gflags",
        "@com_github_google_glog//:glog",
        "@llvm-project//llvm:Core",
        "@llvm-project//llvm:TransformUtils",
        "@llvm-project//llvm:IPO",
    ]
)
 
