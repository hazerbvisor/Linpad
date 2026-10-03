# Linpad architecture

**Experimental:** imported ARM64 terminal app; Linux-host cross-compilation and
rootfs packaging validated. Apple compilation and on-device execution pending.
The detailed imported runtime design is preserved in
[ARCHITECTURE.upstream.md](ARCHITECTURE.upstream.md).

```text
Apple Silicon iPad / native iPadOS app (host sandbox and host kernel)
  -> ARM64 ios-linuxkit / iSH syscall compatibility + Asbestos gadgets
  -> Alpine AArch64 userspace / ordinary Linux ELF programs
  -> terminal frontend today
  -> X11 -> display bridge -> native presentation                 [Planned]
```

Guest basic blocks are decoded into data containing pointers to precompiled
AArch64 gadgets. The default app build forces `jit=false`, `jit_emit=false` and
empty `cli_aot`. iPadOS remains the host kernel. Linux syscalls dispatch to the
userspace compatibility layer, not a virtual hardware machine or a booted kernel.
The imported optional native/AOT research is not enabled by Linpad.

## Existing structure and boundaries

| Path | Responsibility | Status |
| --- | --- | --- |
| asbestos/guest-arm64, emu | AArch64 decoder, gadgets, TLB and execution | Imported; ARM64 cross-build verified |
| kernel | Linux syscall/process/memory/synchronization compatibility | Imported; runtime validation pending here |
| fs, platform | Fakefs, descriptors, sockets, host OS adaptation | Imported; packaging verified |
| app | Native application, terminal UI, sandbox filesystem integration | Imported/rebranded; Apple build pending |
| app/RootfsPatch.bundle | Small Alpine startup overlay | Imported/rebranded |
| scripts, tests/linpad | Optional guest audit and headless X11 evidence | Experimental; host controls only |
| app/Display/DisplayServer | Future host display adapter | Planned; directory/code not created yet |
| app/Display/Input | Future UIKit event to protocol input adapter | Planned |
| app/Display/Surface, app/Metal | Future bounded buffers and presentation | Planned |

Preserve existing upstream paths rather than renaming the runtime wholesale.
The future display adapter receives bounded immutable frame snapshots and sends
bounded input messages through an explicit session interface. UIKit and Metal
remain in `app`; runtime code must not depend on either. Guest descriptors and
addresses are opaque to the host adapter. A future fd bridge must resolve them
through controlled runtime APIs with retention, access checks and cancellation.

## Security and lifecycle

The iOS sandbox is the outer boundary. This compatibility runtime is not a secure
isolation boundary for hostile Linux code; see [SECURITY.md](../SECURITY.md) and
[upstream limitations](LIMITATIONS.md). A guest protocol message is untrusted.
Reject overflow in width × bytes-per-pixel, stride × height, offsets and allocation
sizes; bound dimensions, queued frames, message lengths, fd counts and input rates.
Use app-owned storage and local authenticated sessions. Do not accept guest host
paths or global IOSurface IDs as capabilities. Retain buffers until presentation
completion, coordinate resize generations, and stop/join workers on session exit.
Do not broaden entitlements or sandbox access for display functionality.

Terminal-only launch remains the default. Desktop packages and session processes
must remain opt-in, with foreground/background and memory-pressure behavior tested
on the exact signed app before distribution.
