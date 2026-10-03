// SPDX-License-Identifier: GPL-3.0-only
#import <Foundation/Foundation.h>
// Immutable copied RGBA frames. No guest pointers, UIKit or Metal dependencies.
@interface LinpadDisplaySession : NSObject
- (instancetype)initWithRoot:(NSURL *)trustedRoot identifier:(NSString *)identifier
                       frame:(void (^)(NSData *, uint32_t, uint32_t))frame
                      status:(void (^)(NSString *))status;
- (void)start;
- (void)sendEvent:(NSString *)event;
- (void)pause;
- (void)resume;
- (void)stop;
@end
