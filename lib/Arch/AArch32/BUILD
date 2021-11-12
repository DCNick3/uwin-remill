
cc_library(
    name = "aarch32",
    srcs = glob(["*.cpp", "*.h"]),
    visibility = [ "//lib:__subpackages__" ],
    copts = ["-Wno-c99-designator"], # TODO: is this the best way to express it?
    deps = [
        "//:include-god",
        "@com_github_gflags_gflags//:gflags",
        "@com_github_google_glog//:glog",
        "@com_github_intelxed_xed//:xed",
        "@llvm-project//llvm:Core",
    ]
)
 
