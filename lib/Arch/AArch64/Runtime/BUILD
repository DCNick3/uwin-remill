load(":helper.bzl", "helper")

helper("aarch64be", 64, 0)
helper("aarch64", 64, 1)

filegroup(
  name = "all",
  srcs = [":aarch64", ":aarch64be"],
  visibility = ["//visibility:public"],
)