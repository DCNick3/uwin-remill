
def _compile_xed_impl(ctx):
    cxx_toolchain = ctx.toolchains["@bazel_tools//tools/cpp:toolchain_type"]
    
    #print(cxx_toolchain.toolchain_config)
    
    outdir = ctx.actions.declare_directory('out')
    
    mfile_args = []
    
    mfile_args += ["clean", "install"]
    mfile_args += ["--static"]
    mfile_args += ["--src-dir", ctx.files.source_root[0].path]
    mfile_args += ["--install-dir", outdir.path]

    mfile_args += ["--cc", cxx_toolchain.compiler_executable]
    mfile_args += ["--cxx", cxx_toolchain.compiler_executable]
    mfile_args += ["--as", cxx_toolchain.compiler_executable] # will this work?
    mfile_args += ["--ar", cxx_toolchain.ar_executable]
    mfile_args += ["--strip", cxx_toolchain.strip_executable]
    mfile_args += ["--verbose=0"]
    
    libs = [
        "libxed.a",
        "libxed-ild.a",
    ]
    includes = ['xed-address-width-enum.h', 'xed-agen.h', 'xed-attribute-enum.h', 'xed-attributes.h', 'xed-build-defines.h', 'xed-category-enum.h', 'xed-chip-enum.h', 'xed-chip-features.h', 'xed-common-defs.h', 'xed-common-hdrs.h', 'xed-convert-table-init.h', 'xed-cpuid-bit-enum.h', 'xed-cpuid-rec.h', 'xed-decoded-inst-api.h', 'xed-decoded-inst.h', 'xed-decode.h', 'xed-disas.h', 'xed-encode-check.h', 'xed-encode-direct.h', 'xed-encode.h', 'xed-encoder-gen-defs.h', 'xed-encoder-hl.h', 'xed-encoder-iforms.h', 'xed-error-enum.h', 'xed-exception-enum.h', 'xed-extension-enum.h', 'xed-flag-action-enum.h', 'xed-flag-enum.h', 'xed-flags.h', 'xed-format-options.h', 'xed-gen-table-defs.h', 'xed-get-time.h', 'xed-iclass-enum.h', 'xed-iform-enum.h', 'xed-iformfl-enum.h', 'xed-iform-map.h', 'xed-ild.h', 'xed-immdis.h', 'xed-immed.h', 'xed-init.h', 'xed-init-pointer-names.h', 'xed-inst.h', 'xed-interface.h', 'xed-isa-set-enum.h', 'xed-isa-set.h', 'xed-machine-mode-enum.h', 'xed-nonterminal-enum.h', 'xed-operand-accessors.h', 'xed-operand-action-enum.h', 'xed-operand-action.h', 'xed-operand-convert-enum.h', 'xed-operand-ctype-enum.h', 'xed-operand-ctype-map.h', 'xed-operand-element-type-enum.h', 'xed-operand-element-xtype-enum.h', 'xed-operand-enum.h', 'xed-operand-storage.h', 'xed-operand-type-enum.h', 'xed-operand-values-interface.h', 'xed-operand-visibility-enum.h', 'xed-operand-width-enum.h', 'xed-patch.h', 'xed-portability.h', 'xed-print-info.h', 'xed-reg-class-enum.h', 'xed-reg-class.h', 'xed-reg-enum.h', 'xed-reg-role-enum.h', 'xed-rep-prefix.h', 'xed-state.h', 'xed-syntax-enum.h', 'xed-types.h', 'xed-util.h', 'xed-version.h']
    
    # is this required?
    # should I be declaring directories I do not care about?
    dirs = [
        outdir,
        ctx.actions.declare_directory('out/lib'),
        ctx.actions.declare_directory('out/include'),
        ctx.actions.declare_directory('out/include/xed')
    ]
    
    # same for files: is it required to declare unimportant files?
    outputs = ['out/lib/' + lib for lib in libs] + ['out/include/xed/' + include for include in includes]
    outputs = dirs + [ ctx.actions.declare_file(file) for file in outputs ]
    
    #print(ctx.executable.mfile)
    
    ctx.actions.run(
        inputs = ctx.files.all_sources + cxx_toolchain.all_files.to_list(),
        outputs = outputs,
        arguments = mfile_args,
        executable = ctx.executable.mfile,
        
        mnemonic = "XedCompile",
        progress_message = "Compiling xed using its NIH build system",
    )
    
    return [
        DefaultInfo(files = depset(outputs))
    ]
    
    print(mfile_args)
    pass


compile_xed = rule(
    implementation = _compile_xed_impl,
    toolchains = ["@bazel_tools//tools/cpp:toolchain_type"],
    attrs = {
        "mfile": attr.label(
            mandatory = True,
            executable = True,
            cfg = "exec"
        ),    
        "all_sources": attr.label(
            mandatory = True
        ),
        "source_root": attr.label(
            mandatory = True,
            allow_single_file = True
        )
    }
) 
