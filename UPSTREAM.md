# Provenance

Linpad's initial repository contained only `LICENSE` at
`6e80dd259b71ea502642959be70cd60089861e97`.
That original GPL-3.0 file is preserved byte-for-byte.

## Runtime foundation

- Original projects: iSH, and rcarmo's ARM64 ios-linuxkit derivative.
- Primary upstream: https://github.com/rcarmo/ios-linuxkit
- Branch inspected: `master`, on 2026-10-03.
- Imported revision: **c1a4064e84f1a957280a62f77d107162032dc908**.
- Source version: ios-linuxkit 2.4.1, build 818.
- Earlier iSH origin: https://github.com/ish-app/ish

The bootstrap is an explicit merge of unrelated histories on
`feature/bootstrap-ios-linuxkit`. Upstream commits and authors remain ancestors
of the import commit; upstream source is not represented as original Linpad work.
The import omits the upstream Actions workflow and funding configuration. No
GitHub Actions, Codemagic configuration or publication automation was added.
`README.upstream.md` and `docs/ARCHITECTURE.upstream.md` preserve the imported guides.
Other upstream documentation and dated reports remain attributed historical evidence.

## Dependency pins

| Path | Upstream URL | Imported gitlink |
| --- | --- | --- |
| deps/libapps | https://github.com/ish-app/libapps | b8cacae35e5b11d64bb736a053921c16ca7faf9e |
| deps/libarchive | https://github.com/libarchive/libarchive | fc6563f5130d8a7ee1fc27c0e55baef35119f26c |
| deps/linux | https://github.com/ish-app/linux | 8ec9bf17f89c6dba818f3ed2427de4223e78644a |

`deps/linux` is retained for provenance with upstream's `update = none`; the
supported Linpad build uses `kernel=ish` and does not build or boot this kernel.
Only libapps and libarchive are needed for the iOS terminal app. The app bundles
Ghostty Web, WASM terminal support and fonts with their existing vendoring notices.
Those render terminal text, not Linux X11 windows.

## Licensing

Linpad remains GPL-3.0. `LICENSE.md`, `LICENSE.IOS`, authorship headers, legacy
notices and vendored license files are preserved. The upstream GPLv2 dual-license
statement and iOS distribution notice describe their respective copyright holders;
Linpad does not claim to grant additional permissions for someone else's code.
Submodules retain their own licenses, including libarchive's BSD terms and the
historical Linux tree's terms. Audit all bundled dependency/package notices when
preparing a distributable IPA or rootfs; package installation does not relicense
third-party applications.

## Secondary reference — no code imported

https://github.com/MaxLeiter/jailbreak was inspected at
`4c762e6942245fccb583fc6c150ec0c9cbadc568` on 2026-10-03. Its root license is MIT
(Copyright 2026 Max Leiter); third-party projects have separate terms. Xios uses
Wayland/Xwayland, ANGLE, IOSurface/Metal presentation, and UIKit input, with
applications recompiled as iOS Mach-O binaries. Its jailbreak deployment,
`/var/jb`, task-port surface import, launch daemons and special entitlements are
not suitable dependencies for a normal sideloaded Linpad app. No Xios code,
binaries, private APIs, entitlements or package recipes were copied.

## Linpad changes after the import

- Portable Xcode tool discovery; remove personal tool paths and signing teams.
- ARM64-only host diagnostics and a Linux cross-compilation CLI target.
- Standalone fakefsify logging to repair the packaging tool's unresolved symbols;
  build-host packaging helper independent of the execution engine.
- Linpad user-facing name, configurable bundle identifier, retained internal
  upstream names, and disabled legacy iSH publishing lanes.
- Repo-relative default benchmark/report paths.
- Optional Alpine X11 package installer, headless xclock capture and executable
  GUI syscall prerequisite probes, separate from the runtime.
- Architecture, build, audit, validation and roadmap documentation with explicit
  working/experimental/planned/unsupported status.

No instruction execution backend was replaced. No GUI implementation or guest
GPU acceleration has been demonstrated on iPad.
