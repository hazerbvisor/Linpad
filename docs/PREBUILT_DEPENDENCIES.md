# Prebuilt dependency reuse

**Implemented, Apple linking unverified:** the ARM64 Xcode app selects a pinned
prebuilt iOS libarchive and a checked-in guest VDSO. The native app, ARM64
interpreter, syscall layer and filesystem code still compile with Apple tooling.
There is no verified reusable ios-linuxkit ARM64 runtime binary being substituted.

| Component | Build behavior | Evidence / limits |
| --- | --- | --- |
| iOS libarchive | Download/cache v1.0.0 binary package, link selected static slice | Actual archive/member hashes and ARM64 Mach-O inspected; Apple link and rootfs import with this binary pending |
| Guest AArch64 VDSO | Validate and reuse 3,080-byte checked-in ELF | Fresh ARM64 Linux cross-build passed; source rebuild produced identical bytes |
| Alpine AArch64 rootfs and GUI packages | Use Alpine's existing minirootfs/APK binaries | No guest package compilation required; app/runtime compatibility still needs device tests |
| Ghostty terminal JS/WASM/fonts | Retain upstream vendored assets | Already built; these display terminal text |
| UIKit/Metal/system libraries | Use the Apple SDK | Supplied by Apple; no third-party replacement |
| Linpad native app/runtime | Compile locally or on an Apple Silicon macOS builder | No IPA or device execution validated in this Linux session |

## libarchive artifact and attribution

Distributor/source: https://github.com/matteo-pacini/libarchive-for-swift/tree/v1.0.0

Release artifact:
https://github.com/matteo-pacini/libarchive-for-swift/releases/download/v1.0.0/libarchive.xcframework.zip

Archive SHA-256:
`1db0e56d929358547d3d5cc37646d53bc8c26ff8b54071ed06d2a7eda8f6d416`

The release contains libarchive 3.8.7, zlib 1.3.2, bzip2 1.0.8, xz 5.8.3,
zstd 1.5.7 and lz4 1.10.0 according to its tagged README. Unmodified license
texts from the tagged `LICENSE` and `ThirdPartyLicenses` are tracked in
`app/ThirdPartyNotices.bundle` and copied into the ARM64 app resources. Linpad's
GPL files and the original libarchive submodule/source project remain intact.

The downloaded device slice is a static ARM64 Mach-O archive with iOS 15.0 minimum
and SDK 26.5 build metadata; **Linpad's ARM64 app minimum is now iPadOS 15.0**.
The simulator archive also contains an x86_64 *Apple simulator* slice, which is
never selected as a Linux guest architecture. Linpad requires an ARM64 host.
All 50 libarchive APIs used by `tools/fakefs.c` were found in the device archive.
This checks API availability; it does not establish link compatibility with every
older Xcode SDK. Use a current Xcode and validate the actual app.

`config/prebuilt-libarchive.json` pins the URL, archive size/hash and every extracted
member size/hash. The ARM64 build phase runs `scripts/prepare-prebuilt.py` before
compilation and fails immediately on verification failure. The script downloads
with curl, verifies before replacing the cached ZIP, validates all selected members
before writing outputs, and extracts only explicit safe paths. A verified extracted
cache needs neither a ZIP nor network access. No ZIP, downloaded executables or
unselected platform libraries are bundled in the app.

The roughly 72 MB download is cached by default in `build-data/prebuilt`. The
selected iOS device static archive is about 12 MB before normal linker stripping.
The release bundles its compression libraries; the old libarchive source-target
dependency and separate system bzip2 link are removed from the ARM64 targets.
Native SDK iconv, xml2 and pthread libraries satisfy the artifact's system links.
This is direct static-slice linking; SwiftPM and its Swift wrapper are not required.

For offline cache priming:

```sh
python3 scripts/prepare-prebuilt.py --archive /path/to/libarchive.xcframework.zip
# Optional shared cache, also passed to xcodebuild as LINPAD_PREBUILT_DIR:
python3 scripts/prepare-prebuilt.py --cache /path/to/prebuilt-cache
```

Cache only verified binaries, never signing credentials. A corrupt extracted
member is repaired from the verified ZIP. A corrupt ZIP fails with a removal/retry
message; a failed download leaves no partial cached ZIP. Hashes pin content against
unexpected replacement; they do not constitute an independent security review of
the binary distributor.

To rebuild the replacement from source, follow the pinned distributor's `build.sh`
and exact dependency source versions. Inspect the resulting device/simulator
artifacts and licenses, then deliberately update the URL/local provisioning and
all hashes in the manifest; arbitrary local artifacts are rejected. The original
`deps/libarchive` source project is retained for historical source builds, but is
not a selectable drop-in alternative in the supported ARM64 scheme. Do not restore
only its target dependency while leaving the prebuilt link flags, which would link
two providers. Linux packaging helpers continue using their host's libarchive.

## Guest VDSO

`kernel/vdso.c` embeds `vdso/arm64/prebuilt/libvdso.so.elf` as guest bytes, not as a
native iOS library. The payload originates from the preserved ios-linuxkit ARM64
VDSO sources at the revision recorded in its manifest and [UPSTREAM.md](../UPSTREAM.md).
It remains under the applicable upstream GPL terms with corresponding source
checked in. Its manifest records Clang/LLD 19.1.7, the build command and hashes
for the binary, three source/linker inputs and the runtime allocation header.

Every build validates those hashes, ELF64 little-endian/AArch64 ET_DYN identity,
required helper names and the 8,192-byte allocation limit. Editing a pinned source
or allocation header fails with a stale-binary diagnostic. `vdso_c_args` is rejected
in prebuilt mode rather than silently ignored. Source mode fails if no working
AArch64 ELF linker exists rather than substituting an empty VDSO.

Maintainer regeneration (requires LLVM clang and LLD):

```sh
brew install llvm lld  # macOS; also put their bin directories on PATH
LINPAD_VDSO_CLANG=clang sh scripts/rebuild-vdso.sh
# Review/commit the ELF and updated source/compiler/hash manifest together.
```

A source build without updating the checked-in payload is also available:

```sh
meson setup build-vdso-source --cross-file config/aarch64-linux.ini \
  -Djit=false -Djit_emit=false -Dvdso_mode=source
ninja -C build-vdso-source vdso/arm64/libvdso.so.elf
```

The standard Xcode bridge forces prebuilt mode. Use the explicit maintainer
regeneration path when changing the guest VDSO; ordinary app builders no longer
need a Linux ELF toolchain. Guest package installation and the VDSO introduce no
VM, booted kernel, x86 Linux dependency, JIT requirement or application GPU bridge.

## Validation

Run `python3 -m unittest discover -s tests/linpad -p 'test_prebuilt.py'` for cache
reuse/repair, truncated/hash-mismatched artifacts, failed downloads, unsafe member
paths, stale VDSO sources/allocation header and corrupt/wrong-architecture VDSOs.
The fresh ARM64 cross-build and independent byte-identical source VDSO rebuild
passed. The Xcode project was parsed and the ARM64 dependency/resource/build-phase
references checked. Real iOS compilation/linking and fakefs rootfs import/export
with the new Mach-O library remain required acceptance checks on macOS/iPad.
