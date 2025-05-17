/*
  Simple DirectMedia Layer Extensions
  Copyright (C) 2022 Walker White

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/
#include "SDL_app.h"

#import <Foundation/Foundation.h>
#import <CoreGraphics/CoreGraphics.h>
#import <UIKit/UIKit.h>

/**
 * System dependent version of SDL_GetDisplayPixelBounds
 *
 * @param displayIndex  The display to query
 * @param rect          Rectangle to store the display bounds
 */
void APP_SYS_GetDisplayPixelBounds(int displayIndex, SDL_Rect *rect) {
@autoreleasepool {
	CGRect displayRect;
	UIScreen* screen;
    if (displayIndex == 0) {
        screen = [UIScreen mainScreen];
    } else if (displayIndex < UIScreen.screens.count) {
        screen = UIScreen.screens[displayIndex];
    } else {
        screen = nil;
    }

   if (screen != nil) {
        displayRect = [screen bounds];
        rect->x = displayRect.origin.x;
        rect->y = displayRect.origin.y;
        rect->w = displayRect.size.width;
        rect->h = displayRect.size.height;
        
		// Convert to pixels
		CGRect screenRect  = [screen nativeBounds];    
		if (displayRect.size.width > displayRect.size.height) {
			CGFloat temp = screenRect.size.width;
			screenRect.size.width  = screenRect.size.height;
			screenRect.size.height = temp;
		}
	
		float sx = screenRect.size.width/displayRect.size.width;
		float sy = screenRect.size.height/displayRect.size.height;
		rect->x *= sx;
		rect->w *= sx;
		rect->y *= sy;
		rect->h *= sy;
    } else {
        SDL_GetDisplayBounds(displayIndex,rect);
    }
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
	if (displayIndex != 0) {
        APP_SYS_GetDisplayPixelBounds(displayIndex,rect);
        return;
    }

    CGRect displayRect;
    UIScreen* screen;
    screen = [UIScreen mainScreen];
    
    // Convert to SDL_Rect
    displayRect = [screen bounds];
    rect->x = displayRect.origin.x;
    rect->y = displayRect.origin.y;
    rect->w = displayRect.size.width;
    rect->h = displayRect.size.height;
    
    if (@available(iOS 11.0, *)) {
        NSArray<UIWindow *> *windows = [[UIApplication sharedApplication] windows];
        if (windows.count > 0) {
            UIWindow* main = windows[0];
            rect->x += main.safeAreaInsets.left;
            rect->w -= (main.safeAreaInsets.left+main.safeAreaInsets.right);
            rect->y += main.safeAreaInsets.bottom;
            rect->h -= (main.safeAreaInsets.top+main.safeAreaInsets.bottom);
        }
    }

	// Convert to pixels
	CGRect screenRect  = [screen nativeBounds];    
	if (displayRect.size.width > displayRect.size.height) {
		CGFloat temp = screenRect.size.width;
		screenRect.size.width = screenRect.size.height;
		screenRect.size.height = temp;
	}
	
	float sx = screenRect.size.width/displayRect.size.width;
	float sy = screenRect.size.height/displayRect.size.height;
    rect->x *= sx;
    rect->w *= sx;
    rect->y *= sy;
    rect->h *= sy;
}
}

/**
 * System dependent version of SDL_CheckDisplayNotch
 *
 * @param displayIndex  The display to query
 *
 * @return 1 if this device has a notch, 0 otherwise
 */
int APP_SYS_CheckDisplayNotch(int displayIndex) {
	if (@available(iOS 11.0, *)) {
        NSArray<UIWindow *> *windows = [[UIApplication sharedApplication] windows];
        if (windows.count > 0) {
            UIWindow* main = windows[0];
           	return main.safeAreaInsets.bottom > 0 ? 1 : 0;
        }
    }
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
    UIScreen* screen;
    if (displayIndex == 0) {
        screen = [UIScreen mainScreen];
    } else if (displayIndex < UIScreen.screens.count) {
        screen = UIScreen.screens[displayIndex];
    } else {
        screen = nil;
    }
    
    if (screen == nil) {
        return -1;
    }
    
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
}

/**
 * System dependent version of APP_GetDeviceOrientation
 *
 * @param displayIndex  The display to query
 *
 * @return the current device orientation.
 */
SDL_DisplayOrientation APP_SYS_GetDeviceOrientation(int displayIndex) {
	if (displayIndex != 0) {
		return SDL_ORIENTATION_UNKNOWN;
	}

	switch ([[UIDevice currentDevice] orientation]) {
		case UIDeviceOrientationUnknown:
			return SDL_ORIENTATION_UNKNOWN;
		case UIDeviceOrientationPortrait:
			return SDL_ORIENTATION_PORTRAIT;
		case UIDeviceOrientationPortraitUpsideDown:
			return SDL_ORIENTATION_PORTRAIT_FLIPPED;
		case UIDeviceOrientationLandscapeLeft:
			return SDL_ORIENTATION_LANDSCAPE;
		case UIDeviceOrientationLandscapeRight:
			return SDL_ORIENTATION_LANDSCAPE_FLIPPED;
		case UIDeviceOrientationFaceUp:
			return SDL_ORIENTATION_UNKNOWN;
		case UIDeviceOrientationFaceDown:
			return SDL_ORIENTATION_UNKNOWN;
	}
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
	if (displayIndex != 0) {
		return SDL_ORIENTATION_UNKNOWN;
	}

    return SDL_ORIENTATION_PORTRAIT;
}

/* vi: set ts=4 sw=4 expandtab: */
