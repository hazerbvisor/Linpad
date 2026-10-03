// SPDX-License-Identifier: GPL-3.0-only
// Legacy dispatch separated from imported fs/sock.c; see UPSTREAM.md.
#include "kernel/calls.h"

// Legacy socketcall uses 32-bit argument words, not the ARM64 syscall ABI.
#define SOCKETCALL_ADAPTER(function, ...) \
    static int_t socketcall_##function(const dword_t *args) { \
        return function(__VA_ARGS__); \
    }
SOCKETCALL_ADAPTER(sys_socket, args[0], args[1], args[2])
SOCKETCALL_ADAPTER(sys_bind, args[0], args[1], args[2])
SOCKETCALL_ADAPTER(sys_connect, args[0], args[1], args[2])
SOCKETCALL_ADAPTER(sys_listen, args[0], args[1])
SOCKETCALL_ADAPTER(sys_accept, args[0], args[1], args[2])
SOCKETCALL_ADAPTER(sys_getsockname, args[0], args[1], args[2])
SOCKETCALL_ADAPTER(sys_getpeername, args[0], args[1], args[2])
SOCKETCALL_ADAPTER(sys_socketpair, args[0], args[1], args[2], args[3])
SOCKETCALL_ADAPTER(sys_send, args[0], args[1], args[2], args[3])
SOCKETCALL_ADAPTER(sys_recv, args[0], args[1], args[2], args[3])
SOCKETCALL_ADAPTER(sys_sendto, args[0], args[1], args[2], args[3], args[4], args[5])
SOCKETCALL_ADAPTER(sys_recvfrom, args[0], args[1], args[2], args[3], args[4], args[5])
SOCKETCALL_ADAPTER(sys_shutdown, args[0], args[1])
SOCKETCALL_ADAPTER(sys_setsockopt, args[0], args[1], args[2], args[3], args[4])
SOCKETCALL_ADAPTER(sys_getsockopt, args[0], args[1], args[2], args[3], args[4])
SOCKETCALL_ADAPTER(sys_sendmsg, args[0], args[1], args[2])
SOCKETCALL_ADAPTER(sys_recvmsg, args[0], args[1], args[2])
SOCKETCALL_ADAPTER(sys_sendmmsg, args[0], args[1], args[2], args[3])
#undef SOCKETCALL_ADAPTER
static const struct socket_call {
    int_t (*func)(const dword_t *args);
    int args;
} socket_calls[] = {
    {NULL, 0},
    {socketcall_sys_socket, 3},
    {socketcall_sys_bind, 3},
    {socketcall_sys_connect, 3},
    {socketcall_sys_listen, 2},
    {socketcall_sys_accept, 3},
    {socketcall_sys_getsockname, 3},
    {socketcall_sys_getpeername, 3},
    {socketcall_sys_socketpair, 4},
    {socketcall_sys_send, 4}, // send
    {socketcall_sys_recv, 4}, // recv
    {socketcall_sys_sendto, 6},
    {socketcall_sys_recvfrom, 6},
    {socketcall_sys_shutdown, 2},
    {socketcall_sys_setsockopt, 5},
    {socketcall_sys_getsockopt, 5},
    {socketcall_sys_sendmsg, 3},
    {socketcall_sys_recvmsg, 3},
    {NULL, 0}, // accept4
    {NULL, 0}, // recvmmsg
    {socketcall_sys_sendmmsg, 4},
};

int_t sys_socketcall(dword_t call_num, addr_t args_addr) {
    STRACE("%d ", call_num);
    if (call_num < 1 || call_num >= sizeof(socket_calls)/sizeof(socket_calls[0]))
        return _EINVAL;
    struct socket_call call = socket_calls[call_num];
    if (call.func == NULL) {
        FIXME("socketcall %d", call_num);
        return _ENOSYS;
    }

    dword_t args[6] = {0};
    if (user_read(args_addr, args, sizeof(dword_t) * call.args))
        return _EFAULT;
    return call.func(args);
}
