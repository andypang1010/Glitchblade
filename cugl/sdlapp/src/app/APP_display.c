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
#include "APP_sysdisplay.h"

/** Macro for error checking */
#define CHECK_DISPLAY_INDEX(displayIndex, retval) \
    if (SDL_GetNumVideoDisplays() == 0) { \
        SDL_SetError("Video subsystem has not been initialized"); \
        return retval; \
    } \
    if (displayIndex < 0 || displayIndex >= SDL_GetNumVideoDisplays()) { \
        SDL_SetError("displayIndex must be in the range 0 - %d", \
                     SDL_GetNumVideoDisplays() - 1); \
        return retval; \
    }


/**
 * Returns the usable full screen resolution for this display
 *
 * Usable is a subjective term defined by the operating system. In general, it
 * means the full screen minus any space used by important user interface
 * elements, like a status bar (iPhone), menu bar (OS X), or task bar (Windows).
 *
 * Because the usable bounds depends on orientation, the bounds computed will
 * use the current device orientation. If the orientation is unknown or on face
 * (either face-up or face-down), this will use the current orientation of the
 * display (not the device).
 *
 * The values stored in the rectangle represent pixels.
 *
 * @param displayIndex  The display to query
 * @param rect          Rectangle to store the display bounds
 *
 * @return 0 on success; -1 if on failure (displayIndex is invalid)
 */
int APP_GetDisplayPixelBounds(int displayIndex, SDL_Rect *rect) {
	CHECK_DISPLAY_INDEX(displayIndex, -1);
	if (rect != NULL) {
		APP_SYS_GetDisplayPixelBounds(displayIndex,rect);
	}
	return rect != NULL ? 0 : -1;
}

/**
 * Returns the usable full screen resolution for this display
 *
 * Usable is a subjective term defined by the operating system. In general, it
 * means the full screen minus any space used by important user interface
 * elements, like a status bar (iPhone), menu bar (OS X), or task bar (Windows).
 *
 * Because the usable bounds depends on orientation, the bounds computed will
 * use the current device orientation. If the orientation is unknown or on face
 * (either face-up or face-down), this will use the current orientation of the
 * display (not the device).
 *
 * The values stored in the rectangle represent pixels.
 *
 * @param displayIndex  The display to query
 * @param rect          Rectangle to store the display bounds
 *
 * @return 0 on success; -1 if on failure (displayIndex is invalid)
 */
int APP_GetDisplaySafeBounds(int displayIndex, SDL_Rect *rect) {
	CHECK_DISPLAY_INDEX(displayIndex, -1);
	if (rect != NULL) {
		APP_SYS_GetDisplaySafeBounds(displayIndex,rect);
	}
	return rect != NULL ? 0 : -1;
}

/**
 * Returns 1 if this device has a notch, 0 otherwise
 *
 * Notched devices are edgeless smartphones or tablets that include at dedicated
 * area in the screen for a camera. Examples include modern iPhones
 *
 * If a device is notched you should call {@link APP_GetDisplaySafeBounds()}
 * before laying out UI elements. It is acceptable to animate and draw
 * backgrounds behind the notch, but it is not acceptable to place UI elements
 * outside of these bounds.
 *
 * @param displayIndex  The display to query
 *
 * @return 1 if this device has a notch, 0 otherwise
 */
int APP_CheckDisplayNotch(int displayIndex) {
	CHECK_DISPLAY_INDEX(displayIndex, 0);
	return APP_SYS_CheckDisplayNotch(displayIndex);
}

/**
 * Returns the number of pixels for each point.
 *
 * A point is a logical screen pixel. If you are using a traditional display,
 * points and pixels are the same. However, on Retina displays and other high
 * dpi monitors, they may be different. In particular, the number of pixels
 * per point is a scaling factor times the point.
 *
 * You should never need to use these scaling factor for anything, as it is not
 * useful for determining anything other than whether a high DPI display is
 * present. It does not necessarily refer to physical pixel on the screen. In
 * some cases (macOS X Retina displays), it refers to the pixel density of the
 * backing framebuffer, which may be different from the physical framebuffer.
 *
 * @param displayIndex  The display to query
 *
 * @return the number of pixels for each point (-1 if displayIndex invalid)
 */
float APP_GetDisplayPixelDensity(int displayIndex) {
	CHECK_DISPLAY_INDEX(displayIndex, 0);
	return APP_SYS_GetPixelDensity(displayIndex);
}

/**
 * Returns the current device orientation.
 *
 * The device orientation is the orientation of a mobile device, as held by the
 * user. This is not necessarily the same as the display orientation (as returned
 * by {@link SDL_GetDisplayOrientation}, as some applications may have locked
 * their display into a fixed orientation. Indeed, it is generally a bad idea
 * to let an OpenGL context auto-rotate when the device orientation changes.
 *
 * The purpose of this function is to use device orientation as a (discrete)
 * control input while still permitting the OpenGL context to be locked.
 *
 * If this display is not a mobile device, this function will always return
 * {@link SDL_ORIENTATION_UNKNOWN}.
 *
 * @param displayIndex  The display to query
 *
 * @return the current device orientation.
 */
SDL_DisplayOrientation APP_GetDeviceOrientation(int displayIndex) {
	CHECK_DISPLAY_INDEX(displayIndex, SDL_ORIENTATION_UNKNOWN);
	return APP_SYS_GetDeviceOrientation(displayIndex);
}

/**
 * Returns the default orientation of this device.
 *
 * The default orientation corresponds to the intended orientiation that this
 * mobile device should be held. For devices with home buttons, this home button
 * is always expected at the bottom. For the vast majority of devices, this
 * means the intended orientation is Portrait. However, some Samsung tablets
 * have the home button oriented for Landscape.
 *
 * This function is important because the accelerometer axis is oriented relative
 * to the default orientation. So a default landscape device will have a
 * different accelerometer orientation than a portrait device.
 *
 * If this display is not a mobile device, this function will always return
 * {@link SDL_ORIENTATION_UNKNOWN}.
 *
 * @param displayIndex  The display to query
 *
 * @return the default orientation of this device.
 */
SDL_DisplayOrientation APP_GetDefaultOrientation(int displayIndex) {
	CHECK_DISPLAY_INDEX(displayIndex, SDL_ORIENTATION_UNKNOWN);
	return APP_SYS_GetDefaultOrientation(displayIndex);
}

/* vi: set ts=4 sw=4 expandtab: */
