#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-only
# Real X11 clients/server. No native recreation; no Metal/GPU acceleration.
set -eu
inside=0
if [ "${1:-}" = --inside ]; then inside=1; shift; fi
id=${1:-}
app=${2:-xclock}
case "$id" in ''|*[!0-9a-f]*) echo 'Invalid session identifier' >&2; exit 1;; esac
[ "${#id}" = 32 ] || { echo 'Session identifier must be 32 hex digits' >&2; exit 1; }
case "$app" in xclock|xeyes|xterm) ;; *) echo 'Unsupported X11 proof application' >&2; exit 1;; esac
dir=/tmp/linpad-x11-$id
if [ "$inside" = 0 ]; then
    [ -f /etc/alpine-release ] && [ "$(apk --print-arch)" = aarch64 ] || {
        echo 'Run inside Linpad Alpine AArch64.' >&2; exit 1;
    }
    for tool in Xvfb xvfb-run xauth xwininfo xwd xdotool "$app"; do
        command -v "$tool" >/dev/null || { echo "Missing $tool. Run sh /usr/share/linpad/install-x11.sh" >&2; exit 1; }
    done
    umask 077
    mkdir "$dir" || { echo 'Session directory already exists. Reopen the GUI viewer for a new session.' >&2; exit 1; }
    : > "$dir/input"
    apk info -v "$app" xvfb xdotool > "$dir/packages.txt"
    uname -a > "$dir/context.txt"
    export LIBGL_ALWAYS_SOFTWARE=1 GALLIUM_DRIVER=softpipe
    # Explicit xwd snapshots avoid dependence on guest MAP_SHARED/host coherence.
    # xvfb-run manages Xauthority and kills its own X server when this exits.
    exec xvfb-run -a -e "$dir/server.log" \
        -s '-screen 0 800x600x24 -nolisten tcp -noreset -extension MIT-SHM -extension GLX' \
        sh "$0" --inside "$id" "$app"
fi
[ -d "$dir" ] && [ -f "$dir/input" ] || { echo 'Missing session directory/input' >&2; exit 1; }
export LC_ALL=C
case "$app" in
    xclock) xclock -name linpad-x11-app -geometry 280x200+40+40 -update 1 > "$dir/client.log" 2>&1 & ;;
    xeyes) xeyes -name linpad-x11-app -geometry 280x200+40+40 > "$dir/client.log" 2>&1 & ;;
    xterm) xterm -name linpad-x11-app -title linpad-x11-app -geometry 80x24+20+20 > "$dir/client.log" 2>&1 & ;;
esac
client=$!
held=
release_held() {
    for key in $held; do xdotool keyup "$key" 2>/dev/null || true; done
    held=
    xdotool keyup Shift_L Shift_R Control_L Control_R Alt_L Alt_R Super_L Super_R 2>/dev/null || true
}
cleanup() {
    release_held
    xdotool keyup Shift_L Shift_R Control_L Control_R Alt_L Alt_R Super_L Super_R 2>/dev/null || true
    kill "$client" 2>/dev/null || true
    wait "$client" 2>/dev/null || true
    rm -f "$dir/frame.pending" "$dir/events.batch"
}
trap cleanup EXIT
trap 'exit 0' INT TERM HUP
ready=0
for attempt in 1 2 3 4 5 6 7 8 9 10; do
    kill -0 "$client" || { cat "$dir/client.log" >&2; exit 1; }
    # Shells can change xterm WM_NAME. Identify the stable Xt instance instead.
    window=$(xdotool search --classname '^linpad-x11-app$' 2>/dev/null | head -n 1)
    if [ -n "$window" ] && xwininfo -id "$window" > "$dir/window.txt" 2>/dev/null &&
        grep -q 'Map State: IsViewable' "$dir/window.txt"; then ready=1; break; fi
    sleep 1
done
[ "$ready" = 1 ] || { echo 'No mapped X11 window' >> "$dir/client.log"; exit 1; }
[ -n "$window" ] && xdotool windowfocus "$window" 2>/dev/null || true
printf 'Real %s mapped on DISPLAY=%s. Close the native viewer or Ctrl-C to stop.\n' "$app" "$DISPLAY"
next=1
number() {
    case "$1" in ''|*[!0-9]*) return 1;; esac
    [ "${#1}" -le 4 ] && [ "$1" -le "$2" ]
}
while kill -0 "$client" 2>/dev/null; do
    xwd -root -silent -out "$dir/frame.pending" 2>> "$dir/client.log"
    mv -f "$dir/frame.pending" "$dir/frame.xwd"
    # The host app appends complete short records. Retain partial records for
    # the next pass; never truncate the mailbox or execute shell-provided code.
    tail -n +"$next" "$dir/input" > "$dir/events.batch"
    while IFS=' ' read -r action a b c extra; do
        next=$((next+1))
        [ -z "$extra" ] || continue
        case "$action" in
            stop) if [ -z "$a$b$c" ]; then exit 0; fi ;;
            release) if [ -z "$a$b$c" ]; then release_held; fi ;;
            move|click)
                number "$a" 799 && number "$b" 599 || continue
                if [ "$action" = move ]; then
                    [ -z "$c" ] && xdotool mousemove "$a" "$b" || true
                else
                    number "$c" 3 && [ "$c" -ge 1 ] || continue
                    xdotool mousemove "$a" "$b" click "$c" || true
                fi ;;
            down|up)
                [ -z "$b$c" ] && [ -n "$a" ] && [ "${#a}" -le 24 ] || continue
                case "$a" in *[!A-Za-z0-9_]*) continue;; esac
                if [ "$action" = down ]; then
                    case " $held " in *" $a "*) ;; *) held="$held $a";; esac
                    xdotool keydown "$a" || true
                else
                    remaining=
                    for key in $held; do [ "$key" = "$a" ] || remaining="$remaining $key"; done
                    held=$remaining
                    xdotool keyup "$a" || true
                fi ;;
        esac
    done < "$dir/events.batch"
    sleep 0.5
done
