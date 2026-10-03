// SPDX-License-Identifier: GPL-3.0-only
#import "Session.h"
#include "FileBridge.h"
#include "../Surface/XWD.h"
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

@interface LinpadDisplaySession () {
    int _rootFD;
    int _sessionFD;
    BOOL _stopped;
    BOOL _framePending;
}
@property NSURL *root;
@property NSString *identifier;
@property dispatch_queue_t queue;
@property dispatch_source_t timer;
@property NSData *lastFrame;
@property NSString *lastStatus;
@property (copy) void (^frame)(NSData *, uint32_t, uint32_t);
@property (copy) void (^status)(NSString *);
@end
@implementation LinpadDisplaySession
- (instancetype)initWithRoot:(NSURL *)root identifier:(NSString *)identifier
                       frame:(void (^)(NSData *, uint32_t, uint32_t))frame
                      status:(void (^)(NSString *))status {
    if ((self=[super init])) {
        _rootFD=_sessionFD=-1;
        self.root=root; self.identifier=identifier;
        self.frame=frame; self.status=status;
        self.queue=dispatch_queue_create("org.linpad.x11.snapshot",DISPATCH_QUEUE_SERIAL);
    }
    return self;
}
- (void)report:(NSString *)text {
    if ([text isEqualToString:self.lastStatus]) return;
    self.lastStatus=text;
    void (^status)(NSString *)=self.status;
    dispatch_async(dispatch_get_main_queue(), ^{ status(text); });
}
- (void)start {
    dispatch_async(self.queue, ^{
        if (self.timer || self->_stopped) return;
        self->_rootFD=open(self.root.fileSystemRepresentation,O_RDONLY|O_NOFOLLOW|O_CLOEXEC);
        if (self->_rootFD<0) { [self report:@"Cannot open the app's Alpine storage."]; return; }
        self.timer=dispatch_source_create(DISPATCH_SOURCE_TYPE_TIMER,0,0,self.queue);
        dispatch_source_set_timer(self.timer,DISPATCH_TIME_NOW,500*NSEC_PER_MSEC,50*NSEC_PER_MSEC);
        __weak typeof(self) weakSelf=self;
        dispatch_source_set_event_handler(self.timer, ^{ [weakSelf readFrame]; });
        dispatch_resume(self.timer);
    });
}
- (void)readFrame {
    if (_stopped || _framePending) return;
    if (_sessionFD<0) _sessionFD=linpad_display_open_session(_rootFD,self.identifier.UTF8String);
    if (_sessionFD<0) { [self report:@"Waiting for the guest session. Run the copied command in the terminal."]; return; }
    uint8_t *bytes=NULL; size_t length=0;
    if (linpad_display_read(_sessionFD,LINPAD_FRAME,&bytes,&length)<0) {
        NSMutableString *message=[@"Waiting for an X11 frame." mutableCopy];
        for (enum linpad_display_file log=LINPAD_SERVER_LOG; log<=LINPAD_CLIENT_LOG; log++) {
            if (linpad_display_read(_sessionFD,log,&bytes,&length)==0) {
                NSData *data=[NSData dataWithBytesNoCopy:bytes length:length freeWhenDone:YES];
                NSString *text=[[NSString alloc] initWithData:data encoding:NSUTF8StringEncoding];
                if (text.length) [message appendFormat:@"\n%@",[text substringFromIndex:text.length>600?text.length-600:0]];
            }
        }
        [self report:message]; return;
    }
    NSData *source=[NSData dataWithBytesNoCopy:bytes length:length freeWhenDone:YES];
    if ([source isEqualToData:self.lastFrame]) return;
    struct linpad_surface surface;
    const char *error=linpad_xwd_decode(source.bytes,source.length,&surface);
    if (error) { [self report:@(error)]; return; }
    self.lastFrame=source;
    NSData *rgba=[NSData dataWithBytesNoCopy:surface.rgba length:(NSUInteger)surface.width*surface.height*4 freeWhenDone:YES];
    uint32_t w=surface.width,h=surface.height;
    void (^frame)(NSData *,uint32_t,uint32_t)=self.frame;
    self.lastStatus=nil;
    _framePending=YES;
    // Keep at most one decoded frame queued for the main thread.
    dispatch_async(dispatch_get_main_queue(), ^{
        frame(rgba,w,h);
        dispatch_async(self.queue, ^{ self->_framePending=NO; });
    });
}
- (void)sendEvent:(NSString *)event {
    if (event.length>80) return;
    dispatch_async(self.queue, ^{
        if (self->_stopped || self->_sessionFD<0) return;
        if (linpad_display_event(self->_sessionFD,event.UTF8String)<0)
            [self report:@"Input mailbox unavailable/full. Close the viewer and stop the guest session with Ctrl-C."];
    });
}
- (void)pause {
    dispatch_async(self.queue, ^{
        if (self.timer) dispatch_source_set_timer(self.timer,DISPATCH_TIME_FOREVER,500*NSEC_PER_MSEC,50*NSEC_PER_MSEC);
    });
}
- (void)resume {
    dispatch_async(self.queue, ^{
        if (self.timer && !self->_stopped) {
            self.lastFrame=nil; // Re-present a static frame after returning to the viewer.
            dispatch_source_set_timer(self.timer,DISPATCH_TIME_NOW,500*NSEC_PER_MSEC,50*NSEC_PER_MSEC);
        }
    });
}
- (void)stop {
    dispatch_async(self.queue, ^{
        if (self->_stopped) return;
        self->_stopped=YES;
        if (self->_sessionFD<0 && self->_rootFD>=0)
            self->_sessionFD=linpad_display_open_session(self->_rootFD,self.identifier.UTF8String);
        if (self->_sessionFD>=0) {
            linpad_display_event(self->_sessionFD,"release\n");
            linpad_display_event(self->_sessionFD,"stop\n");
            close(self->_sessionFD); self->_sessionFD=-1;
        }
        if (self.timer) { dispatch_source_cancel(self.timer); self.timer=nil; }
        if (self->_rootFD>=0) { close(self->_rootFD); self->_rootFD=-1; }
        self.lastFrame=nil;
    });
}
- (void)dealloc {
    if (_timer) dispatch_source_cancel(_timer);
    if (_sessionFD>=0) close(_sessionFD);
    if (_rootFD>=0) close(_rootFD);
}
@end
