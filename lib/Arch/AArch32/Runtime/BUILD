load(":helper.bzl", "helper")

helper("aarch32", 32, 1)

filegroup(
  name = "all",
  srcs = [":aarch32"],
  visibility = ["//visibility:public"],
)