exports_files(["Arch.h"])

cc_library(
    name = "arch",
    srcs = glob(["*.cpp", "*.h"]),
    copts = ["-Wno-c99-designator"], # TODO: is this the best way to express it?
    deps = [
        "//lib/Arch/AArch32:aarch32",
        "//lib/Arch/AArch64:aarch64",
        "//lib/Arch/SPARC32:sparc32",
        "//lib/Arch/SPARC64:sparc64",
        "//lib/Arch/X86:x86",
        "//:remill-base"
    ],
    visibility = ["//:__pkg__"]
)
 
