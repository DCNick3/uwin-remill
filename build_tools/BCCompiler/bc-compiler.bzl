
load("@bazel_skylib//lib:paths.bzl", "paths")

DEFAULT_BC_FLAGS = [
    '-emit-llvm', '-Wno-unknown-warning-option', '-Wall', '-Wshadow', '-Wconversion', '-Wpadded', '-pedantic', 
    '-Wshorten-64-to-32', '-Wgnu-alignof-expression', '-Wno-gnu-anonymous-struct', '-Wno-return-type-c-linkage', 
    '-Wno-gnu-zero-variadic-macro-arguments', '-Wno-nested-anon-types', '-Wno-extended-offsetof', 
    '-Wno-gnu-statement-expression', '-Wno-c99-extensions', '-Wno-ignored-attributes', '-mtune=generic', 
    '-fno-vectorize', '-fno-slp-vectorize', '-Wno-variadic-macros', '-Wno-c11-extensions', 
    '-Wno-c++11-extensions', '-ffreestanding', '-fno-common', '-fno-builtin', '-fno-exceptions', '-fno-rtti',
    '-fno-asynchronous-unwind-tables', '-Wno-unneeded-internal-declaration', '-Wno-unused-function',
    '-Wgnu-inline-cpp-without-extern', '-std=c++14', '-Wno-pass-failed=transform-warning',
    '-nostdinc', '-nostdinc++'
]

ARCH_TO_TARGET_TRIPLE = {
    "x86_64": "x86_64-linux-gnu",
    "aarch64": "aarch64-linux-gnu",
    "armhf": "arm-linux-gnueabihf"
}

def _build_std_include_directories(sysroot_path, target_triple):
    # the sysroot paths
    # c++ includes should go first (magic)
    include_directories = []
    
    include_directories.append(paths.join(sysroot_path, "usr/include/c++/6"))
    include_directories.append(paths.join(sysroot_path, "usr/include/%s/c++/6" % target_triple))
    include_directories.append(paths.join(sysroot_path, "usr/include"))
    include_directories.append(paths.join(sysroot_path, "usr/include/%s" % target_triple))
    
    return include_directories

def _compile_bc_obj(ctx, source, obj_output, additional_flags, clang, target_triple, sysroot, include_directories, common_deps):
    #bits = ctx.attr.bits
    #obj_output = ctx.actions.declare_file(ctx.label.name + "_" + source.basename + ".obj.bc")

    #object_files.append(obj_output)

    args = ctx.actions.args()
    args.add_all(DEFAULT_BC_FLAGS)
    #args.add_all(["-D%s=%s" % kv for kv in ctx.attr.definitions.items()])
    #args.add("-DADDRESS_SIZE_BITS=%d" % bits)
    args.add_all(include_directories, before_each = "-I")
    args.add("-isysroot", sysroot)
    args.add("--target=" + target_triple)

    args.add_all(additional_flags)

    args.add("-c")
    args.add(source)
    args.add("-o", obj_output)

    ctx.actions.run(
        outputs = [obj_output],
        inputs = depset([source], transitive=[common_deps]),

        executable = clang,
        arguments = [args],
        mnemonic = "BcCompile",
        progress_message = "Compiling bitcode of %s for %s" % (source.basename, ctx.label.name),
    )


def _bc_runtime_impl(ctx):
    sources = \
        [(x, ["-O3", "-g0"]) for x in ctx.files.instructions_src] + \
        [(x, ["-O0", "-g3"]) for x in ctx.files.basic_block_src] + \
        [(x, ["-O0", "-g0"]) for x in ctx.files.intrinsics_src]
    additional_deps = depset(ctx.files.additional_deps)
    sysroot_deps = depset(ctx.files.sysroot_deps)
    clang_deps = depset(ctx.files._clang_deps)

    sysroot = ctx.files.sysroot[0]
    include_directories = [ x.path or "." for x in ctx.files.include_directories ]

    clang_path = ctx.files._clang[0].path

    # the internal clang search directory
    include_directories.append(paths.join(
            paths.dirname(clang_path),
            "staging/include"
        )
    )

    target_triple = ARCH_TO_TARGET_TRIPLE[ctx.attr.target]
    include_directories += _build_std_include_directories(sysroot.path, target_triple)

    output = ctx.actions.declare_file(ctx.label.name + ".bc")

    clang = ctx.executable._clang
    llvm_link = ctx.executable._llvm_link

    common_deps = depset([], transitive = [additional_deps, sysroot_deps, clang_deps],)

    object_files = []

    for source, additional_flags in sources:
        bits = ctx.attr.bits

        obj_output = ctx.actions.declare_file(ctx.label.name + "_" + source.basename + ".obj.bc")

        object_files.append(obj_output)

        additional_flags = ["-D%s=%s" % kv for kv in ctx.attr.definitions.items()]
        additional_flags.append("-DADDRESS_SIZE_BITS=%d" % bits)
        if bits == 32:
            additional_flags.append("-m32")

        _compile_bc_obj(ctx, source, obj_output,
            additional_flags = additional_flags,
            clang = clang,
            target_triple = target_triple,
            sysroot = sysroot,
            include_directories = include_directories,
            common_deps = common_deps,
        )

    args = ctx.actions.args()
    args.add_all(object_files)
    args.add("-o", output)

    ctx.actions.run(
        outputs = [output],
        inputs = object_files,

        executable = llvm_link,
        arguments = [args],
        mnemonic = "BcLink",
        progress_message = "Linking bitcode for %s" % ctx.label.name,
    )

    return [
        DefaultInfo(files = depset([output]))
    ]

def _bc_object_impl(ctx):
    additional_deps = depset(ctx.files.additional_deps)
    sysroot_deps = depset(ctx.files.sysroot_deps)
    clang_deps = depset(ctx.files._clang_deps)

    sysroot = ctx.files.sysroot[0]
    include_directories = [ x.path or "." for x in ctx.files.include_directories ]

    clang_path = ctx.files._clang[0].path

    # the internal clang search directory
    include_directories.append(paths.join(
            paths.dirname(clang_path),
            "staging/include"
        )
    )

    target_triple = ARCH_TO_TARGET_TRIPLE[ctx.attr.target]
    include_directories += _build_std_include_directories(sysroot.path, target_triple)

    output = ctx.actions.declare_file(ctx.label.name + ".bc")
    clang = ctx.executable._clang
    common_deps = depset([], transitive = [additional_deps, sysroot_deps, clang_deps],)
    additional_flags = ["-D%s=%s" % kv for kv in ctx.attr.definitions.items()]
    additional_flags += ctx.attr.opts

    _compile_bc_obj(ctx, ctx.file.src, output,
        additional_flags = additional_flags,
        clang = clang,
        target_triple = target_triple,
        sysroot = sysroot,
        include_directories = include_directories,
        common_deps = common_deps,
    )

    return [
        DefaultInfo(files = depset([output]))
    ]

CLANG_INTERNAL_HEADERS = ["stddef.h", "stdint.h", "stdbool.h", "stdarg.h", "float.h"]


def _select_sysroot(target):
    sysroots = {
        'x86_64': "@bitcode_sysroot_amd64",
        'aarch64': "@bitcode_sysroot_arm64",
        'armhf': "@bitcode_sysroot_armhf",
    }
    sysroot = sysroots[target]
    sysroot_deps = sysroot + "//:all_files"
    sysroot = sysroot + "//:."
    return (sysroot, sysroot_deps)
    
_bc_runtime = rule(
    implementation = _bc_runtime_impl,
    attrs = {
        "instructions_src": attr.label(
            mandatory = True,
            allow_single_file = [".cpp"],
            doc = "Files to be fed into the compiler to produce instructions bitcode",
        ),
        "basic_block_src": attr.label(
            mandatory = True,
            allow_single_file = [".cpp"],
            doc = "Files to be fed into the compiler to produce basic block bitcode",
        ),
        "intrinsics_src": attr.label(
            allow_single_file = [".cpp"],
            default = "//lib/Arch/Runtime:Intrinsics.cpp",
            doc = "Files to be fed into the compiler to produce intrinsics bitcode",
        ),
        "additional_deps": attr.label_list(
            mandatory = True,
            doc = "Dependencies to be injected into the compiler invocation (useful for files included and such)",
        ),
        "definitions": attr.string_dict(
            doc = "Compiler definitions for this bitcode module",
        ),
        "bits": attr.int(
            mandatory = True,
            values = [32, 64],
            doc = "Width of the word for the target. Used to populate the ADDRESS_SIZE_BITS definition and set the compiler target",
        ),
        "include_directories": attr.label_list(
            allow_files = True,
            doc = "List of additional include directories. The actual header files should be specified as additional_deps",
        ),
        "target": attr.string(
            values = ["x86_64", "aarch64", "armhf"],
            doc = "Target to build for (only processor; linux is assumed); x86_64 + bits=32 -> i686",
        ),
        "sysroot": attr.label(
            allow_single_file = True,
            mandatory = True,
        ),
        "sysroot_deps": attr.label(
            mandatory = True,
        ),
        "_llvm_link": attr.label(
            executable = True,
            cfg = "exec",
            default = "@llvm-project//llvm:llvm-link",
        ),
        "_clang": attr.label(
            executable = True,
            cfg = "exec",
            default = "@llvm-project//clang:clang",
        ),
        "_clang_deps": attr.label_list(
            allow_files = True,
            cfg = "exec",
            default = [ "@llvm-project//clang:staging/include/" + include for include in CLANG_INTERNAL_HEADERS ]
        )
    }
)
    
_bc_object = rule(
    implementation = _bc_object_impl,
    attrs = {
        "src": attr.label(
            mandatory = True,
            allow_single_file = [".cpp"],
            doc = "Files to be fed into the compiler to produce bitcode object",
        ),
        "opts": attr.string_list(
            doc = "Additional flags to pass to clang during compilation",
        ),
        "additional_deps": attr.label_list(
            mandatory = True,
            doc = "Dependencies to be injected into the compiler invocation (useful for files included and such)",
        ),
        "definitions": attr.string_dict(
            doc = "Compiler definitions for this bitcode module",
        ),
        "include_directories": attr.label_list(
            allow_files = True,
            doc = "List of additional include directories. The actual header files should be specified as additional_deps",
        ),
        "target": attr.string(
            values = ["x86_64", "aarch64", "armhf"],
            doc = "Target to build for (only processor; linux is assumed); x86_64 + bits=32 -> i686",
        ),
        "sysroot": attr.label(
            allow_single_file = True,
            mandatory = True,
        ),
        "sysroot_deps": attr.label(
            mandatory = True,
        ),
        "_clang": attr.label(
            executable = True,
            cfg = "exec",
            default = "@llvm-project//clang:clang",
        ),
        "_clang_deps": attr.label_list(
            allow_files = True,
            cfg = "exec",
            default = [ "@llvm-project//clang:staging/include/" + include for include in CLANG_INTERNAL_HEADERS ]
        )
    }
)

def bc_runtime(name, instructions_src, basic_block_src, additional_deps, definitions, bits, include_directories, target):
    sysroot, sysroot_deps = _select_sysroot(target)

    _bc_runtime(
        name = name,
        instructions_src = instructions_src,
        basic_block_src = basic_block_src,
        additional_deps = additional_deps,
        definitions = definitions,
        bits = bits,
        include_directories = include_directories,
        target = target,
        sysroot = sysroot,
        sysroot_deps = sysroot_deps,
    )

def bc_object(name, src, additional_deps, definitions, include_directories, target, opts):
    sysroot, sysroot_deps = _select_sysroot(target)

    _bc_object(
        name = name,
        src = src,
        additional_deps = additional_deps,
        definitions = definitions,
        include_directories = include_directories,
        target = target,
        opts = opts,
        sysroot = sysroot,
        sysroot_deps = sysroot_deps,
    )
