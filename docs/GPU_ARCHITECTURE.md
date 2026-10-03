# Guest GPU architecture research

**Planned; unsupported today.** No guest GPU interface, EGL/GLES/Vulkan shim,
ANGLE integration, Metal presenter or GPU workload proof exists in Linpad.
Implementation follows stable X11, input and an optional lightweight desktop.

## Keep rendering and presentation separate

```text
First display pipeline:
Linux app -> CPU X11/pixman rendering -> software framebuffer
           -> host snapshot -> (later) Metal texture -> iPad display
```

This accelerates presentation at most; it is not Linux application GPU rendering.

```text
Possible future application GPU pipeline:
Linux app -> guest EGL/GLES ABI library -> bounded graphics protocol
          -> native host ANGLE / Metal -> Apple GPU
          -> retained surface + completion fence -> Linpad compositor
```

This requires real guest-to-host API virtualization. The Linux guest has no
`/dev/dri`, Linux GPU driver, GBM device or Vulkan ICD that accesses Apple hardware.
Installing Mesa, linking a native iOS ANGLE library or choosing MoltenVK does not
make ordinary Linux graphics binaries reach the GPU.

## Candidate design and unresolved questions

Start research with a deliberately limited GLES/EGL guest shim rather than
claiming a complete OpenGL/Vulkan implementation. A shim must preserve the Linux
AArch64/musl ABI expected by ordinary dynamically linked clients. Investigate
library dispatch, EGL platform selection, supported extensions, context ownership,
thread affinity, GL object IDs, error behavior, shader translation and toolkit
fallbacks. Some clients bypass a chosen ABI; those require separate support.
The host side can investigate ANGLE's public Metal backend. Audit its license and
all transitive licenses before vendoring. IOSurface and Metal APIs must work in a
normal app's entitlements; no Xios task-port machinery may be inherited.

The protocol needs version/feature negotiation, bounded commands and resource
budgets, validated offset/length arithmetic, retained opaque resource handles,
backpressure and cancellation. Never send host pointers or arbitrary OS handles
to the guest. Shared buffers need an explicit VFS/descriptor bridge with checked
rights, accounting, resize generations and release acknowledgments. GPU queues
need producer/consumer ordering and completion fences; do not reuse a texture
while either client or presenter owns it. CPU readback is a separately measured
fallback, not evidence of an accelerated guest pipeline.

Vulkan/MoltenVK would require a Linux Vulkan dispatch/ICD shim, translation of
commands, memory and synchronization, plus a validated host protocol. It is a
separate undertaking, not an automatic consequence of Vulkan -> Metal support.
Keep X11 software-only until device evidence justifies expanding this scope.

## Proof required before any acceleration claim

Run an ordinary AArch64 Linux GLES test inside Linpad, record its loaded guest
libraries and negotiated renderer, and capture a Metal GPU trace of host work
originating from guest rendering commands. Verify pixel correctness, fences,
resource cleanup, malformed-command rejection and context-loss handling.
Compare CPU software rendering and guest GPU rendering on the same device/workload;
report uploads, readbacks, copies and CPU/GPU time separately. A Metal trace of
only a framebuffer blit does not meet Milestone 7.
