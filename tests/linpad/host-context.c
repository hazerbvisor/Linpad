// SPDX-License-Identifier: GPL-3.0-only
// Exercise public signal-context layouts; no handler or context-switch APIs.
#include "platform/host_context_aarch64.h"
#include <assert.h>
#include <stdio.h>

#if !defined(__aarch64__)
#error "Run this check on an ARM64 host, or syntax-check an ARM64 target"
#endif

int main(void) {
    ucontext_t context = {0};
    siginfo_t info = {0};
#if defined(__APPLE__)
    _STRUCT_MCONTEXT machine = {0};
    context.uc_mcontext = &machine;
    for (unsigned reg = 0; reg < 29; reg++)
        machine.__ss.__x[reg] = 0x1000 + reg;
    machine.__ss.__lr = 0x2000;
    machine.__es.__esr = 0x90000000;
#elif defined(__linux__)
    for (unsigned reg = 0; reg < 29; reg++)
        context.uc_mcontext.regs[reg] = 0x1000 + reg;
    context.uc_mcontext.regs[30] = 0x2000;
    struct esr_context *esr = (void *) context.uc_mcontext.__reserved;
    esr->head.magic = ESR_MAGIC;
    esr->head.size = sizeof(*esr);
    esr->esr = 0x90000000;
#else
#error "Signal-context check supports Darwin and Linux"
#endif
    for (unsigned reg = 0; reg < 29; reg++)
        assert(host_ctx_aarch64_reg(&context, reg) == 0x1000 + reg);
    assert(host_ctx_aarch64_lr(&context) == 0x2000);
    host_ctx_aarch64_set_pc(&context, 0x3000);
    host_ctx_aarch64_set_sp(&context, 0x4000);
    assert(host_ctx_aarch64_pc(&context) == 0x3000);
    assert(host_ctx_aarch64_sp(&context) == 0x4000);
    assert(host_ctx_aarch64_lr(&context) == 0x2000);
    assert(host_ctx_aarch64_reg(&context, 1) == 0x1001);
    assert(host_ctx_aarch64_esr(&context) == 0x90000000);
    assert(!host_ctx_aarch64_fault_was_write(&context, &info));
#if defined(__APPLE__)
    machine.__es.__esr |= 0x40;
#else
    esr->esr |= 0x40;
#endif
    assert(host_ctx_aarch64_fault_was_write(&context, &info));
    assert(host_ctx_aarch64_esr(&context) == 0x90000040);
    puts("ARM64 signal-context helpers passed");
    return 0;
}
