# Guest ELF bytes are embedded into the host app, not linked as a host library.
if get_option('vdso_mode') == 'prebuilt'
    if get_option('vdso_c_args') != ''
        error('vdso_c_args requires -Dvdso_mode=source')
    endif
    vdso = custom_target('vdso-arm64-prebuilt',
        input: ['prebuilt/libvdso.so.elf', 'prebuilt/manifest.json'],
        depend_files: ['vdso.c', 'vdso.S', 'vdso.lds', '../../kernel/vdso.h', 'prepare-prebuilt.py'],
        output: 'libvdso.so.elf',
        command: [find_program('python3'), files('prepare-prebuilt.py'), '@OUTPUT@'],
        build_always_stale: true)
else
    # Try to find an aarch64 compiler
    clang = find_program('/usr/local/opt/llvm/bin/clang', '/opt/homebrew/opt/llvm/bin/clang', '/opt/local/bin/clang', 'clang', required: false)
    has_vdso_compiler = false

    if clang.found()
        # Check if the compiler supports aarch64 target
        check_cc = find_program('./check-cc-arm64.sh', required: false)
        if check_cc.found()
            result = run_command(check_cc, clang, check: false)
            if result.returncode() == 0
                has_vdso_compiler = true
            else
                message('ARM64 VDSO: Cross-compiler check failed')
                message(result.stdout() + result.stderr())
            endif
        endif
    endif

    if has_vdso_compiler
        vdso_compiler = [clang, '-target', 'aarch64-linux-gnu', '-fuse-ld=lld']

        vdso = custom_target('vdso-arm64',
            input: ['vdso.S', 'vdso.c', 'vdso.lds'],
            output: 'libvdso.so.elf',
            command: vdso_compiler + [
                '-o', '@OUTPUT@', '@INPUT0@', '@INPUT1@',
                '-nostdlib', '-Wl,-T,@INPUT2@', '-Wl,--hash-style,sysv',
                '-shared', '-fPIC', '-ffreestanding'
            ] + get_option('vdso_c_args').split())
    else
        error('Source VDSO requires an AArch64 ELF compiler/linker; install LLVM/LLD or use -Dvdso_mode=prebuilt')
    endif
endif
