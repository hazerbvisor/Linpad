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

Current status: **Experimental CLI and X11 proof of concept**. After the startup
fix, the user reports the app working on iPad. This development host independently
verified ARM64 cross-compilation and terminal assets, but cannot verify device boot.
An optional GUI screen now presents real guest Xvfb software snapshots and sends
XTEST input. Native Linux host controls passed; **Alpine GUI execution and visible
interactive output on iPad are still unverified**. There is no Metal display
presenter, Wayland compositor or guest GPU acceleration.

Start with [building](docs/BUILDING.md), [validation evidence](docs/BOOTSTRAP_VALIDATION.md),
and [the roadmap](docs/ROADMAP.md). To test the new window path, use
[the X11 device instructions](docs/GUI_ARCHITECTURE.md#try-on-ipad).
The optional installer adds ordinary Alpine AArch64 xclock/xeyes/xterm and Xvfb;
GUI packages stay out of the terminal-only base filesystem.

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
