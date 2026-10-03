// SPDX-License-Identifier: GPL-3.0-only
// Compile inside Alpine AArch64 with cc -O2 -Wall -Wextra -pthread.
// Host controls validate the probe, never establish Linpad compatibility.
#define _GNU_SOURCE
#include <errno.h>
#include <fcntl.h>
#include <linux/futex.h>
#include <poll.h>
#include <pthread.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/eventfd.h>
#include <sys/mman.h>
#include <sys/select.h>
#include <sys/shm.h>
#include <sys/socket.h>
#include <sys/syscall.h>
#include <sys/un.h>
#include <sys/wait.h>
#include <unistd.h>

static int failures;
#define REQUIRE(x) do { if (!(x)) { fprintf(stderr, "%s:%d: %s (errno=%d: %s)\n", \
    __func__, __LINE__, #x, errno, strerror(errno)); return 0; } } while (0)
static void report(const char *name, int result) {
    printf("%s\t%s\n", result ? "PASS" : "FAIL", name);
    if (!result) failures++;
}

static int unix_stream(int abstract) {
    struct sockaddr_un address = {.sun_family = AF_UNIX};
    snprintf(address.sun_path + abstract, sizeof(address.sun_path) - abstract,
             abstract ? "linpad-%ld" : "/tmp/linpad-%ld.sock", (long)getpid());
    socklen_t length = offsetof(struct sockaddr_un, sun_path) +
        strlen(address.sun_path + abstract) + 1;
    if (!abstract) unlink(address.sun_path);
    int listener = socket(AF_UNIX, SOCK_STREAM, 0);
    REQUIRE(listener >= 0);
    REQUIRE(bind(listener, (struct sockaddr *)&address, length) == 0);
    REQUIRE(listen(listener, 1) == 0);
    int client = socket(AF_UNIX, SOCK_STREAM, 0);
    REQUIRE(client >= 0);
    REQUIRE(connect(client, (struct sockaddr *)&address, length) == 0);
    int server = accept(listener, NULL, NULL);
    REQUIRE(server >= 0);
    REQUIRE(write(client, "X", 1) == 1);
    char byte;
    REQUIRE(read(server, &byte, 1) == 1 && byte == 'X');
    close(server); close(client); close(listener);
    if (!abstract) unlink(address.sun_path);
    return 1;
}

static int fd_passing(int type) {
    int sockets[2], pipefd[2];
    REQUIRE(socketpair(AF_UNIX, type, 0, sockets) == 0);
    REQUIRE(pipe(pipefd) == 0);
    char path[] = "/tmp/linpad-fd-XXXXXX";
    int file = mkstemp(path);
    REQUIRE(file >= 0);
    unlink(path);
    REQUIRE(write(file, "buffer", 6) == 6);
    REQUIRE(lseek(file, 0, SEEK_SET) == 0);
    int descriptors[2] = {pipefd[0], file};
    union { struct cmsghdr align; char bytes[CMSG_SPACE(sizeof(descriptors))]; } control = {0};
    char byte = 'F';
    struct iovec iov = {&byte, 1};
    struct msghdr message = {.msg_iov = &iov, .msg_iovlen = 1,
        .msg_control = control.bytes, .msg_controllen = sizeof(control.bytes)};
    struct cmsghdr *header = CMSG_FIRSTHDR(&message);
    header->cmsg_level = SOL_SOCKET;
    header->cmsg_type = SCM_RIGHTS;
    header->cmsg_len = CMSG_LEN(sizeof(descriptors));
    memcpy(CMSG_DATA(header), descriptors, sizeof(descriptors));
    REQUIRE(sendmsg(sockets[0], &message, 0) == 1);
    close(pipefd[0]); close(file); // Receiver must retain guest descriptor ownership.
    memset(control.bytes, 0, sizeof(control.bytes));
    byte = 0;
    message.msg_controllen = sizeof(control.bytes);
    REQUIRE(recvmsg(sockets[1], &message, 0) == 1 && byte == 'F');
    REQUIRE(!(message.msg_flags & MSG_CTRUNC));
    header = CMSG_FIRSTHDR(&message);
    REQUIRE(header && header->cmsg_level == SOL_SOCKET && header->cmsg_type == SCM_RIGHTS);
    REQUIRE(header->cmsg_len == CMSG_LEN(sizeof(descriptors)));
    memcpy(descriptors, CMSG_DATA(header), sizeof(descriptors));
    REQUIRE(write(pipefd[1], "P", 1) == 1);
    REQUIRE(read(descriptors[0], &byte, 1) == 1 && byte == 'P');
    char buffer[6];
    REQUIRE(read(descriptors[1], buffer, sizeof(buffer)) == 6);
    REQUIRE(memcmp(buffer, "buffer", 6) == 0);
    close(descriptors[0]); close(descriptors[1]); close(pipefd[1]);
    close(sockets[0]); close(sockets[1]);
    return 1;
}

static int mappings(void) {
    size_t size = 4096;
    char *anonymous = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    REQUIRE(anonymous != MAP_FAILED);
    anonymous[0] = 'A';
    REQUIRE(mprotect(anonymous, size, PROT_READ) == 0);
    REQUIRE(anonymous[0] == 'A'); // Does not prove forbidden-write fault enforcement.
    REQUIRE(munmap(anonymous, size) == 0);
    char path[] = "/tmp/linpad-map-XXXXXX";
    int fd = mkstemp(path);
    REQUIRE(fd >= 0);
    unlink(path);
    REQUIRE(ftruncate(fd, size) == 0);
    char *shared = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    REQUIRE(shared != MAP_FAILED);
    shared[0] = 'S';
    pid_t child = fork();
    REQUIRE(child >= 0);
    if (child == 0) {
        char *other = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
        if (other == MAP_FAILED || other[0] != 'S') _exit(1);
        other[0] = 'C';
        _exit(0);
    }
    int status;
    REQUIRE(waitpid(child, &status, 0) == child && WIFEXITED(status) && WEXITSTATUS(status) == 0);
    REQUIRE(shared[0] == 'C');
    char *private = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
    REQUIRE(private != MAP_FAILED);
    private[0] = 'P';
    REQUIRE(shared[0] == 'C');
    REQUIRE(munmap(private, size) == 0 && munmap(shared, size) == 0);
    close(fd);
    return 1;
}

static int sysv_shm(void) {
    int id = shmget(IPC_PRIVATE, 4096, IPC_CREAT | 0600);
    REQUIRE(id >= 0);
    char *first = shmat(id, NULL, 0);
    char *second = shmat(id, NULL, 0);
    REQUIRE(first != (void *)-1 && second != (void *)-1);
    first[0] = 'S';
    REQUIRE(second[0] == 'S');
    REQUIRE(shmctl(id, IPC_RMID, NULL) == 0);
    REQUIRE(shmdt(first) == 0 && shmdt(second) == 0);
    return 1;
}

static int memfd_mapping(void) {
    int fd = memfd_create("linpad-probe", MFD_CLOEXEC);
    REQUIRE(fd >= 0 && ftruncate(fd, 4096) == 0);
    char *buffer = mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    REQUIRE(buffer != MAP_FAILED);
    buffer[0] = 'M';
    char byte;
    REQUIRE(pread(fd, &byte, 1, 0) == 1 && byte == 'M');
    REQUIRE(munmap(buffer, 4096) == 0);
    close(fd);
    return 1;
}

static int event_io(void) {
    int pipefd[2];
    REQUIRE(pipe(pipefd) == 0);
    REQUIRE(write(pipefd[1], "E", 1) == 1);
    struct pollfd polling = {.fd = pipefd[0], .events = POLLIN};
    REQUIRE(poll(&polling, 1, 1000) == 1 && (polling.revents & POLLIN));
    fd_set readfds;
    FD_ZERO(&readfds); FD_SET(pipefd[0], &readfds);
    struct timeval timeout = {.tv_sec = 1};
    REQUIRE(select(pipefd[0] + 1, &readfds, NULL, NULL, &timeout) == 1);
    int epollfd = epoll_create1(EPOLL_CLOEXEC);
    REQUIRE(epollfd >= 0);
    struct epoll_event event = {.events = EPOLLIN, .data.u64 = 42};
    REQUIRE(epoll_ctl(epollfd, EPOLL_CTL_ADD, pipefd[0], &event) == 0);
    REQUIRE(epoll_wait(epollfd, &event, 1, 1000) == 1 && event.data.u64 == 42);
    int counter = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    REQUIRE(counter >= 0);
    uint64_t value = 3;
    REQUIRE(write(counter, &value, sizeof(value)) == sizeof(value));
    REQUIRE(read(counter, &value, sizeof(value)) == sizeof(value) && value == 3);
    REQUIRE(read(counter, &value, sizeof(value)) == -1 && errno == EAGAIN);
    close(counter); close(epollfd); close(pipefd[0]); close(pipefd[1]);
    return 1;
}

static int futex_basic(void) {
    int word = 1;
    REQUIRE(syscall(SYS_futex, &word, FUTEX_WAIT_PRIVATE, 0, NULL, NULL, 0) == -1 && errno == EAGAIN);
    struct timespec timeout = {.tv_nsec = 1000000};
    REQUIRE(syscall(SYS_futex, &word, FUTEX_WAIT_PRIVATE, 1, &timeout, NULL, 0) == -1 && errno == ETIMEDOUT);
    REQUIRE(syscall(SYS_futex, &word, FUTEX_WAKE_PRIVATE, 1, NULL, NULL, 0) == 0);
    return 1;
}

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t condition = PTHREAD_COND_INITIALIZER;
static int ready;
static void *thread_main(void *unused) {
    (void)unused;
    if (pthread_mutex_lock(&mutex)) return (void *)1;
    ready = 1;
    int result = pthread_cond_signal(&condition);
    if (pthread_mutex_unlock(&mutex)) result = 1;
    return result ? (void *)1 : NULL;
}
static int threads(void) {
    pthread_t thread;
    REQUIRE(pthread_mutex_lock(&mutex) == 0);
    REQUIRE(pthread_create(&thread, NULL, thread_main, NULL) == 0);
    while (!ready) REQUIRE(pthread_cond_wait(&condition, &mutex) == 0);
    REQUIRE(pthread_mutex_unlock(&mutex) == 0);
    void *result;
    REQUIRE(pthread_join(thread, &result) == 0 && result == NULL);
    return 1;
}

int main(void) {
    setvbuf(stdout, NULL, _IONBF, 0);
    alarm(30); // Failure must not leave a permanently blocked audit process.
    printf("Linpad GUI prerequisite probe; record host/runtime context separately.\n");
    report("AF_UNIX pathname SOCK_STREAM", unix_stream(0));
    report("AF_UNIX abstract SOCK_STREAM (text name)", unix_stream(1));
    report("sendmsg/recvmsg SCM_RIGHTS stream: pipe + file", fd_passing(SOCK_STREAM));
    report("SOCK_DGRAM + SCM_RIGHTS: pipe + file", fd_passing(SOCK_DGRAM));
    report("anonymous/mprotect/munmap + shared fork/file + private COW", mappings());
    report("SysV shared memory", sysv_shm());
    report("memfd + shared mapping", memfd_mapping());
    report("pipe/poll/select/epoll/eventfd", event_io());
    report("futex mismatch/timeout/wake", futex_basic());
    report("pthread/mutex/condition", threads());
    return failures ? 1 : 0;
}
