#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-only
# Run inside Linpad; install build-base explicitly first if needed.
set -eu
project=$(CDPATH= cd -- "$(dirname "$0")/.." && pwd)
[ -f /etc/alpine-release ] && [ "$(apk --print-arch)" = aarch64 ] || {
    echo 'Run this probe inside Alpine AArch64; native host results are controls only.' >&2
    exit 1
}
command -v cc >/dev/null || { echo 'Install optional probe tools: apk add build-base' >&2; exit 1; }
tmp=$(mktemp -d /tmp/linpad-gui-probe.XXXXXX)
trap 'rm -rf "$tmp"' EXIT
cc -O2 -Wall -Wextra -pthread "$project/tests/linpad/gui-probe.c" -o "$tmp/gui-probe"
printf 'Alpine: '; cat /etc/alpine-release
uname -a
timeout 40 "$tmp/gui-probe"
