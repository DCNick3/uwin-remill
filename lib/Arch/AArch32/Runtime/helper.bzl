load("//build_tools/BCCompiler:bc-compiler.bzl", "bc_module")

def helper(name, bits, le):
  bc_module(
    name = name,
    basic_block_src = "BasicBlock.cpp",
    instructions_src = "Instructions.cpp",
    additional_deps = ["//lib/Arch/AArch32/Semantics:all", "//include:all_headers"],
    definitions = {
      "LITTLE_ENDIAN": "1" if le else "0",
    },
    bits = bits,
    include_directories = ["//include:.", "//:."],
    target = "armhf",
  )