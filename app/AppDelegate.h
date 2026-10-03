//
//  AppDelegate.h
//  iSH
//
//  Created by Theodore Dubois on 10/17/17.
//

#import <UIKit/UIKit.h>

@interface AppDelegate : UIResponder <UIApplicationDelegate>

@property (strong, nonatomic) UIWindow *window;
- (void)exitApp;

#if !ISH_LINUX
+ (int)bootError;
+ (BOOL)bootCompleted;
+ (NSString *)bootPhase;
+ (NSString *)bootFailureReason;
+ (void)beginBoot;
#endif

@end

#if !ISH_LINUX
extern NSString *const ProcessExitedNotification;
extern NSString *const StartupDidChangeNotification;
#else
extern NSString *const KernelPanicNotification;
#endif
