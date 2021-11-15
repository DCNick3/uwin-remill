
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


def _bc_module_impl(ctx):
    sources = \
        [(x, ["-O3", "-g0"]) for x in ctx.files.instructions_src] + \
        [(x, ["-O0", "-g3"]) for x in ctx.files.basic_block_src]
    additional_deps = ctx.files.additional_deps
    sysroot_deps = ctx.files._sysroot_deps
    clang_deps = ctx.files._clang_deps

    sysroot = ctx.files._sysroot[0]
    include_directories = [ x.path or "." for x in ctx.files.include_directories ]

    clang_path = ctx.files._clang[0].path

    # the internal clang search directory
    include_directories.append(paths.join(
            paths.dirname(clang_path),
            "staging/include"
        )
    )
    # the sysroot paths
    # c++ includes should go first (magic)
    include_directories.append(paths.join(sysroot.path, "usr/include/c++/6"))
    include_directories.append(paths.join(sysroot.path, "usr/include/x86_64-linux-gnu/c++/6"))
    include_directories.append(paths.join(sysroot.path, "usr/include"))
    include_directories.append(paths.join(sysroot.path, "usr/include/x86_64-linux-gnu"))

    output = ctx.actions.declare_file(ctx.label.name + ".bc")

    clang = ctx.executable._clang
    llvm_link = ctx.executable._llvm_link

    full_inputs = [f for f,a in sources] + additional_deps + sysroot_deps + clang_deps

    object_files = []

    for source, additional_flags in sources:
        obj_output = ctx.actions.declare_file(ctx.label.name + "_" + source.basename + ".obj.bc")

        object_files.append(obj_output)

        args = ctx.actions.args()
        args.add_all(DEFAULT_BC_FLAGS)
        args.add_all(["-D%s=%s" % kv for kv in ctx.attr.definitions.items()])
        args.add("-DADDRESS_SIZE_BITS=%d" % ctx.attr.bits)
        args.add_all(include_directories, before_each = "-I")
        args.add("-isysroot", sysroot)

        # TODO: how do arch other than x86 compile?
        args.add_all(additional_flags)

        args.add("-c")
        args.add(source)
        args.add("-o", obj_output)

        ctx.actions.run(
            outputs = [obj_output],
            inputs = full_inputs,

            executable = clang,
            arguments = [args],
            mnemonic = "BcCompile",
            progress_message = "Compiling bitcode of %s for %s" % (source.basename, ctx.label.name),
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

CLANG_INTERNAL_HEADERS = ["stddef.h", "stdint.h", "stdbool.h", "stdarg.h", "float.h"]

bc_module = rule(
    implementation = _bc_module_impl,
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
            doc = "List of additional include directories. The actual header files should be specified as additional_deps"
        ),
        "_sysroot": attr.label(
            allow_single_file = True,
            default = "@bitcode_sysroot//:."
        ),
        "_sysroot_deps": attr.label(
            default = "@bitcode_sysroot//:all_files"
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
