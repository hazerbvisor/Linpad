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

## Broader Apple compile/link and dispatch audit

A later user-supplied Codemagic excerpt ended with 18 socket function-pointer cast
warnings and `ninja: build stopped`. It omitted the earlier `FAILED:` command and
fatal diagnostic. These warnings alone do not identify the observed failure;
the exact failure from that run remains unconfirmed without its full log.

Branch `fix/apple-build-audit` was based on main after the ucontext fix was merged.
A source-wide ARM64 iOS audit reproduced missing `dispatch_once_t`/`dispatch_once`
declarations in `platform/darwin.c`; it now explicitly includes the public dispatch
header. Initial PATH_MAX failures in two files were traced to a missing limits.h
in the scratch reference SDK; both source files already included it correctly
and required no changes. After SDK setup was corrected, all runtime sources compiled.

The full-object Mach-O link exposed four duplicate gadget definitions (sxtw,
uxtb, uxth and rev32) in bits.S/math.S. Static archive extraction had hidden these
in the Linux CLI link. Redundant placeholder definitions were removed from bits.S;
the generator's existing math.S implementations retain their stream/operand ABI.
The build now rejects duplicate gadget symbols even when normal extraction would
hide them. SDK SQLite is explicitly linked, and the Xcode Meson bridge sets SDK
and deployment flags for both compilation and linking instead of relying on
compiler environment inference.

Both the 211 distinct ARM64 syscall handlers and all 18 legacy socketcall handlers
now have correctly typed adapters. Syscall numbers and existing full-width result/
errno normalization are preserved. The legacy dispatcher was separated into
fs/socketcall.c with the same 32-bit guest words and unsupported entries; it reads
only the selected arity and no longer evaluates uninitialized unused words or
invokes functions through an incompatible six-argument function type.

Checks passed on the x86_64 Linux development host:

- All **88** runtime C/assembly sources compiled to ARM64 iOS Mach-O objects with
  Clang 19.1.7, the non-JIT defines, generated offsets and real public iPhoneOS 16.5
  reference SDK headers. The SDK/tools remain untracked scratch data.
- All runtime objects linked with ld64.lld into an ARM64 iOS audit dylib against
  the reference SDK's system/Foundation/CoreFoundation/SQLite stubs. A second
  link included the **actual pinned iOS libarchive static binary** and newly
  compiled fakefs import/export client, plus SDK iconv/xml2/pthread. No unresolved
  symbols or duplicate definitions remained. These are audit libraries, not IPAs.
- ARM64 Linux runtime/CLI rebuild passed; the backend archive has **757 unique
  gadget definitions**. The checker also passed an archive of the Apple-target
  Mach-O backend objects and rejected the original Linux archive's duplicates.
- Native host syscall adapters passed arities 0-6, signed errors, unsigned results,
  full-width addresses/offsets and narrowing controls. The real legacy dispatcher
  passed all 18 mappings, signed argument/error checks, exact copy lengths, guest
  copy faults, and invalid/unsupported call paths. Both ran with Clang's undefined-
  behavior and function-call sanitizers; these are controls, not guest execution.
- Fifteen Python dependency/log-summary regression tests passed. A failed-build
  control retained status 65 and the earlier fatal diagnostic in build-errors.log.
- Actual bridge controls generated matching SDK/compile/link flags for device,
  simulator and macOS, including paths with spaces/apostrophes, parsed with Meson's
  machine-file parser. Codemagic's official
  YAML schema and all seven shell-step syntax checks passed.

Ninja now uses -k 0 to collect independent compiler failures in one attempt.
Codemagic keeps the complete log and an error/context summary while preserving
the failed build status. Required next evidence remains a successful full Xcode
26.6/iPhoneOS 26.5 app build on the new branch, then signed installation and Alpine
CLI launch. No app execution, GUI, new JIT requirement or GPU acceleration is claimed.

## Blank-screen startup repair

After PR #5 was merged, the user reported an installed app opening to a black
screen. The screenshot alone does not establish the guest boot state or installed
revision. Branch `fix/visible-startup` started from updated main.

A real browser load reproduced a deterministic frontend failure: term.html uses
classic script tags, but ghostty-web.js contains top-level ES exports. The script
throws `Unexpected token 'export'`; term.js then throws because window.ghosttyWeb
is undefined, and no terminal-ready message arrives. The existing classic
xterm.js Canvas page passed, so the ARM64 configuration now selects it without
new downloads or dependency compilation. Vendored code/licenses are unchanged.

Other startup repairs address independently identified launch hazards:

- App Group lookup now uses the configured identifier and the public OS container
  API. Missing/stripped entitlements fall back to the main app's own sandbox.
  The unchecked code-signature parser is removed; the Files extension requires
  shared storage and declines access when it is unavailable.
- Alpine import/boot starts after native UIKit presentation, on a worker. Main
  never waits for that worker; Roots can safely marshal collection updates to it.
  Terminal sessions wait for boot completion, including restoration. Reachability
  callbacks wait for a valid boot, avoiding an uninitialized guest task.
- Root-directory/import/missing-archive failures propagate beyond release-disabled
  assertions. Native startup status, init-exit handling, WebKit navigation/process
  errors and JavaScript/missing-script errors produce a visible report. The report
  includes the git revision embedded by scripts/build-ios.sh.

Validation completed here:

- All seven changed native implementation files compiled into ARM64 iOS Mach-O
  objects with Clang 19 and actual public iPhoneOS 16.5 reference headers, ARC,
  blocks and the xterm.js selection. Initial scratch-header acquisition failures
  were corrected (UIKit dependency count and WebKit's SDK Cryptex symlink).
- The Foundation storage fallback control compiled against iOS headers. Codemagic
  is configured to execute it on native macOS; it has not run on an Apple host here.
- Chromium loaded the real bundled xterm.js assets over local HTTP, emitted the
  ready message with nonzero dimensions and drew control text into Canvas pixels.
  Injected JavaScript errors and a missing vendor script reached the native-error
  channel. The retained optional browser test checks these paths; no Linux guest
  was involved. Local file navigation was blocked by the development browser's
  policy, so this is not a WKWebView file-origin check.

A complete current Xcode app build, signed installation and on-iPad Alpine shell
still require a rerun. A JavaScript terminal renderer is not the Linux X11 xterm
application, an X11 bridge, Metal presentation or application GPU acceleration.
