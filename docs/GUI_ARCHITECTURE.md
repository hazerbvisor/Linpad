# First X11 window

**Planned interactive architecture; experimental headless tooling only.**
No Alpine GUI has run inside Linpad in this session and no iPad graphical output
has been demonstrated. [Validation](BOOTSTRAP_VALIDATION.md) distinguishes native
host controls from guest execution.

## Options evaluated

| Option | Benefits | Costs / blocking assumptions | Decision |
| --- | --- | --- | --- |
| A: guest X server -> software framebuffer -> host view | Existing complete X11 protocol; ordinary unmodified Alpine clients; no native FD export required for a file snapshot | Xvfb syscall compatibility unverified; validated XWD reader and live synchronization/input bridge needed | Provisional MVP selection |
| B: X11 socket -> native Linpad X server | Direct host rendering and input; fewer guest server processes | A complete X server needs extensive protocol/extensions/fonts/security support; minimal fake implementations cannot run ordinary xterm reliably | Defer; do not write a toy X server |
| C: guest Xvnc -> embedded native RFB viewer | Existing real server; framebuffer and input protocol; no shared FD handoff; useful fallback | More dependencies and copies; bounded authenticated RFB decoder needed; viewer licensing must be checked | Temporary fallback if A's display/input integration is too costly |

Select **A with Alpine Xvfb** for the first protocol/framebuffer subproof. It is
available in the exact pinned Alpine release, uses software rendering, and
`-fbdir` exports XWD-formatted screen storage without inventing an X protocol
implementation. Do not launch a hardware Xorg server or require `/dev/dri`.
Guest-only MIT-SHM can be tested later; disable optional extensions if needed
and retain ordinary X11 socket rendering as a fallback.

The committed optional installer and runner start a real `xclock`, wait for its
mapped X11 window, query the server, and capture `root.xwd` and `Xvfb_screen0`.
They retain client/server logs and package versions, use xauth, and disable TCP
listeners. These scripts work as a native Debian host control; their Linpad guest
execution remains unverified. This is a protocol subproof, not Milestone 2.

## Route to the actual on-iPad proof

1. Build the unchanged terminal foundation with Apple tools and launch Alpine
   BusyBox/apk on an M-series iPad. Record revision, app identity, signed archive,
   iPadOS and device; fix baseline failures before GUI host integration.
2. Run `scripts/run-gui-probe.sh` inside that runtime, then install optional X11
   packages and run `scripts/run-x11-poc.sh`. Keep the base rootfs unchanged.
3. Build a separate `app/Display/Surface` reader for app-owned framebuffer files.
   Resolve guest paths through a narrow, checked fakefs interface; never accept
   arbitrary host paths from guest code. Parse the real XWD header, byte order,
   dimensions, depth, masks, stride, color tables and data bounds. Bound buffers
   (initially 800×600 with an explicit memory cap), reject unsupported formats,
   snapshot consistently while the server writes, and handle resize generations.
   Start with a safe copied CPU snapshot, then optimize. Merely displaying a
   downloaded screenshot does not prove live Linux execution.
4. Add an optional GUI screen to `app/UI` using public iOS drawing APIs first,
   with session cancellation and a terminal-only fallback. Confirm the running
   clock changes and its window appears on the physical iPad.
5. Implement pointer/button/keyboard injection through a guest helper using XTEST
   and a bounded local session channel. The native view sends logical coordinates
   and explicit key/button up/down transitions; the guest helper owns Xlib calls.
   Do not inject through unrelated process fds or alter syscall code to call UIKit.
6. Verify focus, hardware keyboard text/modifiers, touch click/drag and mouse
   pointer movement with real xterm/xeyes. Capture live/device evidence and an
   interaction log before marking Milestones 2 and 3 complete.

A is provisional until the guest server and XTEST path work on Darwin. If Xvfb
fails, use logs and focused syscall probes to locate missing semantics. If a
robust live framebuffer/input bridge needs excessive X server changes, try C:
Alpine Xvnc CPU rendering with a bounded embedded viewer. Bind only app-local
loopback, authenticate the connection, validate RFB dimensions/rectangle lengths,
cap decompression/allocations and limit queued updates. No public VNC service,
external remote desktop or unapproved native socket capability is part of the MVP.
That path must be labeled temporary VNC software rendering and later replaced.

## Input and scaling contract

The first phase needs touch -> pointer/left click, external pointer movement and
hardware key press/release for xterm; none is implemented for GUI yet. The existing
terminal keyboard is a separate text-terminal bridge. Convert UIKit points to
guest logical pixels exactly once, clamp to framebuffer bounds, and keep Retina
scale distinct from desktop resolution. Release modifiers/buttons on loss of
focus, cancellation and backgrounding. Scroll/right click/software keyboard and
Magic Keyboard gestures follow once basic interaction is proven.

## Native presentation and later protocols

Move CPU-frame presentation to validated host buffers, IOSurface where useful,
Metal textures and CAMetalLayer in Milestone 5. Keep a stable display/session
interface so a VNC experiment can be replaced. Avoid holding runtime memory locks
while calling the UI or submitting GPU work. Triple/bounded buffering, damage
tracking, fences and measured copy counts follow correctness.

Metal uploading/presenting CPU pixels is **software rendering with Metal
presentation**, not Linux application GPU acceleration. Wayland/Xwayland require
separate shared-buffer and FD lifetime work; they follow working X11.

## Xios reference findings

Reference revision and license are recorded in [UPSTREAM.md](../UPSTREAM.md).
`x11/apps/Xios/Sources/XScreen.swift` maps UIKit touch/pointer/text input and owns
the display presentation. `XSurface.c` receives IOSurface Mach ports.
`x11/wayland/iosc_iosurface.c` implements an IOSurface wl_buffer protocol;
`iosc_input.c` handles compositor input; Xwayland/ANGLE provide native GLES/Metal
rendering for recompiled iOS programs. Surface validation, geometry ownership,
input protocol separation and GPU completion fences are useful design ideas.
Task-port lookup, jailbreak daemons/entitlements, Procursus binaries and `/var/jb`
are excluded. Linpad's clients remain ordinary Linux AArch64 ELF applications.
