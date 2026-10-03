# First X11 window

**Experimental implementation; iPad GUI proof pending.** The user reports the
terminal app working after the startup fix. This branch adds the first real X11
display path. Native Linux host controls verify real Xclock/Xterm protocol and
input mechanics; they are not Alpine AArch64 execution under Linpad.

## Options evaluated

| Option | Benefits | Costs / assumptions | Decision |
| --- | --- | --- | --- |
| A: guest X server → software framebuffer → host view | Complete existing X11 implementation; unmodified Alpine clients; no native FD export | Guest Xvfb compatibility unverified; copied snapshots are expensive | Selected for this proof |
| B: X11 socket → native Linpad X server | Direct presentation and input | Implementing the complete protocol/extensions/fonts is substantial; a toy server cannot prove ordinary application compatibility | Deferred |
| C: guest Xvnc → embedded RFB viewer | Existing real server and input protocol; no shared FD export | Extra dependencies, viewer licensing and validated RFB decoding; temporary only | Fallback if guest Xvfb cannot work |

Current path:

```text
Alpine AArch64 xclock / xeyes / xterm (ordinary packaged ELF)
  → guest Unix X11 socket + Xauthority
  → Alpine Xvfb (CPU software rendering; TCP, GLX and MIT-SHM disabled)
  → guest xwd snapshots → atomic frame.xwd rename
  → bounded app-owned file reader → validated immutable RGBA
  → CoreGraphics / UIKit UIImageView → iPad display

UIKit touch / pointer / hardware keys
  → bounded precreated input mailbox
  → guest shell parser → Alpine xdotool → XTEST → actual X11 client
```

Explicit snapshots avoid depending on guest MAP_SHARED coherence with native
Darwin mappings. xwd reads the running X server; no supplied screenshot or native
recreation of a Linux application is used. A 500 ms producer and host timer give
a provisional 2 FPS ceiling, not a measured iPad frame rate. This path has CPU
copies and disk IO. It uses **no VNC, native X server, Metal presenter or Linux
application GPU acceleration**.

## Try on iPad

1. Build `feature/phase1-x11` using the existing Codemagic workflow or xcodebuild,
   sign the resulting IPA and install it. Keep the working terminal available.
2. Tap the terminal gear and select **Linux GUI (experimental)**. Tap **Copy
   install command**, then **Return to Terminal (keep GUI session)**. Paste/run:
   `sh /usr/share/linpad/install-x11.sh`. Packages are optional, installed from
   the existing Alpine AArch64 repositories; the base rootfs stays small.
3. Reopen Linux GUI, select **xclock**, and tap **Copy start command**. Return to
   Terminal using the keep-session button and run that command. It includes the
   viewer's fresh 32-digit hex session ID. Leave the command running in the
   foreground; reopen Linux GUI to see its live framebuffer.
4. Confirm the clock changes. Test xeyes pointer movement and xterm text/Return
   with a hardware keyboard in fresh sessions. **Done** stops the guest session;
   reopening creates a new ID. **Return to Terminal** preserves the current ID.
   The viewer does not install packages or launch a desktop automatically.
5. Retain **Copy GUI report**, the displayed build revision, device/iPadOS version
   and `/tmp/linpad-x11-<id>/{server.log,client.log,window.txt,packages.txt,context.txt}`.
   A successful device test requires the real packaged app running in Linpad,
   changing visible pixels, and actual input responses.

Native Xeyes currently maps but fails the changing-pupil control; its input
behavior is unresolved. Start with Xclock and Xterm; do not interpret selecting
Xeyes as a compatibility guarantee.

If installation fails, retain apk's exact error and `/etc/apk/repositories`.
The matching Alpine release's main/community repositories must be available;
the installer does not silently switch releases or upgrade the rootfs.
If the session fails or shows no frame, return to Terminal to read its output and
logs. Stop with Ctrl-C if Done cannot reach the input mailbox. Keep the terminal
baseline intact while diagnosing concrete missing syscalls. No desktop packages
or GPU experiments are started before this window proof succeeds.

## Module boundary and untrusted files

- `app/Display/Surface/XWD.c`: standalone C decoder. Accepts version-7 ZPixmap
  TrueColor RGB888, 24/32-bit pixels and either byte order; validates header,
  masks, stride, color-table offsets and exact pixel length. Caps geometry at
  1024×768 and files at 4 MiB. Unsupported formats are explicit failures.
- `app/Display/DisplayServer/FileBridge.c`: fixed session/file names below the
  selected root's app-owned fakefs `data` directory. Opens each descendant with
  `openat`/`O_NOFOLLOW`, rejects symlinks, FIFOs, devices and multiply linked files,
  and bounds log/frame reads. Guest-provided host paths are never accepted.
- `Session.m`: Foundation/serial queue owns descriptors and a pausable timer.
  Delivers immutable RGBA on the main queue, skips identical snapshots and keeps
  at most one decoded frame queued. Does not export guest FDs or call UIKit.
- `Input/Keysym.m` and `X11ViewController.m`: public UIKit controls, aspect-fit
  geometry and input. Runtime/syscall/emulation files have no UIKit dependencies.
- Rootfs overlay version 6 installs the two small guest scripts. Failed/short
  writes do not advance the overlay marker, so installation can retry on boot.

The guest creates the input inode before native appends, preserving fakefs
metadata. Each native input record is at most 80 bytes; command names, numeric
bounds and keysyms use a closed grammar. The guest parser repeats validation,
never uses eval, and tracks held keys. The append-only mailbox is capped at
64 KiB plus 128 bytes reserved for release/stop; restart a full session. Logs
are capped at 16 KiB per native read; the report shows only a short tail. Guest
processes can modify their own files, but those bytes are never trusted as host
paths, allocation sizes or rendering commands. No extra entitlements, host
control sockets or broader filesystem permissions are added.

Frame files are normally published by atomic rename. In-place malicious edits
can make a frame inconsistent; every read is bounded and decoded independently.
Xvfb's server and Xauthority lifecycle belong to xvfb-run. Done sends release
and stop; guest cleanup releases keys and reaps the client. Backgrounding or
hiding the viewer releases keys and pauses host reads. Device suspension/force
quit, memory pressure and fakefs/Darwin coherence still need physical testing.

## Input and scaling

Implemented experimentally: touch left tap, pointer movement through touch pan
or mouse/trackpad hover, and hardware key down/up for basic text, navigation,
modifiers and F1–F12. Modifier events in a simultaneous batch are ordered before
ordinary presses and after releases. Focus/cancellation/backgrounding sends
release. UIKit points map once to guest pixels with letterbox bounds checking;
Retina scale does not multiply the X11 desktop resolution.

Not implemented yet: held-button dragging, right click, scroll, software keyboard,
IME/composed text and full layout/modifier reconciliation. Physical keyboard
mapping and Magic Keyboard delivery need iPad validation. The terminal keyboard
remains separate. Do not claim Milestone 3 complete from native input controls.

## Native presentation and later protocols

Milestone 5 will replace snapshots with validated host buffers, IOSurface where
useful, Metal textures and CAMetalLayer, after live X11 works. Measure copies,
input latency and lifecycle behavior first. Presenting CPU-rendered pixels with
Metal will remain software application rendering. Wayland/Xwayland require
separate FD/shared-buffer lifetime work; true guest GPU work requires an explicit
guest-to-host graphics protocol and measured Apple GPU execution.

## Xios reference

Reference revision/license are in [UPSTREAM.md](../UPSTREAM.md). XScreen.swift's
input/presentation boundary, XSurface.c's surface handling, and the Wayland seat,
buffer and ANGLE presentation design informed the research. No Xios code was
copied. Task-port lookup, jailbreak daemons/entitlements, Procursus binaries and
`/var/jb` are excluded; Linpad clients remain ordinary Linux AArch64 programs.
