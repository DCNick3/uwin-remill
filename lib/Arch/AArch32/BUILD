
cc_library(
    name = "aarch32",
    srcs = glob(["*.cpp", "*.h"]) + ["//lib/Arch:Arch.h"],
    visibility = [ "//lib:__subpackages__" ],
    copts = ["-Wno-c99-designator"], # TODO: is this the best way to express it?
    deps = [
        "//:remill-base",
    ]
)
 
