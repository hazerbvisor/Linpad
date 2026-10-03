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

For the prebuilt dependency changes before their PR is merged, use
`feature/prebuilt-dependencies`. Pins and licenses are in [UPSTREAM.md](../UPSTREAM.md). `deps/linux` is retained but intentionally
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
no upstream account or certificate repository configured. Use local xcodebuild;
Codemagic configuration and release automation are deferred. Future caches can
retain submodules, `build-data/prebuilt` (or the configured prebuilt cache),
verified rootfs downloads and SDK/configuration-specific
DerivedData; never cache signing credentials in this repository.

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
