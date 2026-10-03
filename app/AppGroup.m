//
//  AppGroup.m
//  iSH
//
//  Created by Theodore Dubois on 2/28/20.
//

#import "AppGroup.h"

NSURL *SharedContainerURL(void) {
    NSString *group = [NSBundle.mainBundle objectForInfoDictionaryKey:@"LinpadAppGroupIdentifier"];
    if (![group isKindOfClass:NSString.class] || group.length == 0 || [group containsString:@"$("])
        return nil;
    // The OS checks the actual signature's entitlement. A configured identifier
    // alone never grants access outside the sandbox.
    return [NSFileManager.defaultManager containerURLForSecurityApplicationGroupIdentifier:group];
}

NSURL *ContainerURL(void) {
    static NSURL *container;
    static dispatch_once_t once;
    dispatch_once(&once, ^{
        container = SharedContainerURL();
        if (container == nil) {
            NSURL *support = [NSFileManager.defaultManager URLsForDirectory:NSApplicationSupportDirectory
                                                                 inDomains:NSUserDomainMask].firstObject;
            container = [support URLByAppendingPathComponent:@"Linpad" isDirectory:YES];
            NSLog(@"[Linpad] App Group unavailable; using private app storage");
        }
    });
    return container;
}
