// SPDX-License-Identifier: GPL-3.0-only
// Host ABI controls: no guest execution and no function-pointer casts.
#include "kernel/syscall_adapter.h"
#include <assert.h>
#include <stdio.h>

static int32_t signed_error(void) { return -9; }
static uint32_t unsigned_result(uint32_t flags) {
    assert(flags == 0x12345678);
    return 0x80000000;
}
static uint64_t address_result(uint64_t address, uint64_t size) {
    assert(size == 0x100000000);
    return address;
}
static int64_t seek_result(int32_t fd, int64_t offset, uint32_t whence) {
    assert(fd == -100 && whence == 2);
    return offset;
}
static int32_t four_args(uint32_t a, uint64_t b, int32_t c, uint64_t d) {
    assert(a == 1 && b == 0x100000002 && c == -3 && d == 0x200000004);
    return -14;
}
static int64_t five_args(uint64_t a, uint64_t b, uint64_t c, uint64_t d, uint64_t e) {
    assert(a == 1 && b == 2 && c == 3 && d == 4 && e == 0x100000005);
    return -5000;
}
static int32_t six_args(int32_t fd, uint64_t ptr, uint32_t size,
                       uint32_t flags, uint64_t address, uint64_t length_ptr) {
    assert(fd == -100 && ptr == 0x123456789abc && size == 7 && flags == 8);
    assert(address == 0x23456789abcd && length_ptr == 0x3456789abcde);
    return -22;
}
static uint32_t unsigned_error(void) { return (uint32_t) -38; }

ISH_SYSCALL_ADAPTER(signed_error, 0)
ISH_SYSCALL_ADAPTER(unsigned_result, 1)
ISH_SYSCALL_ADAPTER(address_result, 2)
ISH_SYSCALL_ADAPTER(seek_result, 3)
ISH_SYSCALL_ADAPTER(four_args, 4)
ISH_SYSCALL_ADAPTER(five_args, 5)
ISH_SYSCALL_ADAPTER(six_args, 6)
ISH_SYSCALL_ADAPTER(unsigned_error, 0)

typedef int64_t (*entry_t)(uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t);
int main(void) {
    entry_t entries[] = {syscall_entry_signed_error, syscall_entry_unsigned_result,
        syscall_entry_address_result, syscall_entry_seek_result,
        syscall_entry_four_args, syscall_entry_five_args, syscall_entry_six_args,
        syscall_entry_unsigned_error};
    assert(entries[0](1, 2, 3, 4, 5, 6) == -9);
    assert(entries[1](0xffffffff12345678, 2, 3, 4, 5, 6) == 0x80000000);
    assert(entries[2](0x123456789abc, 0x100000000, 3, 4, 5, 6) == 0x123456789abc);
    // A valid full-width lseek result near the 32-bit errno range stays positive.
    assert(entries[3](0x12345678ffffff9c, 0xfffffff2, 0xffffffff00000002, 4, 5, 6) == 0xfffffff2);
    assert(entries[3](0xffffffffffffff9c, -14, 2, 4, 5, 6) == -14);
    assert(entries[4](0x100000001, 0x100000002, 0xfffffffffffffffd, 0x200000004, 5, 6) == -14);
    assert(entries[5](1, 2, 3, 4, 0x100000005, 6) == -5000);
    assert(entries[6](0xffffff9c, 0x123456789abc, 0xffffffff00000007,
                      0xffffffff00000008, 0x23456789abcd, 0x3456789abcde) == -22);
    // Preserve unsigned 32-bit errors for the dispatcher's existing normalization.
    assert(entries[7](1, 2, 3, 4, 5, 6) == (uint32_t) -38);
    puts("Syscall adapters passed: arities 0-6, truncation, signed errors, 64-bit addresses/offsets");
    return 0;
}
