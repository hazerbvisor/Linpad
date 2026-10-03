#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-only
# Headless subproof only: real Alpine xclock -> X11 -> Xvfb -> XWD.
set -eu
project=$(CDPATH= cd -- "$(dirname "$0")/.." && pwd)
[ -f /etc/alpine-release ] && [ "$(apk --print-arch)" = aarch64 ] || {
    echo 'Run inside Linpad Alpine AArch64 after scripts/install-x11-poc.sh.' >&2
    exit 1
}
for tool in Xvfb xvfb-run xauth xclock xwininfo xwd timeout; do
    command -v "$tool" >/dev/null || { echo "Missing $tool; run scripts/install-x11-poc.sh" >&2; exit 1; }
done
# mktemp gives an owned, private directory; never reuse a guest-selected socket.
output=$(mktemp -d /tmp/linpad-x11.XXXXXX)
printf 'Evidence directory: %s\n' "$output"
apk info -v xclock xvfb > "$output/packages.txt"
uname -a > "$output/context.txt"
export LIBGL_ALWAYS_SOFTWARE=1 GALLIUM_DRIVER=softpipe
# xvfb-run allocates the display, creates a temporary Xauthority cookie, and
# cleans up the X server. TCP listeners are disabled; do not add -ac.
timeout 45 xvfb-run -a -e "$output/server.log" \
    -s "-screen 0 800x600x24 -nolisten tcp -noreset -fbdir $output" \
    sh "$project/tests/linpad/x11-session.sh" "$output"
echo 'Headless X11 evidence captured. This does not prove a visible iPad window or input.'
