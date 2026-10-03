// ARM64 (AArch64) Linux syscall table
// Syscall numbers follow the Linux aarch64 ABI (asm-generic based)

#include <stddef.h>
#include "kernel/calls.h"
#include "kernel/syscall_adapter.h"

ISH_SYSCALL_ADAPTER(syscall_stub, 0)
ISH_SYSCALL_ADAPTER(sys_xattr_stub, 5)
ISH_SYSCALL_ADAPTER(sys_getcwd, 2)
ISH_SYSCALL_ADAPTER(sys_eventfd2, 2)
ISH_SYSCALL_ADAPTER(sys_epoll_create, 1)
ISH_SYSCALL_ADAPTER(sys_epoll_ctl, 4)
ISH_SYSCALL_ADAPTER(sys_epoll_pwait, 6)
ISH_SYSCALL_ADAPTER(sys_dup, 1)
ISH_SYSCALL_ADAPTER(sys_dup3, 3)
ISH_SYSCALL_ADAPTER(sys_fcntl, 3)
ISH_SYSCALL_ADAPTER(sys_inotify_init1, 1)
ISH_SYSCALL_ADAPTER(sys_inotify_add_watch, 3)
ISH_SYSCALL_ADAPTER(sys_inotify_rm_watch, 2)
ISH_SYSCALL_ADAPTER(sys_ioctl, 3)
ISH_SYSCALL_ADAPTER(sys_ioprio_set, 3)
ISH_SYSCALL_ADAPTER(sys_ioprio_get, 3)
ISH_SYSCALL_ADAPTER(sys_flock, 2)
ISH_SYSCALL_ADAPTER(sys_mknodat, 4)
ISH_SYSCALL_ADAPTER(sys_mkdirat, 3)
ISH_SYSCALL_ADAPTER(sys_unlinkat, 3)
ISH_SYSCALL_ADAPTER(sys_symlinkat, 3)
ISH_SYSCALL_ADAPTER(sys_linkat, 4)
ISH_SYSCALL_ADAPTER(sys_renameat, 4)
ISH_SYSCALL_ADAPTER(sys_umount2, 2)
ISH_SYSCALL_ADAPTER(sys_mount, 5)
ISH_SYSCALL_ADAPTER(sys_statfs_arm64, 2)
ISH_SYSCALL_ADAPTER(sys_fstatfs_arm64, 2)
ISH_SYSCALL_ADAPTER(sys_truncate64, 2)
ISH_SYSCALL_ADAPTER(sys_ftruncate64, 2)
ISH_SYSCALL_ADAPTER(sys_fallocate, 4)
ISH_SYSCALL_ADAPTER(sys_faccessat, 4)
ISH_SYSCALL_ADAPTER(sys_chdir, 1)
ISH_SYSCALL_ADAPTER(sys_fchdir, 1)
ISH_SYSCALL_ADAPTER(sys_chroot, 1)
ISH_SYSCALL_ADAPTER(sys_fchmod, 2)
ISH_SYSCALL_ADAPTER(sys_fchmodat, 3)
ISH_SYSCALL_ADAPTER(sys_fchownat, 5)
ISH_SYSCALL_ADAPTER(sys_fchown32, 3)
ISH_SYSCALL_ADAPTER(sys_openat, 4)
ISH_SYSCALL_ADAPTER(sys_close, 1)
ISH_SYSCALL_ADAPTER(sys_pipe2, 2)
ISH_SYSCALL_ADAPTER(sys_getdents64, 3)
ISH_SYSCALL_ADAPTER(sys_lseek64, 3)
ISH_SYSCALL_ADAPTER(sys_read, 3)
ISH_SYSCALL_ADAPTER(sys_write, 3)
ISH_SYSCALL_ADAPTER(sys_readv, 3)
ISH_SYSCALL_ADAPTER(sys_writev, 3)
ISH_SYSCALL_ADAPTER(sys_pread, 4)
ISH_SYSCALL_ADAPTER(sys_pwrite, 4)
ISH_SYSCALL_ADAPTER(sys_preadv, 4)
ISH_SYSCALL_ADAPTER(sys_pwritev, 4)
ISH_SYSCALL_ADAPTER(sys_sendfile64, 4)
ISH_SYSCALL_ADAPTER(sys_pselect, 6)
ISH_SYSCALL_ADAPTER(sys_ppoll, 5)
ISH_SYSCALL_ADAPTER(sys_signalfd4, 4)
ISH_SYSCALL_ADAPTER(sys_splice, 6)
ISH_SYSCALL_ADAPTER(sys_readlinkat, 4)
ISH_SYSCALL_ADAPTER(sys_fstatat64, 4)
ISH_SYSCALL_ADAPTER(sys_fstat64, 2)
ISH_SYSCALL_ADAPTER(syscall_success_stub, 0)
ISH_SYSCALL_ADAPTER(sys_fsync, 1)
ISH_SYSCALL_ADAPTER(sys_timerfd_create, 2)
ISH_SYSCALL_ADAPTER(sys_timerfd_settime, 4)
ISH_SYSCALL_ADAPTER(sys_timerfd_gettime, 2)
ISH_SYSCALL_ADAPTER(sys_utimensat, 4)
ISH_SYSCALL_ADAPTER(sys_capget, 2)
ISH_SYSCALL_ADAPTER(sys_capset, 2)
ISH_SYSCALL_ADAPTER(sys_personality, 1)
ISH_SYSCALL_ADAPTER(sys_exit, 1)
ISH_SYSCALL_ADAPTER(sys_exit_group, 1)
ISH_SYSCALL_ADAPTER(sys_waitid, 4)
ISH_SYSCALL_ADAPTER(sys_set_tid_address, 1)
ISH_SYSCALL_ADAPTER(sys_futex, 6)
ISH_SYSCALL_ADAPTER(sys_set_robust_list, 2)
ISH_SYSCALL_ADAPTER(sys_get_robust_list, 3)
ISH_SYSCALL_ADAPTER(sys_nanosleep, 2)
ISH_SYSCALL_ADAPTER(sys_setitimer, 3)
ISH_SYSCALL_ADAPTER(sys_timer_create, 3)
ISH_SYSCALL_ADAPTER(sys_timer_gettime, 2)
ISH_SYSCALL_ADAPTER(sys_timer_getoverrun, 1)
ISH_SYSCALL_ADAPTER(sys_timer_settime, 4)
ISH_SYSCALL_ADAPTER(sys_timer_delete, 1)
ISH_SYSCALL_ADAPTER(sys_clock_settime, 2)
ISH_SYSCALL_ADAPTER(sys_clock_gettime, 2)
ISH_SYSCALL_ADAPTER(sys_clock_getres, 2)
ISH_SYSCALL_ADAPTER(sys_clock_nanosleep, 4)
ISH_SYSCALL_ADAPTER(sys_syslog, 3)
ISH_SYSCALL_ADAPTER(sys_ptrace, 4)
ISH_SYSCALL_ADAPTER(sys_sched_setparam, 2)
ISH_SYSCALL_ADAPTER(sys_sched_setscheduler, 3)
ISH_SYSCALL_ADAPTER(sys_sched_getscheduler, 1)
ISH_SYSCALL_ADAPTER(sys_sched_getparam, 2)
ISH_SYSCALL_ADAPTER(sys_sched_setaffinity, 3)
ISH_SYSCALL_ADAPTER(sys_sched_getaffinity, 3)
ISH_SYSCALL_ADAPTER(sys_sched_yield, 0)
ISH_SYSCALL_ADAPTER(sys_sched_get_priority_max, 1)
ISH_SYSCALL_ADAPTER(sys_kill, 2)
ISH_SYSCALL_ADAPTER(sys_tkill, 2)
ISH_SYSCALL_ADAPTER(sys_tgkill, 3)
ISH_SYSCALL_ADAPTER(sys_sigaltstack, 2)
ISH_SYSCALL_ADAPTER(sys_rt_sigsuspend, 2)
ISH_SYSCALL_ADAPTER(sys_rt_sigaction, 4)
ISH_SYSCALL_ADAPTER(sys_rt_sigprocmask, 4)
ISH_SYSCALL_ADAPTER(sys_rt_sigpending, 2)
ISH_SYSCALL_ADAPTER(sys_rt_sigtimedwait, 4)
ISH_SYSCALL_ADAPTER(sys_rt_sigreturn, 0)
ISH_SYSCALL_ADAPTER(sys_setpriority, 3)
ISH_SYSCALL_ADAPTER(sys_getpriority, 2)
ISH_SYSCALL_ADAPTER(sys_reboot, 3)
ISH_SYSCALL_ADAPTER(sys_setregid, 2)
ISH_SYSCALL_ADAPTER(sys_setgid, 1)
ISH_SYSCALL_ADAPTER(sys_setreuid, 2)
ISH_SYSCALL_ADAPTER(sys_setuid, 1)
ISH_SYSCALL_ADAPTER(sys_setresuid, 3)
ISH_SYSCALL_ADAPTER(sys_getresuid, 3)
ISH_SYSCALL_ADAPTER(sys_setresgid, 3)
ISH_SYSCALL_ADAPTER(sys_getresgid, 3)
ISH_SYSCALL_ADAPTER(sys_times, 1)
ISH_SYSCALL_ADAPTER(sys_setpgid, 2)
ISH_SYSCALL_ADAPTER(sys_getpgid, 1)
ISH_SYSCALL_ADAPTER(sys_getsid, 0)
ISH_SYSCALL_ADAPTER(sys_setsid, 0)
ISH_SYSCALL_ADAPTER(sys_getgroups, 2)
ISH_SYSCALL_ADAPTER(sys_setgroups, 2)
ISH_SYSCALL_ADAPTER(sys_uname, 1)
ISH_SYSCALL_ADAPTER(sys_sethostname, 2)
ISH_SYSCALL_ADAPTER(sys_getrlimit64, 2)
ISH_SYSCALL_ADAPTER(sys_setrlimit64, 2)
ISH_SYSCALL_ADAPTER(sys_getrusage, 2)
ISH_SYSCALL_ADAPTER(sys_umask, 1)
ISH_SYSCALL_ADAPTER(sys_prctl, 5)
ISH_SYSCALL_ADAPTER(sys_getcpu, 3)
ISH_SYSCALL_ADAPTER(sys_gettimeofday, 2)
ISH_SYSCALL_ADAPTER(sys_settimeofday, 2)
ISH_SYSCALL_ADAPTER(sys_getpid, 0)
ISH_SYSCALL_ADAPTER(sys_getppid, 0)
ISH_SYSCALL_ADAPTER(sys_getuid32, 0)
ISH_SYSCALL_ADAPTER(sys_geteuid32, 0)
ISH_SYSCALL_ADAPTER(sys_getgid32, 0)
ISH_SYSCALL_ADAPTER(sys_getegid32, 0)
ISH_SYSCALL_ADAPTER(sys_gettid, 0)
ISH_SYSCALL_ADAPTER(sys_sysinfo, 1)
ISH_SYSCALL_ADAPTER(sys_mq_open, 4)
ISH_SYSCALL_ADAPTER(sys_mq_unlink, 1)
ISH_SYSCALL_ADAPTER(sys_mq_timedsend, 5)
ISH_SYSCALL_ADAPTER(sys_mq_timedreceive, 5)
ISH_SYSCALL_ADAPTER(sys_mq_notify, 2)
ISH_SYSCALL_ADAPTER(sys_mq_getsetattr, 3)
ISH_SYSCALL_ADAPTER(sys_msgget, 2)
ISH_SYSCALL_ADAPTER(sys_msgctl, 3)
ISH_SYSCALL_ADAPTER(sys_msgrcv, 5)
ISH_SYSCALL_ADAPTER(sys_msgsnd, 4)
ISH_SYSCALL_ADAPTER(sys_semget, 3)
ISH_SYSCALL_ADAPTER(sys_semctl, 4)
ISH_SYSCALL_ADAPTER(sys_semtimedop, 4)
ISH_SYSCALL_ADAPTER(sys_semop, 3)
ISH_SYSCALL_ADAPTER(sys_shmget, 3)
ISH_SYSCALL_ADAPTER(sys_shmctl, 3)
ISH_SYSCALL_ADAPTER(sys_shmat, 3)
ISH_SYSCALL_ADAPTER(sys_shmdt, 1)
ISH_SYSCALL_ADAPTER(sys_socket, 3)
ISH_SYSCALL_ADAPTER(sys_socketpair, 4)
ISH_SYSCALL_ADAPTER(sys_bind, 3)
ISH_SYSCALL_ADAPTER(sys_listen, 2)
ISH_SYSCALL_ADAPTER(sys_accept, 3)
ISH_SYSCALL_ADAPTER(sys_connect, 3)
ISH_SYSCALL_ADAPTER(sys_getsockname, 3)
ISH_SYSCALL_ADAPTER(sys_getpeername, 3)
ISH_SYSCALL_ADAPTER(sys_sendto, 6)
ISH_SYSCALL_ADAPTER(sys_recvfrom, 6)
ISH_SYSCALL_ADAPTER(sys_setsockopt, 5)
ISH_SYSCALL_ADAPTER(sys_getsockopt, 5)
ISH_SYSCALL_ADAPTER(sys_shutdown, 2)
ISH_SYSCALL_ADAPTER(sys_sendmsg, 3)
ISH_SYSCALL_ADAPTER(sys_recvmsg, 3)
ISH_SYSCALL_ADAPTER(sys_brk, 1)
ISH_SYSCALL_ADAPTER(sys_munmap, 2)
ISH_SYSCALL_ADAPTER(sys_mremap, 4)
ISH_SYSCALL_ADAPTER(sys_clone, 5)
ISH_SYSCALL_ADAPTER(sys_execve, 3)
ISH_SYSCALL_ADAPTER(sys_mmap64, 6)
ISH_SYSCALL_ADAPTER(sys_fadvise64, 4)
ISH_SYSCALL_ADAPTER(sys_mprotect, 3)
ISH_SYSCALL_ADAPTER(sys_msync, 3)
ISH_SYSCALL_ADAPTER(sys_mlock, 2)
ISH_SYSCALL_ADAPTER(sys_mincore, 3)
ISH_SYSCALL_ADAPTER(sys_madvise, 3)
ISH_SYSCALL_ADAPTER(sys_mbind, 6)
ISH_SYSCALL_ADAPTER(sys_accept4, 4)
ISH_SYSCALL_ADAPTER(sys_recvmmsg, 5)
ISH_SYSCALL_ADAPTER(sys_wait4, 4)
ISH_SYSCALL_ADAPTER(sys_prlimit64, 4)
ISH_SYSCALL_ADAPTER(sys_sendmmsg, 4)
ISH_SYSCALL_ADAPTER(sys_process_vm_readv, 6)
ISH_SYSCALL_ADAPTER(sys_process_vm_writev, 6)
ISH_SYSCALL_ADAPTER(sys_renameat2, 5)
ISH_SYSCALL_ADAPTER(sys_getrandom, 3)
ISH_SYSCALL_ADAPTER(sys_memfd_create, 2)
ISH_SYSCALL_ADAPTER(sys_membarrier, 3)
ISH_SYSCALL_ADAPTER(sys_copy_file_range, 6)
ISH_SYSCALL_ADAPTER(sys_preadv2, 5)
ISH_SYSCALL_ADAPTER(sys_pwritev2, 5)
ISH_SYSCALL_ADAPTER(sys_statx, 5)
ISH_SYSCALL_ADAPTER(sys_rseq, 4)
ISH_SYSCALL_ADAPTER(syscall_silent_stub, 0)
ISH_SYSCALL_ADAPTER(sys_pidfd_open, 2)
ISH_SYSCALL_ADAPTER(sys_clone3, 2)
ISH_SYSCALL_ADAPTER(sys_close_range, 3)
ISH_SYSCALL_ADAPTER(sys_openat2, 4)
ISH_SYSCALL_ADAPTER(sys_faccessat2, 4)
ISH_SYSCALL_ADAPTER(sys_fchmodat2, 4)

const syscall_t syscall_stub_entry = syscall_entry_syscall_stub;

/*
 * ARM64 Linux syscall numbers are based on asm-generic and are mostly sequential.
 * The ABI uses individual socket and IPC syscalls, 64-bit stat structures, and
 * ARM64-specific argument ordering where Linux defines it.
 */

syscall_t syscall_table[] = {
    // I/O syscalls
    [0]   = syscall_entry_syscall_stub, // io_setup
    [1]   = syscall_entry_syscall_stub, // io_destroy
    [2]   = syscall_entry_syscall_stub, // io_submit
    [3]   = syscall_entry_syscall_stub, // io_cancel
    [4]   = syscall_entry_syscall_stub, // io_getevents
    [5 ... 16] = syscall_entry_sys_xattr_stub, // xattr syscalls
    [17]  = syscall_entry_sys_getcwd,
    [18]  = syscall_entry_syscall_stub, // lookup_dcookie
    [19]  = syscall_entry_sys_eventfd2,
    [20]  = syscall_entry_sys_epoll_create,
    [21]  = syscall_entry_sys_epoll_ctl,
    [22]  = syscall_entry_sys_epoll_pwait,
    [23]  = syscall_entry_sys_dup,
    [24]  = syscall_entry_sys_dup3,
    [25]  = syscall_entry_sys_fcntl,
    [26]  = syscall_entry_sys_inotify_init1,
    [27]  = syscall_entry_sys_inotify_add_watch,
    [28]  = syscall_entry_sys_inotify_rm_watch,
    [29]  = syscall_entry_sys_ioctl,
    [30]  = syscall_entry_sys_ioprio_set,
    [31]  = syscall_entry_sys_ioprio_get,
    [32]  = syscall_entry_sys_flock,
    [33]  = syscall_entry_sys_mknodat,
    [34]  = syscall_entry_sys_mkdirat,
    [35]  = syscall_entry_sys_unlinkat,
    [36]  = syscall_entry_sys_symlinkat,
    [37]  = syscall_entry_sys_linkat,
    [38]  = syscall_entry_sys_renameat,
    [39]  = syscall_entry_sys_umount2,
    [40]  = syscall_entry_sys_mount,
    [41]  = syscall_entry_syscall_stub, // pivot_root
    [42]  = syscall_entry_syscall_stub, // nfsservctl
    [43]  = syscall_entry_sys_statfs_arm64,
    [44]  = syscall_entry_sys_fstatfs_arm64,
    [45]  = syscall_entry_sys_truncate64,
    [46]  = syscall_entry_sys_ftruncate64,
    [47]  = syscall_entry_sys_fallocate,
    [48]  = syscall_entry_sys_faccessat,
    [49]  = syscall_entry_sys_chdir,
    [50]  = syscall_entry_sys_fchdir,
    [51]  = syscall_entry_sys_chroot,
    [52]  = syscall_entry_sys_fchmod,
    [53]  = syscall_entry_sys_fchmodat,
    [54]  = syscall_entry_sys_fchownat,
    [55]  = syscall_entry_sys_fchown32,
    [56]  = syscall_entry_sys_openat,
    [57]  = syscall_entry_sys_close,
    [58]  = syscall_entry_syscall_stub, // vhangup
    [59]  = syscall_entry_sys_pipe2,
    [60]  = syscall_entry_syscall_stub, // quotactl
    [61]  = syscall_entry_sys_getdents64,
    [62]  = syscall_entry_sys_lseek64,
    [63]  = syscall_entry_sys_read,
    [64]  = syscall_entry_sys_write,
    [65]  = syscall_entry_sys_readv,
    [66]  = syscall_entry_sys_writev,
    [67]  = syscall_entry_sys_pread,
    [68]  = syscall_entry_sys_pwrite,
    [69]  = syscall_entry_sys_preadv,
    [70]  = syscall_entry_sys_pwritev,
    [71]  = syscall_entry_sys_sendfile64,
    [72]  = syscall_entry_sys_pselect,
    [73]  = syscall_entry_sys_ppoll,
    [74]  = syscall_entry_sys_signalfd4,
    [75]  = syscall_entry_syscall_stub, // vmsplice
    [76]  = syscall_entry_sys_splice,
    [77]  = syscall_entry_syscall_stub, // tee
    [78]  = syscall_entry_sys_readlinkat,
    [79]  = syscall_entry_sys_fstatat64,
    [80]  = syscall_entry_sys_fstat64,
    [81]  = syscall_entry_syscall_success_stub, // sync
    [82]  = syscall_entry_sys_fsync,
    [83]  = syscall_entry_sys_fsync, // fdatasync
    [84]  = syscall_entry_syscall_stub, // sync_file_range
    [85]  = syscall_entry_sys_timerfd_create,
    [86]  = syscall_entry_sys_timerfd_settime,
    [87]  = syscall_entry_sys_timerfd_gettime,
    [88]  = syscall_entry_sys_utimensat,
    [89]  = syscall_entry_syscall_stub, // acct
    [90]  = syscall_entry_sys_capget,
    [91]  = syscall_entry_sys_capset,
    [92]  = syscall_entry_sys_personality,
    [93]  = syscall_entry_sys_exit,
    [94]  = syscall_entry_sys_exit_group,
    [95]  = syscall_entry_sys_waitid,
    [96]  = syscall_entry_sys_set_tid_address,
    [97]  = syscall_entry_syscall_stub, // unshare
    [98]  = syscall_entry_sys_futex,
    [99]  = syscall_entry_sys_set_robust_list,
    [100] = syscall_entry_sys_get_robust_list,
    [101] = syscall_entry_sys_nanosleep,
    [102] = syscall_entry_syscall_stub, // getitimer
    [103] = syscall_entry_sys_setitimer,
    [104] = syscall_entry_syscall_stub, // kexec_load
    [105] = syscall_entry_syscall_stub, // init_module
    [106] = syscall_entry_syscall_stub, // delete_module
    [107] = syscall_entry_sys_timer_create,
    [108] = syscall_entry_sys_timer_gettime,
    [109] = syscall_entry_sys_timer_getoverrun,
    [110] = syscall_entry_sys_timer_settime,
    [111] = syscall_entry_sys_timer_delete,
    [112] = syscall_entry_sys_clock_settime,
    [113] = syscall_entry_sys_clock_gettime,
    [114] = syscall_entry_sys_clock_getres,
    [115] = syscall_entry_sys_clock_nanosleep,
    [116] = syscall_entry_sys_syslog,
    [117] = syscall_entry_sys_ptrace,
    [118] = syscall_entry_sys_sched_setparam,
    [119] = syscall_entry_sys_sched_setscheduler,
    [120] = syscall_entry_sys_sched_getscheduler,
    [121] = syscall_entry_sys_sched_getparam,
    [122] = syscall_entry_sys_sched_setaffinity,
    [123] = syscall_entry_sys_sched_getaffinity,
    [124] = syscall_entry_sys_sched_yield,
    [125] = syscall_entry_sys_sched_get_priority_max,
    [126] = syscall_entry_sys_sched_get_priority_max, // get_priority_min - should use different func
    [127] = syscall_entry_syscall_stub, // sched_rr_get_interval
    [128] = syscall_entry_syscall_stub, // restart_syscall
    [129] = syscall_entry_sys_kill,
    [130] = syscall_entry_sys_tkill,
    [131] = syscall_entry_sys_tgkill,
    [132] = syscall_entry_sys_sigaltstack,
    [133] = syscall_entry_sys_rt_sigsuspend,
    [134] = syscall_entry_sys_rt_sigaction,
    [135] = syscall_entry_sys_rt_sigprocmask,
    [136] = syscall_entry_sys_rt_sigpending,
    [137] = syscall_entry_sys_rt_sigtimedwait,
    [138] = syscall_entry_syscall_stub, // rt_sigqueueinfo
    [139] = syscall_entry_sys_rt_sigreturn,
    [140] = syscall_entry_sys_setpriority,
    [141] = syscall_entry_sys_getpriority,
    [142] = syscall_entry_sys_reboot,
    [143] = syscall_entry_sys_setregid,
    [144] = syscall_entry_sys_setgid,
    [145] = syscall_entry_sys_setreuid,
    [146] = syscall_entry_sys_setuid,
    [147] = syscall_entry_sys_setresuid,
    [148] = syscall_entry_sys_getresuid,
    [149] = syscall_entry_sys_setresgid,
    [150] = syscall_entry_sys_getresgid,
    [151] = syscall_entry_syscall_stub, // setfsuid
    [152] = syscall_entry_syscall_stub, // setfsgid
    [153] = syscall_entry_sys_times,
    [154] = syscall_entry_sys_setpgid,
    [155] = syscall_entry_sys_getpgid,
    [156] = syscall_entry_sys_getsid,
    [157] = syscall_entry_sys_setsid,
    [158] = syscall_entry_sys_getgroups,
    [159] = syscall_entry_sys_setgroups,
    [160] = syscall_entry_sys_uname,
    [161] = syscall_entry_sys_sethostname,
    [162] = syscall_entry_syscall_stub, // setdomainname
    [163] = syscall_entry_sys_getrlimit64,
    [164] = syscall_entry_sys_setrlimit64,
    [165] = syscall_entry_sys_getrusage,
    [166] = syscall_entry_sys_umask,
    [167] = syscall_entry_sys_prctl,
    [168] = syscall_entry_sys_getcpu,
    [169] = syscall_entry_sys_gettimeofday,
    [170] = syscall_entry_sys_settimeofday,
    [171] = syscall_entry_syscall_stub, // adjtimex
    [172] = syscall_entry_sys_getpid,
    [173] = syscall_entry_sys_getppid,
    [174] = syscall_entry_sys_getuid32,
    [175] = syscall_entry_sys_geteuid32,
    [176] = syscall_entry_sys_getgid32,
    [177] = syscall_entry_sys_getegid32,
    [178] = syscall_entry_sys_gettid,
    [179] = syscall_entry_sys_sysinfo,
    [180] = syscall_entry_sys_mq_open,
    [181] = syscall_entry_sys_mq_unlink,
    [182] = syscall_entry_sys_mq_timedsend,
    [183] = syscall_entry_sys_mq_timedreceive,
    [184] = syscall_entry_sys_mq_notify,
    [185] = syscall_entry_sys_mq_getsetattr,
    [186] = syscall_entry_sys_msgget,
    [187] = syscall_entry_sys_msgctl,
    [188] = syscall_entry_sys_msgrcv,
    [189] = syscall_entry_sys_msgsnd,
    [190] = syscall_entry_sys_semget,
    [191] = syscall_entry_sys_semctl,
    [192] = syscall_entry_sys_semtimedop,
    [193] = syscall_entry_sys_semop,
    [194] = syscall_entry_sys_shmget,
    [195] = syscall_entry_sys_shmctl,
    [196] = syscall_entry_sys_shmat,
    [197] = syscall_entry_sys_shmdt,
    // Socket syscalls (no multiplexer on arm64)
    [198] = syscall_entry_sys_socket,
    [199] = syscall_entry_sys_socketpair,
    [200] = syscall_entry_sys_bind,
    [201] = syscall_entry_sys_listen,
    [202] = syscall_entry_sys_accept,
    [203] = syscall_entry_sys_connect,
    [204] = syscall_entry_sys_getsockname,
    [205] = syscall_entry_sys_getpeername,
    [206] = syscall_entry_sys_sendto,
    [207] = syscall_entry_sys_recvfrom,
    [208] = syscall_entry_sys_setsockopt,
    [209] = syscall_entry_sys_getsockopt,
    [210] = syscall_entry_sys_shutdown,
    [211] = syscall_entry_sys_sendmsg,
    [212] = syscall_entry_sys_recvmsg,
    [213] = syscall_entry_syscall_stub, // readahead
    [214] = syscall_entry_sys_brk,
    [215] = syscall_entry_sys_munmap,
    [216] = syscall_entry_sys_mremap,
    [217] = syscall_entry_syscall_stub, // add_key
    [218] = syscall_entry_syscall_stub, // request_key
    [219] = syscall_entry_syscall_stub, // keyctl
    [220] = syscall_entry_sys_clone,
    [221] = syscall_entry_sys_execve,
    [222] = syscall_entry_sys_mmap64,
    [223] = syscall_entry_sys_fadvise64, // fadvise64
    [224] = syscall_entry_syscall_stub, // swapon
    [225] = syscall_entry_syscall_stub, // swapoff
    [226] = syscall_entry_sys_mprotect,
    [227] = syscall_entry_sys_msync,
    [228] = syscall_entry_sys_mlock,
    [229] = syscall_entry_syscall_stub, // munlock
    [230] = syscall_entry_syscall_stub, // mlockall
    [231] = syscall_entry_syscall_stub, // munlockall
    [232] = syscall_entry_sys_mincore, // mincore
    [233] = syscall_entry_sys_madvise,
    [234] = syscall_entry_syscall_stub, // remap_file_pages
    [235] = syscall_entry_sys_mbind,
    [236] = syscall_entry_syscall_stub, // get_mempolicy
    [237] = syscall_entry_syscall_stub, // set_mempolicy
    [238] = syscall_entry_syscall_stub, // migrate_pages
    [239] = syscall_entry_syscall_stub, // move_pages
    [240] = syscall_entry_syscall_stub, // rt_tgsigqueueinfo
    [241] = syscall_entry_syscall_stub, // perf_event_open
    [242] = syscall_entry_sys_accept4,
    [243] = syscall_entry_sys_recvmmsg,
    [260] = syscall_entry_sys_wait4,
    [261] = syscall_entry_sys_prlimit64,
    [262] = syscall_entry_syscall_stub, // fanotify_init
    [263] = syscall_entry_syscall_stub, // fanotify_mark
    [264] = syscall_entry_syscall_stub, // name_to_handle_at
    [265] = syscall_entry_syscall_stub, // open_by_handle_at
    [266] = syscall_entry_syscall_stub, // clock_adjtime
    [267] = syscall_entry_syscall_stub, // syncfs
    [268] = syscall_entry_syscall_stub, // setns
    [269] = syscall_entry_sys_sendmmsg,
    [270] = syscall_entry_sys_process_vm_readv,
    [271] = syscall_entry_sys_process_vm_writev,
    [272] = syscall_entry_syscall_stub, // kcmp
    [273] = syscall_entry_syscall_stub, // finit_module
    [274] = syscall_entry_syscall_stub, // sched_setattr
    [275] = syscall_entry_syscall_stub, // sched_getattr
    [276] = syscall_entry_sys_renameat2,
    [277] = syscall_entry_syscall_stub, // seccomp
    [278] = syscall_entry_sys_getrandom,
    [279] = syscall_entry_sys_memfd_create,
    [280] = syscall_entry_syscall_stub, // bpf
    [281] = syscall_entry_sys_execve, // execveat
    [282] = syscall_entry_syscall_stub, // userfaultfd
    [283] = syscall_entry_sys_membarrier,
    [284] = syscall_entry_syscall_stub, // mlock2
    [285] = syscall_entry_sys_copy_file_range,
    [286] = syscall_entry_sys_preadv2,
    [287] = syscall_entry_sys_pwritev2,
    [288] = syscall_entry_syscall_stub, // pkey_mprotect
    [289] = syscall_entry_syscall_stub, // pkey_alloc
    [290] = syscall_entry_syscall_stub, // pkey_free
    [291] = syscall_entry_sys_statx,
    [293] = syscall_entry_sys_rseq,
    [425] = syscall_entry_syscall_silent_stub, // io_uring_setup
    [426] = syscall_entry_syscall_silent_stub, // io_uring_enter
    [427] = syscall_entry_syscall_silent_stub, // io_uring_register
    [434] = syscall_entry_sys_pidfd_open, // pidfd_open
    [435] = syscall_entry_sys_clone3,
    [436] = syscall_entry_sys_close_range,
    [437] = syscall_entry_sys_openat2,
    [438] = syscall_entry_syscall_silent_stub, // pidfd_getfd
    [439] = syscall_entry_sys_faccessat2,
    [452] = syscall_entry_sys_fchmodat2,
};

#define NUM_SYSCALLS (sizeof(syscall_table) / sizeof(syscall_table[0]))
size_t syscall_table_size = NUM_SYSCALLS;
