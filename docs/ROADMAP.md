# Roadmap

Status reflects Linpad evidence, not inherited upstream reports.

| Milestone | Engineering result | Status / gate |
| --- | --- | --- |
| 0 | Bootstrap from ios-linuxkit with history, attribution and licensing | Working: imported; ARM64 Linux build and host packaging verified; Apple build pending |
| 1 | Existing Alpine AArch64 CLI launches successfully | Experimental foundation; no Linpad runtime execution verified in this session |
| 2 | Real Alpine xclock/xterm visible on iPad | Planned; optional headless scripts only |
| 3 | Touch and keyboard interaction | Planned; GUI input bridge absent |
| 4 | Optional Openbox, then XFCE session | Planned; wait for interactive real GUI; keep terminal-only mode |
| 5 | Native Metal display presentation replaces temporary path | Planned; CPU Linux rendering remains distinct |
| 6 | Wayland proof of concept | Planned; FD/shared buffer semantics gate |
| 7 | True guest GPU acceleration proof of concept | Planned; guest graphics protocol and device GPU evidence gate |
| 8 | Optimized Linux desktop | Planned; measurable latency, memory and power targets |

Next gate: unsigned `iSH-ARM64` xcodebuild on Apple Silicon, signed installation,
Alpine BusyBox/apk CLI smoke on an M-series iPad, then GUI prerequisite probes and
real Xvfb/xclock execution there. No desktop/GPU work precedes those gates.

After Milestone 3, provide a separate `scripts/install-desktop.sh` for Openbox
first, XFCE next, LXQt later. Do not include GNOME/KDE or desktop packages in the
base rootfs. The initial X11 scripts install only optional proof packages.

All work uses feature branches and reviewable PRs to `main`; no self-merge,
force-push to main, new Actions or Codemagic release pipeline in this bootstrap.
