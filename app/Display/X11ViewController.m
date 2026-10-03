// SPDX-License-Identifier: GPL-3.0-only
#import "X11ViewController.h"
#import "DisplayServer/Session.h"
#import "Input/Keysym.h"
#import "../Roots.h"
#include <math.h>

@interface LinpadX11ViewController ()
@property LinpadDisplaySession *session;
@property NSString *identifier;
@property UIImageView *screen;
@property UILabel *status;
@property NSString *diagnostic;
@property UISegmentedControl *application;
@property uint32_t width,height;
@property CFAbsoluteTime lastMotion;
@property BOOL active;
@property BOOL finished;
@end
@implementation LinpadX11ViewController
- (BOOL)canBecomeFirstResponder { return YES; }
- (void)viewDidLoad {
    [super viewDidLoad];
    self.active=YES;
    self.identifier=[[NSUUID.UUID.UUIDString stringByReplacingOccurrencesOfString:@"-" withString:@""] lowercaseString];
    self.title=@"Linux X11 — experimental";
    self.view.backgroundColor=UIColor.systemBackgroundColor;
    self.navigationItem.leftBarButtonItem=[[UIBarButtonItem alloc] initWithBarButtonSystemItem:UIBarButtonSystemItemDone target:self action:@selector(close:)];
    self.application=[[UISegmentedControl alloc] initWithItems:@[@"xclock",@"xeyes",@"xterm"]];
    self.application.selectedSegmentIndex=0;
    UILabel *instructions=[UILabel new];
    instructions.numberOfLines=0;
    instructions.font=[UIFont preferredFontForTextStyle:UIFontTextStyleFootnote];
    instructions.text=@"1. Copy install command, return to Terminal and run it once.\n2. Choose an app, copy start command and run it in Terminal.\n3. Reopen this viewer using the same session.\nSoftware X11 snapshots, up to 2 FPS. Touch/mouse + hardware keyboard are experimental.";
    UIButton *install=[UIButton buttonWithType:UIButtonTypeSystem];
    [install setTitle:@"Copy install command" forState:UIControlStateNormal];
    [install addTarget:self action:@selector(copyInstall:) forControlEvents:UIControlEventTouchUpInside];
    UIButton *launch=[UIButton buttonWithType:UIButtonTypeSystem];
    [launch setTitle:@"Copy start command" forState:UIControlStateNormal];
    [launch addTarget:self action:@selector(copyStart:) forControlEvents:UIControlEventTouchUpInside];
    UIButton *terminal=[UIButton buttonWithType:UIButtonTypeSystem];
    [terminal setTitle:@"Return to Terminal (keep GUI session)" forState:UIControlStateNormal];
    [terminal addTarget:self action:@selector(returnToTerminal:) forControlEvents:UIControlEventTouchUpInside];
    UIButton *report=[UIButton buttonWithType:UIButtonTypeSystem];
    [report setTitle:@"Copy GUI report" forState:UIControlStateNormal];
    [report addTarget:self action:@selector(copyReport:) forControlEvents:UIControlEventTouchUpInside];
    self.status=[UILabel new]; self.status.numberOfLines=3;
    self.status.font=[UIFont preferredFontForTextStyle:UIFontTextStyleCaption1];
    self.status.text=@"Waiting for the Alpine X11 session.";
    UIStackView *controls=[[UIStackView alloc] initWithArrangedSubviews:@[instructions,self.application,install,launch,terminal,report,self.status]];
    controls.axis=UILayoutConstraintAxisVertical; controls.spacing=6;
    controls.translatesAutoresizingMaskIntoConstraints=NO;
    [self.view addSubview:controls];
    self.screen=[UIImageView new]; self.screen.contentMode=UIViewContentModeScaleAspectFit;
    self.screen.backgroundColor=UIColor.blackColor; self.screen.userInteractionEnabled=YES;
    self.screen.translatesAutoresizingMaskIntoConstraints=NO;
    [self.view addSubview:self.screen];
    UILayoutGuide *safe=self.view.safeAreaLayoutGuide;
    [NSLayoutConstraint activateConstraints:@[
        [controls.topAnchor constraintEqualToAnchor:safe.topAnchor constant:8],
        [controls.leadingAnchor constraintEqualToAnchor:safe.leadingAnchor constant:12],
        [controls.trailingAnchor constraintEqualToAnchor:safe.trailingAnchor constant:-12],
        [self.screen.topAnchor constraintEqualToAnchor:controls.bottomAnchor constant:8],
        [self.screen.leadingAnchor constraintEqualToAnchor:safe.leadingAnchor],
        [self.screen.trailingAnchor constraintEqualToAnchor:safe.trailingAnchor],
        [self.screen.bottomAnchor constraintEqualToAnchor:safe.bottomAnchor],
    ]];
    UITapGestureRecognizer *tap=[[UITapGestureRecognizer alloc] initWithTarget:self action:@selector(tap:)];
    [self.screen addGestureRecognizer:tap];
    UIPanGestureRecognizer *pan=[[UIPanGestureRecognizer alloc] initWithTarget:self action:@selector(move:)];
    [self.screen addGestureRecognizer:pan];
    UIHoverGestureRecognizer *hover=[[UIHoverGestureRecognizer alloc] initWithTarget:self action:@selector(move:)];
    [self.screen addGestureRecognizer:hover];
    NSURL *root=[[Roots.instance rootUrl:Roots.instance.defaultRoot] URLByAppendingPathComponent:@"data" isDirectory:YES];
    __weak typeof(self) weakSelf=self;
    self.session=[[LinpadDisplaySession alloc] initWithRoot:root identifier:self.identifier frame:^(NSData *rgba,uint32_t w,uint32_t h) {
        typeof(self) self=weakSelf;
        if (!self || !self.active) return;
        CGColorSpaceRef color=CGColorSpaceCreateDeviceRGB();
        CGDataProviderRef provider=CGDataProviderCreateWithCFData((__bridge CFDataRef)rgba);
        CGImageRef image=CGImageCreate(w,h,8,32,(size_t)w*4,color,kCGImageAlphaPremultipliedLast|kCGBitmapByteOrder32Big,provider,NULL,false,kCGRenderingIntentDefault);
        BOOL presented=image!=NULL;
        if (presented) self.screen.image=[UIImage imageWithCGImage:image];
        CGImageRelease(image); CGDataProviderRelease(provider); CGColorSpaceRelease(color);
        if (!presented) { self.status.text=@"Cannot allocate the native display image."; return; }
        self.width=w; self.height=h;
        self.status.text=[NSString stringWithFormat:@"X11 framebuffer %u×%u — software rendering",w,h];
        self.diagnostic=self.status.text;
    } status:^(NSString *text) { weakSelf.diagnostic=text; if (weakSelf.active) weakSelf.status.text=text; }];
    [self.session start];
    [NSNotificationCenter.defaultCenter addObserver:self selector:@selector(releaseKeys:) name:UIApplicationWillResignActiveNotification object:nil];
    [NSNotificationCenter.defaultCenter addObserver:self selector:@selector(foreground:) name:UIApplicationDidBecomeActiveNotification object:nil];
}
- (void)viewDidAppear:(BOOL)animated { [super viewDidAppear:animated]; self.active=YES; [self.session resume]; [self becomeFirstResponder]; }
- (void)viewDidDisappear:(BOOL)animated { [super viewDidDisappear:animated]; self.active=NO; [self.session sendEvent:@"release\n"]; [self.session pause]; }
- (void)copyInstall:(id)sender { (void)sender; UIPasteboard.generalPasteboard.string=@"sh /usr/share/linpad/install-x11.sh"; self.status.text=@"Install command copied. Run it in the Alpine terminal."; }
- (void)copyStart:(id)sender {
    (void)sender;
    NSString *app=[self.application titleForSegmentAtIndex:self.application.selectedSegmentIndex];
    UIPasteboard.generalPasteboard.string=[NSString stringWithFormat:@"sh /usr/share/linpad/x11-session.sh %@ %@",self.identifier,app];
    self.status.text=@"Start command copied. Run it in Terminal, then reopen this GUI session.";
}
- (void)copyReport:(id)sender {
    (void)sender;
    UIPasteboard.generalPasteboard.string=[NSString stringWithFormat:@"Linpad X11 experimental\nBuild: %@\nSession: %@\nGuest evidence: /tmp/linpad-x11-%@\nFramebuffer: %u×%u\n%@\nRendering: Xvfb software → XWD → UIKit; no guest GPU acceleration",NSBundle.mainBundle.infoDictionary[@"LinpadBuildRevision"] ?: @"unknown",self.identifier,self.identifier,self.width,self.height,self.diagnostic ?: @"No frame received."];
    self.status.text=@"GUI report copied. Nothing was uploaded.";
}
- (void)returnToTerminal:(id)sender { (void)sender; [self.session sendEvent:@"release\n"]; [self dismissViewControllerAnimated:YES completion:nil]; }
- (void)close:(id)sender { (void)sender; self.active=NO; self.finished=YES; [self.session stop]; [self dismissViewControllerAnimated:YES completion:nil]; }
- (void)releaseKeys:(NSNotification *)note { (void)note; [self.session sendEvent:@"release\n"]; [self.session pause]; }
- (void)foreground:(NSNotification *)note { (void)note; if (self.view.window && self.active) [self.session resume]; }
- (BOOL)point:(CGPoint)point x:(int *)x y:(int *)y {
    if (!self.width || !self.height) return NO;
    CGSize bounds=self.screen.bounds.size;
    CGFloat scale=MIN(bounds.width/self.width,bounds.height/self.height);
    if (!isfinite(scale) || scale<=0) return NO;
    CGFloat left=(bounds.width-self.width*scale)/2,top=(bounds.height-self.height*scale)/2;
    CGFloat px=(point.x-left)/scale,py=(point.y-top)/scale;
    if (!isfinite(px) || !isfinite(py) || px<0 || py<0 || px>=self.width || py>=self.height) return NO;
    *x=(int)px; *y=(int)py; return YES;
}
- (void)tap:(UITapGestureRecognizer *)gesture {
    int x,y;
    if ([self point:[gesture locationInView:self.screen] x:&x y:&y])
        [self.session sendEvent:[NSString stringWithFormat:@"click %d %d 1\n",x,y]];
    [self becomeFirstResponder];
}
- (void)move:(UIGestureRecognizer *)gesture {
    CFAbsoluteTime now=CFAbsoluteTimeGetCurrent();
    if (now-self.lastMotion<0.1) return;
    self.lastMotion=now;
    int x,y;
    if ([self point:[gesture locationInView:self.screen] x:&x y:&y])
        [self.session sendEvent:[NSString stringWithFormat:@"move %d %d\n",x,y]];
}
- (void)keys:(NSSet<UIPress *> *)presses down:(BOOL)down {
    for (unsigned pass=0; pass<2; pass++) {
        for (UIPress *press in presses) {
            NSString *key=press.key ? LinpadKeysym(press.key) : nil;
            BOOL modifier=[key hasPrefix:@"Shift_"] || [key hasPrefix:@"Control_"] || [key hasPrefix:@"Alt_"] || [key hasPrefix:@"Super_"];
            if (key && modifier==(down ? pass==0 : pass==1))
                [self.session sendEvent:[NSString stringWithFormat:@"%@ %@\n",down?@"down":@"up",key]];
        }
    }
}
- (void)pressesBegan:(NSSet<UIPress *> *)presses withEvent:(UIPressesEvent *)event { (void)event; [self keys:presses down:YES]; }
- (void)pressesEnded:(NSSet<UIPress *> *)presses withEvent:(UIPressesEvent *)event { (void)event; [self keys:presses down:NO]; }
- (void)pressesCancelled:(NSSet<UIPress *> *)presses withEvent:(UIPressesEvent *)event { (void)presses; (void)event; [self.session sendEvent:@"release\n"]; }
- (void)dealloc { [NSNotificationCenter.defaultCenter removeObserver:self]; [self.session stop]; }
@end
