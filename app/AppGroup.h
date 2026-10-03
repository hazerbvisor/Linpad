//
//  AppGroup.h
//  iSH
//
//  Created by Theodore Dubois on 2/28/20.
//

#import <Foundation/Foundation.h>

// Shared storage requires a provisioned App Group; otherwise returns nil.
NSURL * _Nullable SharedContainerURL(void);
// Main app storage falls back to its own Application Support sandbox.
NSURL * _Nullable ContainerURL(void);
