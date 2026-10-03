//
//  ViewController.m
//  iSH
//
//  Created by Theodore Dubois on 10/17/17.
//

#import "TerminalViewController.h"
#import "AppDelegate.h"
#import "AppGroup.h"
#if !ISH_LINUX && defined(GUEST_ARM64)
#import "Display/X11ViewController.h"
#endif
#import "TerminalView.h"
#import "BarButton.h"
#import "ArrowBarButton.h"
#import "UserPreferences.h"
#import "AboutViewController.h"
#import "NSObject+SaneKVO.h"
#import "LinuxInterop.h"
#include "kernel/init.h"
#include "kernel/task.h"
#include "kernel/calls.h"
#include "fs/devices.h"

@interface TerminalViewController () <UIGestureRecognizerDelegate>

@property UITapGestureRecognizer *tapRecognizer;
@property (weak, nonatomic) IBOutlet TerminalView *termView;
@property (weak, nonatomic) IBOutlet NSLayoutConstraint *bottomConstraint;

@property (weak, nonatomic) IBOutlet UIButton *tabKey;
@property (weak, nonatomic) IBOutlet UIButton *controlKey;
@property (weak, nonatomic) IBOutlet UIButton *escapeKey;
@property (strong, nonatomic) IBOutletCollection(id) NSArray *barButtons;
@property (strong, nonatomic) IBOutletCollection(id) NSArray *barControls;

@property (weak, nonatomic) IBOutlet UIInputView *barView;
@property (weak, nonatomic) IBOutlet UIStackView *bar;
@property (weak, nonatomic) IBOutlet NSLayoutConstraint *barTop;
@property (weak, nonatomic) IBOutlet NSLayoutConstraint *barBottom;
@property (weak, nonatomic) IBOutlet NSLayoutConstraint *barLeading;
@property (weak, nonatomic) IBOutlet NSLayoutConstraint *barTrailing;
@property (weak, nonatomic) IBOutlet NSLayoutConstraint *barButtonWidth;
@property (weak, nonatomic) IBOutlet NSLayoutConstraint *barHeight;

@property (weak, nonatomic) IBOutlet UIButton *infoButton;
@property (weak, nonatomic) IBOutlet UIButton *pasteButton;
@property (weak, nonatomic) IBOutlet UIButton *hideKeyboardButton;

@property int sessionPid;
@property (nonatomic) Terminal *sessionTerminal;

#if !ISH_LINUX
#ifdef GUEST_ARM64
@property LinpadX11ViewController *x11Controller;
#endif
@property UIView *startupOverlay;
@property UILabel *startupLabel;
@property NSTimer *startupTimer;
@property NSDate *startupTime;
@property NSString *startupFailure;
@property BOOL sessionRequested;
@property NSUUID *pendingReconnectUUID;
#endif

@property BOOL ignoreKeyboardMotion;
@property (nonatomic) BOOL hasExternalKeyboard;

@end

@implementation TerminalViewController

- (void)viewDidLoad {
    [super viewDidLoad];

#if !ISH_LINUX
    [self installStartupOverlay];
    [NSNotificationCenter.defaultCenter addObserver:self selector:@selector(startupChanged:)
                                               name:StartupDidChangeNotification object:nil];
#endif

    self.terminal = self.terminal;
#if ISH_LINUX
    [self.termView becomeFirstResponder];
#endif

    NSNotificationCenter *center = [NSNotificationCenter defaultCenter];
    [center addObserver:self
               selector:@selector(keyboardDidSomething:)
                   name:UIKeyboardWillChangeFrameNotification
                 object:nil];
    [center addObserver:self
               selector:@selector(keyboardDidSomething:)
                   name:UIKeyboardDidChangeFrameNotification
                 object:nil];
    [self _updateStyleFromPreferences:NO];
    
    if (UIDevice.currentDevice.userInterfaceIdiom == UIUserInterfaceIdiomPad) {
        [self.bar removeArrangedSubview:self.hideKeyboardButton];
        [self.hideKeyboardButton removeFromSuperview];
    }
    if (UIDevice.currentDevice.userInterfaceIdiom == UIUserInterfaceIdiomPhone) {
        self.barHeight.constant = 36;
    } else {
        self.barHeight.constant = 43;
    }
    
    // SF Symbols is cool
    if (@available(iOS 13, *)) {
        [self.infoButton setImage:[UIImage systemImageNamed:@"gear"] forState:UIControlStateNormal];
        [self.pasteButton setImage:[UIImage systemImageNamed:@"doc.on.clipboard"] forState:UIControlStateNormal];
        [self.hideKeyboardButton setImage:[UIImage systemImageNamed:@"keyboard.chevron.compact.down"] forState:UIControlStateNormal];
        
        [self.tabKey setTitle:nil forState:UIControlStateNormal];
        [self.tabKey setImage:[UIImage systemImageNamed:@"arrow.right.to.line.alt"] forState:UIControlStateNormal];
        [self.controlKey setTitle:nil forState:UIControlStateNormal];
        [self.controlKey setImage:[UIImage systemImageNamed:@"control"] forState:UIControlStateNormal];
        [self.escapeKey setTitle:nil forState:UIControlStateNormal];
        [self.escapeKey setImage:[UIImage systemImageNamed:@"escape"] forState:UIControlStateNormal];
    }
    
    [UserPreferences.shared observe:@[@"hideStatusBar"] options:0 owner:self usingBlock:^(typeof(self) self) {
        dispatch_async(dispatch_get_main_queue(), ^{
            [self setNeedsStatusBarAppearanceUpdate];
        });
    }];
    [UserPreferences.shared observe:@[@"colorScheme", @"theme", @"hideExtraKeysWithExternalKeyboard"]
                            options:0 owner:self usingBlock:^(typeof(self) self) {
        dispatch_async(dispatch_get_main_queue(), ^{
            [self _updateStyleFromPreferences:YES];
        });
    }];
}

- (void)dealloc {
    [NSNotificationCenter.defaultCenter removeObserver:self];
#if !ISH_LINUX
    [self.startupTimer invalidate];
    [_terminal removeObserver:self forKeyPath:@"loaded"];
    [_terminal removeObserver:self forKeyPath:@"frontendError"];
#endif
}

- (void)awakeFromNib {
    [super awakeFromNib];
#if !ISH_LINUX
    [NSNotificationCenter.defaultCenter addObserver:self
                                           selector:@selector(processExited:)
                                               name:ProcessExitedNotification
                                             object:nil];
#else
    [NSNotificationCenter.defaultCenter addObserver:self
                                           selector:@selector(kernelPanicked:)
                                               name:KernelPanicNotification
                                             object:nil];
#endif
}

- (void)viewDidAppear:(BOOL)animated {
    [super viewDidAppear:animated];
#if !ISH_LINUX
    [AppDelegate beginBoot];
    [self updateStartupDisplay];
#endif
}

#if !ISH_LINUX
- (void)installStartupOverlay {
    self.startupTime = NSDate.date;
    self.startupOverlay = [[UIView alloc] initWithFrame:self.view.bounds];
    self.startupOverlay.autoresizingMask = UIViewAutoresizingFlexibleWidth | UIViewAutoresizingFlexibleHeight;
    self.startupOverlay.backgroundColor = UIColor.systemBackgroundColor;
    UILabel *title = [UILabel new];
    title.text = @"Linpad";
    title.font = [UIFont preferredFontForTextStyle:UIFontTextStyleTitle1];
    title.textAlignment = NSTextAlignmentCenter;
    self.startupLabel = [UILabel new];
    self.startupLabel.numberOfLines = 0;
    self.startupLabel.textAlignment = NSTextAlignmentCenter;
    self.startupLabel.text = [AppDelegate bootPhase];
    UIButton *copy = [UIButton buttonWithType:UIButtonTypeSystem];
    [copy setTitle:@"Copy startup report" forState:UIControlStateNormal];
    [copy addTarget:self action:@selector(copyStartupReport:) forControlEvents:UIControlEventTouchUpInside];
    UIStackView *stack = [[UIStackView alloc] initWithArrangedSubviews:@[title, self.startupLabel, copy]];
    stack.axis = UILayoutConstraintAxisVertical;
    stack.spacing = 20;
    stack.translatesAutoresizingMaskIntoConstraints = NO;
    [self.startupOverlay addSubview:stack];
    [NSLayoutConstraint activateConstraints:@[
        [stack.centerXAnchor constraintEqualToAnchor:self.startupOverlay.centerXAnchor],
        [stack.centerYAnchor constraintEqualToAnchor:self.startupOverlay.centerYAnchor],
        [stack.widthAnchor constraintLessThanOrEqualToConstant:640],
        [stack.leadingAnchor constraintGreaterThanOrEqualToAnchor:self.startupOverlay.safeAreaLayoutGuide.leadingAnchor constant:24],
        [stack.trailingAnchor constraintLessThanOrEqualToAnchor:self.startupOverlay.safeAreaLayoutGuide.trailingAnchor constant:-24],
    ]];
    [self.view addSubview:self.startupOverlay];
    __weak typeof(self) weakSelf = self;
    self.startupTimer = [NSTimer scheduledTimerWithTimeInterval:0.5 repeats:YES block:^(NSTimer *timer) {
        (void)timer;
        [weakSelf updateStartupDisplay];
    }];
}

- (void)startupChanged:(NSNotification *)notification {
    if ([AppDelegate bootCompleted] && self.sessionRequested) {
        NSUUID *uuid = self.pendingReconnectUUID;
        self.pendingReconnectUUID = nil;
        self.sessionRequested = NO;
        if (uuid != nil)
            [self reconnectSessionFromTerminalUUID:uuid];
        else
            [self startNewSession];
    }
    [self updateStartupDisplay];
}

- (void)updateStartupDisplay {
    NSString *message = [AppDelegate bootPhase];
    if ([AppDelegate bootCompleted]) {
        if ([AppDelegate bootError] < 0) {
            message = [NSString stringWithFormat:@"Startup failed during: %@\n%@\nLinux error %d",
                       [AppDelegate bootPhase], [AppDelegate bootFailureReason] ?: @"Could not boot Alpine.", [AppDelegate bootError]];
        } else if (self.startupFailure != nil) {
            message = self.startupFailure;
        } else if (self.terminal.frontendError != nil) {
            message = [@"Terminal display failed: " stringByAppendingString:self.terminal.frontendError];
        } else if (self.terminal.loaded) {
            BOOL wasVisible = !self.startupOverlay.hidden;
            self.startupOverlay.hidden = YES;
            [self.startupTimer invalidate];
            self.startupTimer = nil;
            if (wasVisible)
                [self.termView becomeFirstResponder];
            return;
        } else {
            message = @"Loading terminal display";
        }
    }
    self.startupOverlay.hidden = NO;
    if (-self.startupTime.timeIntervalSinceNow > 20)
        message = [message stringByAppendingString:@"\nStill waiting. Copy the startup report if this does not finish."];
    self.startupLabel.text = message;
}

- (void)copyStartupReport:(id)sender {
    NSString *revision = [NSBundle.mainBundle objectForInfoDictionaryKey:@"LinpadBuildRevision"] ?: @"unknown";
    UIPasteboard.generalPasteboard.string = [NSString stringWithFormat:
        @"Linpad %@ (%@)\niPadOS %@\nBuild %@\nPhase: %@\nBoot complete: %@; error: %d\nFailure: %@\nInit executable: %@\nSession executable: %@\nTerminal ready: %@\nFrontend: %@\nStorage: %@",
        [NSBundle.mainBundle objectForInfoDictionaryKey:@"CFBundleShortVersionString"],
        [NSBundle.mainBundle objectForInfoDictionaryKey:@"CFBundleVersion"], UIDevice.currentDevice.systemVersion,
        revision, [AppDelegate bootPhase], [AppDelegate bootCompleted] ? @"yes" : @"no", [AppDelegate bootError],
        self.startupFailure ?: [AppDelegate bootFailureReason] ?: @"none",
        UserPreferences.shared.bootCommand.firstObject ?: @"none", UserPreferences.shared.launchCommand.firstObject ?: @"none",
        self.terminal.loaded ? @"yes" : @"no", self.terminal.frontendError ?: @"none",
        SharedContainerURL() != nil ? @"App Group" : @"private app sandbox"];
}
#endif

- (void)startNewSession {
#if !ISH_LINUX
    if (![AppDelegate bootCompleted]) {
        self.sessionRequested = YES;
        return;
    }
    if ([AppDelegate bootError] < 0) {
        [self updateStartupDisplay];
        return;
    }
    if (self.startupFailure != nil) {
        [self updateStartupDisplay];
        return;
    }
    self.sessionRequested = NO;
#endif
    int err = [self startSession];
    if (err < 0) {
        NSLog(@"could not start session: %d", err);
#if !ISH_LINUX
        self.startupFailure = [NSString stringWithFormat:@"Could not start the Alpine terminal (error %d).", err];
        [self updateStartupDisplay];
#else
        [self showMessage:@"could not start session" subtitle:[NSString stringWithFormat:@"error code %d", err]];
#endif
    }
}

- (void)reconnectSessionFromTerminalUUID:(NSUUID *)uuid {
#if !ISH_LINUX
    if (![AppDelegate bootCompleted]) {
        self.pendingReconnectUUID = uuid;
        self.sessionRequested = YES;
        return;
    }
    if ([AppDelegate bootError] < 0)
        return;
#endif
    self.sessionTerminal = [Terminal terminalWithUUID:uuid];
    if (self.sessionTerminal == nil)
        [self startNewSession];
}

- (NSUUID *)sessionTerminalUUID {
    return self.terminal.uuid;
}

- (int)startSession {
    NSArray<NSString *> *command = UserPreferences.shared.launchCommand;
    if (command.count == 0) {
        NSLog(@"terminal session launch command is empty");
        return _EINVAL;
    }
    NSLog(@"starting terminal session with command: %@", [command componentsJoinedByString:@" "]);

#if !ISH_LINUX
    if (pid_get_task(1) == NULL)
        return _ECHILD;
    int err = become_new_init_child();
    if (err < 0)
        return err;
    struct tty *tty;
    self.sessionTerminal = nil;
    Terminal *terminal = [Terminal createPseudoTerminal:&tty];
    if (terminal == nil) {
        NSAssert(IS_ERR(tty), @"tty should be error");
        return (int) PTR_ERR(tty);
    }
    self.sessionTerminal = terminal;
    NSString *stdioFile = [NSString stringWithFormat:@"/dev/pts/%d", tty->num];
    err = create_stdio(stdioFile.fileSystemRepresentation, TTY_PSEUDO_SLAVE_MAJOR, tty->num);
    if (err < 0)
        return err;
    tty_release(tty);

    char argv[4096];
    [Terminal convertCommand:command toArgs:argv limitSize:sizeof(argv)];
    const char *envp = "TERM=xterm-256color\0"
#if defined(GUEST_ARM64)
        "PYTHONMALLOC=malloc\0"
#endif
        ;
    err = do_execve(command[0].UTF8String, command.count, argv, envp);
    if (err < 0)
        return err;
    self.sessionPid = current->pid;
    NSLog(@"started terminal session pid %d on %@", self.sessionPid, stdioFile);
    err = task_start(current);
    if (err < 0) {
        task_discard_unstarted(current);
        current = NULL;
        self.sessionPid = 0;
        return err;
    }
#else
    const char *argv_arr[command.count + 1];
    for (NSUInteger i = 0; i < command.count; i++)
        argv_arr[i] = command[i].UTF8String;
    argv_arr[command.count] = NULL;
    const char *envp_arr[] = {
        "TERM=xterm-256color",
        NULL,
    };
    const char *const *argv = argv_arr;
    const char *const *envp = envp_arr;
    __block Terminal *terminal = nil;
    __block int sessionPid = 0;
    __block int err = 1;
    sync_do_in_workqueue(^(void (^done)(void)) {
        linux_start_session(argv[0], argv, envp, ^(int retval, int pid, nsobj_t term) {
            err = retval;
            if (term)
                terminal = CFBridgingRelease(term);
            sessionPid = pid;
            done();
        });
    });
    NSAssert(err <= 0, @"session start did not finish??");
    if (err < 0)
        return err;
    self.sessionTerminal = terminal;
    self.sessionPid = sessionPid;
    NSLog(@"started terminal session pid %d", self.sessionPid);
#endif
    return 0;
}

#if !ISH_LINUX
- (void)processExited:(NSNotification *)notif {
    int pid = [notif.userInfo[@"pid"] intValue];
    if (pid == 1) {
        self.startupFailure = [NSString stringWithFormat:@"Alpine init exited (code %@). Close and reopen Linpad.", notif.userInfo[@"code"]];
        [self updateStartupDisplay];
        return;
    }
    if (pid != self.sessionPid)
        return;

    int code = [notif.userInfo[@"code"] intValue];
    NSLog(@"terminal session pid %d exited with code %d", pid, code);
    [self.sessionTerminal destroy];
    self.sessionTerminal = nil;
    current = NULL; // it's been freed
    [self startNewSession];
}
#endif

#if ISH_LINUX
- (void)kernelPanicked:(NSNotification *)notif {
    UIAlertController *alert = [UIAlertController alertControllerWithTitle:@"panik" message:notif.userInfo[@"message"] preferredStyle:UIAlertControllerStyleAlert];
    [alert addAction:[UIAlertAction actionWithTitle:@"k" style:UIAlertActionStyleDefault handler:nil]];
    [self presentViewController:alert animated:YES completion:nil];
}
#endif

- (void)showMessage:(NSString *)message subtitle:(NSString *)subtitle {
    dispatch_async(dispatch_get_main_queue(), ^{
        UIAlertController *alert = [UIAlertController alertControllerWithTitle:message message:subtitle preferredStyle:UIAlertControllerStyleAlert];
        [alert addAction:[UIAlertAction actionWithTitle:@"k"
                                                  style:UIAlertActionStyleDefault
                                                handler:nil]];
        [self presentViewController:alert animated:YES completion:nil];
    });
}

- (void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context {
#if !ISH_LINUX
    if (object == self.terminal) {
        [self updateStartupDisplay];
        return;
    }
#endif
    if (object == [UserPreferences shared]) {
        [self _updateStyleFromPreferences:YES];
    } else {
        [super observeValueForKeyPath:keyPath ofObject:object change:change context:context];
    }
}

- (void)_updateStyleFromPreferences:(BOOL)animated {
    NSAssert(NSThread.isMainThread, @"This method needs to be called on the main thread");
    NSTimeInterval duration = animated ? 0.1 : 0;
    [UIView animateWithDuration:duration animations:^{
        self.view.backgroundColor = [[UIColor alloc] ish_initWithHexString:UserPreferences.shared.palette.backgroundColor];
        UIKeyboardAppearance keyAppearance = UserPreferences.shared.keyboardAppearance;
        self.termView.keyboardAppearance = keyAppearance;
        for (BarButton *button in self.barButtons) {
            button.keyAppearance = keyAppearance;
        }
        UIColor *tintColor = keyAppearance == UIKeyboardAppearanceLight ? UIColor.blackColor : UIColor.whiteColor;
        for (UIControl *control in self.barControls) {
            control.tintColor = tintColor;
        }
    }];
    UIView *oldBarView = self.termView.inputAccessoryView;
    if (UserPreferences.shared.hideExtraKeysWithExternalKeyboard && self.hasExternalKeyboard) {
        self.termView.inputAccessoryView = nil;
    } else {
        self.termView.inputAccessoryView = self.barView;
    }
    if (self.termView.inputAccessoryView != oldBarView && self.termView.isFirstResponder) {
        dispatch_async(dispatch_get_main_queue(), ^{
            self.ignoreKeyboardMotion = YES; // avoid infinite recursion
            [self.termView reloadInputViews];
            self.ignoreKeyboardMotion = NO;
        });
    }
}
- (void)_updateStyleAnimated {
    [self _updateStyleFromPreferences:YES];
}

- (UIStatusBarStyle)preferredStatusBarStyle {
    return UserPreferences.shared.statusBarStyle;
}

- (BOOL)prefersStatusBarHidden {
    return UserPreferences.shared.hideStatusBar;
}

- (void)keyboardDidSomething:(NSNotification *)notification {
    if (self.ignoreKeyboardMotion)
        return;

    CGRect screenKeyboardFrame = [notification.userInfo[UIKeyboardFrameEndUserInfoKey] CGRectValue];
    UIScreen *screen = UIScreen.mainScreen;
    // notification.object is nil before iOS 16.1 and the correct UIScreen after iOS 16.1
    if (notification.object != nil)
        screen = notification.object;
    CGRect keyboardFrame = [self.view convertRect:screenKeyboardFrame fromCoordinateSpace:screen.coordinateSpace];
    if (CGRectEqualToRect(keyboardFrame, CGRectZero))
        return;
    CGRect intersection = CGRectIntersection(keyboardFrame, self.view.bounds);
    keyboardFrame = intersection;
    NSLog(@"%@ %@", notification.name, @(keyboardFrame));
    self.hasExternalKeyboard = keyboardFrame.size.height < 100;
    CGFloat pad = CGRectGetMaxY(self.view.bounds) - CGRectGetMinY(keyboardFrame);
    // The keyboard appears to be undocked. This means it can either be split or
    // truly floating. In the former case we want to keep the pad, but in the
    // latter we should fall back to the input accessory view instead of the
    // keyboard.
    if (pad != keyboardFrame.size.height && keyboardFrame.size.width != UIScreen.mainScreen.bounds.size.width) {
        pad = MAX(self.view.safeAreaInsets.bottom, self.termView.inputAccessoryView.frame.size.height);
    }
    // NSLog(@"pad %f", pad);
    self.bottomConstraint.constant = pad;

    BOOL initialLayout = self.termView.needsUpdateConstraints;
    [self.view setNeedsUpdateConstraints];
    if (!initialLayout) {
        // if initial layout hasn't happened yet, the terminal view is going to be at a really weird place, so animating it is going to look really bad
        NSNumber *interval = notification.userInfo[UIKeyboardAnimationDurationUserInfoKey];
        NSNumber *curve = notification.userInfo[UIKeyboardAnimationCurveUserInfoKey];
        [UIView animateWithDuration:interval.doubleValue
                              delay:0
                            options:curve.integerValue << 16
                         animations:^{
                             [self.view layoutIfNeeded];
                         }
                         completion:nil];
    }
}

- (void)setHasExternalKeyboard:(BOOL)hasExternalKeyboard {
    _hasExternalKeyboard = hasExternalKeyboard;
    [self _updateStyleFromPreferences:YES];
}

- (void)prepareForSegue:(UIStoryboardSegue *)segue sender:(id)sender {
    if ([segue.identifier isEqualToString:@"embed"]) {
        // You might want to check if this is your embed segue here
        // in case there are other segues triggered from this view controller.
        segue.destinationViewController.view.translatesAutoresizingMaskIntoConstraints = NO;
    }
}

- (void)traitCollectionDidChange:(UITraitCollection *)previousTraitCollection {
    // Hack to resolve a layering mismatch between the UI and preferences.
    if (@available(iOS 12.0, *)) {
        if (previousTraitCollection.userInterfaceStyle != self.traitCollection.userInterfaceStyle) {
            // Ensure that the relevant things listening for this will update.
            UserPreferences.shared.colorScheme = UserPreferences.shared.colorScheme;
        }
    }
}

#pragma mark Bar

- (IBAction)showAbout:(id)sender {
#if !ISH_LINUX && defined(GUEST_ARM64)
    if (![sender isKindOfClass:UIGestureRecognizer.class]) {
        UIAlertController *menu=[UIAlertController alertControllerWithTitle:@"Linpad" message:nil preferredStyle:UIAlertControllerStyleActionSheet];
        [menu addAction:[UIAlertAction actionWithTitle:@"Settings" style:UIAlertActionStyleDefault handler:^(UIAlertAction *action) {
            (void)action; [self presentSettings:nil];
        }]];
        [menu addAction:[UIAlertAction actionWithTitle:@"Linux GUI (experimental)" style:UIAlertActionStyleDefault handler:^(UIAlertAction *action) {
            (void)action;
            if (!self.x11Controller || self.x11Controller.finished) self.x11Controller=[LinpadX11ViewController new];
            UINavigationController *nav=[[UINavigationController alloc] initWithRootViewController:self.x11Controller];
            nav.modalPresentationStyle=UIModalPresentationFullScreen;
            [self.termView resignFirstResponder];
            [self presentViewController:nav animated:YES completion:nil];
        }]];
        [menu addAction:[UIAlertAction actionWithTitle:@"Cancel" style:UIAlertActionStyleCancel handler:nil]];
        menu.popoverPresentationController.sourceView=self.infoButton;
        menu.popoverPresentationController.sourceRect=self.infoButton.bounds;
        [self presentViewController:menu animated:YES completion:nil];
        return;
    }
#endif
    [self presentSettings:sender];
}

- (void)presentSettings:(id)sender {
    UINavigationController *navigationController = [[UIStoryboard storyboardWithName:@"About" bundle:nil] instantiateInitialViewController];
    if ([sender isKindOfClass:[UIGestureRecognizer class]]) {
        UIGestureRecognizer *recognizer = sender;
        if (recognizer.state == UIGestureRecognizerStateBegan) {
            AboutViewController *aboutViewController = (AboutViewController *) navigationController.topViewController;
            aboutViewController.includeDebugPanel = YES;
        } else {
            return;
        }
    }
    [self presentViewController:navigationController animated:YES completion:nil];
    [self.termView resignFirstResponder];
}

- (void)resizeBar {
    CGSize bar = self.barView.bounds.size;
    // set sizing parameters on bar
    // numbers stolen from iVim and modified somewhat
    if (UIDevice.currentDevice.userInterfaceIdiom == UIUserInterfaceIdiomPhone) {
        // phone
        [self setBarHorizontalPadding:6 verticalPadding:6 buttonWidth:32];
    } else if (bar.width >= 450) {
        // wide ipad
        [self setBarHorizontalPadding:15 verticalPadding:8 buttonWidth:43];
    } else {
        // narrow ipad (slide over)
        [self setBarHorizontalPadding:10 verticalPadding:8 buttonWidth:36];
    }
    [UIView performWithoutAnimation:^{
        [self.barView layoutIfNeeded];
    }];
}

- (void)setBarHorizontalPadding:(CGFloat)horizontal verticalPadding:(CGFloat)vertical buttonWidth:(CGFloat)buttonWidth {
    self.barLeading.constant = self.barTrailing.constant = horizontal;
    self.barTop.constant = self.barBottom.constant = vertical;
    self.barButtonWidth.constant = buttonWidth;
}

- (IBAction)pressEscape:(id)sender {
    [self pressKey:@"\x1b"];
}
- (IBAction)pressTab:(id)sender {
    [self pressKey:@"\t"];
}
- (void)pressKey:(NSString *)key {
    [self.termView insertText:key];
}

- (IBAction)pressControl:(id)sender {
    self.controlKey.selected = !self.controlKey.selected;
}
    
- (IBAction)pressArrow:(ArrowBarButton *)sender {
    switch (sender.direction) {
        case ArrowUp: [self pressKey:[self.terminal arrow:'A']]; break;
        case ArrowDown: [self pressKey:[self.terminal arrow:'B']]; break;
        case ArrowLeft: [self pressKey:[self.terminal arrow:'D']]; break;
        case ArrowRight: [self pressKey:[self.terminal arrow:'C']]; break;
        case ArrowNone: break;
    }
}

- (void)switchTerminal:(UIKeyCommand *)sender {
    unsigned i = (unsigned) sender.input.integerValue;
    if (i == 7)
        self.terminal = self.sessionTerminal;
    else
        self.terminal = [Terminal terminalWithType:TTY_CONSOLE_MAJOR number:i];
}

- (void)increaseFontSize:(UIKeyCommand *)command {
    self.termView.overrideFontSize = self.termView.effectiveFontSize + 1;
}
- (void)decreaseFontSize:(UIKeyCommand *)command {
    self.termView.overrideFontSize = self.termView.effectiveFontSize - 1;
}
- (void)resetFontSize:(UIKeyCommand *)command {
    self.termView.overrideFontSize = 0;
}

- (NSArray<UIKeyCommand *> *)keyCommands {
    static NSMutableArray<UIKeyCommand *> *commands = nil;
    if (commands == nil) {
        commands = [NSMutableArray new];
        for (unsigned i = 1; i <= 7; i++) {
            [commands addObject:
             [UIKeyCommand keyCommandWithInput:[NSString stringWithFormat:@"%d", i]
                                 modifierFlags:UIKeyModifierCommand|UIKeyModifierAlternate|UIKeyModifierShift
                                        action:@selector(switchTerminal:)]];
        }
        [commands addObject:
         [UIKeyCommand keyCommandWithInput:@"+"
                             modifierFlags:UIKeyModifierCommand
                                    action:@selector(increaseFontSize:)
                      discoverabilityTitle:@"Increase Font Size"]];
        [commands addObject:
         [UIKeyCommand keyCommandWithInput:@"="
                             modifierFlags:UIKeyModifierCommand
                                    action:@selector(increaseFontSize:)]];
        [commands addObject:
         [UIKeyCommand keyCommandWithInput:@"-"
                             modifierFlags:UIKeyModifierCommand
                                    action:@selector(decreaseFontSize:)
                      discoverabilityTitle:@"Decrease Font Size"]];
        [commands addObject:
         [UIKeyCommand keyCommandWithInput:@"0"
                             modifierFlags:UIKeyModifierCommand
                                    action:@selector(resetFontSize:)
                      discoverabilityTitle:@"Reset Font Size"]];
        [commands addObject:
         [UIKeyCommand keyCommandWithInput:@","
                             modifierFlags:UIKeyModifierCommand
                                    action:@selector(showAbout:)
                      discoverabilityTitle:@"Settings"]];
    }
    return commands;
}

- (void)setTerminal:(Terminal *)terminal {
#if !ISH_LINUX
    [_terminal removeObserver:self forKeyPath:@"loaded"];
    [_terminal removeObserver:self forKeyPath:@"frontendError"];
#endif
    _terminal = terminal;
    self.termView.terminal = self.terminal;
#if !ISH_LINUX
    [_terminal addObserver:self forKeyPath:@"loaded" options:0 context:nil];
    [_terminal addObserver:self forKeyPath:@"frontendError" options:0 context:nil];
    [self updateStartupDisplay];
#endif
}

- (void)setSessionTerminal:(Terminal *)sessionTerminal {
    if (_terminal == _sessionTerminal)
        self.terminal = sessionTerminal;
    _sessionTerminal = sessionTerminal;
}

@end

@interface BarView : UIInputView
@property (weak) IBOutlet TerminalViewController *terminalViewController;
@property (nonatomic) IBInspectable BOOL allowsSelfSizing;
@end
@implementation BarView
@dynamic allowsSelfSizing;

- (void)layoutSubviews {
    [self.terminalViewController resizeBar];
}

@end
