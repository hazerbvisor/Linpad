// SPDX-License-Identifier: GPL-3.0-only
// Native Foundation control: no provisioned App Group or guest execution.
#import <Foundation/Foundation.h>
#import "app/AppGroup.h"
#include <assert.h>

int main(void) {
    @autoreleasepool {
        // This standalone executable has no configured App Group identifier.
        assert(SharedContainerURL() == nil);
        NSURL *support = [NSFileManager.defaultManager URLsForDirectory:NSApplicationSupportDirectory
                                                             inDomains:NSUserDomainMask].firstObject;
        NSURL *expected = [support URLByAppendingPathComponent:@"Linpad" isDirectory:YES];
        NSURL *actual = ContainerURL();
        assert(actual != nil);
        assert([actual.standardizedURL isEqual:expected.standardizedURL]);
        assert([ContainerURL() isEqual:actual]);
        puts("Storage fallback passed: private Application Support; no shared extension access");
    }
    return 0;
}
