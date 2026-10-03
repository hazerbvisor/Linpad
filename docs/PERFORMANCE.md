# Performance measurement plan

**Planned.** No Linpad on-iPad benchmark or comparative VM performance result was
collected. The design omits a complete guest kernel/virtual hardware machine; this
is an architectural distinction, not proof of lower CPU time or memory use.
Historical imported benchmarks apply only to their named upstream revisions,
hardware and workloads. Do not claim Linpad is faster than UTM without identical
hardware and comparable, documented workloads.

| Metric | Proposed repeatable workload | Evidence to retain |
| --- | --- | --- |
| Command execution / process startup | BusyBox echo, repeated fork/exec/wait | Median/p95 elapsed time, warm/cold state |
| Memory usage | Terminal idle and representative app | Host resident/peak footprint, guest workload, jetsam |
| Desktop startup / idle CPU | Optional Openbox/XFCE launch and idle interval | Time to usable screen, memory, CPU and power state |
| X11 latency | Roundtrip request, map/expose acknowledgment | Guest-server and host-present timing separately |
| Framebuffer FPS | Real client redraw with fixed resolution | Produced/presented frames, dropped frames, bytes copied |
| Input latency | Timestamp input -> changed pixels -> presentation | End-to-end distribution, keyboard/touch/pointer separately |
| GTK redraw | Scripted scroll/resize in a fixed app | Toolkit version, software renderer, damage/frame counts |
| OpenGL test | Defined correctness scene once GPU bridge exists | CPU/GPU times, renderer identity, Metal trace |
| Filesystem IO | Sequential/random IO and package extraction | Fakefs path, cache state, bytes and elapsed time |

Record device model/RAM, iPadOS, thermal/power state, app/signing build, runtime
commit, guest rootfs/package hashes, display size/Retina scale and repetitions.
Separate compilation and package-download time from execution. Use warmups and
report distributions; bound every workload and retain raw samples. Measure CPU
software rendering, framebuffer transport and Metal presentation independently.
Tests should not mutate the user's primary filesystem: use disposable rootfs copies.

The imported `make perf-bench` measures ARM64 Linux-host runtime behavior;
its reports now default to repo-relative `build-data/reports`. It is not an iPad
benchmark, and the performance CPU pin must be set for the actual host.
