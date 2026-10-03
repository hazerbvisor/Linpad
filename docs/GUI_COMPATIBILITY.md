# GUI compatibility audit

**Experimental source audit**, 2026-10-03, foundation
`c1a4064e84f1a957280a62f77d107162032dc908`.
None of the entries below is an on-iPad compatibility certification.
“Supported” means a source implementation exists; “Partial” identifies known
semantic restrictions or boundaries; “Missing” means no implementation/path was
found. Blank cells mean the feature is in another column. Executable guest results
must be recorded separately; native host control results do not change this table.

| Feature | Supported | Partial | Missing | Required for |
| --- | --- | --- | --- | --- |
| AF_UNIX pathname SOCK_STREAM | Source | | | Local X11 / Wayland |
| AF_UNIX SOCK_DGRAM / socketpair | Source | | | IPC; descriptor tests |
| Abstract Unix names | | Text-keyed emulation; embedded NUL names need audit | | X11 fallback; Linux IPC |
| sendmsg / recvmsg | | AArch64 layout conversion; lengths narrow internally | | XCB; Wayland |
| SCM_RIGHTS guest-to-guest | | Retained guest fd queue + host dummy fd; truncation/peek/concurrency need tests | | MIT-SHM fd variant; Wayland |
| SCM_RIGHTS guest-to-native | | | No usable host-fd export contract | Native Wayland compositor |
| Ancillary data beyond rights | | General SOL_SOCKET ancillary types rejected by send path | | Credentials / some IPC libraries |
| mmap anonymous / private | Source | | | musl, Xlib, allocators |
| mmap file-backed / MAP_SHARED | | Host mappings exist; coherence and lifetime need guest/iOS tests | | Framebuffers; wl_shm |
| munmap / mprotect | | Implemented; fault recovery weakens strict protection assumptions | | Memory management |
| SysV shmget / shmat / shmctl / shmdt | Source | | | X11 MIT-SHM |
| memfd_create / POSIX shm via files | | Shared host-fd backing exists; seal semantics incomplete | | wl_shm; modern graphics libraries |
| futex | | WAIT/WAKE/requeue families; keyed by memory object; no general cross-process/PI guarantee | | musl pthread synchronization |
| pthreads / mutex / condition variables | | clone/TLS/futex foundations; guest workloads must verify musl behavior | | X server; toolkit worker threads |
| poll / select / ppoll / pselect | Source | | | Xlib/XCB event IO |
| epoll | | Source; edge/oneshot and close races require audit | | Servers, GLib event loops |
| eventfd | | Counter read/write; EFD_SEMAPHORE rejected | | GLib/Qt; render signaling |
| pipe / pipe2 / PTY | Source | | | xterm; launchers; session IO |
| X11 server and local protocol transport | | Optional Xvfb scripts supplied, unexecuted in Linpad | No bundled display server | xclock/xterm |
| Native graphical output/input | | | No guest GUI view/input adapter | Visible interactive iPad window |
| DRM / /dev/dri / GBM / GPU driver | | | No Linux GPU device | Mesa hardware paths |
| Wayland compositor / buffer export | | | No Linpad compositor | Wayland milestone |

## Source evidence

- `kernel/arch/arm64/calls.c` dispatches socket operations, mmap64, futex, clone,
  SysV shared memory, memfd, pselect/ppoll, epoll and eventfd2.
- `fs/sock.c` maps guest Unix paths to app-owned host temporary socket names.
  Abstract names are hashed/compared as C strings: Linux's arbitrary byte-name
  namespace is not fully equivalent. Guest socket paths are not native host paths.
- `fs/sock.c::sys_sendmsg/sys_recvmsg` handle AArch64 msghdr/iovec/cmsghdr layouts.
  Internal fields narrow some 64-bit lengths. Control data is bounded to 2048
  bytes on send and fd counts to 253. FD transfer queues `struct fd` references
  and sends one dummy host FD. A native receiver cannot interpret that dummy as
  the actual shared buffer. Receiving insufficient control space returns EINVAL
  in the examined path; Linux MSG_CTRUNC/MSG_PEEK/CLOEXEC behavior needs tests.
- `kernel/mmap.c`, `fs/real.c::realfs_mmap`, `kernel/ipc.c` and
  `platform/platform.h` cover memory. `kernel/fs.c::sys_memfd_create`
  accepts MFD_ALLOW_SEALING but this does not establish seal support.
- `kernel/futex.c`, `kernel/fork.c`, `kernel/tls.c` implement thread foundations.
  Futex identity includes `current->mem`; process-shared futex semantics cannot
  be inferred from ordinary pthread success.
- `kernel/poll.c`, `kernel/epoll.c`, `fs/poll.c`, `kernel/eventfd.c` and `fs/pipe.c`
  provide event IO. `eventfd2` accepts CLOEXEC/NONBLOCK flags, not SEMAPHORE.
- [LIMITATIONS.md](LIMITATIONS.md) describes instruction gaps, permissive memory
  fault recovery, sandbox constraints, and Linux-versus-Darwin differences.

## Packages worth attempting

The Alpine v3.24 AArch64 indexes inspected in this session contain xclock
1.1.1-r0, xeyes 1.3.1-r0, xterm 410-r0, xvfb 21.1.24-r0 and TigerVNC 1.16.2-r0.
Availability is not runtime compatibility. xclock's ordinary Alpine executable
was downloaded and verified as AArch64 ELF with the musl AArch64 interpreter;
it was not executed successfully in Linpad.

Start with xclock + Xvfb, core fonts, Xlib/Xt/Xaw and CPU pixman rendering.
Then test xeyes (Shape/XInput/Render dependencies) and xterm (PTY, terminal input,
fonts, locale). GTK3 with X11 and software Cairo is a later candidate. GTK4/Qt,
Openbox/XFCE and D-Bus add dependencies and must wait for the first interactive
window. Xorg hardware drivers, GBM and Linux Mesa hardware rendering have no
guest GPU device; they are not a viable acceleration shortcut. Xvfb itself pulls
Mesa libraries, but installing those libraries does not create a GPU bridge.

## Reproducible probes

Copy this checkout's scripts/tests into a disposable guest-visible directory.
Inside Linpad Alpine AArch64:

```sh
apk add build-base
sh scripts/run-gui-probe.sh
sh scripts/install-x11-poc.sh
sh scripts/run-x11-poc.sh
```

`tests/linpad/gui-probe.c` checks pathname/abstract stream sockets, datagram
socketpairs, two-fd transfers with retained ownership, shared-file visibility
across fork, private COW, anonymous mapping/protection calls, SysV shm, memfd,
poll/select/level epoll/eventfd, futex timeout/mismatch/wake, and pthread
mutex/condition behavior. It does not establish malicious-input safety, native
FD export, full futex/epoll semantics or forbidden-write protection.
The runner prints release/uname; attach app revision, device/iPadOS version and
exact logs. Native controls passed 10 groups on Linux x86_64 in this session;
AArch64 static probe compilation passed, but no guest run was possible.

Before Wayland, add negative probes for malformed cmsg, truncation, peek,
CLOEXEC, concurrent sockets, fd exhaustion, cancellation and buffer lifetime.
Run all semantics on Darwin/iPad, not only native Linux.
