#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-only
# Run manually INSIDE a disposable Linpad Alpine rootfs. Base rootfs stays small.
set -eu
[ -f /etc/alpine-release ] || { echo 'Requires Alpine Linux' >&2; exit 1; }
[ "$(apk --print-arch)" = aarch64 ] || { echo 'Requires Alpine AArch64' >&2; exit 1; }
# Do not change repository branches or upgrade the user's rootfs implicitly.
apk add xclock xvfb xvfb-run xauth xwininfo xwd font-misc-misc
echo 'Installed optional X11 software-rendering proof packages. No iPad display bridge is installed.'
