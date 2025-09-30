package org.libsdl.app;


import android.content.Context;
import android.content.pm.ActivityInfo;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;
import android.os.Build;
import android.util.DisplayMetrics;
import android.util.Log;
import android.view.Display;
import android.view.InputDevice;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;
import android.view.WindowManager;


/**
    SDLSurface. This is what we draw on, so we need to know when it's created
    in order to do anything useful.

    Because of this, that's where we set up the SDL thread
*/
public class SDLSurface extends SurfaceView implements SurfaceHolder.Callback,
    View.OnKeyListener, View.OnTouchListener, SensorEventListener  {

    // Keep track of the surface size to normalize touch events
    protected float mWidth, mHeight;

    // Is SurfaceView ready for rendering
    public boolean mIsSurfaceReady;

    // Startup
    public SDLSurface(Context context) {
        super(context);
        getHolder().addCallback(this);

        setFocusable(true);
        setFocusableInTouchMode(true);
        requestFocus();
        setOnKeyListener(this);
        setOnTouchListener(this);

        setOnGenericMotionListener(SDLActivity.mMotionListener);

        // Some arbitrary defaults to avoid a potential division by zero
        mWidth = 1.0f;
        mHeight = 1.0f;

        mIsSurfaceReady = false;
    }

    public void destroy() {
        getHolder().removeCallback(this);
        setOnGenericMotionListener(null);
        setEnabled(false);
    }

    public void handlePause() {
        enableSensor(Sensor.TYPE_ACCELEROMETER, false);
    }

    public void handleResume() {
        setFocusable(true);
        setFocusableInTouchMode(true);
        requestFocus();
        setOnKeyListener(this);
        setOnTouchListener(this);
        enableSensor(Sensor.TYPE_ACCELEROMETER, true);
    }

    public Surface getNativeSurface() {
        return getHolder().getSurface();
    }

    // Called when we have a valid drawing surface
    @Override
    public void surfaceCreated(SurfaceHolder holder) {
        Log.v("SDL", "surfaceCreated()");
        SDLActivity.onNativeSurfaceCreated();
    }

    // Called when we lose the surface
    @Override
    public void surfaceDestroyed(SurfaceHolder holder) {
        Log.v("SDL", "surfaceDestroyed()");

        // Transition to pause, if needed
        SDLActivity.switchNativeState(SDLActivity.NativeState.PAUSED);

        mIsSurfaceReady = false;
        SDLActivity.onNativeSurfaceDestroyed();
    }

    // Called when the surface is resized
    @Override
    public void surfaceChanged(SurfaceHolder holder,
                               int format, int width, int height) {
        Log.v("SDL", "surfaceChanged()");

        SDLActivity activity = SDLActivity.mSingleton;
        Display display = SDLActivity.getCurrentDisplay();

        mWidth = width;
        mHeight = height;
        int nDeviceWidth = width;
        int nDeviceHeight = height;
        try
        {
            if (Build.VERSION.SDK_INT >= 17 /* Android 4.2 (JELLY_BEAN_MR1) */) {
                DisplayMetrics realMetrics = new DisplayMetrics();
                display.getRealMetrics(realMetrics);
                nDeviceWidth = realMetrics.widthPixels;
                nDeviceHeight = realMetrics.heightPixels;
            }
        } catch (Exception ignored) {
        }

        synchronized(activity) {
            // In case we're waiting on a size change after going fullscreen, send a notification.
            activity.notifyAll();
        }

        Log.v("SDL", "Window size: " + width + "x" + height);
        Log.v("SDL", "Device size: " + nDeviceWidth + "x" + nDeviceHeight);
        SDLActivity.nativeSetScreenResolution(width, height, nDeviceWidth, nDeviceHeight, display.getRefreshRate());
        SDLActivity.onNativeResize();

        // Prevent a screen distortion glitch,
        // for instance when the device is in Landscape and a Portrait App is resumed.
        boolean skip = false;
        int requestedOrientation = activity.getRequestedOrientation();

        if (requestedOrientation == ActivityInfo.SCREEN_ORIENTATION_PORTRAIT || requestedOrientation == ActivityInfo.SCREEN_ORIENTATION_SENSOR_PORTRAIT) {
            if (mWidth > mHeight) {
               skip = true;
            }
        } else if (requestedOrientation == ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE || requestedOrientation == ActivityInfo.SCREEN_ORIENTATION_SENSOR_LANDSCAPE) {
            if (mWidth < mHeight) {
               skip = true;
            }
        }

        // Special Patch for Square Resolution: Black Berry Passport
        if (skip) {
           double min = Math.min(mWidth, mHeight);
           double max = Math.max(mWidth, mHeight);

           if (max / min < 1.20) {
              Log.v("SDL", "Don't skip on such aspect-ratio. Could be a square resolution.");
              skip = false;
           }
        }

        // Don't skip if we might be multi-window or have popup dialogs
        if (Build.VERSION.SDK_INT >= 24 /* Android 7.0 (N) */) {
            skip = false;
        }

        if (skip) {
           Log.v("SDL", "Skip .. Surface is not ready.");
           mIsSurfaceReady = false;
           return;
        }

        /* If the surface has been previously destroyed by onNativeSurfaceDestroyed, recreate it here */
        SDLActivity.onNativeSurfaceChanged();

        /* Surface is ready */
        mIsSurfaceReady = true;

        SDLActivity.switchNativeState(SDLActivity.NativeState.RESUMED);
    }

    // Key events
    @Override
    public boolean onKey(View v, int keyCode, KeyEvent event) {
        return SDLActivity.handleKeyEvent(v, keyCode, event, null);
    }

    private static float getNormalized(float v, float max)
    {
        if (max <= 1) {
            return 0.5f;
        } else {
            return (v / (max - 1));
        }
    }

    // Touch events
    @Override
    public boolean onTouch(View v, MotionEvent event) {
        /* Ref: http://developer.android.com/training/gestures/multi.html */
        int touchDevId = event.getDeviceId();
        int pointerCount = event.getPointerCount();
        int action = event.getActionMasked();
        int i = 0;
        float x,y,p;

        /*
         * Prevent id to be -1, since it's used in SDL internal for synthetic events
         * Appears when using Android emulator, eg:
         *  adb shell input mouse tap 100 100
         *  adb shell input touchscreen tap 100 100
         */
        if (touchDevId < 0) {
            touchDevId -= 1;
        }

        if (action == MotionEvent.ACTION_POINTER_UP || action == MotionEvent.ACTION_POINTER_DOWN) {
            i = event.getActionIndex();
            pointerCount = i;
        }

        do {
            int toolType = event.getToolType(i);

            if (toolType == MotionEvent.TOOL_TYPE_MOUSE) {
                int buttonState = event.getButtonState();
                boolean relative;

                // We need to check if we're in relative mouse mode and get the axis offset rather than the x/y values
                // if we are. We'll leverage our existing mouse motion listener
                SDLGenericMotionListener_API14 motionListener = SDLActivity.mMotionListener;
                x = motionListener.getEventX(event, i);
                y = motionListener.getEventY(event, i);
                relative = motionListener.inRelativeMode();

                SDLActivity.onNativeMouse(buttonState, action, x, y, relative);
            } else if (toolType == MotionEvent.TOOL_TYPE_FINGER || toolType == MotionEvent.TOOL_TYPE_UNKNOWN) {
                int pointerId = event.getPointerId(i);
                x = getNormalized(event.getX(i), mWidth);
                y = getNormalized(event.getY(i), mHeight);
                p = event.getPressure(i);
                if (p > 1.0f) {
                    // may be larger than 1.0f on some devices
                    // see the documentation of getPressure(i)
                    p = 1.0f;
                }

                SDLActivity.onNativeTouch(touchDevId, pointerId,
                        action == MotionEvent.ACTION_CANCEL ? MotionEvent.ACTION_UP : action, x, y, p);
            }
        } while (++i < pointerCount);

        return true;
    }

    // Sensor events
    public void enableSensor(int sensortype, boolean enabled) {
        // TODO: This uses getDefaultSensor - what if we have >1 accels?
        SensorManager sensorManager = (SensorManager)SDLActivity.mSingleton.getSystemService(Context.SENSOR_SERVICE);
        Sensor ds = sensorManager.getDefaultSensor(sensortype);
        if (enabled) {
            sensorManager.registerListener(this, ds,
                            SensorManager.SENSOR_DELAY_GAME, null);
        } else {
            sensorManager.unregisterListener(this, ds);
        }
    }

    @Override
    public void onAccuracyChanged(Sensor sensor, int accuracy) {
        // TODO
    }

    @Override
    public void onSensorChanged(SensorEvent event) {
        if (event.sensor.getType() == Sensor.TYPE_ACCELEROMETER) {

            // Since we may have an orientation set, we won't receive onConfigurationChanged events.
            // We thus should check here.
            int newOrientation = SDLActivity.getCurrentOrientation();

            float x, y;
            switch (newOrientation) {
                case SDLActivity.SDL_ORIENTATION_LANDSCAPE:
                    x = -event.values[1];
                    y = event.values[0];
                    break;
                case SDLActivity.SDL_ORIENTATION_LANDSCAPE_FLIPPED:
                    x = event.values[1];
                    y = -event.values[0];
                    break;
                case SDLActivity.SDL_ORIENTATION_PORTRAIT_FLIPPED:
                    x = -event.values[0];
                    y = -event.values[1];
                    break;
                case SDLActivity.SDL_ORIENTATION_PORTRAIT:
                default:
                    x = event.values[0];
                    y = event.values[1];
                    break;
            }

            if (newOrientation != SDLActivity.mCurrentOrientation) {
                SDLActivity.mCurrentOrientation = newOrientation;
                SDLActivity.onNativeOrientationChanged(newOrientation);
            }

            SDLActivity.onNativeAccel(-x / SensorManager.GRAVITY_EARTH,
                                      y / SensorManager.GRAVITY_EARTH,
                                      event.values[2] / SensorManager.GRAVITY_EARTH);


        }
    }

    // Captured pointer events for API 26.
    public boolean onCapturedPointerEvent(MotionEvent event)
    {
        int action = event.getActionMasked();
        int pointerCount = event.getPointerCount();
        boolean relative, consumed = false;

        for (int i = 0; i < pointerCount; i++) {
            float x, y;
            int button = 0;
            switch (action) {
                case MotionEvent.ACTION_SCROLL:
                    x = event.getAxisValue(MotionEvent.AXIS_HSCROLL, i);
                    y = event.getAxisValue(MotionEvent.AXIS_VSCROLL, i);
                    relative = false;
                    break;

                case MotionEvent.ACTION_BUTTON_PRESS:
                case MotionEvent.ACTION_BUTTON_RELEASE:

                    // Change our action value to what SDL's code expects.
                    if (action == MotionEvent.ACTION_BUTTON_PRESS) {
                        action = MotionEvent.ACTION_DOWN;
                    } else { /* MotionEvent.ACTION_BUTTON_RELEASE */
                        action = MotionEvent.ACTION_UP;
                    }
                    button = event.getButtonState();
                    /* fall-through */
                case MotionEvent.ACTION_HOVER_MOVE:
                case MotionEvent.ACTION_MOVE:
                    x = event.getX(i);
                    y = event.getY(i);
                    relative = true;
                    break;

                default:
                    continue;
            }
            SDLActivity.onNativeMouse(button, action, x, y, relative);
            consumed = true;
        }

        return consumed;
    }
}
