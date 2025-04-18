//
//  CUHaptics.cpp
//  Cornell University Game Library (CUGL)
//
//  This module is a singleton providing display information about the device.
//  Originally, we had made this part of Application.  However, we discovered
//  that we needed platform specfic code for this, so we factored it out.
//
//  This singleton is also responsible for initializing (and disposing) the
//  OpenGL context.  That is because that context is tightly coupled to the
//  orientation information, which is provided by this class.
//
//  Because this is a singleton, there are no publicly accessible constructors
//  or intializers.  Use the static methods instead.
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

#include <stdio.h>
#include "CUHaptics.h"
#include <cugl/cugl.h>
#include "CUHaptics-impl.h"

using namespace cugl;
using namespace cugl::impl;

/** The single instance of the Haptics. */
Haptics* Haptics::_singelton = nullptr;

/**
 * Creates a new, unitialized Haptics.
 *
 * All of the values are set to 0 or UNKNOWN, depending on their type. You
 * must initialize the Display to access its values.
 *
 * WARNING: This class is a singleton. You should never access this
 * constructor directly. Use the start() method instead.
 */
Haptics::Haptics() {}

/**
 * Initializes the Haptics.
 *
 * This method initializes the platform specific haptics engine if the device
 * supports haptics.
 *
 * WARNING: This class is a singleton. You should never access this
 * initializer directly. Use the start() method instead.
 *
 * @return If initialization was successful.
 */
bool Haptics::init() {
    if (!HapticsIsSupported()) {
        CULogError("Start aborted. Haptics is not supported on this device.");
        return false;
    }
    HapticsInit();
    return true;
}

/**
 * Uninitializes the Haptics, releasing all resources.
 *
 * This method uninitializes the platform specific haptics engine if the
 * device supports haptics.
 *
 * WARNING: This class is a singleton. You should never access this
 * initializer directly. Use the  stop() method instead.
 *
 * @return If initialization was successful.
 */
void Haptics::dispose() {
    HapticsDispose();
}

/**
 * Starts up the Haptic manager and native haptics engine.
 *
 * Once this method is called, the get() method will no longer
 * return a nullptr.
 *
 * @return If the haptics manager was successfully initialized
 */
bool Haptics::start() {
    if (_singelton != nullptr) {
        CUAssertLog(false, "The haptics engine is already initialized");
        return false;
    }
    _singelton = new Haptics();
    return _singelton->init();
}


/**
 * Shuts down the Haptic manager and native haptics engine.
 *
 * Once this method is called, the get() method return a nullptr.
 */
void Haptics::stop() {
    if (_singelton == nullptr) {
        CUAssertLog(false, "The haptics engine is not initialized");
        return;
    }
    delete _singelton;
    _singelton = nullptr;
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
bool Haptics::playTransient(float intensity, float sharpness) {
    if (!HapticsIsSupported()) {
        CULogError("Play transient aborted. Haptics is not supported on this device.");
    }
    return HapticsPlayTransient(intensity, sharpness);
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
 * @param foce Wether to stop all currently playing continuous haptic events.
 * @return If the haptic event successfully played.
 */
bool Haptics::playContinuous(float intensity, float sharpness, float duration, bool force) {
    if (!HapticsIsSupported()) {
        CULogError("Play continuous aborted. Haptics is not supported on this device.");
    }
    return HapticsPlayContinuous(intensity, sharpness, duration, force);
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
bool Haptics::updateContinuous(float intensity, float sharpness) {
    if (!HapticsIsSupported()) {
        CULogError("Update continuous aborted. Haptics is not supported on this device.");
    }
    return HapticsUpdateContinuous(intensity, sharpness);
}




