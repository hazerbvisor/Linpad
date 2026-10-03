# Bootstrap validation record

Session: 2026-10-03. Host: Debian 13 Linux **x86_64**, no Xcode/Apple SDK,
no attached AArch64 execution host and no iPad. Foundation:
`c1a4064e84f1a957280a62f77d107162032dc908`.

## What passed

- Linpad original `LICENSE` unchanged; local import merge retains upstream
  revision as an ancestor and all three dependency gitlinks. Git publishing
  returned HTTP 401; the API-published review branch preserves the exact import
  tree and separate changes, with full history retained locally/in a Git bundle.
  Its remote import commit has only Linpad's main commit as parent.
- Imported baseline `libish.a`, `libish_emu.a`, `libfakefs.a` and real AArch64
  ELF VDSO compiled with Clang 19.1.7/LLD 19.1.7, Meson 1.7.0/Ninja 1.12.1.
  The Linux CLI was first manually linked because upstream excludes executables
  in cross builds; the Linpad change enables that target for Linux cross builds.
- Full Linpad cross-build subsequently compiled and linked the ARM64 Linux `ish`
  CLI with JIT/emission disabled. ELF identity: ARM aarch64, dynamic GNU/Linux
  interpreter. This is not an iPadOS application build.
- Standalone build-host `fakefsify` and `unfakefsify` compiled and ran.
  Real Alpine 3.24.2 AArch64 rootfs imported and exported. BusyBox bytes,
  Alpine release, repository contents, mode/uid/gid and `/bin/sh` symlink
  matched the input archive in roundtrip checks.
- Rootfs downloader regression passed: good archive, failed fetch, wrong hash,
  wrong architecture, missing/malformed pin, existing archive preserved.
- Actual Alpine rootfs SHA-256 matched:
  `9bf70a7f18ea44094cbb5f70c58f9af129c8214745743db0e68e5502cc2ce773`.
- New GUI prerequisite C probe compiled with warnings-as-errors and passed 10
  groups **natively on the x86_64 host**. A static AArch64 Linux probe also
  compiled; it was not executed. Probe results establish only control behavior.
- New X11 session script ran a real **Debian host xclock**, queried its mapped
  window, and captured root XWD/Xvfb software framebuffer, using xauth and no TCP
  listener. This validates script mechanics, not Linpad or Alpine compatibility.
- Downloaded Alpine community AArch64 `xclock-1.1.1-r0.apk`, extracted the actual
  program and checked ARM64 ELF / `/lib/ld-musl-aarch64.so.1` identity.
  Archive SHA-256:
  `4f51408869711c2263d2082e34dd486acddc2de6ce0d08430734cb29cee0c15f`.

## Failures and their resolution / remaining limits

1. System package installation lacked root permission. Development tools were
   downloaded through normal HTTPS/APT and extracted into scratch directories;
   repository build scripts do not depend on that session-specific path.
2. Cross-link discovery initially lacked a complete sysroot. Correcting the
   local sysroot/toolchain configuration resolved it; this was a host setup issue.
3. `fakefsify` exposed unresolved `die`/`ish_printk` references from upstream
   database code. Added `tools/packaging-log.c` only to the packaging tool,
   leaving runtime logging unchanged; compilation and rootfs roundtrip passed.
4. Executing the AArch64 CLI on x86_64 returned **Exec format error**. This is
   expected architecture incompatibility, not an Alpine runtime result. No
   emulator/VM workaround or x86 Linux execution dependency was introduced.
5. iOS build helper correctly rejects this non-Apple host. Apple compilation,
   linking, signing, installation, CLI launch and sandbox networking remain
   unverified. Removing known personal paths/Team IDs is not proof they work.
6. GUI guest runners correctly reject this non-Alpine/non-AArch64 host. Alpine
   xclock/xeyes/xterm have not run under Linpad; no native framebuffer reader,
   GUI input bridge, Metal presenter or GPU interface was implemented.

Warnings in inherited runtime code (including syscall function pointer casts)
remain; they did not prevent the tested ARM64 build. Imported upstream release
2.4.1 also explicitly lacks Apple archive/device validation.

## Next acceptance record

Build on an Apple Silicon Mac and collect the xcodebuild log for `iSH-ARM64`,
then sign/install on an M-series iPad and prove `/bin/echo`, `/bin/sh`, `apk` and
filesystem persistence in the exact app. Run GUI probes in that app, then real
Alpine Xvfb/xclock. A mapped window/XWD on the host is insufficient: Milestone 2
requires a live visible window on the iPad, and Milestone 3 requires verified
GUI touch/keyboard interaction. Do not mark either complete until that happens.

## Prebuilt dependency follow-up

Same Linux host/date; branch `feature/prebuilt-dependencies`, based on merged
bootstrap main. No new Apple/device runtime claim.

- Downloaded and verified the actual `libarchive-for-swift` v1.0.0 XCFramework
  archive (SHA-256 in `config/prebuilt-libarchive.json`). Inspected the ARM64
  device Mach-O static archive and iOS 15.0 minimum. Found all 50 libarchive APIs
  used by Linpad's fakefs import/export implementation; that source passed a
  host syntax check against the replacement headers. Preserved the tagged
  wrapper/compression/libarchive license notices in application resources.
- Verified real archive extraction and offline reuse with per-member hashes.
  Eleven regression tests passed, including artifact failure paths and stale or
  invalid guest VDSO rejection.
- Fresh default/prebuilt ARM64 Linux cross-build compiled `libish.a`,
  `libish_emu.a`, `libfakefs.a` and linked the AArch64 Linux CLI. Its Ninja VDSO
  rule validates/copies the payload with Python; it does not invoke an ELF compiler.
- Independent `-Dvdso_mode=source` compilation produced byte-identical guest
  VDSO content with Clang/LLD 19.1.7. Source rebuild support is retained.
- Xcode project syntax and ARM64 target references were checked. A failing
  preparation command was exercised through the actual parsed shell phase and
  halted before native compilation. Standard build
  dependencies no longer require LLVM/LLD or the libarchive source submodule.
  Apple SDK linking, signing, runtime launch and rootfs import/export with the
  replacement libarchive are still unverified. Test with current Xcode on an
  Apple Silicon builder and an iPad running iPadOS 15 or later.


## Reported Codemagic compile failure and signal-context fix

The user supplied the first Apple build failure from Codemagic on Xcode 26.6 /
iPhoneOS SDK 26.5. Compilation stopped in `platform/native_fault.c` because
`platform/host_context_aarch64.h` included `<ucontext.h>` without `_XOPEN_SOURCE`.
That Apple umbrella header gates deprecated getcontext/makecontext/setcontext/
swapcontext routines; Linpad only needs the signal-context type definitions.
The excerpt also reported the preparatory target's inherited iOS 11 minimum as
outside this SDK's supported range. No successful app link/IPA/runtime was reported.

Branch `fix/apple-ucontext-build` selects public `<sys/ucontext.h>` on Apple and
retains `<ucontext.h>` on Linux. It introduces no feature-macro changes or
context-switch calls. The shared project minimum is now iOS 15, matching the
existing ARM64 app/prebuilt libarchive minimum, including preparatory/extension
targets. Codemagic now compiles/runs a register/PC/SP/ESR helper check on ARM64
macOS, then syntax-checks it against the chosen iPhoneOS SDK before building the app.

Local checks on the x86_64 Linux development host:

- The old header reproduced the identical `_XOPEN_SOURCE` error using Clang 19.1.7
  targeting ARM64 iOS 15 with real public iPhoneOS 16.5 SDK headers; the changed
  header/helper check passed with warnings-as-errors and no global feature macro.
- The actual previously failing `platform/native_fault.c` compiled to an ARM64
  iOS Mach-O object with those headers and existing generated ARM64 offsets.
  This is a targeted Linux cross-compile, not an Apple Xcode 26.6 app build/link.
- ARM64 Linux helper syntax check and runtime/CLI rebuild passed, retaining the
  Linux context layout and the non-JIT configuration.
- Updated Codemagic YAML passed the official schema and all six shell-step syntax
  checks. SDK reference headers/tools remain in scratch; none are vendored in Linpad.

Required next evidence: rerun `linpad-ios-unsigned` on the fix branch with Xcode
26.6/SDK 26.5. The new native helper test has not run on an Apple host here;
a successful full app link and on-iPad Alpine CLI launch remain unverified.
