//
//  CUHaptics.h
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

#ifndef __CU_HAPTICS_H__
#define __CU_HAPTICS_H__

namespace cugl {

/**
 * This class is a singleton used to control device haptics.
 *
 * The static methods of this class  start() and  stop() the
 * haptics manager. Without it, you cannot play any haptics.
 * 
 * If the device does not have haptics, all the methods in this manager will be
 * no-ops.
 *
 * This singleton haptics manager can play two types of haptics, transient and 
 * continuous. Transient haptics are like a single-tap, great for buttons.
 * Continuous haptics can be as long as you want, this also means you can update
 * them in real time.
 */
class Haptics {
  
protected:
  /** The single instance of the Haptics. */
  static Haptics* _singelton;
  
  /**
   * Creates a new, unitialized Haptics.
   *
   * All of the values are set to 0 or UNKNOWN, depending on their type. You
   * must initialize the Display to access its values.
   *
   * WARNING: This class is a singleton. You should never access this
   * constructor directly. Use the start() method instead.
   */
  Haptics();

   /**
   * Deletes this object, releasing all resources.
   *
   * WARNING: This class is a singleton. You should never access this
   * destructor directly. Use the stop() method instead.
   */
  ~Haptics() { dispose(); }
  
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
  bool init();
  
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
  void dispose();
  
public:
  
  /**
   * Starts up the Haptic manager and native haptics engine.
   *
   * Once this method is called, the get() method will no longer
   * return a nullptr.
   *
   * @return If the haptics manager was successfully initialized
   */
  static bool start();
  
  /**
   * Shuts down the Haptic manager and native haptics engine.
   *
   * Once this method is called, the get() method return a nullptr.
   */
  static void stop();
  
   /**
   * Returns the singleton instance for the haptics manager.
   *
   * You must call this static method first to get information about your
   * specific display.  This method will return nullptr until  start()
   * is called first.
   *
   * @return The singleton instance for the haptics manager.
   */
  static Haptics* get() { return _singelton; }
  
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
  bool playTransient(float intensity, float sharpness);
  
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
  bool playContinuous(float intensity, float sharpness, float duration, bool force = false);
  
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
  bool updateContinuous(float intensity, float sharpness);
  
  // No copy or reassignment.
  Haptics(Haptics const &) = delete;
  void operator=(Haptics const &) = delete;
};


}  // cugl


#endif /* __CU_HAPTICS_H__ */
