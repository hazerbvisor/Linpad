#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-only
set -eu
output=$1
xclock -name linpad-x11-probe -digital -update 1 > "$output/client.log" 2>&1 &
client=$!
trap 'kill "$client" 2>/dev/null || true; wait "$client" 2>/dev/null || true' EXIT
# Wait for a mapped real client window, with bounded readiness retries.
ready=0
for attempt in 1 2 3 4 5 6 7 8 9 10; do
    kill -0 "$client" || { cat "$output/client.log" >&2; exit 1; }
    if xwininfo -name linpad-x11-probe > "$output/window.txt" 2>/dev/null &&
       grep -q 'Map State: IsViewable' "$output/window.txt"; then
        ready=1
        break
    fi
    sleep 1
done
[ "$ready" = 1 ] || { echo 'No mapped xclock window observed' >&2; exit 1; }
sleep 2
xwininfo -root -tree > "$output/tree.txt"
xwd -root -silent -out "$output/root.xwd"
test -s "$output/root.xwd"
test -s "$output/Xvfb_screen0"
printf 'Mapped xclock; X11 window and software framebuffer captured.\n'
