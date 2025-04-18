//
//  CUHaptics-iOS.mm
//  Cornell University Game Library (CUGL)
//
//  The SDL haptics tools do not work on OS X and iOS.  Therefore, we have
//  factored this functionality out into platform specific files.  This module
//  is the iOS implementation.
//
//  Note the .mm suffix.  That suffix is necessary in XCode for any module that
//  combines C++ and Objective-C code.
//
//  CUGL MIT License:
//      This software is provided 'as-is', without any express or implied
//      warranty.  In no event will the authors be held liable for any damages
//      arising from the use of this software.
//
//      Permission is granted to anyone to use this software for any purpose,
//      including commercial applications, and to alter it and redistribute it
//      freely, subject to the following restrictions:
//
//      1. The origin of this software must not be misrepresented; you must not
//      claim that you wrote the original software. If you use this software
//      in a product, an acknowledgment in the product documentation would be
//      appreciated but is not required.
//
//      2. Altered source versions must be plainly marked as such, and must not
//      be misrepresented as being the original software.
//
//      3. This notice may not be removed or altered from any source distribution.
//
//
//  Created by Marcos Eizayaga on 6/7/22.
//  Copyright © 2022 Game Design Initiative at Cornell. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <CoreHaptics/CoreHaptics.h>
#import "CUHaptics-impl.h"
#include <cugl/util/CUDebug.h>

#pragma mark -
#pragma mark LocalMethodsProperties

/* A strong reference to the Core Haptics Engine. */
CHHapticEngine* _engine;

/* The player for the continuous haptics player for updating while playing. */
id<CHHapticAdvancedPatternPlayer> _continuousPlayer;

/* If the engine has already been started by the system. */
BOOL _isEngineStarted; 

/* A local variable to easily access if haptics is supported. */
BOOL _isHapticsSupported;

#pragma mark -
#pragma mark LocalMethodsFunctions
        
/**
 * A helper method to start the haptics engine
 */
void startEngine() {
    if (_isHapticsSupported && !_isEngineStarted) {
        NSError* error = nil;
        [_engine startAndReturnError:&error];

        if (error != nil) {
            NSLog(@"ERROR: [CUHaptics-iOS] Engine start error --> %@", error);
        } else {
            _isEngineStarted = true;
        }
    }
}
        
/**
 * A helper method to create the haptics engine
 */
bool createEngine() {
    if (!_isHapticsSupported) return false;

    NSError* error;
    CULog("Initializing Haptics...");
    _engine = [[CHHapticEngine alloc] initAndReturnError:&error];
    
    if (error != nil) {
#if DEBUG
        NSLog(@"ERROR: [CUHaptics-iOS] While initializing Haptics --> %@", error);
#endif
        return false;
    }
    
    _engine.playsHapticsOnly = YES;
    
    _engine.resetHandler = ^{
            startEngine();
    };
    
    // The stopped handler alerts engine stoppage.
    [_engine setStoppedHandler:^(CHHapticEngineStoppedReason reason){
        CULogError("Haptic Engine STOPPED!");
        _isEngineStarted = false;
        switch (reason) {
            case CHHapticEngineStoppedReasonAudioSessionInterrupt: {
                CULogError("REASON: Audio Session Interrupt");
                // A phone call or notification could have come in, so take note to restart the haptic engine after the call ends. Wait for user-initiated playback.
                break;
            }
            case CHHapticEngineStoppedReasonApplicationSuspended: {
                CULogError("REASON: Application Suspended");
                // The user could have backgrounded your app, so take note to restart the haptic engine when the app reenters the foreground. Wait for user-initiated playback.
                break;
            }
            case CHHapticEngineStoppedReasonIdleTimeout: {
                CULogError("REASON: Idle Timeout");
                // The system stopped an idle haptic engine to conserve power, so restart it before your app must play the next haptic pattern.
                break;
            }
            case CHHapticEngineStoppedReasonSystemError: {
                CULogError("REASON: System Error");
                // The system faulted, so either continue without haptics or terminate the app.
                break;
            }
            case CHHapticEngineStoppedReasonGameControllerDisconnect: {
                CULogError("REASON: Game Controller Disconnected");
                break;
            }
            case CHHapticEngineStoppedReasonNotifyWhenFinished: {
                CULogError("REASON: Regular Stop");
                break;
            }
            case CHHapticEngineStoppedReasonEngineDestroyed: {
                CULogError("REASON: Engine Destroyed");
                break;
            }
            default: {
                    break;
            }
        }
    }];
    
    return true;
}

#pragma mark -
#pragma mark ImplMethods

/** 
 * This method returns if haptics is supported for the current device. Not every
 * device has haptics enabled or has a usable API.
 * 
 * @return If haptics is supported. 
 */
bool cugl::impl::HapticsIsSupported() {
    // supportHaptics is a BOOL, native to Obj-C, that is the reason for this awkward code.
    if (CHHapticEngine.capabilitiesForHardware.supportsHaptics) return true;
    return false;
}

/**
 * Initialize the current device's haptic engine and all other resources 
 * required.
 * 
 * @return If the haptics engine was initialized correctly.
 */
bool cugl::impl::HapticsInit() {
    _isHapticsSupported = CHHapticEngine.capabilitiesForHardware.supportsHaptics;
    
    if (!_isHapticsSupported) {
        CULogError("Init aborted. Haptics is not supported.");
        return false;
    }
    
    createEngine();
    return true;
}

/**
 * Dispose and stop the current device's haptic engine and release all other 
 * resources initialized.
 * 
 * @return If the haptics engine was disposed correctly.
 */
bool cugl::impl::HapticsDispose() {
    if (!_isHapticsSupported) {
        CULogError("Dispose aborted. Haptics is not supported.");
        return false;
    }
    
    NSError* error;
    [_continuousPlayer stopAtTime:0 error:&error];
    if (error != nil) {
#if DEBUG
        NSLog(@"ERROR: [CUHaptics-iOS] Stop continuous player error --> %@", error);
#endif
        return false;
    }
    
    _engine = NULL;
    _continuousPlayer = NULL;
    return true;
}


/**
 * Play a Transient haptic pattern. A transient haptic pattern is a short 
 * burst of haptic, like an impact. 
 * 
 * A transient pattern has two inputs, intensity and sharpness. Intensity 
 * defines the amplitude of the wave and in turn how strong the user feels it. 
 * Sharpness changes the frequency of the wave. You might use sharpness values 
 * to convey an experience that's soft, rounded, or organic, or one that's 
 * crisp, precise, or mechanical. 
 * 
 * @param intensity The strength of the haptic pattern, between [0.0f, 1.0f]
 * @param sharpness The frequency of the haptic pattern, between [0.0f, 1.0f]
 * @return If the haptic event successfully played.
 */
bool cugl::impl::HapticsPlayTransient(float intensity, float sharpness) {
    if (!_isHapticsSupported || _engine == NULL) {
        CULogError("Play Transient aborted. Haptics is not supported or engine is not initialized.");
        return false;
    }
    
    if (intensity < 0.0f || intensity > 1.0f) return false;
    if (sharpness < 0.0f || sharpness > 1.0f) return false;
    
    startEngine();
    
    CHHapticEventParameter* intensityParam = [[CHHapticEventParameter alloc] initWithParameterID:CHHapticEventParameterIDHapticIntensity value:intensity];
    CHHapticEventParameter* sharpnessParam = [[CHHapticEventParameter alloc] initWithParameterID:CHHapticEventParameterIDHapticSharpness value:sharpness];

    CHHapticEvent* event = [[CHHapticEvent alloc] initWithEventType:CHHapticEventTypeHapticTransient parameters:@[intensityParam, sharpnessParam] relativeTime:0];

    NSError* error = nil;
    CHHapticPattern* pattern = [[CHHapticPattern alloc] initWithEvents:@[event] parameters:@[] error:&error];
    
    if (error != nil) {
#if DEBUG
        NSLog(@"ERROR: [CUHaptics-iOS] Create transient pattern error --> %@", error);
#endif
        return false;
    }

    id<CHHapticPatternPlayer> player = [_engine createPlayerWithPattern:pattern error:&error];
    
    if (error != nil) {
#if DEBUG
        NSLog(@"ERROR: [CUHaptics-iOS] Create transient player error --> %@", error);
#endif
        return false;
    }
    
    [player startAtTime:0 error:&error];
    
    if (error != nil) {
#if DEBUG
        NSLog(@"ERROR: [CUHaptics-iOS] Engine play transient error --> %@", error);
#endif
        return false;
    }
    
    return true;
}

/**
 * Play a Continuous haptic pattern. A continuous haptic pattern is a 
 * sustained vibration that can be updated in real time in iOS not Android 
 * using updateContinuous.
 * 
 * A continuous pattern has three inputs, intensity, sharpness, and duration. 
 * Intensity defines the amplitude of the wave and in turn how strong the user 
 * feels it. Sharpness changes the frequency of the wave. You might use 
 * sharpness values to convey an experience that's soft, rounded, or organic, 
 * or one that's crisp, precise, or mechanical. The duration is how long the
 * event plays in seconds.
 * 
 * @param intensity The strength of the haptic pattern, between [0.0f, 1.0f]
 * @param sharpness The frequency of the haptic pattern, between [0.0f, 1.0f]
 * @param duration The length of the haptic pattern, must be greater than 0.0f
 * @param force Wether to force any playing continuous haptics event out.
 * @return If the haptic event successfully played.
 */
bool cugl::impl::HapticsPlayContinuous(float intensity, float sharpness, float duration, bool force) {
    if (!_isHapticsSupported || _engine == NULL) {
        CULogError("[CUHaptics-iOS] Play Continuous aborted. Haptics is not supported or engine is not initialized.");
        return false;
    }
        
    if (intensity < 0.0f || intensity > 1.0f) return false;
    if (sharpness < 0.0f || sharpness > 1.0f) return false;
    if (duration <= 0.0f) return false;
    
    if (force) {
        NSError* error = nil;
        [_continuousPlayer stopAtTime:0 error:&error];
    }
    
    startEngine();

    CHHapticEventParameter* intensityParam = [[CHHapticEventParameter alloc] initWithParameterID:CHHapticEventParameterIDHapticIntensity value:intensity];
    CHHapticEventParameter* sharpnessParam = [[CHHapticEventParameter alloc] initWithParameterID:CHHapticEventParameterIDHapticSharpness value:sharpness];

    CHHapticEvent* event = [[CHHapticEvent alloc] initWithEventType:CHHapticEventTypeHapticContinuous parameters:@[intensityParam, sharpnessParam] relativeTime:0 duration:duration];

    NSError* error = nil;
    CHHapticPattern* pattern = [[CHHapticPattern alloc] initWithEvents:@[event] parameters:@[] error:&error];
    
    if (error != nil) {
#if DEBUG
        NSLog(@"ERROR: [CUHaptics-iOS] Create continuous pattern error --> %@", error);
#endif
        return false;
    }

    _continuousPlayer = [_engine createAdvancedPlayerWithPattern:pattern error:&error];
    
    if (error != nil) {
#if DEBUG
        NSLog(@"ERROR: [CUHaptics-iOS] Create continuous player error --> %@", error);
#endif
        return false;
    }
    
    [_continuousPlayer startAtTime:0 error:&error];
    
    if (error != nil) {
#if DEBUG
        NSLog(@"ERROR: [CUHaptics-iOS] Engine play continuous error --> %@", error);
#endif
        return false;
    }
    
    return true;
}

/**
 * Update a running Continuous haptic pattern. Will only work on iOS not 
 * Android.
 * 
 * This will update two parameters, intensity and sharpness. Intensity defines 
 * the amplitude of the wave and in turn how strong the user feels it. 
 * Sharpness changes the frequency of the wave. You might use sharpness values
 * to convey an experience that's soft, rounded, or organic, or one that's 
 * crisp, precise, or mechanical. The duration is how long the event plays 
 * in seconds.
 * 
 * @param intensity The strength of the haptic pattern, between [0.0f, 1.0f]
 * @param sharpness The frequency of the haptic pattern, between [0.0f, 1.0f]
 * @return If the haptic event successfully updated.
 */
bool cugl::impl::HapticsUpdateContinuous(float intensity, float sharpness) {
    if (!_isHapticsSupported || _engine == NULL) {
        CULogError("[CUHaptics-iOS] Play Continuous aborted. Haptics is not supported or engine is not initialized.");
        return false;
    }
    
    if (_continuousPlayer != NULL) {
        
        CHHapticDynamicParameter* intensityParam = [[CHHapticDynamicParameter alloc] initWithParameterID:CHHapticDynamicParameterIDHapticIntensityControl value:intensity relativeTime:0];
        CHHapticDynamicParameter* sharpnessParam = [[CHHapticDynamicParameter alloc] initWithParameterID:CHHapticDynamicParameterIDHapticSharpnessControl value:sharpness relativeTime:0];

        NSError* error = nil;
        [_continuousPlayer sendParameters:@[intensityParam, sharpnessParam] atTime:0 error:&error];

        if (error != nil) {
#if DEBUG
            NSLog(@"ERROR: [CUHaptics-iOS] Update contuous parameters error --> %@", error);
#endif
            return false;
        }
    }
    return true;
}
