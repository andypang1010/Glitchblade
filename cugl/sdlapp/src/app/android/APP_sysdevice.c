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

#define MAX_SIZE 1024

/**
 * System dependent version of APP_GetDeviceName
 *
 * @return the name of this device
 */
const char* APP_SYS_GetDeviceName(void) {
   static char device_name[MAX_SIZE];
   
    JNIEnv* env = (JNIEnv*)SDL_AndroidGetJNIEnv();
    jobject activity = (jobject)SDL_AndroidGetActivity();
    jclass clazz = (*env)->GetObjectClass(env, activity);
    jmethodID method_id = (*env)->GetStaticMethodID(env, clazz, "getDeviceName", "()Ljava/lang/String;");
    jobject value = (*env)->CallStaticObjectMethod(env, clazz, method_id);
    const char *nativeString = (*env)->GetStringUTFChars(env, value, 0);

    size_t len = strlen(nativeString);
    if (len >= MAX_SIZE) { len = MAX_SIZE-1; }
    strncat(device_name, nativeString, len);

    // Clean up
    (*env)->ReleaseStringUTFChars(env, value, nativeString);
    (*env)->DeleteLocalRef(env, activity);
    (*env)->DeleteLocalRef(env, clazz);
    (*env)->DeleteLocalRef(env, value);

    return device_name;
}

/**
 * System dependent version of APP_GetDeviceModel
 *
 * @return the model of this device
 */
const char* APP_SYS_GetDeviceModel(void) {
    static char device_model[MAX_SIZE];
    
    JNIEnv* env = (JNIEnv*)SDL_AndroidGetJNIEnv();
    jobject activity = (jobject)SDL_AndroidGetActivity();
    jclass clazz = (*env)->GetObjectClass(env, activity);
    jmethodID method_id = (*env)->GetStaticMethodID(env, clazz, "getDeviceModel", "()Ljava/lang/String;");
    jobject value = (*env)->CallStaticObjectMethod(env, clazz, method_id);
    const char *nativeString = (*env)->GetStringUTFChars(env, value, 0);

    size_t len = strlen(nativeString);
    if (len >= MAX_SIZE) { len = MAX_SIZE-1; }
    strncat(device_model, nativeString, len);

    // Clean up
    (*env)->ReleaseStringUTFChars(env, value, nativeString);
	(*env)->DeleteLocalRef(env, activity);
    (*env)->DeleteLocalRef(env, clazz);
    (*env)->DeleteLocalRef(env, value);

    return device_model;
}

/**
 * System dependent version of APP_GetDeviceOS
 *
 * @return the operating system running this device
 */
const char* APP_SYS_GetDeviceOS(void) {
	return "Android";
}

/**
 * System dependent version of APP_GetDeviceOSVersion
 *
 * @return the operating system version of this device
 */
const char* APP_SYS_GetDeviceOSVersion(void) {
    static char os_version[64];
    
    JNIEnv* env = (JNIEnv*)SDL_AndroidGetJNIEnv();
    jobject activity = (jobject)SDL_AndroidGetActivity();
    jclass clazz = (*env)->GetObjectClass(env, activity);
    jmethodID method_id = (*env)->GetStaticMethodID(env, clazz, "getDeviceOSVersion", "()Ljava/lang/String;");
    jobject value = (*env)->CallStaticObjectMethod(env, clazz, method_id);
    const char *nativeString = (*env)->GetStringUTFChars(env, value, 0);

    size_t len = strlen(nativeString);
    if (len >= 64) { len = 63; }
    strncat(os_version, nativeString, len);

    // Clean up
    (*env)->ReleaseStringUTFChars(env, value, nativeString);
    (*env)->DeleteLocalRef(env, activity);
    (*env)->DeleteLocalRef(env, clazz);
    (*env)->DeleteLocalRef(env, value);

    return os_version;
}

/**
 * System dependent version of APP_GetDeviceID
 *
 * @return a unique identifier for this device
 */
const char* APP_SYS_GetDeviceID(void) {
    static char device_id[MAX_SIZE];
    
    JNIEnv* env = (JNIEnv*)SDL_AndroidGetJNIEnv();
    jobject activity = (jobject)SDL_AndroidGetActivity();
    jclass clazz = (*env)->GetObjectClass(env, activity);
    jmethodID method_id = (*env)->GetStaticMethodID(env, clazz, "getDeviceID", "()Ljava/lang/String;");
    jobject value = (*env)->CallStaticObjectMethod(env, clazz, method_id);
    const char *nativeString = (*env)->GetStringUTFChars(env, value, 0);

    size_t len = strlen(nativeString);
    if (len >= MAX_SIZE) { len = MAX_SIZE-1; }
    strncat(device_id, nativeString, len);

    // Clean up
    (*env)->ReleaseStringUTFChars(env, value, nativeString);
    (*env)->DeleteLocalRef(env, activity);
    (*env)->DeleteLocalRef(env, clazz);
    (*env)->DeleteLocalRef(env, value);

    return device_id;
}
