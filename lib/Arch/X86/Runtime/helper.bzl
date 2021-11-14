load("//build_tools/BCCompiler:bc-compiler.bzl", "bc_module")

def helper(name, sources, bits, avx, avx512):
  bc_module(
    name = name,
    srcs = sources,
    additional_deps = ["//lib/Arch/X86/Semantics:all", "//include:all_headers"],
    definitions = {
      "HAS_FEATURE_AVX": "1" if avx else "0",
      "HAS_FEATURE_AVX512": "1" if avx512 else "0",
    },
    bits = bits,
    include_directories = ["//include:.", "//:."]
  )