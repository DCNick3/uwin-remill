
cc_library(
    name = "sparc64",
    srcs = glob(["*.cpp", "*.h"]),
    visibility = [ "//lib:__subpackages__" ],
    copts = ["-Wno-c99-designator"], # TODO: is this the best way to express it?
    deps = [
        "//lib/Arch/SPARC32:sparc32",
        "//:remill-base",
    ]
)
 
