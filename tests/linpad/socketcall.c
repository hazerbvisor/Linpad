// SPDX-License-Identifier: GPL-3.0-only
// Link the real legacy dispatcher against host controls for its socket handlers.
#include "kernel/calls.h"
#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

static unsigned called, copied, read_count;
static bool fault;
static dword_t guest_args[6];
static uint64_t observed[6];
void ish_printk(const char *message, ...) { (void) message; }
int user_read(addr_t address, void *buffer, size_t count) {
    assert(address == 0x123456789abc && count <= sizeof(guest_args));
    read_count++;
    if (fault) return 1;
    copied = count;
    memcpy(buffer, guest_args, count);
    return 0;
}
static int_t record(unsigned number, unsigned count, const uint64_t *args) {
    called = number;
    memcpy(observed, args, count * sizeof(*args));
    return -9;
}
#define HANDLER(number, name, prototype, ...) \
    int_t name prototype { \
        const uint64_t args[] = {__VA_ARGS__}; \
        return record(number, sizeof(args) / sizeof(*args), args); \
    }
HANDLER(1, sys_socket, (dword_t a, dword_t b, dword_t c), a, b, c)
HANDLER(2, sys_bind, (fd_t a, addr_t b, uint_t c), a, b, c)
HANDLER(3, sys_connect, (fd_t a, addr_t b, uint_t c), a, b, c)
HANDLER(4, sys_listen, (fd_t a, int_t b), a, b)
HANDLER(5, sys_accept, (fd_t a, addr_t b, addr_t c), a, b, c)
HANDLER(6, sys_getsockname, (fd_t a, addr_t b, addr_t c), a, b, c)
HANDLER(7, sys_getpeername, (fd_t a, addr_t b, addr_t c), a, b, c)
HANDLER(8, sys_socketpair, (dword_t a, dword_t b, dword_t c, addr_t d), a, b, c, d)
HANDLER(9, sys_send, (fd_t a, addr_t b, dword_t c, int_t d), a, b, c, d)
HANDLER(10, sys_recv, (fd_t a, addr_t b, dword_t c, int_t d), a, b, c, d)
HANDLER(11, sys_sendto, (fd_t a, addr_t b, dword_t c, dword_t d, addr_t e, dword_t f), a, b, c, d, e, f)
HANDLER(12, sys_recvfrom, (fd_t a, addr_t b, dword_t c, dword_t d, addr_t e, addr_t f), a, b, c, d, e, f)
HANDLER(13, sys_shutdown, (fd_t a, dword_t b), a, b)
HANDLER(14, sys_setsockopt, (fd_t a, dword_t b, dword_t c, addr_t d, dword_t e), a, b, c, d, e)
HANDLER(15, sys_getsockopt, (fd_t a, dword_t b, dword_t c, addr_t d, addr_t e), a, b, c, d, e)
HANDLER(16, sys_sendmsg, (fd_t a, addr_t b, int_t c), a, b, c)
HANDLER(17, sys_recvmsg, (fd_t a, addr_t b, int_t c), a, b, c)
HANDLER(20, sys_sendmmsg, (fd_t a, addr_t b, uint_t c, int_t d), a, b, c, d)
#undef HANDLER

int main(void) {
    const unsigned counts[] = {0, 3, 3, 3, 2, 3, 3, 3, 4, 4, 4, 6, 6, 2, 5, 5, 3, 3, 0, 0, 4};
    for (unsigned call = 1; call <= 20; call++) {
        if (call == 18 || call == 19) continue;
        for (unsigned arg = 0; arg < 6; arg++) guest_args[arg] = 0x80000010 + arg;
        called = read_count = copied = 0;
        assert(sys_socketcall(call, 0x123456789abc) == -9);
        assert(called == call && read_count == 1 && copied == counts[call] * 4);
        for (unsigned arg = 0; arg < counts[call]; arg++) {
            bool signed_arg = (arg == 0 && call != 1 && call != 8) ||
                (arg == 1 && call == 4) ||
                (arg == 3 && (call == 9 || call == 10 || call == 20)) ||
                (arg == 2 && (call == 16 || call == 17));
            uint64_t expected = signed_arg ? (uint64_t) (int64_t) (int32_t) guest_args[arg] : guest_args[arg];
            assert(observed[arg] == expected);
        }
        fault = true; called = read_count = 0;
        assert(sys_socketcall(call, 0x123456789abc) == _EFAULT);
        assert(called == 0 && read_count == 1);
        fault = false;
    }
    read_count = called = 0;
    assert(sys_socketcall(0, 0) == _EINVAL);
    assert(sys_socketcall(21, 0) == _EINVAL);
    assert(sys_socketcall(UINT32_MAX, 0) == _EINVAL);
    assert(sys_socketcall(18, 0) == _ENOSYS);
    assert(sys_socketcall(19, 0) == _ENOSYS);
    assert(read_count == 0 && called == 0);
    puts("Socketcall passed: all 18 mappings, signed arguments/errors, exact reads, fault and invalid-call paths");
    return 0;
}
