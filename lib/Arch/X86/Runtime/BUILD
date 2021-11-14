load(":helper.bzl", "helper")

srcs = glob(["*.cpp"])

helper("x86", srcs, 32, 0, 0)
helper("x86_avx", srcs, 32, 1, 0)
helper("x86_avx512", srcs, 32, 1, 1)

helper("amd64", srcs, 64, 0, 0)
helper("amd64_avx", srcs, 64, 1, 0)
helper("amd64_avx512", srcs, 64, 1, 1)
