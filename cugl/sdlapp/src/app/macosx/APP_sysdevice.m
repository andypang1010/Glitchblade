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
#import <Foundation/Foundation.h>
#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/IOKitLib.h>
#if TARGET_OS_MACCATALYST
    #import <UIKit/UIKit.h>
#else
    #import <AppKit/AppKit.h>
#endif
#include <sys/types.h>
#include <sys/sysctl.h>
#include <string.h>

#define MAX_SIZE 1024

/**
 * System dependent version of APP_GetDeviceName
 *
 * @return the name of this device
 */
const char* APP_SYS_GetDeviceName(void) {
    static char device_name[MAX_SIZE];
    NSString* result = [[NSProcessInfo processInfo] hostName];
    strcpy(device_name, result.UTF8String);
    return device_name;
}

/**
 * System dependent version of APP_GetDeviceModel
 *
 * @return the model of this device
 */
const char* APP_SYS_GetDeviceModel(void) {
    static char device_model[MAX_SIZE];
    int mib[2];
    mib[0] = CTL_HW;
    mib[1] = HW_MODEL;
    
    size_t len = MAX_SIZE-1;
    
    sysctl( mib, 2, device_model, &len, NULL, 0 );
    device_model[len] = 0;
    return device_model;
}

/**
 * System dependent version of APP_GetDeviceOS
 *
 * @return the operating system running this device
 */
const char* APP_SYS_GetDeviceOS(void) {
    return "macOS";
}

/**
 * System dependent version of APP_GetDeviceOSVersion
 *
 * @return the operating system version of this device
 */
const char* APP_SYS_GetDeviceOSVersion(void) {
    static char os_version[64];
    NSString* result = [[NSProcessInfo processInfo] operatingSystemVersionString];
    strcpy(os_version, result.UTF8String);
    return os_version;
}

/**
 * System dependent version of APP_GetDeviceID
 *
 * @return a unique identifier for this device
 */
const char* APP_SYS_GetDeviceID(void) {
    static char device_id[MAX_SIZE];
    NSString *serial = nil;

    io_service_t platformExpert = IOServiceGetMatchingService(kIOMasterPortDefault,
                                     IOServiceMatching("IOPlatformExpertDevice"));

    if (platformExpert) {
        CFTypeRef serialNumberAsCFString =
        IORegistryEntryCreateCFProperty(platformExpert,
                                        CFSTR(kIOPlatformSerialNumberKey),
                                        kCFAllocatorDefault, 0);
        if (serialNumberAsCFString) {
            serial = CFBridgingRelease(serialNumberAsCFString);
        }

        IOObjectRelease(platformExpert);
    }
    if (serial != nil) {
        strcpy(device_id,serial.UTF8String);
    } else {
        device_id[0] = 0;
    }
    return device_id;
}
