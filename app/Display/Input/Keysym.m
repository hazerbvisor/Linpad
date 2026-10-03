// SPDX-License-Identifier: GPL-3.0-only
#import "Keysym.h"
NSString *LinpadKeysym(UIKey *key) {
    switch (key.keyCode) {
        case UIKeyboardHIDUsageKeyboardReturnOrEnter: return @"Return";
        case UIKeyboardHIDUsageKeyboardEscape: return @"Escape";
        case UIKeyboardHIDUsageKeyboardDeleteOrBackspace: return @"BackSpace";
        case UIKeyboardHIDUsageKeyboardTab: return @"Tab";
        case UIKeyboardHIDUsageKeyboardSpacebar: return @"space";
        case UIKeyboardHIDUsageKeyboardLeftArrow: return @"Left";
        case UIKeyboardHIDUsageKeyboardRightArrow: return @"Right";
        case UIKeyboardHIDUsageKeyboardUpArrow: return @"Up";
        case UIKeyboardHIDUsageKeyboardDownArrow: return @"Down";
        case UIKeyboardHIDUsageKeyboardHome: return @"Home";
        case UIKeyboardHIDUsageKeyboardEnd: return @"End";
        case UIKeyboardHIDUsageKeyboardPageUp: return @"Page_Up";
        case UIKeyboardHIDUsageKeyboardPageDown: return @"Page_Down";
        case UIKeyboardHIDUsageKeyboardDeleteForward: return @"Delete";
        case UIKeyboardHIDUsageKeyboardLeftControl: return @"Control_L";
        case UIKeyboardHIDUsageKeyboardRightControl: return @"Control_R";
        case UIKeyboardHIDUsageKeyboardLeftShift: return @"Shift_L";
        case UIKeyboardHIDUsageKeyboardRightShift: return @"Shift_R";
        case UIKeyboardHIDUsageKeyboardLeftAlt: return @"Alt_L";
        case UIKeyboardHIDUsageKeyboardRightAlt: return @"Alt_R";
        case UIKeyboardHIDUsageKeyboardLeftGUI: return @"Super_L";
        case UIKeyboardHIDUsageKeyboardRightGUI: return @"Super_R";
        default: break;
    }
    if (key.keyCode>=UIKeyboardHIDUsageKeyboardF1 && key.keyCode<=UIKeyboardHIDUsageKeyboardF12)
        return [NSString stringWithFormat:@"F%ld",(long)(key.keyCode-UIKeyboardHIDUsageKeyboardF1+1)];
    NSString *text=key.charactersIgnoringModifiers.lowercaseString;
    if (text.length==1) {
        unichar c=[text characterAtIndex:0];
        if ((c>='a' && c<='z') || (c>='0' && c<='9')) return text;
        if (c>=32 && !CFStringIsSurrogateHighCharacter(c) && !CFStringIsSurrogateLowCharacter(c))
            return [NSString stringWithFormat:@"U%04X",(unsigned)c];
    }
    return nil;
}
