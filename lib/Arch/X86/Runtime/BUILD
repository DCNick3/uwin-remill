load(":helper.bzl", "helper")

helper("x86", 32, 0, 0)
helper("x86_avx", 32, 1, 0)
helper("x86_avx512", 32, 1, 1)

helper("amd64", 64, 0, 0)
helper("amd64_avx", 64, 1, 0)
helper("amd64_avx512", 64, 1, 1)

filegroup(
  name = "all",
  srcs = [":x86", ":x86_avx", ":x86_avx512", ":amd64", ":amd64_avx", ":amd64_avx512"],
  visibility = ["//visibility:public"],
)
