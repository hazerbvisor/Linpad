// SPDX-License-Identifier: GPL-3.0-only
// Exercise the real parsers/filesystem bridge, not a duplicate implementation.
#include "app/Display/Surface/XWD.h"
#include "app/Display/DisplayServer/FileBridge.h"
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
static void setword(uint8_t *p,uint32_t n) { p[0]=n>>24;p[1]=n>>16;p[2]=n>>8;p[3]=n; }
static void frame(uint8_t *data,unsigned bpp,unsigned order) {
    memset(data,0,128);
    unsigned h[25]={101,7,2,24,2,1,0,order,32,0,32,bpp,8,4,0xff0000,0xff00,0xff,8,256,0,2,1,0,0,0};
    for(unsigned i=0;i<25;i++)setword(data+4*i,h[i]);
    if(order==0) { data[101]=0x33;data[102]=0x22;data[103]=0x11; }
    else { unsigned first=bpp/8==4?102:101;data[first]=0x11;data[first+1]=0x22;data[first+2]=0x33; }
}
static void parse_controls(void) {
    uint8_t data[128];struct linpad_surface s;
    for(unsigned bpp=24;bpp<=32;bpp+=8)for(unsigned order=0;order<2;order++) {
        frame(data,bpp,order);assert(linpad_xwd_decode(data,109,&s)==NULL);
        assert(s.width==2&&s.height==1&&s.rgba[0]==0x11&&s.rgba[1]==0x22&&s.rgba[2]==0x33&&s.rgba[3]==255);
        linpad_surface_free(&s);
    }
    frame(data,32,0);
    for(size_t length=0;length<109;length++)assert(linpad_xwd_decode(data,length,&s)!=NULL&&s.rgba==NULL);
    for(unsigned i=0;i<20;i++) {
        uint8_t changed[128];memcpy(changed,data,sizeof(data));setword(changed+4*i,UINT32_MAX);
        assert(linpad_xwd_decode(changed,109,&s)!=NULL);assert(s.rgba==NULL);
    }
    assert(linpad_xwd_decode(data,110,&s)!=NULL);
    setword(data+4*19,4096);assert(linpad_xwd_decode(data,109,&s)!=NULL);
}
static void writefile(int dir,const char *name,const void *data,size_t size) {
    int fd=openat(dir,name,O_WRONLY|O_CREAT|O_TRUNC,0600);assert(fd>=0);
    assert(write(fd,data,size)==(ssize_t)size);close(fd);
}
static void file_controls(void) {
    char temp[]="/tmp/linpad-display-test.XXXXXX";assert(mkdtemp(temp));
    int root=open(temp,O_RDONLY);assert(root>=0);assert(mkdirat(root,"tmp",0700)==0);
    int tmp=openat(root,"tmp",O_RDONLY);assert(tmp>=0);
    const char *id="0123456789abcdef0123456789abcdef";
    const char *name="linpad-x11-0123456789abcdef0123456789abcdef";
    assert(mkdirat(tmp,name,0700)==0);
    int session=linpad_display_open_session(root,id);assert(session>=0);
    assert(linpad_display_open_session(root,"../../outside")<0);
    uint8_t data[128];frame(data,32,0);writefile(session,"frame.xwd",data,109);
    uint8_t *bytes;size_t length;
    assert(linpad_display_read(session,LINPAD_FRAME,&bytes,&length)==0&&length==109);free(bytes);
    assert(linpad_display_read(session,(enum linpad_display_file)99,&bytes,&length)<0);
    assert(unlinkat(session,"frame.xwd",0)==0);assert(symlinkat("/etc/passwd",session,"frame.xwd")==0);
    assert(linpad_display_read(session,LINPAD_FRAME,&bytes,&length)<0&&bytes==NULL);
    unlinkat(session,"frame.xwd",0);
    assert(mkfifoat(session,"frame.xwd",0600)==0);
    assert(linpad_display_read(session,LINPAD_FRAME,&bytes,&length)<0);unlinkat(session,"frame.xwd",0);
    int oversized=openat(session,"frame.xwd",O_WRONLY|O_CREAT,0600);assert(oversized>=0);
    assert(ftruncate(oversized,LINPAD_MAX_FRAME+1)==0);close(oversized);
    assert(linpad_display_read(session,LINPAD_FRAME,&bytes,&length)<0&&bytes==NULL);
    unlinkat(session,"frame.xwd",0);
    writefile(session,"input","",0);
    assert(linpad_display_event(session,"click 40 50 1\n")==0);
    assert(linpad_display_event(session,"move 0 0\n")==0);
    assert(linpad_display_event(session,"down Control_L\n")==0);
    assert(linpad_display_event(session,"up U002F\n")==0);
    const char *bad[]={"click -1 0 1\n","click 1 1 0\n","click 1024 0 1\n","move 999999999999999999999999 1\n","down $(cmd)\n","up key\nstop\n","down \n","stop now\n","move 1 2 extra\n"};
    for(unsigned i=0;i<sizeof(bad)/sizeof(*bad);i++)assert(linpad_display_event(session,bad[i])<0&&errno==EINVAL);
    int input=openat(session,"input",O_WRONLY);assert(input>=0);assert(ftruncate(input,65536)==0);close(input);
    assert(linpad_display_event(session,"down a\n")<0&&errno==ENOSPC);
    assert(linpad_display_event(session,"stop\n")==0);
    unlinkat(session,"input",0);writefile(session,"other","",0);
    assert(linkat(session,"other",session,"input",0)==0);
    assert(linpad_display_event(session,"stop\n")<0);
    unlinkat(session,"input",0);unlinkat(session,"other",0);close(session);
    assert(unlinkat(tmp,name,AT_REMOVEDIR)==0);assert(symlinkat("/tmp",tmp,name)==0);
    assert(linpad_display_open_session(root,id)<0);unlinkat(tmp,name,0);close(tmp);
    assert(unlinkat(root,"tmp",AT_REMOVEDIR)==0);assert(symlinkat("/tmp",root,"tmp")==0);
    assert(linpad_display_open_session(root,id)<0);unlinkat(root,"tmp",0);close(root);assert(rmdir(temp)==0);
}
int main(int argc,char **argv) {
    if(argc==2) {
        FILE *f=fopen(argv[1],"rb");assert(f);assert(fseek(f,0,SEEK_END)==0);long length=ftell(f);assert(length>0&&length<=LINPAD_MAX_FRAME);rewind(f);
        uint8_t *data=malloc((size_t)length);assert(data);assert(fread(data,1,(size_t)length,f)==(size_t)length);fclose(f);
        struct linpad_surface s;const char *error=linpad_xwd_decode(data,(size_t)length,&s);
        if(error){fprintf(stderr,"%s\n",error);free(data);return 1;}
        unsigned nonblack=0;for(size_t i=0;i<(size_t)s.width*s.height;i++)if(s.rgba[4*i]||s.rgba[4*i+1]||s.rgba[4*i+2])nonblack++;
        assert(nonblack);printf("Decoded real X11 XWD %ux%u; %u nonblack pixels\n",s.width,s.height,nonblack);
        linpad_surface_free(&s);free(data);return 0;
    }
    parse_controls();file_controls();puts("Display controls passed: formats, bounds, links, FIFO, events and mailbox cap");return 0;
}
