# Building Linpad

**Working:** ARM64 Linux cross-build and build-host fakefs packaging.
**Unverified:** Apple compilation, signing, installation and app runtime.
No IPA or iPad GUI was produced in the bootstrap session.

## Checkout and dependencies

```sh
git clone https://github.com/hazerbvisor/Linpad.git
cd Linpad
git switch main
git submodule update --init deps/libapps
```

Pins and licenses are in [UPSTREAM.md](../UPSTREAM.md). `deps/linux` is retained but intentionally
not initialized/built for this userspace application. Never select inherited
Linux-kernel build configurations. Use the `iSH-ARM64` app scheme and `kernel=ish`.

## Apple Silicon Mac / unsigned iPadOS build

Install a recent full Xcode, select its developer directory using Apple's normal
`xcode-select` workflow, and install portable command-line dependencies:

```sh
brew install meson ninja python
sh scripts/build-ios.sh
```

Equivalent build command, separating compilation from signing:

```sh
xcodebuild -project iSH.xcodeproj -scheme iSH-ARM64 \
  -configuration Release -destination 'generic/platform=iOS' \
  -derivedDataPath build-ios ARCHS=arm64 ONLY_ACTIVE_ARCH=YES \
  CODE_SIGNING_ALLOWED=NO build
```

The app product is `Linpad.app`; internal project/scheme names are retained for
upstream maintainability. The bootstrap does not claim this command has passed
on macOS. On Intel Macs, use an Apple Silicon builder: the execution engine has
only AArch64 host gadgets. A simulator, if tested later, must also be ARM64.

`app/build-tools.sh` discovers installed Homebrew tools without developer-specific
paths. For a custom installation, export `LINPAD_TOOLCHAIN_BIN` with the directory
containing tools before building. Apple clang/ar still compile the native application
and runtime libraries via `xcrun`. The ARM64 app uses a verified prebuilt libarchive
and a checked-in guest VDSO, so LLVM/LLD are no longer required for the standard
iOS build. This configuration targets iPadOS 15 or later. See
[prebuilt dependencies](PREBUILT_DEPENDENCIES.md) for pins, licenses and source rebuilds.
The first build downloads a roughly 72 MB libarchive ZIP and verifies SHA-256;
subsequent builds verify the extracted cache without downloading. Set
`LINPAD_PREBUILT_DIR=/path/to/cache` as an xcodebuild setting to relocate that cache.
Only the selected device/simulator static library is linked; the ZIP is not bundled.
Meson/Ninja state stays in Xcode's configuration build directory. Do not reuse
Meson directories across different SDKs/architectures or native/AOT experiments.
The standard app forcibly disables JIT/emission and clears AOT image options.

### Signing is explicit

Default bundle identity: `org.linpad.Linpad`; no development Team ID is configured.
Use your own identifier and Apple development team when you choose to install:

```sh
sh scripts/build-ios.sh \
  CODE_SIGNING_ALLOWED=YES \
  ROOT_BUNDLE_IDENTIFIER=com.example.Linpad \
  DEVELOPMENT_TEAM=YOUR_TEAM_ID
```

App-group and extension identities derive from the root identifier. Configure
matching app/extension provisioning and app-group capabilities in your Apple
account. An unsigned build is not installable on an ordinary iPad; installation
needs a valid normal sideload signature/provisioning. Archive/export an IPA with
your own export options after build/device checks. Do not copy upstream developer
identities or disable iOS security. No new entitlements were added here.

Legacy Fastlane publishing is deliberately blocked before any lane executes.
Its historical lane bodies remain for attribution; its Appfile/Matchfile have
no upstream account or certificate repository configured. Use local xcodebuild
or the Codemagic workflow below. Store signing credentials outside this repository.

## Codemagic unsigned build

[The root codemagic.yaml](../codemagic.yaml) defines `linpad-ios-unsigned`, a manual
workflow on an Apple Silicon M2 macOS builder with `xcode: latest`. It calls the
same `scripts/build-ios.sh` used locally. **The first reported Codemagic app build
failed at Apple's deprecated `ucontext.h` feature-macro gate.** After that fix,
a later excerpt reported another Ninja failure but omitted its failing command.
The broader audit now repairs a missing Darwin dispatch header, conflicting
gadget definitions and incompatible socket/syscall dispatch casts. Full Apple
app compilation and IPA packaging still need a successful rerun. See [validation evidence](BOOTSTRAP_VALIDATION.md).
The YAML passed Codemagic's official JSON schema, all shell steps passed syntax
checks, and a failing build control retained its error status/log through `tee`.

1. Connect `hazerbvisor/Linpad` in Codemagic and use its repository YAML configuration.
2. Select `fix/visible-startup` to test the startup/display fixes before their PR is
   merged; afterward select `main`.
3. Start workflow `linpad-ios-unsigned` manually. No automatic triggers are configured.
4. Download `Linpad-unsigned.ipa`, `xcodebuild.log`, `build-errors.log`, the toolchain/dependency logs
   and `Linpad-build.xcresult` from build artifacts. Logs and any generated result
   bundle are collected on failure too; the IPA is produced only after a successful build.

The workflow installs Meson, Ninja and Python, initializes only the pinned
`deps/libapps` submodule, verifies/downloads the pinned libarchive, runs dependency
and build-log regression checks, then builds the `iSH-ARM64` Release scheme for a generic
iOS ARM64 device. The Xcode phases verify the guest VDSO and download/verify Alpine
AArch64. They do not compile a Linux kernel or guest applications.

Before the full app build, an ARM64 signal-context check compiles/runs natively on
macOS and syntax-checks against the selected iPhoneOS SDK. It checks register,
PC/SP and exception-status helpers without installing handlers or enabling JIT.
A separate native control checks typed syscall argument/return widths and all
18 legacy socket mappings, including guest-copy faults, with undefined-behavior
sanitization. These controls are not Alpine execution results. After Ninja,
compiled runtime gadget symbols are checked for duplicate definitions.

Meson now receives explicit SDK and deployment flags for both compile and link
steps. The app explicitly links Apple's SDK SQLite library used by fakefs;
link dependencies discovered by Meson are not automatically passed to Xcode.
Ninja uses `-k 0` to collect all independent failures in one attempt. The build
step preserves its error status and creates `build-errors.log` with earlier
compiler/linker errors and context, even when later warnings fill the log tail.
The shared project minimum is iOS 15.0, matching the app and libarchive dependency,
including preparatory and extension targets.

`build-data/prebuilt` is cached and revalidated each run. DerivedData and Meson
build directories are fresh for each run, preventing SDK/toolchain cache mixing;
the rootfs downloader currently downloads and validates the small minirootfs on
each app build. No signing secrets, provisioning profiles or credentials are needed
for this workflow. `LINPAD_BUNDLE_ID` is configurable in YAML and defaults to
`org.linpad.Linpad`; no developer Team ID is configured.

The packaged IPA contains the real compiled `.app` and its embedded extension in
`Payload/`; its executable architecture and bundle identifier are checked before
packaging. **It is unsigned and cannot be installed directly on an ordinary iPad.**
Use a normal sideload signing process with your own identity and matching app-group
and extension provisioning. Creating a ZIP/IPA does not sign it or prove Alpine boots.
A signed Codemagic export workflow needs your own signing configuration separately.

`latest` lets the first build use Codemagic's current stable Xcode, appropriate for
the binary dependency's recent SDK metadata. The toolchain log records the selected
Xcode/SDK; after a successful build, pin that verified version in YAML for repeatability.
Codemagic may require selecting an available compatible Xcode image in its UI.
No App Store publishing, notifications or GitHub Actions are configured.

## AArch64 Linux development host

```sh
# Debian/Ubuntu on a real ARM64 host:
sudo apt install clang make meson ninja-build pkg-config \
  libsqlite3-dev libarchive-dev git curl file tar
CC=clang make build-arm64-linux MESON_SETUP_ARGS='-Djit=false -Djit_emit=false'
```

The default build reuses the same validated guest VDSO. LLVM/LLD are required
only when rebuilding it from source; Linux libarchive uses the distribution
package rather than the Apple binary. The Linux CLI runs as an ordinary AArch64
host process, not a VM. Create the exact
pinned rootfs and import fakefs:

```sh
mkdir -p build-data
curl -fL --retry 2 \
  https://dl-cdn.alpinelinux.org/alpine/v3.24/releases/aarch64/alpine-minirootfs-3.24.2-aarch64.tar.gz \
  -o build-data/alpine-minirootfs-3.24.2-aarch64.tar.gz
printf '%s  %s\n' \
  9bf70a7f18ea44094cbb5f70c58f9af129c8214745743db0e68e5502cc2ce773 \
  build-data/alpine-minirootfs-3.24.2-aarch64.tar.gz | sha256sum -c -
./build-arm64-linux/tools/fakefsify \
  build-data/alpine-minirootfs-3.24.2-aarch64.tar.gz alpine-arm64-fakefs
./build-arm64-linux/ish -f alpine-arm64-fakefs /bin/echo Linpad
./build-arm64-linux/ish -f alpine-arm64-fakefs /bin/sh
```

The iOS bundle uses the same URL/hash in `app/GuestARM64.xcconfig` and
`app/download-root.sh`, which verifies SHA-256 and the BusyBox ELF architecture
before atomically installing the rootfs. Existing user rootfs is not silently
replaced by a pin change. No desktop packages are added to the base archive.

## Compile on x86_64 without running the runtime

The runtime cannot execute here. Cross compilation can catch build/link errors:

```sh
# Debian/Ubuntu; configure ARM64 package sources for your distribution first.
sudo dpkg --add-architecture arm64
sudo apt update
sudo apt install clang lld meson ninja-build pkg-config \
  gcc-aarch64-linux-gnu libsqlite3-dev:arm64
meson setup build-arm64-cross --cross-file config/aarch64-linux.ini \
  --buildtype=release -Dguest_arch=arm64 -Djit=false -Djit_emit=false
ninja -C build-arm64-cross
file build-arm64-cross/ish
```

`config/aarch64-linux.ini` is a conventional Debian/Ubuntu example. Adapt the
cross-file/sysroot for other toolchains; all paths are outside user home folders.
libarchive is optional for this cross-build; use the build-host packaging helper
below. A cross-compiled Linux binary is not an iOS binary and does not establish
runtime compatibility. No QEMU or x86 guest fallback is used to execute it.

### Build-host rootfs packaging without ARM execution

On Linux, install host `libsqlite3-dev`, `libarchive-dev`, pkg-config and a C
compiler. On macOS, install libarchive and SQLite and configure pkg-config paths
if needed. Then:

```sh
sh scripts/build-packaging-tools.sh
mkdir -p build-data
./build-packaging/fakefsify \
  build-data/alpine-minirootfs-3.24.2-aarch64.tar.gz build-data/alpine-fakefs
./build-packaging/unfakefsify build-data/alpine-fakefs build-data/alpine-export.tar.gz
sh tests/arm64/rootfs/download-root.sh
```

The helper compiles only filesystem packaging code, with standalone logging;
it can run on the build machine regardless of guest architecture. It neither
boots Alpine nor simulates ARM64 instructions.

## GUI prerequisites and evidence

Only after the app's CLI baseline works, copy the checkout's optional scripts and
`tests/linpad` into a guest-visible directory (normal sandbox document import or a
checked app-owned bind mount), then follow [GUI_COMPATIBILITY.md](GUI_COMPATIBILITY.md).
The checked-in scripts are not injected into every user's rootfs. A future
integration can provide an explicit guest installer from app resources after
its build and package behavior is verified.

## Startup and sideloading

The ARM64 app now selects the already bundled **xterm.js Canvas terminal frontend**.
This displays guest terminal bytes; it is separate from the Linux X11 `xterm`
application and does not meet the GUI milestone. The imported default Ghostty
artifact is an ES module loaded as a classic script, which failed before terminal
initialization in a browser control.

Alpine import and boot begin on a worker after UIKit presents the native Linpad
screen. Session creation waits for boot completion. The startup screen shows the
current stage and offers **Copy startup report** if loading stalls or fails. Paste
that report when reporting a launch failure; it includes the build revision, boot
stage/error, executable paths, frontend error and storage mode. It does not upload
anything automatically. A working terminal hides the startup screen and stops its
poll timer; WebKit errors remain observable.

A provisioned App Group is optional for the main app. With a granted group,
Linpad uses its shared container; otherwise it stores roots in its own sandbox's
Application Support/Linpad directory. The Files extension requires the shared
group and returns an authentication error when it is unavailable. Re-signing does
not require parsing the Mach-O code signature or adding sandbox permissions.
`PRODUCT_APP_GROUP_IDENTIFIER` remains configurable; neither it nor the bundle
identifier grants an entitlement by itself.

Codemagic compiles and runs a native Foundation storage fallback control. Local
optional browser checks use Playwright (HTTP asset loading on Chromium, not
WKWebView/device validation):

```sh
python3 -m pip install playwright
python3 -m playwright install chromium
python3 tests/linpad/terminal-frontend.py
```

This checks the real bundled classic frontend's ready message, Canvas pixels,
nonzero dimensions and JavaScript/missing-resource error reporting. It feeds
control text, not a Linux process. Signed on-iPad Alpine execution is still the
next required check.
