#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-only
# Optional packages only. Never run automatically at app startup.
set -eu
[ -f /etc/alpine-release ] && [ "$(apk --print-arch)" = aarch64 ] || {
    echo 'Linpad requires Alpine AArch64.' >&2; exit 1;
}
# Use the user's existing repositories; do not silently switch branches/upgrade.
apk add xclock xeyes xterm xvfb xvfb-run xauth xwininfo xwd xdotool font-misc-misc
printf '\nX11 proof packages installed. Open Linpad Settings > Linux GUI and copy the session command.\n'
