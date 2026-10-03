// SPDX-License-Identifier: GPL-3.0-only
#include "FileBridge.h"
#include "../Surface/XWD.h"
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

static int directory(int parent, const char *name) {
    int fd=openat(parent,name,O_RDONLY|O_NOFOLLOW|O_NONBLOCK|O_CLOEXEC);
    if (fd<0) return -1;
    struct stat st;
    if (fstat(fd,&st)<0 || !S_ISDIR(st.st_mode)) { close(fd); errno=ENOTDIR; return -1; }
    return fd;
}
int linpad_display_open_session(int root, const char *identifier) {
    if (!identifier || strlen(identifier)!=32) { errno=EINVAL; return -1; }
    for (unsigned i=0;i<32;i++)
        if (!((identifier[i]>='0' && identifier[i]<='9') || (identifier[i]>='a' && identifier[i]<='f'))) {
            errno=EINVAL; return -1;
        }
    char name[64]; snprintf(name,sizeof(name),"linpad-x11-%s",identifier);
    int tmp=directory(root,"tmp");
    if (tmp<0) return -1;
    int fd=directory(tmp,name), saved=errno;
    close(tmp); errno=saved; return fd;
}
int linpad_display_read(int session, enum linpad_display_file which, uint8_t **bytes, size_t *length) {
    if (!bytes || !length) { errno=EINVAL; return -1; }
    *bytes=NULL; *length=0;
    const char *name;
    size_t maximum;
    switch (which) {
        case LINPAD_FRAME: name="frame.xwd"; maximum=LINPAD_MAX_FRAME; break;
        case LINPAD_SERVER_LOG: name="server.log"; maximum=16384; break;
        case LINPAD_CLIENT_LOG: name="client.log"; maximum=16384; break;
        default: errno=EINVAL; return -1;
    }
    int fd=openat(session,name,O_RDONLY|O_NOFOLLOW|O_NONBLOCK|O_CLOEXEC);
    if (fd<0) return -1;
    struct stat st;
    if (fstat(fd,&st)<0 || !S_ISREG(st.st_mode) || st.st_nlink!=1 || st.st_size<0 || (uint64_t)st.st_size>maximum) {
        close(fd); errno=EINVAL; return -1;
    }
    size_t size=(size_t)st.st_size;
    uint8_t *data=malloc(size ? size : 1);
    if (!data) { close(fd); errno=ENOMEM; return -1; }
    size_t done=0;
    while (done<size) {
        ssize_t n=read(fd,data+done,size-done);
        if (n<0 && errno==EINTR) continue;
        if (n<=0) { free(data); close(fd); errno=EIO; return -1; }
        done+=(size_t)n;
    }
    uint8_t extra;
    ssize_t trailing;
    do { trailing=read(fd,&extra,1); } while (trailing<0 && errno==EINTR);
    close(fd);
    if (trailing!=0) { free(data); errno=EAGAIN; return -1; }
    *bytes=data; *length=size; return 0;
}
static int number(const char **text, unsigned maximum, unsigned *out) {
    const char *p=*text;
    if (*p<'0' || *p>'9') return 0;
    unsigned value=0;
    while (*p>='0' && *p<='9') {
        unsigned digit=(unsigned)(*p-'0');
        if (value>maximum/10 || (value==maximum/10 && digit>maximum%10)) return 0;
        value=value*10+digit; p++;
    }
    *out=value; *text=p; return 1;
}
static int valid_event(const char *line) {
    if (!line) return 0;
    size_t length=strnlen(line,81);
    if (!length || length>80 || line[length-1]!='\n') return 0;
    if (!strcmp(line,"stop\n") || !strcmp(line,"release\n")) return 1;
    const char *p=NULL;
    int click=0;
    if (!strncmp(line,"click ",6)) { p=line+6; click=1; }
    if (!strncmp(line,"move ",5)) p=line+5;
    if (p) {
        unsigned x,y,button;
        if (!number(&p,LINPAD_MAX_WIDTH-1,&x) || *p++!=' ' || !number(&p,LINPAD_MAX_HEIGHT-1,&y)) return 0;
        if (click && (*p++!=' ' || !number(&p,3,&button) || button<1)) return 0;
        return *p=='\n' && p[1]==0;
    }
    const char *key=NULL;
    if (!strncmp(line,"down ",5)) key=line+5;
    if (!strncmp(line,"up ",3)) key=line+3;
    if (!key) return 0;
    size_t keys=length-1-(size_t)(key-line);
    if (!keys || keys>24) return 0;
    for (size_t i=0;i<keys;i++) {
        unsigned char c=(unsigned char)key[i];
        if (!((c>='a' && c<='z') || (c>='A' && c<='Z') || (c>='0' && c<='9') || c=='_')) return 0;
    }
    return 1;
}
int linpad_display_event(int session, const char *event) {
    if (!valid_event(event)) { errno=EINVAL; return -1; }
    int fd=openat(session,"input",O_WRONLY|O_APPEND|O_NOFOLLOW|O_NONBLOCK|O_CLOEXEC);
    if (fd<0) return -1;
    struct stat st;
    size_t length=strlen(event);
    if (fstat(fd,&st)<0 || !S_ISREG(st.st_mode) || st.st_nlink!=1 || st.st_size<0 || st.st_size>((!strcmp(event,"stop\n") || !strcmp(event,"release\n")) ? 65664 : 65536)-(off_t)length) {
        close(fd); errno=ENOSPC; return -1;
    }
    // One write publishes one short line. A failed/partial write is not retried
    // as a second line; the guest ignores incomplete input records.
    ssize_t n;
    do { n=write(fd,event,length); } while (n<0 && errno==EINTR);
    close(fd);
    if (n!=(ssize_t)length) { errno=EIO; return -1; }
    return 0;
}
