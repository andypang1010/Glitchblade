/*
 * DeviceOrientation.java
 *
 * A class to measure orientation on a locked display. 
 */
package org.libsdl.app;

import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;
import android.content.Context;
import android.util.DisplayMetrics;
import android.view.*;

@SuppressWarnings( "deprecation" )
/**
 * A class to measure orientation on a locked display. 
 *
 * Device orientation differs from display orientation in that it indicates how the
 * device is being held, not the orientation of the screen. It is inferred from the
 * accelerometer. 
 *
 * Note that accelerometer results are relative to the home button, as that is the
 * "natural" position of the device. For the vast majority of devices, this means that
 * portrait mode is the default device orientation.  However, older Samsung tablets 
 * actually have the home button in landscape mode, and this can affect the results.
 * For that reason, when a device orientation is constructed, we should indicate whether
 * or not portrait is indeed the default orientation.
 *
 * This class is adapted from DeviceOrientation by abdelhady to support SDL Activities.
 * Original: https://gist.github.com/abdelhady/501f6e48c1f3e32b253a#file-deviceorientation
 * Adapted from: http://stackoverflow.com/questions/11175599/how-to-measure-the-tilt-of-the-phone-in-xy-plane-using-accelerometer-in-android/15149421#15149421
 *
 * @author abdelhady, walkerwhite
 * @date 11/12/22
 */
public class DeviceOrientation {
    
    /** The window size for average computations; increasing makes it less twitchy */
    int smoothness = 1;
    /** The average pitch measurement over the smoothness window */
    private float averagePitch = 0;
    /** The average pitch measurement over the smoothness window */
    private float averageRoll = 0;
    /** The current device orientation */
    private int orientation = SDLActivity.SDL_ORIENTATION_UNKNOWN;

    /** The window storing the most recent pitch measurements */
    private float[] pitches;
    /** The window storing the most recent roll measurements */
    private float[] rolls;
    
    /** Whether this device orientation is standard (portrait default) */
    private boolean standard;
    
    /**
     * Creates a new DeviceOrientation object with portrait standard
     */
    public DeviceOrientation() {
        this(true);
    }

    /**
     * Creates a new DeviceOrientation object
     *
     * @param portrait  Whether or not portrait is standard
     */
    public DeviceOrientation(boolean portrait) {
        standard = portrait;
        pitches = new float[smoothness];
        rolls = new float[smoothness];
    }
    
    /**
     * Returns the event lister for this DeviceOrientation
     *
     * @return the event lister for this DeviceOrientation
     */
    public SensorEventListener getEventListener() {
        return sensorEventListener;
    }

    /**
     * Returns the current device orientation.
     *
     * This value returned is a constant defined in SDLActivity
     *
     * @return the current device orientation.
     */
    public int getOrientation() {
        return orientation;
    }

    /** The  event lister for this DeviceOrientation */
    SensorEventListener sensorEventListener = new SensorEventListener() {
        float[] mGravity;
        float[] mGeomagnetic;

        @Override
        public void onSensorChanged(SensorEvent event) {
            if (event.sensor.getType() == Sensor.TYPE_ACCELEROMETER) {
                mGravity = event.values;
            }
            if (event.sensor.getType() == Sensor.TYPE_MAGNETIC_FIELD) {
                mGeomagnetic = event.values;
            }
            if (mGravity != null && mGeomagnetic != null) {
                float R[] = new float[9];
                float I[] = new float[9];
                boolean success = SensorManager.getRotationMatrix(R, I, mGravity, mGeomagnetic);
                if (success) {
                    float orientationData[] = new float[3];
                    SensorManager.getOrientation(R, orientationData);
                    averagePitch = addValue(orientationData[1], pitches);
                    averageRoll = addValue(orientationData[2], rolls);
                    orientation = calculateOrientation();
                }
            }
        }

        @Override
        public void onAccuracyChanged(Sensor sensor, int accuracy) {
            // PASS
        }
    };

    /** 
     * Returns the average when the value is added to the given window
     *
     * The array values is a "window". As new values are added, the oldest values
     * are removed. Adding a value shifts all elements right in the array, and 
     * computes the new average.
     *
     * @param value     The value to add
     * @param values    The window to add to
     */
    private float addValue(float value, float[] values) {
        value = (float) Math.round((Math.toDegrees(value)));
        float average = 0;
        for (int i = 1; i < smoothness; i++) {
            values[i - 1] = values[i];
            average += values[i];
        }
        values[smoothness - 1] = value;
        average = (average + value) / smoothness;
        return average;
    }

    /**
     * Computes the current device orientation.
     *
     * This value returned is a constant defined in SDLActivity
     *
     * @return the current device orientation.
     */
    private int calculateOrientation() {
        Context context = SDLActivity.getContext();
        Display display = ((WindowManager) context.getSystemService(Context.WINDOW_SERVICE)).getDefaultDisplay();

        // finding local orientation dip
        if (((orientation == SDLActivity.SDL_ORIENTATION_PORTRAIT || 
            orientation == SDLActivity.SDL_ORIENTATION_PORTRAIT_FLIPPED)
                && (averageRoll > -30 && averageRoll < 30))) {
            if (averagePitch > 0)
                return standard ? SDLActivity.SDL_ORIENTATION_PORTRAIT_FLIPPED : SDLActivity.SDL_ORIENTATION_LANDSCAPE_FLIPPED;
            else
                return standard ? SDLActivity.SDL_ORIENTATION_PORTRAIT : SDLActivity.SDL_ORIENTATION_LANDSCAPE;
        } else {
            // divides between all orientations
            if (Math.abs(averagePitch) >= 30) {
                if (averagePitch > 0)
                    return standard ? SDLActivity.SDL_ORIENTATION_PORTRAIT_FLIPPED : SDLActivity.SDL_ORIENTATION_LANDSCAPE_FLIPPED;
                else
                    return standard ? SDLActivity.SDL_ORIENTATION_PORTRAIT : SDLActivity.SDL_ORIENTATION_LANDSCAPE;
            } else {
                if (averageRoll > 0) {
                    return standard ? SDLActivity.SDL_ORIENTATION_LANDSCAPE_FLIPPED : SDLActivity.SDL_ORIENTATION_PORTRAIT;
                } else {
                    return standard ? SDLActivity.SDL_ORIENTATION_LANDSCAPE : SDLActivity.SDL_ORIENTATION_PORTRAIT_FLIPPED;
                }
            }
        }
    }
}