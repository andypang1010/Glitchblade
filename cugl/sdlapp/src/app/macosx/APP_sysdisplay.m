/*
 * SDL_app:  An all-in-one library for packing SDL applications.
 * Copyright (C) 2022-2023 Walker M. White
 *
 * This library is built on the assumption that an application built for SDL
 * will contain its own versions of the SDL libraries (either statically linked
 * or packaged with a specific set of dynamic libraries).  While this is not
 * considered the right way to do it on Unix, it makes one step installation
 * easier for Mac and Windows. It is also the only way to create SDL apps for
 * mobile devices.
 *
 * SDL License:
 *
 * This software is provided 'as-is', without any express or implied
 * warranty.  In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 */
#include "SDL_app.h"

#import <Foundation/Foundation.h>
#import <CoreGraphics/CoreGraphics.h>
#if TARGET_OS_MACCATALYST
    #import <UIKit/UIKit.h>
#else
    #import <AppKit/AppKit.h>
#endif

/**
 * System dependent version of SDL_GetDisplayPixelBounds
 *
 * @param displayIndex  The display to query
 * @param rect          Rectangle to store the display bounds
 */
void APP_SYS_GetDisplayPixelBounds(int displayIndex, SDL_Rect *rect) {
@autoreleasepool {
    if (rect == NULL) {
        return;
    }
    
#if TARGET_OS_MACCATALYST
    UIScreen* screen;
    if (displayIndex == 0) {
        screen = [UIScreen mainScreen];
    } else if (displayIndex < UIScreen.screens.count) {
        screen = UIScreen.screens[displayIndex];
    } else {
        screen = nil;
    }
#else
    NSScreen* screen;
    if (displayIndex == 0) {
        screen = [NSScreen mainScreen];
    } else if (displayIndex < NSScreen.screens.count) {
        screen = NSScreen.screens[displayIndex];
    } else {
        screen = nil;
    }
#endif
    if (screen == nil) {
        SDL_GetDisplayBounds(displayIndex,rect);
        return;
    }

#if TARGET_OS_MACCATALYST
    CGRect displayRect = [screen bounds];
    
    // Get pixel density
    CGRect screenRect  = [screen nativeBounds];
    if (displayRect.size.width > displayRect.size.height) {
        CGFloat temp = screenRect.size.width;
        screenRect.size.width  = screenRect.size.height;
        screenRect.size.height = temp;
    }

    float sx = screenRect.size.width/displayRect.size.width;
    float sy = screenRect.size.height/displayRect.size.height;
#else
    CGRect displayRect = [screen frame];

    // Get pixel density
    float sx = [screen backingScaleFactor];
    float sy = sx;
#endif

    rect->x = displayRect.origin.x*sx;
    rect->y = displayRect.origin.y*sy;
    rect->w = displayRect.size.width*sx;
    rect->h = displayRect.size.height*sy;
}
}

/**
 * System dependent version of APP_GetDisplaySafeBounds
 *
 * @param displayIndex  The display to query
 * @param rect          Rectangle to store the display bounds
 */
void APP_SYS_GetDisplaySafeBounds(int displayIndex, SDL_Rect *rect) {
@autoreleasepool {
    if (rect == NULL) {
        return;
    } else if (displayIndex != 0) {
        APP_SYS_GetDisplayPixelBounds(displayIndex,rect);
        return;
    }

#if TARGET_OS_MACCATALYST
    UIScreen* screen = [UIScreen mainScreen];
    CGRect displayRect = [screen bounds];

    // Get pixel density
    CGRect screenRect  = [screen nativeBounds];
    if (displayRect.size.width > displayRect.size.height) {
        CGFloat temp = screenRect.size.width;
        screenRect.size.width = screenRect.size.height;
        screenRect.size.height = temp;
    }
    
    float sx = screenRect.size.width/displayRect.size.width;
    float sy = screenRect.size.height/displayRect.size.height;

    // Pull out safe area
    if (@available(iOS 11.0, *)) {
        NSArray<UIWindow *> *windows = [[UIApplication sharedApplication] windows];
        if (windows.count > 0) {
            UIWindow* main = windows[0];
            displayRect.origin.x += main.safeAreaInsets.left;
            displayRect.size.width -= (main.safeAreaInsets.left+main.safeAreaInsets.right);
            displayRect.origin.y += main.safeAreaInsets.bottom;
            displayRect.size.height -= (main.safeAreaInsets.top+main.safeAreaInsets.bottom);
        }
    }
#else
    NSScreen* screen = [NSScreen mainScreen];
    CGRect displayRect = [screen visibleFrame]; //[screen frame];

    // Get pixel density
    float sx = [screen backingScaleFactor];
    float sy = sx;
#endif
    
    rect->x = displayRect.origin.x*sx;
    rect->y = displayRect.origin.y*sy;
    rect->w = displayRect.size.width*sx;
    rect->h = displayRect.size.height*sy;

}
}

/**
 * System dependent version of APP_CheckDisplayNotch
 *
 * @param displayIndex  The display to query
 *
 * @return 1 if this device has a notch, 0 otherwise
 */
int APP_SYS_CheckDisplayNotch(int displayIndex) {
    return 0;
}

/**
 * System dependent version of APP_GetPixelDensity
 *
 * @param displayIndex  The display to query
 *
 * @return the number of pixels for each point.
 */
float APP_SYS_GetPixelDensity(int displayIndex) {
@autoreleasepool {

#if TARGET_OS_MACCATALYST
    UIScreen* screen;
    if (displayIndex == 0) {
        screen = [UIScreen mainScreen];
    } else if (displayIndex < UIScreen.screens.count) {
        screen = UIScreen.screens[displayIndex];
    } else {
        screen = nil;
    }
#else
    NSScreen* screen;
    if (displayIndex == 0) {
        screen = [NSScreen mainScreen];
    } else if (displayIndex < NSScreen.screens.count) {
        screen = NSScreen.screens[displayIndex];
    } else {
        screen = nil;
    }
#endif
    
    if (screen == nil) {
        return -1;
    }

    
#if TARGET_OS_MACCATALYST
    CGRect screenRect  = [screen nativeBounds];
    CGRect displayRect = [screen bounds];
    
    if (displayRect.size.width > displayRect.size.height) {
        CGFloat temp = screenRect.size.width;
        screenRect.size.width = screenRect.size.height;
        screenRect.size.height= temp;
    }
    
    float w = (float)screenRect.size.width/displayRect.size.width;
    float h = (float)screenRect.size.height/displayRect.size.height;
    return h < w ? h : w;
#else
    return [screen backingScaleFactor];
#endif
}
}
/**
 * System dependent version of APP_GetDeviceOrientation
 *
 * @param displayIndex  The display to query
 *
 * @return the current device orientation.
 */
SDL_DisplayOrientation APP_SYS_GetDeviceOrientation(int displayIndex) {
    return SDL_ORIENTATION_UNKNOWN;
}

/**
 * System dependent version of APP_GetDefaultOrientation
 *
 * @param displayIndex  The display to query
 *
 * @return the default orientation of this device.
 */
SDL_DisplayOrientation APP_SYS_GetDefaultOrientation(int displayIndex) {
    return SDL_ORIENTATION_UNKNOWN;
}

/* vi: set ts=4 sw=4 expandtab: */
