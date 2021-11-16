def helper(name, bits, avx, avx512):
  bc_module(
    name = name,
    basic_block_src = "BasicBlock.cpp",
    instructions_src = "Instructions.cpp",
    additional_deps = ["//lib/Arch/X86/Semantics:all", "//include:all_headers"],
    definitions = {
      "HAS_FEATURE_AVX": "1" if avx else "0",
      "HAS_FEATURE_AVX512": "1" if avx512 else "0",
    },
    bits = bits,
    include_directories = ["//include:.", "//:."],
    target = "x86_64",
  )