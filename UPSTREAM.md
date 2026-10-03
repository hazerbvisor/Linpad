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

The local bootstrap used an explicit merge of unrelated histories, preserving
upstream commits/authors as ancestors. Git smart-HTTP publishing returned HTTP
401 even though the authenticated GitHub API reported repository write access.
The review branch was therefore published through the GitHub Git Objects API
as an attributed snapshot, followed by separate Linpad commits. Its initial
import tree matches the local merge import tree exactly, but its commit parents
do not include upstream history. This transport limitation is not a claim of
original Linpad authorship. Full upstream history remains available from the
source repository; the local merge branch and `Linpad-upstream-history.bundle`
retain it for a future normal Git import. No bundle is added to the application
or tracked source tree.

The import omits the upstream Actions workflow and funding configuration. No
GitHub Actions, Codemagic configuration or publication automation was added
during bootstrap. A later explicit user request adds `codemagic.yaml` for a
manual unsigned ARM64 app build; it contains no signing credentials or publishing.
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
Only libapps needs initialization for the supported ARM64 iOS app; its
libarchive dependency now uses the separately pinned binary below. The original
libarchive submodule/project is retained for provenance and legacy source builds. The app bundles
Ghostty Web, WASM terminal support and fonts with their existing vendoring notices.
Those render terminal text, not Linux X11 windows.

## Prebuilt dependencies

The ARM64 app links Matteo Pacini's `libarchive-for-swift` **v1.0.0** release
containing libarchive **3.8.7** and bundled compression libraries. The exact URL,
archive/member SHA-256 pins and iOS 15 minimum are recorded in
`config/prebuilt-libarchive.json`. Source and unmodified dependency notices:
https://github.com/matteo-pacini/libarchive-for-swift/tree/v1.0.0 . Notices are
tracked in `app/ThirdPartyNotices.bundle` and included in the ARM64 app resources.
No downloaded binary is treated as original Linpad code.

The small guest VDSO is compiled from the imported ios-linuxkit ARM64 source
and stored in `vdso/arm64/prebuilt`, with binary/source/header hashes, compiler
and command in its manifest. This ELF is embedded as guest data; it does not
replace the native ARM64 interpreter or link a Linux library into iOS.
Source regeneration and validation details: [PREBUILT_DEPENDENCIES.md](docs/PREBUILT_DEPENDENCIES.md).

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

- Verified prebuilt iOS libarchive with bundled license notices; cached download
  and selected-slice linking replace the ARM64 app's libarchive source build.
- Validated guest VDSO reuse with source/header pins and an explicit rebuild path.
- Portable Xcode tool discovery; remove personal tool paths and signing teams.
- ARM64-only host diagnostics and a Linux cross-compilation CLI target.
- Standalone fakefsify logging to repair the packaging tool's unresolved symbols;
  build-host packaging helper independent of the execution engine.
- Linpad user-facing name, configurable bundle identifier, retained internal
  upstream names, and disabled legacy iSH publishing lanes.
- Repo-relative default benchmark/report paths.
- Optional Alpine X11 package installer, headless xclock capture and executable
  GUI syscall prerequisite probes, separate from the runtime.
- Manual Codemagic Apple Silicon workflow for unsigned ARM64 compilation,
  verified prebuilt caching, logs/result bundles and unsigned IPA packaging.
- Architecture, build, audit, validation and roadmap documentation with explicit
  working/experimental/planned/unsupported status.

No instruction execution backend was replaced. No GUI implementation or guest
GPU acceleration has been demonstrated on iPad.
