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
#include <SDL_system.h>
#include <jni.h>

/**
 * System dependent version of SDL_GetDisplayPixelBounds
 *
 * @param displayIndex  The display to query
 * @param rect          Rectangle to store the display bounds
 */
void APP_SYS_GetDisplayPixelBounds(int displayIndex, SDL_Rect *rect) {
	SDL_GetDisplayBounds(displayIndex,rect);
	// Test.  Do I need scaling?
}

/**
 * System dependent version of SDL_GetDisplaySafeBounds
 *
 * @param displayIndex  The display to query
 * @param rect          Rectangle to store the display bounds
 */
void APP_SYS_GetDisplaySafeBounds(int displayIndex, SDL_Rect *rect) {
    SDL_GetDisplayBounds(displayIndex,rect);

	SDL_Rect insets;
    JNIEnv* env = (JNIEnv*)SDL_AndroidGetJNIEnv();
    jobject activity = (jobject)SDL_AndroidGetActivity();
    jclass clazz = (*env)->GetObjectClass(env, activity);
    jmethodID method_id = (*env)->GetStaticMethodID(env, clazz, "getSafeInsetLeft", "()I");
    insets.x = (*env)->CallStaticIntMethod(env, clazz, method_id);
    method_id = (*env)->GetStaticMethodID(env, clazz, "getSafeInsetTop", "()I");
    insets.y = (*env)->CallStaticIntMethod(env, clazz, method_id);
    method_id = (*env)->GetStaticMethodID(env, clazz, "getSafeInsetRight", "()I");
    insets.w = (*env)->CallStaticIntMethod(env, clazz, method_id);
    method_id = (*env)->GetStaticMethodID(env, clazz, "getSafeInsetBottom", "()I");
    insets.h = (*env)->CallStaticIntMethod(env, clazz, method_id);
	(*env)->DeleteLocalRef(env, activity);
    (*env)->DeleteLocalRef(env, clazz);

    
    rect->w -= (insets.x+insets.w);
    rect->h -= (insets.y+insets.h);
    rect->x += insets.x;
    rect->y += insets.y;
}

/**
 * System dependent version of SDL_CheckDisplayNotch
 *
 * @param displayIndex  The display to query
 *
 * @return 1 if this device has a notch, 0 otherwise
 */
int APP_SYS_CheckDisplayNotch(int displayIndex) {
	// retrieve the JNI environment.
    JNIEnv* env = (JNIEnv*)SDL_AndroidGetJNIEnv();
    jobject activity = (jobject)SDL_AndroidGetActivity();
    jclass clazz = (*env)->GetObjectClass(env, activity);
    jmethodID method_id = (*env)->GetStaticMethodID(env, clazz, "hasNotch", "()Z");
    int result = (*env)->CallStaticBooleanMethod(env, clazz, method_id);

    // Clean up
	(*env)->DeleteLocalRef(env, activity);
    (*env)->DeleteLocalRef(env, clazz);

    return result ? 1 : 0;
}

/**
 * System dependent version of SDL_GetPixelDensity
 *
 * @param displayIndex  The display to query
 *
 * @return the number of pixels for each point.
 */
float APP_SYS_GetPixelDensity(int displayIndex) {
	// retrieve the JNI environment.
    JNIEnv* env = (JNIEnv*)SDL_AndroidGetJNIEnv();
    jobject activity = (jobject)SDL_AndroidGetActivity();
    jclass clazz = (*env)->GetObjectClass(env, activity);
    jmethodID method_id = (*env)->GetStaticMethodID(env, clazz, "convertDpToPixel", "(F)I");
    int result = (*env)->CallStaticIntMethod(env, clazz, method_id, 1.0f);

    // Clean up
	(*env)->DeleteLocalRef(env, activity);
    (*env)->DeleteLocalRef(env, clazz);
    return result;
}

/**
 * System dependent version of SDL_GetDeviceOrientation
 *
 * @param displayIndex  The display to query
 *
 * @return the current device orientation.
 */
SDL_DisplayOrientation APP_SYS_GetDeviceOrientation(int displayIndex) {
	if (displayIndex != 0) {
		return SDL_ORIENTATION_UNKNOWN;
	}

	// retrieve the JNI environment.
    JNIEnv* env = (JNIEnv*)SDL_AndroidGetJNIEnv();
    jobject activity = (jobject)SDL_AndroidGetActivity();
    jclass clazz = (*env)->GetObjectClass(env, activity);
    jmethodID method_id = (*env)->GetStaticMethodID(env, clazz, "getDeviceOrientation", "()I");
    int current = (*env)->CallStaticIntMethod(env, clazz, method_id);
    
    // Clean up
	(*env)->DeleteLocalRef(env, activity);
    (*env)->DeleteLocalRef(env, clazz);
    
	switch (current) {
		case 0:
			return SDL_ORIENTATION_UNKNOWN;
		case 1:
			return SDL_ORIENTATION_LANDSCAPE;
		case 2:
			return SDL_ORIENTATION_LANDSCAPE_FLIPPED;
		case 3:
			return SDL_ORIENTATION_PORTRAIT;
		case 4:
			return SDL_ORIENTATION_PORTRAIT_FLIPPED;
		default:
			return SDL_ORIENTATION_UNKNOWN;
	}
}

/**
 * System dependent version of SDL_GetDefaultOrientation
 *
 * @param displayIndex  The display to query
 *
 * @return the default orientation of this device.
 */
SDL_DisplayOrientation APP_SYS_GetDefaultOrientation(int displayIndex) {
	if (displayIndex != 0) {
		return SDL_ORIENTATION_UNKNOWN;
	}

	// retrieve the JNI environment.
    JNIEnv* env = (JNIEnv*)SDL_AndroidGetJNIEnv();
    jobject activity = (jobject)SDL_AndroidGetActivity();
    jclass clazz = (*env)->GetObjectClass(env, activity);
    jmethodID method_id = (*env)->GetStaticMethodID(env, clazz, "getDeviceDefaultOrientation", "()I");
    int current = (*env)->CallStaticIntMethod(env, clazz, method_id);
    
    // Clean up
	(*env)->DeleteLocalRef(env, activity);
    (*env)->DeleteLocalRef(env, clazz);
    
	switch (current) {
		case 0:
			return SDL_ORIENTATION_UNKNOWN;
		case 1:
			return SDL_ORIENTATION_LANDSCAPE;
		case 2:
			return SDL_ORIENTATION_LANDSCAPE_FLIPPED;
		case 3:
			return SDL_ORIENTATION_PORTRAIT;
		case 4:
			return SDL_ORIENTATION_PORTRAIT_FLIPPED;
		default:
			return SDL_ORIENTATION_UNKNOWN;
	}
}

/* vi: set ts=4 sw=4 expandtab: */
