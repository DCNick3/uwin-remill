
load("//build_tools/embed_data:build_defs.bzl", "cc_embed_data")

cc_embed_data(
    name = "semantics",
    srcs = [
        "//lib/Arch/X86/Runtime:all",
        "//lib/Arch/AArch64/Runtime:all",
        "//lib/Arch/AArch32/Runtime:all",
    ],
    cpp_namespace = "remill",
    h_file_output = "semantics.h",
)

cc_library(
    name = "bc",
    srcs = glob(["*.cpp", "*.h"]),
    deps = [
        "//:remill-base",
        ":semantics",
    ]
)
 
