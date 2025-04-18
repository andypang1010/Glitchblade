//
//  CUHaptics-SDL.cpp
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
//  Created by Marcos Eizayaga on 6/15/22.
//  Copyright © 2022 Game Design Initiative at Cornell. All rights reserved.
//

#include <stdio.h>
#include "CUHaptics-impl.h"

/** 
 * This method returns if haptics is supported for the current device. Not every
 * device has haptics enabled or has a usable API.
 * 
 * @return If haptics is supported. 
 */
bool cugl::impl::HapticsIsSupported() { return false; }

/**
 * Initialize the current device's haptic engine and all other resources 
 * required.
 * 
 * @return If the haptics engine was initialized correctly.
 */
bool cugl::impl::HapticsInit() { return true; }
  
/**
 * Dispose and stop the current device's haptic engine and release all other 
 * resources initialized.
 * 
 * @return If the haptics engine was disposed correctly.
 */
bool cugl::impl::HapticsDispose() { return true; }

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
bool cugl::impl::HapticsPlayTransient(float intensity, float sharpness) { return false; }
  
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
 * @return If the haptic event successfully played.
 */
bool cugl::impl::HapticsPlayContinuous(float intensity, float sharpness, float duration, bool force) { return false; }
  
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
bool cugl::impl::HapticsUpdateContinuous(float intensity, float sharpness) { return false; }

