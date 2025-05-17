/*
 * APPActivity.java
 *
 * Extension to SDLActivity supporting the extensions in SDL_App.
 */
package org.libsdl.app;

import android.content.Context;
import android.content.res.Configuration;
import android.hardware.Sensor;
import android.hardware.SensorManager;
import android.os.Build;
import android.os.Bundle;
import android.util.DisplayMetrics;
import android.view.Display;
import android.view.DisplayCutout;
import android.view.Surface;
import android.view.WindowManager;
import android.content.res.Resources;
import android.graphics.Rect;
import android.provider.Settings;
import android.provider.Settings.Secure;

@SuppressWarnings("deprecated")
/**
 * An extension to SDLActivity supporting orientation and insets
 *
 * CUGL relies on exposing a few extra features on mobile devices. These
 * features are defined in the extras folder. But on Android they require
 * us to add a few new methods to the activity. That is the purpose of this
 * class.
 *
 * @author walkerwhite
 * @date 11/12/22
 */
public class APPActivity extends SDLActivity {

    /** Access to the Android sensor manager */
    protected static SensorManager mSensorManager;
    /** Access to the Android accelerometer */
    protected static Sensor mAccelerometer;
    /** Access to the Android magnetometer */
    protected static Sensor mMagnetometer;
    /** Object to monitor the device orientation */
    protected static DeviceOrientation mDeviceOrientation;
    /** The initial device orientation */
    protected static int  mInitialOrientation;
    /** The default device orientation (if not static) */
    protected static int  mDefaultOrientation;
    /** The insets for notched phones */
    protected static Rect mSafeInsets;
    /** Whether or not a cutout exists */
    protected static boolean hasCutout;

    /**
     * Initializes this activity.
     *
     * This method just calles the initialization for the subclass and
     * then gives default values to our new attributes.
     */
    public static void initialize() {
        SDLActivity.initialize();
        mSensorManager = null;
        mAccelerometer = null;
        mMagnetometer  = null;
        mDeviceOrientation = null;
        mInitialOrientation = -1;
        mDefaultOrientation = -1;
    }

    /**
     * This method is called by SDL before loading the native shared libraries.
     * It can be overridden to provide names of shared libraries to be loaded.
     * The default implementation returns the defaults. It never returns null.
     * An array returned by a new implementation must at least contain "SDL2".
     * Also keep in mind that the order the libraries are loaded may matter.
     * @return names of shared libraries to be loaded (e.g. "SDL2", "main").
     */
    @Override
    protected String[] getLibraries() {
        return new String[] {
                "SDL2",
                "SDL2_image",
                "SDL2_ttf",
                "SDL2_atk",
                "SDL2_app",
                "main"
        };
    }

    /**
     * Called upon activity creation
     *
     * In addition to the subclass onCreate, this method initializes the sensor manager
     * and determines the initial device orientation.
     */
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        mDefaultOrientation = getDeviceDefaultOrientation();
        mSensorManager = (SensorManager)getSystemService(SENSOR_SERVICE);
        mAccelerometer = mSensorManager.getDefaultSensor(Sensor.TYPE_ACCELEROMETER);
        mMagnetometer  = mSensorManager.getDefaultSensor(Sensor.TYPE_MAGNETIC_FIELD);
        mDeviceOrientation = new DeviceOrientation(mDefaultOrientation == SDL_ORIENTATION_PORTRAIT);
        mInitialOrientation = mCurrentOrientation;
        computeSafeInsets();
    }

    /**
     * Called when we need to cleanly pause the native thread
     */
    @Override
    protected void pauseNativeThread() {
        // NOTE: This method is hard overridden because the final line is not super() compatible.
        mNextNativeState = NativeState.PAUSED;
        mIsResumedCalled = false;

        if (SDLActivity.mBrokenLibraries) {
            return;
        }

        mSensorManager.unregisterListener(mDeviceOrientation.getEventListener());
        SDLActivity.handleNativeState();
    }

    /**
     * Called when we need to resume the native thread
     */
    @Override
    protected void resumeNativeThread() {
        // NOTE: This method is hard overridden because the final line is not super() compatible.
        mNextNativeState = NativeState.RESUMED;
        mIsResumedCalled = true;

        if (SDLActivity.mBrokenLibraries) {
            return;
        }

        mSensorManager.registerListener(mDeviceOrientation.getEventListener(), mAccelerometer, SensorManager.SENSOR_DELAY_UI);
        mSensorManager.registerListener(mDeviceOrientation.getEventListener(), mMagnetometer,  SensorManager.SENSOR_DELAY_UI);
        SDLActivity.handleNativeState();
    }

    /**
     * Returns the current display orientation of the device.
     *
     * This method overrides the subclass implementation of getCurrentOrientation to
     * account for the fact that some Android tablets use LANDSCAPE as their default
     * orientation.
     *
     * @return the current display orientation of the device.
     */
    @SuppressWarnings("deprecation")
    public static int getCurrentOrientation() {
        final Context context = SDLActivity.getContext();
        final Display display = ((WindowManager) context.getSystemService(Context.WINDOW_SERVICE)).getDefaultDisplay();

        // Because some home buttons are in landscape mode
        int result = mDefaultOrientation;
        boolean standard = mDefaultOrientation == SDL_ORIENTATION_PORTRAIT;

        //System.out.println("Default is standard "+standard);
        switch (display.getRotation()) {
            case Surface.ROTATION_0:
                result = standard ? SDL_ORIENTATION_PORTRAIT : SDL_ORIENTATION_LANDSCAPE;
                break;
            case Surface.ROTATION_90:
                result = standard ? SDL_ORIENTATION_LANDSCAPE : SDL_ORIENTATION_PORTRAIT_FLIPPED;
                break;
            case Surface.ROTATION_180:
                result = standard ? SDL_ORIENTATION_PORTRAIT_FLIPPED : SDL_ORIENTATION_LANDSCAPE_FLIPPED;
                break;
            case Surface.ROTATION_270:
                result = standard ? SDL_ORIENTATION_LANDSCAPE_FLIPPED : SDL_ORIENTATION_PORTRAIT;
                break;
        }
        return result;
    }

    /**
     * Invokes the initial insets computation
     */
    @Override
    public void onAttachedToWindow() {
        super.onAttachedToWindow();
        computeSafeInsets();
    }

    /**
     * Recomputes the insets when the orientation changes
     */
    @Override
    public void onConfigurationChanged(Configuration newConfig) {
        super.onConfigurationChanged(newConfig);
        computeSafeInsets();
    }

    /**
     * Computes the the insets for notched devices
     */
    @SuppressWarnings("deprecation")
    public void computeSafeInsets() {
        DisplayMetrics vmetrics = new DisplayMetrics();
        DisplayMetrics rmetrics = new DisplayMetrics();
        getWindowManager().getDefaultDisplay().getMetrics(vmetrics);
        getWindowManager().getDefaultDisplay().getMetrics(rmetrics);
        mSafeInsets = new Rect(0,0,0,0);

        if (Build.VERSION.SDK_INT >= 28) {
            //System.out.println("SDK_28");
            android.view.WindowInsets insets = getWindow().getDecorView().getRootWindowInsets();
            if (insets != null) {
                DisplayCutout cutout = insets.getDisplayCutout();
                //System.out.println("Cutout is "+cutout);
                if (cutout != null) {
                    mSafeInsets.left  = Math.max(insets.getStableInsetLeft(),cutout.getSafeInsetLeft());
                    mSafeInsets.right = Math.max(insets.getStableInsetRight(),cutout.getSafeInsetRight());
                    mSafeInsets.top   = Math.max(insets.getStableInsetTop(),cutout.getSafeInsetTop());
                    mSafeInsets.bottom = Math.max(insets.getStableInsetBottom(),cutout.getSafeInsetBottom());
                    hasCutout = true;
                } else {
                    mSafeInsets.left  = insets.getStableInsetLeft();
                    mSafeInsets.right = insets.getStableInsetRight();
                    mSafeInsets.top   = insets.getStableInsetTop();
                    mSafeInsets.bottom = insets.getStableInsetBottom();
                    hasCutout = false;
                }
            }
        } else {
            int statusBar = 0;
            int resourceId = getResources().getIdentifier("status_bar_height", "dimen", "android");
            if (resourceId > 0) {
                statusBar = getResources().getDimensionPixelSize(resourceId);
            }
            mSafeInsets.top += statusBar;
            hasCutout = statusBar > convertDpToPixel(24);
        }

        int remainW = (rmetrics.widthPixels-vmetrics.widthPixels)- mSafeInsets.left-mSafeInsets.right;
        int remainH = (rmetrics.heightPixels-vmetrics.heightPixels)- mSafeInsets.top-mSafeInsets.bottom;

        if (remainW > 0) {
            mSafeInsets.right -= remainW;
        }
        if (remainH > 0) {
            mSafeInsets.bottom -= remainH;
        }
        //System.out.println("Safe area is "+mSafeInsets);
    }

    /**
     * Returns the left inset amount
     *
     * @return the left inset amount
     */
    public static int getSafeInsetLeft() {
        return mSafeInsets.left;
    }

    /**
     * Returns the right inset amount
     *
     * @return the right inset amount
     */
    public static int getSafeInsetRight() {
        return mSafeInsets.right;
    }

    /**
     * Returns the top inset amount
     *
     * @return the top inset amount
     */
    public static int getSafeInsetTop() {
        return mSafeInsets.top;
    }

    /**
     * Returns the bottom inset amount
     *
     * @return the bottom inset amount
     */
    public static int getSafeInsetBottom() {
        return mSafeInsets.bottom;
    }

    /**
     * Returns the current device orientation
     *
     * @return the current device orientation
     */
    public static int getDeviceOrientation() {
        return mDeviceOrientation.getOrientation() ;
    }

    /**
     * Returns the initial device orientation
     *
     * @return the initial device orientation
     */
    public static int getInitialOrientation() {
        return mInitialOrientation;
    }

    /**
     * Returns the default device orientation
     *
     * @return the default device orientation
     */
    @SuppressWarnings("deprecation")
    public static int getDeviceDefaultOrientation() {
        try {
            final Context context = SDL.getContext();
            final WindowManager windowManager = (WindowManager) context.getSystemService(Context.WINDOW_SERVICE);

            Configuration config = context.getResources().getConfiguration();
            int rotation = windowManager.getDefaultDisplay().getRotation();

            if (((rotation == Surface.ROTATION_0 || rotation == Surface.ROTATION_180) &&
                    config.orientation == Configuration.ORIENTATION_LANDSCAPE)
                    || ((rotation == Surface.ROTATION_90 || rotation == Surface.ROTATION_270) &&
                    config.orientation == Configuration.ORIENTATION_PORTRAIT)) {
                return SDL_ORIENTATION_LANDSCAPE;
            } else {
                return SDL_ORIENTATION_PORTRAIT;
            }
        } catch (Exception e) {
            return SDL_ORIENTATION_UNKNOWN;
        }
    }

    /**
     * Returns the number of pixels for the given device independent points
     *
     * @param dp	Measurement in device independent points
     *
     * @return he number of pixels for the given device independent points
     */
    public static int convertDpToPixel (float dp){
        DisplayMetrics metrics = Resources.getSystem().getDisplayMetrics();
        float px = dp * (metrics.densityDpi / 160f);
        return Math.round(px);
    }

    /**
     * Returns true if this device is notched
     *
     * @return true if this device is notched
     */
    public static boolean hasNotch() {
        return hasCutout;
    }

    /**
     * Returns the name of this device
     *
     * This value is identical to Build.DEVICE
     *
     * @return the name of this device
     */
    public static String getDeviceName() {
        // https://stackoverflow.com/questions/16704597/how-do-you-get-the-user-defined-device-name-in-android
        String deviceName = Settings.Global.getString(getContext().getContentResolver(), Settings.Global.DEVICE_NAME);
        if(deviceName == null) {
            deviceName = Settings.Secure.getString(getContext().getContentResolver(), "bluetooth_name");
        }
        return deviceName == null ? Build.DEVICE : deviceName;
    }

    /**
     * Returns the model of this device
     *
     * This value is typically Build.MODEL, unless it is missing manufacturer information.
     * In that case, we append the manufacturer to Build.MODEL.
     *
     * @return the model of this device
     */
    public static String getDeviceModel() {
        String manufacturer = Build.MANUFACTURER;
        String model = Build.MODEL;
        if (model.startsWith(manufacturer)) {
            return model.substring(0,1).toUpperCase() + model.substring(1).toLowerCase();
        } else {
            return (manufacturer.substring(0,1).toUpperCase() + manufacturer.substring(1).toLowerCase() +
                    " " + model);
        }
    }

    /**
     * Returns the OS version of this device
     *
     * This value is the version number (we do not use Android code names) together with the
     * date of the latest patch.
     *
     * @return the OS version of this device
     */
    public static String getDeviceOSVersion() {
        return Build.VERSION.RELEASE + " ("+Build.VERSION.SECURITY_PATCH+")";
    }

    /**
     * Returns the Android ID of this device
     *
     * This is a unique identifier for the device that does not require special privileges (like
     * the serial number does). While this number can be reassigned, it is generally stable enough
     * to use for app tracking.
     *
     * @return the Android ID of this device
     */
    public static String getDeviceID() {
        return Secure.getString(getContext().getContentResolver(), Secure.ANDROID_ID);
    }
}