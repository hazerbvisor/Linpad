# Linpad

A lightweight ARM64 Linux userspace and desktop environment for iPadOS.

Linpad is being bootstrapped from [ios-linuxkit](https://github.com/rcarmo/ios-linuxkit),
which derives from iSH. It executes ordinary AArch64 Linux userspace programs
through the ARM64 Asbestos interpreter and Linux syscall compatibility layer,
inside a native iPadOS application. Alpine Linux AArch64 remains the base userspace.

Linpad is **not a Linux kernel or a full virtual machine**. It does not use UTM,
QEMU system emulation, Hypervisor.framework, a booted Linux kernel, or x86 Linux
emulation. The default execution backend uses precompiled ARM64 gadgets; JIT,
jailbreak, kernel exploits and private-framework additions are not required.

Current status: **Experimental bootstrap**. The ARM64 Linux CLI and runtime
libraries have been cross-compiled; rootfs packaging works on the build host.
An Apple build and Alpine launch inside the app have not yet been verified.
There is no visible Linux GUI, graphical input bridge, Metal display presenter,
Wayland compositor or guest GPU acceleration in Linpad yet.

Start with [building](docs/BUILDING.md), [validation evidence](docs/BOOTSTRAP_VALIDATION.md),
and [the roadmap](docs/ROADMAP.md). Optional scripts can install X11 test packages
into a disposable guest and capture a real xclock window through Xvfb when the
runtime is available. A headless capture does not meet the iPad GUI milestone.
The terminal-only base filesystem is kept small.

- [Codemagic unsigned build workflow](codemagic.yaml) — [setup](docs/BUILDING.md#codemagic-unsigned-build)
- [Prebuilt dependencies and source rebuilds](docs/PREBUILT_DEPENDENCIES.md)
- [Runtime and module boundaries](docs/ARCHITECTURE.md)
- [X11 architecture](docs/GUI_ARCHITECTURE.md) and [compatibility audit](docs/GUI_COMPATIBILITY.md)
- [Future GPU design](docs/GPU_ARCHITECTURE.md) and [benchmark plan](docs/PERFORMANCE.md)
- [Provenance](UPSTREAM.md) and [preserved upstream README](README.upstream.md)

Linpad's original [GPL-3.0 license](LICENSE) is unchanged. The upstream
[license and attribution terms](LICENSE.md), [iOS notice](LICENSE.IOS),
source copyright notices and dependency licenses remain in place. Distributions
must provide the applicable notices and corresponding source. Historical upstream
reports document upstream results, not validation of this Linpad build.
