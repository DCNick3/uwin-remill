
exports_files(["."])

cc_library(
    name = "remill-base",
    hdrs = ["//include:all_headers"],
    strip_include_prefix = "include",
    visibility = ["//lib:__subpackages__"],
    deps = [
        "@llvm-project//llvm:Core",
        "@llvm-project//llvm:TransformUtils",
        "@llvm-project//llvm:IPO",
        "@com_github_gflags_gflags//:gflags",
        "@com_github_google_glog//:glog",
        "@com_github_intelxed_xed//:xed",
    ]
)

cc_library(
    name = "remill",
    deps = [
        "//lib/OS:os",
        "//lib/BC:bc",
        "//lib/Arch:arch",
        "//lib/Version:version",
    ],
    visibility = ["//visibility:public"],
)