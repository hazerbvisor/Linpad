// SPDX-License-Identifier: GPL-3.0-only
// Standalone diagnostics for fakefsify; the app uses kernel/log.c instead.
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include "debug.h"

void ish_vprintk(const char *message, va_list args) {
    vfprintf(stderr, message, args);
}

void ish_printk(const char *message, ...) {
    va_list args;
    va_start(args, message);
    ish_vprintk(message, args);
    va_end(args);
}

static void packaging_die(const char *message) {
    fprintf(stderr, "%s\n", message);
}

void (*die_handler)(const char *) = packaging_die;

_Noreturn void die(const char *message, ...) {
    char buffer[4096];
    va_list args;
    va_start(args, message);
    vsnprintf(buffer, sizeof(buffer), message, args);
    va_end(args);
    die_handler(buffer);
    abort();
}
