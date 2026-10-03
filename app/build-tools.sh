#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-only
# Sourced by Xcode build phases. No developer-specific directories or libraries.
if [ -n "${LINPAD_TOOLCHAIN_BIN:-}" ]; then
    if [ ! -d "$LINPAD_TOOLCHAIN_BIN" ]; then
        echo "LINPAD_TOOLCHAIN_BIN must name an existing tools directory" >&2
        exit 1
    fi
    export PATH="$LINPAD_TOOLCHAIN_BIN:$PATH"
fi

# Xcode does not always inherit the interactive shell's Homebrew PATH.
for linpad_bin in /opt/homebrew/bin /usr/local/bin; do
    if [ -d "$linpad_bin" ]; then
        export PATH="$PATH:$linpad_bin"
    fi
done
if command -v brew >/dev/null 2>&1; then
    for linpad_formula in llvm lld; do
        linpad_prefix=$(brew --prefix "$linpad_formula" 2>/dev/null) || continue
        if [ -d "$linpad_prefix/bin" ]; then
            export PATH="$PATH:$linpad_prefix/bin"
        fi
    done
fi
for linpad_tool in meson ninja python3 clang; do
    if ! command -v "$linpad_tool" >/dev/null 2>&1; then
        echo "Missing $linpad_tool. Install Meson, Ninja and Python; see docs/BUILDING.md." >&2
        exit 1
    fi
done
