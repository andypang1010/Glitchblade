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
#include "../APP_sysdisplay.h"

/**
 * System dependent version of APP_GetDisplayPixelBounds
 *
 * @param displayIndex  The display to query
 * @param rect          Rectangle to store the display bounds
 */
void APP_SYS_GetDisplayPixelBounds(int displayIndex, SDL_Rect *rect) {
	SDL_GetDisplayBounds(displayIndex,rect);
}

/**
 * System dependent version of APP_GetDisplaySafeBounds
 *
 * @param displayIndex  The display to query
 * @param rect          Rectangle to store the display bounds
 */
void APP_SYS_GetDisplaySafeBounds(int displayIndex, SDL_Rect *rect) {
	SDL_GetDisplayUsableBounds(displayIndex,rect);
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
	return 1.0;
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
