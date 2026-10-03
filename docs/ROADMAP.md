# Roadmap

Status reflects Linpad evidence, not inherited upstream reports.

| Milestone | Engineering result | Status / gate |
| --- | --- | --- |
| 0 | Bootstrap from ios-linuxkit with history, attribution and licensing | Working: imported; ARM64 cross-build and host packaging verified |
| 1 | Existing Alpine AArch64 CLI launches successfully | User reports app working after startup fix; independent device evidence pending |
| 2 | Real Alpine xclock/xterm visible on iPad | Experimental: viewer + guest session implemented; live iPad proof pending |
| 3 | Touch and keyboard interaction | Experimental: touch/pointer/hardware-key bridge; iPad interaction pending |
| 4 | Optional Openbox, then XFCE session | Planned; wait for interactive real GUI; keep terminal-only mode |
| 5 | Native Metal display presentation replaces temporary path | Planned; CPU Linux rendering remains distinct |
| 6 | Wayland proof of concept | Planned; FD/shared buffer semantics gate |
| 7 | True guest GPU acceleration proof of concept | Planned; guest graphics protocol and device GPU evidence gate |
| 8 | Optimized Linux desktop | Planned; measurable latency, memory and power targets |

Next gate: build `feature/phase1-x11` with Codemagic/xcodebuild, sign/install,
then run ordinary Alpine AArch64 xclock through the bundled optional Xvfb session
and confirm changing pixels on iPad. Test xeyes pointer movement and xterm hardware
keyboard next. Record the GUI report, guest logs, device and exact revision.
No desktop/GPU work precedes this live visible-window and input proof.

After Milestone 3, provide a separate `scripts/install-desktop.sh` for Openbox
first, XFCE next, LXQt later. Do not include GNOME/KDE or desktop packages in the
base rootfs. The initial X11 scripts install only optional proof packages.

All work uses feature branches and reviewable PRs to `main`; no self-merge,
force-push to main, new GitHub Actions; the manual unsigned Codemagic workflow remains supported.
