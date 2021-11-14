load("//build_tools/BCCompiler:bc-compiler.bzl", "bc_module")

bc_module(
  name = "aarch32",
  srcs = glob(["*.cpp"]),
  additional_deps = ["//lib/Arch/AArch32/Semantics:all", "//include:all_headers"],
  definitions = {"LITTLE_ENDIAN": "1"},
  bits = 32,
  include_directories = ["//include:.", "//:."]
)