// SPDX-License-Identifier: GPL-3.0-only
#ifndef ISH_SYSCALL_ADAPTER_H
#define ISH_SYSCALL_ADAPTER_H

#include <stdint.h>

// The dispatcher always supplies six full-width guest registers. Call each
// implementation through its declared C prototype so argument truncation and
// signed/unsigned return widening are performed by the compiler, not an ABI cast.
#define ISH_SYSCALL_ARGS_0
#define ISH_SYSCALL_ARGS_1 a0
#define ISH_SYSCALL_ARGS_2 a0, a1
#define ISH_SYSCALL_ARGS_3 a0, a1, a2
#define ISH_SYSCALL_ARGS_4 a0, a1, a2, a3
#define ISH_SYSCALL_ARGS_5 a0, a1, a2, a3, a4
#define ISH_SYSCALL_ARGS_6 a0, a1, a2, a3, a4, a5

#define ISH_SYSCALL_ADAPTER(function, count) \
    static int64_t syscall_entry_##function( \
        uint64_t a0 __attribute__((unused)), uint64_t a1 __attribute__((unused)), \
        uint64_t a2 __attribute__((unused)), uint64_t a3 __attribute__((unused)), \
        uint64_t a4 __attribute__((unused)), uint64_t a5 __attribute__((unused))) { \
        return function(ISH_SYSCALL_ARGS_##count); \
    }

#endif
