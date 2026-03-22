package org.libsdl.app;

import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;
import java.util.List;

import android.content.Context;
import android.hardware.input.InputManager;
import android.os.Build;
import android.os.Handler;
import android.os.Looper;
import android.os.VibrationEffect;
import android.os.Vibrator;
import android.os.VibratorManager;
import android.util.Log;
import android.view.InputDevice;
import android.view.KeyCharacterMap;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.View;


public class SDLControllerManager
{
    public static native void nativeSetupJNI();

    public static native void nativeAddJoystick(int device_id, String name, String desc,
                                               int vendor_id, int product_id,
                                               int button_mask,
                                               int naxes, int axis_mask, int nhats, boolean can_rumble);
    public static native void nativeRemoveJoystick(int device_id);
    public static native void nativeAddHaptic(int device_id, String name);
    public static native void nativeRemoveHaptic(int device_id);
    public static native int onNativePadDown(int device_id, int keycode);
    public static native int onNativePadUp(int device_id, int keycode);
    public static native void onNativeJoy(int device_id, int axis,
                                          float value);
    public static native void onNativeHat(int device_id, int hat_id,
                                          int x, int y);

    protected static SDLJoystickHandler mJoystickHandler;
    protected static SDLHapticHandler mHapticHandler;

    private static final String TAG = "SDLControllerManager";

    public static void create() {
        mJoystickHandler = new SDLJoystickHandler();
        mHapticHandler = new SDLHapticHandler();
    }

    public static void destroy() {
        mJoystickHandler = null;
        mHapticHandler = null;
    }

    // Joystick glue code, just a series of stubs that redirect to the SDLJoystickHandler instance
    public static boolean handleJoystickMotionEvent(MotionEvent event) {
        return mJoystickHandler.handleMotionEvent(event);
    }

    /**
     * This method is called by SDL using JNI.
     */
    public static void joystickSubscribe() {
        mJoystickHandler.subscribe();
    }

    /**
     * This method is called by SDL using JNI.
     */
    public static void joystickUnsubscribe() {
        mJoystickHandler.unsubscribe();
    }

    /**
     * This method is called by SDL using JNI.
     */
    public static void joystickRumble(int device_id, char low_frequency_intensity, char high_frequency_intensity, int length) {
        mJoystickHandler.rumble(device_id, low_frequency_intensity, high_frequency_intensity, length);
    }

    /**
     * This method is called by SDL using JNI.
     */
    public static void hapticSubscribe() {
        mHapticHandler.subscribe();
    }

    /**
     * This method is called by SDL using JNI.
     */
    public static void hapticUnsubscribe() {
        mHapticHandler.unsubscribe();
    }

    /**
     * This method is called by SDL using JNI.
     */
    public static void hapticRun(int device_id, char low_frequency_intensity, char high_frequency_intensity, int length) {
        mHapticHandler.run(device_id, low_frequency_intensity, high_frequency_intensity, length);
    }

    /**
     * This method is called by SDL using JNI.
     */
    public static void hapticStop(int device_id)
    {
        mHapticHandler.stop(device_id);
    }

    // Check if a given device is considered a possible SDL joystick
    public static boolean isDeviceSDLJoystick(int deviceId) {
        InputDevice device = InputDevice.getDevice(deviceId);
        if (device == null || device.isVirtual()) {
            return false;
        }
        int sources = device.getSources();

        /* This is called for every button press, so let's not spam the logs */
        /*
        if ((sources & InputDevice.SOURCE_CLASS_JOYSTICK) != 0) {
            Log.v(TAG, "Input device " + device.getName() + " has class joystick.");
        }
        if ((sources & InputDevice.SOURCE_DPAD) == InputDevice.SOURCE_DPAD) {
            Log.v(TAG, "Input device " + device.getName() + " is a dpad.");
        }
        if ((sources & InputDevice.SOURCE_GAMEPAD) == InputDevice.SOURCE_GAMEPAD) {
            Log.v(TAG, "Input device " + device.getName() + " is a gamepad.");
        }
        */

        return ((sources & InputDevice.SOURCE_CLASS_JOYSTICK) != 0 ||
                ((sources & InputDevice.SOURCE_DPAD) == InputDevice.SOURCE_DPAD) ||
                ((sources & InputDevice.SOURCE_GAMEPAD) == InputDevice.SOURCE_GAMEPAD)
        );
    }

    public static String getDeviceDescriptor(InputDevice device) {
        String desc = device.getDescriptor();

        if (desc != null && !desc.isEmpty()) {
            return desc;
        }

        return device.getName();
    }

    public static ArrayList<Vibrator> getDeviceVibrators(InputDevice device) {
        ArrayList<Vibrator> vibrators = new ArrayList<Vibrator>();
        if (Build.VERSION.SDK_INT >= 31 /* Android 12.0 (S) */) {
            VibratorManager manager = device.getVibratorManager();
            int[] vibratorIds = manager.getVibratorIds();
            for (int vibrator_id : vibratorIds) {
                Vibrator vib = manager.getVibrator(vibrator_id);
                if (vib != null && vib.hasVibrator()) {
                    vibrators.add(vib);
                }
            }
        } else {
            Vibrator vib = device.getVibrator();
            if (vib != null && vib.hasVibrator()) {
                vibrators.add(vib);
            }
        }
        return vibrators;
    }

    private static void vibrate(Vibrator vibrator, char intensity, int length) {
        int value = intensity / 256;
        if (value == 0) {
            vibrator.cancel();
            return;
        }
        if (Build.VERSION.SDK_INT >= 26 /* Android 8.0 (O) */) {
            vibrator.vibrate(VibrationEffect.createOneShot(length, value));
        } else {
            vibrator.vibrate(length);
        }
    }

    public static void vibrate(ArrayList<Vibrator> vibrators, char low_frequency_intensity, char high_frequency_intensity, int length) {
        if (vibrators.size() >= 2) {
            vibrate(vibrators.get(0), low_frequency_intensity, length);
            vibrate(vibrators.get(1), high_frequency_intensity, length);
        } else if (vibrators.size() == 1) {
            // char intensity = (low_frequency_intensity * 0.6f) + (high_frequency_intensity * 0.4f);
            char intensity = (char)((((int)low_frequency_intensity * 43) + ((int)high_frequency_intensity * 21)) / 64);
            vibrate(vibrators.get(0), intensity, length);
        }
    }

    public static void cancelVibration(ArrayList<Vibrator> vibrators) {
        if (vibrators.size() >= 1) {
            vibrators.get(0).cancel();
            if (vibrators.size() >= 2) {
                vibrators.get(1).cancel();
            }
        }
    }
}

class SDLJoystickHandler implements InputManager.InputDeviceListener {

    static class SDLJoystick {
        public int device_id;
        public String name;
        public String desc;
        public ArrayList<InputDevice.MotionRange> axes;
        public ArrayList<InputDevice.MotionRange> hats;
        public ArrayList<Vibrator> vibs;
    }
    static class RangeComparator implements Comparator<InputDevice.MotionRange> {
        @Override
        public int compare(InputDevice.MotionRange arg0, InputDevice.MotionRange arg1) {
            // Some controllers, like the Moga Pro 2, return AXIS_GAS (22) for right trigger and AXIS_BRAKE (23) for left trigger - swap them so they're sorted in the right order for SDL
            int arg0Axis = arg0.getAxis();
            int arg1Axis = arg1.getAxis();
            if (arg0Axis == MotionEvent.AXIS_GAS) {
                arg0Axis = MotionEvent.AXIS_BRAKE;
            } else if (arg0Axis == MotionEvent.AXIS_BRAKE) {
                arg0Axis = MotionEvent.AXIS_GAS;
            }
            if (arg1Axis == MotionEvent.AXIS_GAS) {
                arg1Axis = MotionEvent.AXIS_BRAKE;
            } else if (arg1Axis == MotionEvent.AXIS_BRAKE) {
                arg1Axis = MotionEvent.AXIS_GAS;
            }

            // Make sure the AXIS_Z is sorted between AXIS_RY and AXIS_RZ.
            // This is because the usual pairing are:
            // - AXIS_X + AXIS_Y (left stick).
            // - AXIS_RX, AXIS_RY (sometimes the right stick, sometimes triggers).
            // - AXIS_Z, AXIS_RZ (sometimes the right stick, sometimes triggers).
            // This sorts the axes in the above order, which tends to be correct
            // for Xbox-ish game pads that have the right stick on RX/RY and the
            // triggers on Z/RZ.
            //
            // Gamepads that don't have AXIS_Z/AXIS_RZ but use
            // AXIS_LTRIGGER/AXIS_RTRIGGER are unaffected by this.
            //
            // References:
            // - https://developer.android.com/develop/ui/views/touch-and-input/game-controllers/controller-input
            // - https://www.kernel.org/doc/html/latest/input/gamepad.html
            if (arg0Axis == MotionEvent.AXIS_Z) {
                arg0Axis = MotionEvent.AXIS_RZ - 1;
            } else if (arg0Axis > MotionEvent.AXIS_Z && arg0Axis < MotionEvent.AXIS_RZ) {
                --arg0Axis;
            }
            if (arg1Axis == MotionEvent.AXIS_Z) {
                arg1Axis = MotionEvent.AXIS_RZ - 1;
            } else if (arg1Axis > MotionEvent.AXIS_Z && arg1Axis < MotionEvent.AXIS_RZ) {
                --arg1Axis;
            }

            return arg0Axis - arg1Axis;
        }
    }

    private final ArrayList<SDLJoystick> mJoysticks;

    public SDLJoystickHandler() {

        mJoysticks = new ArrayList<SDLJoystick>();
    }

    public void subscribe() {
        InputManager im = (InputManager) SDLActivity.getContext().getSystemService(Context.INPUT_SERVICE);
        im.registerInputDeviceListener(this, new Handler(Looper.getMainLooper()));

        int[] deviceIds = InputDevice.getDeviceIds();
        for (int device_id : deviceIds) {
             this.onInputDeviceAdded(device_id);
        }
    }

    public void unsubscribe() {
        InputManager im = (InputManager) SDLActivity.getContext().getSystemService(Context.INPUT_SERVICE);
        im.unregisterInputDeviceListener(this);

        mJoysticks.clear();
    }

    @Override
    public void onInputDeviceAdded(int device_id) {
        if (SDLControllerManager.isDeviceSDLJoystick(device_id)) {
            SDLJoystick joystick = getJoystick(device_id);
            if (joystick == null) {
                InputDevice joystickDevice = InputDevice.getDevice(device_id);
                joystick = new SDLJoystick();
                joystick.device_id = device_id;
                joystick.name = joystickDevice.getName();
                joystick.desc = SDLControllerManager.getDeviceDescriptor(joystickDevice);
                joystick.axes = new ArrayList<InputDevice.MotionRange>();
                joystick.hats = new ArrayList<InputDevice.MotionRange>();
                joystick.vibs = SDLControllerManager.getDeviceVibrators(joystickDevice);
                List<InputDevice.MotionRange> ranges = joystickDevice.getMotionRanges();
                Collections.sort(ranges, new RangeComparator());
                for (InputDevice.MotionRange range : ranges) {
                    if ((range.getSource() & InputDevice.SOURCE_CLASS_JOYSTICK) != 0) {
                        if (range.getAxis() == MotionEvent.AXIS_HAT_X || range.getAxis() == MotionEvent.AXIS_HAT_Y) {
                            joystick.hats.add(range);
                        } else {
                            joystick.axes.add(range);
                        }
                    }
                }

                mJoysticks.add(joystick);
                SDLControllerManager.nativeAddJoystick(joystick.device_id, joystick.name, joystick.desc,
                        getVendorId(joystickDevice), getProductId(joystickDevice),
                        getButtonMask(joystickDevice), joystick.axes.size(), getAxisMask(joystick.axes), joystick.hats.size()/2, joystick.vibs.size() > 0);
            }
        }
    }

    @Override
    public void onInputDeviceRemoved(int device_id) {
        for (int i = 0; i < mJoysticks.size(); i++) {
            if (mJoysticks.get(i).device_id == device_id) {
                mJoysticks.remove(i);
                SDLControllerManager.nativeRemoveJoystick(device_id);
                break;
            }
        }
    }

    @Override
    public void onInputDeviceChanged(int device_id) {
        // Log.d(TAG, "Input device changed: " + device_id);
    }

    protected SDLJoystick getJoystick(int device_id) {
        for (SDLJoystick joystick : mJoysticks) {
            if (joystick.device_id == device_id) {
                return joystick;
            }
        }
        return null;
    }

    public boolean handleMotionEvent(MotionEvent event) {
        int actionPointerIndex = event.getActionIndex();
        int action = event.getActionMasked();
        if (action == MotionEvent.ACTION_MOVE) {
            SDLJoystick joystick = getJoystick(event.getDeviceId());
            if (joystick != null) {
                for (int i = 0; i < joystick.axes.size(); i++) {
                    InputDevice.MotionRange range = joystick.axes.get(i);
                    /* Normalize the value to -1...1 */
                    float value = (event.getAxisValue(range.getAxis(), actionPointerIndex) - range.getMin()) / range.getRange() * 2.0f - 1.0f;
                    SDLControllerManager.onNativeJoy(joystick.device_id, i, value);
                }
                for (int i = 0; i < joystick.hats.size() / 2; i++) {
                    int hatX = Math.round(event.getAxisValue(joystick.hats.get(2 * i).getAxis(), actionPointerIndex));
                    int hatY = Math.round(event.getAxisValue(joystick.hats.get(2 * i + 1).getAxis(), actionPointerIndex));
                    SDLControllerManager.onNativeHat(joystick.device_id, i, hatX, hatY);
                }
            }
        }
        return true;
    }

    public int getProductId(InputDevice joystickDevice) {
        return joystickDevice.getProductId();
    }
    public int getVendorId(InputDevice joystickDevice) {
        return joystickDevice.getVendorId();
    }
    public int getAxisMask(List<InputDevice.MotionRange> ranges) {
        // For compatibility, keep computing the axis mask like before,
        // only really distinguishing 2, 4 and 6 axes.
        int axis_mask = 0;
        if (ranges.size() >= 2) {
            // ((1 << SDL_GAMEPAD_AXIS_LEFTX) | (1 << SDL_GAMEPAD_AXIS_LEFTY))
            axis_mask |= 0x0003;
        }
        if (ranges.size() >= 4) {
            // ((1 << SDL_GAMEPAD_AXIS_RIGHTX) | (1 << SDL_GAMEPAD_AXIS_RIGHTY))
            axis_mask |= 0x000c;
        }
        if (ranges.size() >= 6) {
            // ((1 << SDL_GAMEPAD_AXIS_LEFT_TRIGGER) | (1 << SDL_GAMEPAD_AXIS_RIGHT_TRIGGER))
            axis_mask |= 0x0030;
        }
        // Also add an indicator bit for whether the sorting order has changed.
        // This serves to disable outdated gamecontrollerdb.txt mappings.
        boolean have_z = false;
        boolean have_past_z_before_rz = false;
        for (InputDevice.MotionRange range : ranges) {
            int axis = range.getAxis();
            if (axis == MotionEvent.AXIS_Z) {
                have_z = true;
            } else if (axis > MotionEvent.AXIS_Z && axis < MotionEvent.AXIS_RZ) {
                have_past_z_before_rz = true;
            }
        }
        if (have_z && have_past_z_before_rz) {
            // If both these exist, the compare() function changed sorting order.
            // Set a bit to indicate this fact.
            axis_mask |= 0x8000;
        }
        return axis_mask;
    }
    public int getButtonMask(InputDevice joystickDevice) {
        int button_mask = 0;
        int[] keys = new int[] {
            KeyEvent.KEYCODE_BUTTON_A,
            KeyEvent.KEYCODE_BUTTON_B,
            KeyEvent.KEYCODE_BUTTON_X,
            KeyEvent.KEYCODE_BUTTON_Y,
            KeyEvent.KEYCODE_BACK,
            KeyEvent.KEYCODE_MENU,
            KeyEvent.KEYCODE_BUTTON_MODE,
            KeyEvent.KEYCODE_BUTTON_START,
            KeyEvent.KEYCODE_BUTTON_THUMBL,
            KeyEvent.KEYCODE_BUTTON_THUMBR,
            KeyEvent.KEYCODE_BUTTON_L1,
            KeyEvent.KEYCODE_BUTTON_R1,
            KeyEvent.KEYCODE_DPAD_UP,
            KeyEvent.KEYCODE_DPAD_DOWN,
            KeyEvent.KEYCODE_DPAD_LEFT,
            KeyEvent.KEYCODE_DPAD_RIGHT,
            KeyEvent.KEYCODE_BUTTON_SELECT,
            KeyEvent.KEYCODE_DPAD_CENTER,

            // These don't map into any SDL controller buttons directly
            KeyEvent.KEYCODE_BUTTON_L2,
            KeyEvent.KEYCODE_BUTTON_R2,
            KeyEvent.KEYCODE_BUTTON_C,
            KeyEvent.KEYCODE_BUTTON_Z,
            KeyEvent.KEYCODE_BUTTON_1,
            KeyEvent.KEYCODE_BUTTON_2,
            KeyEvent.KEYCODE_BUTTON_3,
            KeyEvent.KEYCODE_BUTTON_4,
            KeyEvent.KEYCODE_BUTTON_5,
            KeyEvent.KEYCODE_BUTTON_6,
            KeyEvent.KEYCODE_BUTTON_7,
            KeyEvent.KEYCODE_BUTTON_8,
            KeyEvent.KEYCODE_BUTTON_9,
            KeyEvent.KEYCODE_BUTTON_10,
            KeyEvent.KEYCODE_BUTTON_11,
            KeyEvent.KEYCODE_BUTTON_12,
            KeyEvent.KEYCODE_BUTTON_13,
            KeyEvent.KEYCODE_BUTTON_14,
            KeyEvent.KEYCODE_BUTTON_15,
            KeyEvent.KEYCODE_BUTTON_16,
        };
        int[] masks = new int[] {
            (1 << 0),   // A -> A
            (1 << 1),   // B -> B
            (1 << 2),   // X -> X
            (1 << 3),   // Y -> Y
            (1 << 4),   // BACK -> BACK
            (1 << 6),   // MENU -> START
            (1 << 5),   // MODE -> GUIDE
            (1 << 6),   // START -> START
            (1 << 7),   // THUMBL -> LEFTSTICK
            (1 << 8),   // THUMBR -> RIGHTSTICK
            (1 << 9),   // L1 -> LEFTSHOULDER
            (1 << 10),  // R1 -> RIGHTSHOULDER
            (1 << 11),  // DPAD_UP -> DPAD_UP
            (1 << 12),  // DPAD_DOWN -> DPAD_DOWN
            (1 << 13),  // DPAD_LEFT -> DPAD_LEFT
            (1 << 14),  // DPAD_RIGHT -> DPAD_RIGHT
            (1 << 4),   // SELECT -> BACK
            (1 << 0),   // DPAD_CENTER -> A
            (1 << 15),  // L2 -> ??
            (1 << 16),  // R2 -> ??
            (1 << 17),  // C -> ??
            (1 << 18),  // Z -> ??
            (1 << 20),  // 1 -> ??
            (1 << 21),  // 2 -> ??
            (1 << 22),  // 3 -> ??
            (1 << 23),  // 4 -> ??
            (1 << 24),  // 5 -> ??
            (1 << 25),  // 6 -> ??
            (1 << 26),  // 7 -> ??
            (1 << 27),  // 8 -> ??
            (1 << 28),  // 9 -> ??
            (1 << 29),  // 10 -> ??
            (1 << 30),  // 11 -> ??
            (1 << 31),  // 12 -> ??
            // We're out of room...
            0xFFFFFFFF,  // 13 -> ??
            0xFFFFFFFF,  // 14 -> ??
            0xFFFFFFFF,  // 15 -> ??
            0xFFFFFFFF,  // 16 -> ??
        };
        boolean[] has_keys = joystickDevice.hasKeys(keys);
        for (int i = 0; i < keys.length; ++i) {
            if (has_keys[i]) {
                button_mask |= masks[i];
            }
        }
        return button_mask;
    }
    public void rumble(int device_id, char low_frequency_intensity, char high_frequency_intensity, int length) {
        SDLJoystick joystick = getJoystick(device_id);
        if (joystick != null) {
            SDLControllerManager.vibrate(joystick.vibs, low_frequency_intensity, high_frequency_intensity, length);
        }
    }
}

class SDLHapticHandler implements InputManager.InputDeviceListener {

    static class SDLHaptic {
        public int device_id;
        public String name;
        public ArrayList<Vibrator> vibs;
    }

    private final ArrayList<SDLHaptic> mHaptics;

    public SDLHapticHandler() {
        mHaptics = new ArrayList<SDLHaptic>();
    }

    public void run(int device_id, char low_frequency_intensity, char high_frequency_intensity, int length) {
        SDLHaptic haptic = getHaptic(device_id);
        if (haptic != null) {
            SDLControllerManager.vibrate(haptic.vibs, low_frequency_intensity, high_frequency_intensity, length);
        }
    }

    public void stop(int device_id) {
        SDLHaptic haptic = getHaptic(device_id);
        if (haptic != null) {
            SDLControllerManager.cancelVibration(haptic.vibs);
        }
    }

    private static final int deviceId_VIBRATOR_SERVICE = KeyCharacterMap.VIRTUAL_KEYBOARD;

    public void subscribe() {

        Context context = SDLActivity.getContext();
        Vibrator vib;
        if (Build.VERSION.SDK_INT >= 31 /* Android 12.0 (S) */) {
            VibratorManager vm = (VibratorManager) context.getSystemService(Context.VIBRATOR_MANAGER_SERVICE);
            vib = vm.getDefaultVibrator();
        } else {
            vib = (Vibrator) context.getSystemService(Context.VIBRATOR_SERVICE);
        }

        if (vib != null && vib.hasVibrator()) {
            ArrayList<Vibrator> vibrators = new ArrayList<Vibrator>();
            vibrators.add(vib);
            addHaptic(deviceId_VIBRATOR_SERVICE, "VIBRATOR_SERVICE", vibrators);
        }

        InputManager im = (InputManager) context.getSystemService(Context.INPUT_SERVICE);
        im.registerInputDeviceListener(this, new Handler(Looper.getMainLooper()));

        int[] deviceIds = InputDevice.getDeviceIds();
        for (int device_id : deviceIds) {
             this.onInputDeviceAdded(device_id);
        }
    }

    public void unsubscribe() {
        InputManager im = (InputManager) SDLActivity.getContext().getSystemService(Context.INPUT_SERVICE);
        im.unregisterInputDeviceListener(this);

        mHaptics.clear();

        SDLControllerManager.nativeRemoveHaptic(deviceId_VIBRATOR_SERVICE);
    }

    @Override
    public void onInputDeviceAdded(int device_id) {
        {
            SDLHaptic haptic = getHaptic(device_id);
            if (haptic == null) {
                InputDevice device = InputDevice.getDevice(device_id);
                if (device == null || device.isVirtual()) {
                    return;
                }

                ArrayList<Vibrator> vibrators = SDLControllerManager.getDeviceVibrators(device);
                if (vibrators.size() > 0) {
                    addHaptic(device_id, device.getName(), vibrators);
                }
            }
        }
    }

    @Override
    public void onInputDeviceRemoved(int device_id) {
        for (int i = 0; i < mHaptics.size(); i++) {
            if (mHaptics.get(i).device_id == device_id) {
                mHaptics.remove(i);
                SDLControllerManager.nativeRemoveHaptic(device_id);
                break;
            }
        }
    }

    @Override
    public void onInputDeviceChanged(int device_id) {
        // Log.d(TAG, "InputDeviceChanged: " + device_id);
    }

    public void addHaptic(int device_id, String name, ArrayList<Vibrator> vibs) {
        SDLHaptic haptic = getHaptic(device_id);
        if (haptic == null) {
            haptic = new SDLHaptic();
            haptic.device_id = device_id;
            haptic.name = name;
            haptic.vibs = vibs;
            mHaptics.add(haptic);
            SDLControllerManager.nativeAddHaptic(haptic.device_id, haptic.name);
        }
    }

    protected SDLHaptic getHaptic(int device_id) {
        for (SDLHaptic haptic : mHaptics) {
            if (haptic.device_id == device_id) {
                return haptic;
            }
        }
        return null;
    }
}

class SDLGenericMotionListener_API14 implements View.OnGenericMotionListener {
    // Generic Motion (mouse hover, joystick...) events go here
    @Override
    public boolean onGenericMotion(View v, MotionEvent event) {
        float x, y;
        int action, pointerCount;
        boolean relative, consumed = false;
        if (event.getSource() == InputDevice.SOURCE_JOYSTICK)
            return SDLControllerManager.handleJoystickMotionEvent(event);

        action = event.getActionMasked();
        pointerCount = event.getPointerCount();

        for (int i = 0; i < pointerCount; i++) {
            int toolType = event.getToolType(i);

            if (toolType == MotionEvent.TOOL_TYPE_MOUSE) {
                switch (action) {
                    case MotionEvent.ACTION_SCROLL:
                        x = event.getAxisValue(MotionEvent.AXIS_HSCROLL, i);
                        y = event.getAxisValue(MotionEvent.AXIS_VSCROLL, i);
                        relative = false;
                        break;

                    case MotionEvent.ACTION_HOVER_MOVE:
                        x = getEventX(event, i);
                        y = getEventY(event, i);
                        relative = checkRelativeEvent(event);
                        break;

                    default:
                        continue;
                }
                SDLActivity.onNativeMouse(i, action, x, y, relative);
                consumed = true;
            }
        }

        return consumed;
    }

    public boolean supportsRelativeMouse() {
        return false;
    }

    public boolean inRelativeMode() {
        return false;
    }

    public void setRelativeMouseEnabled(boolean enabled) {
    }

    public void reclaimRelativeMouseModeIfNeeded() {

    }

    public boolean checkRelativeEvent(MotionEvent event) {
        return inRelativeMode();
    }

    public float getEventX(MotionEvent event, int pointerIndex) {
        return event.getX(pointerIndex);
    }

    public float getEventY(MotionEvent event, int pointerIndex) {
        return event.getY(pointerIndex);
    }

}

class SDLGenericMotionListener_API24 extends SDLGenericMotionListener_API14 {
    // Generic Motion (mouse hover, joystick...) events go here

    private boolean mRelativeModeEnabled;

    @Override
    public boolean supportsRelativeMouse() {
        // DeX mode in Samsung Experience 9.0 and earlier doesn't support relative mice properly under
        // Android 7 APIs, and simply returns no data under Android 8 APIs.
        //
        // This is fixed in Samsung Experience 9.5, which corresponds to Android 8.1.0, and
        // thus SDK version 27.  If we are in DeX mode and not API 27 or higher, as a result,
        // we should stick to relative mode.
        //
        return (Build.VERSION.SDK_INT >= 27 /* Android 8.1 (O_MR1) */ || !SDLActivity.isDeXMode());
    }

    @Override
    public boolean inRelativeMode() {
        return mRelativeModeEnabled;
    }

    @Override
    public void setRelativeMouseEnabled(boolean enabled) {
        mRelativeModeEnabled = enabled;
    }

    @Override
    public float getEventX(MotionEvent event, int pointerIndex) {
        if (mRelativeModeEnabled /*&& event.getToolType(pointerIndex) == MotionEvent.TOOL_TYPE_MOUSE*/) {
            return event.getAxisValue(MotionEvent.AXIS_RELATIVE_X, pointerIndex);
        } else {
            return event.getX(pointerIndex);
        }
    }

    @Override
    public float getEventY(MotionEvent event, int pointerIndex) {
        if (mRelativeModeEnabled /*&& event.getToolType(pointerIndex) == MotionEvent.TOOL_TYPE_MOUSE*/) {
            return event.getAxisValue(MotionEvent.AXIS_RELATIVE_Y, pointerIndex);
        } else {
            return event.getY(pointerIndex);
        }
    }
}

class SDLGenericMotionListener_API26 extends SDLGenericMotionListener_API24 {
    // Generic Motion (mouse hover, joystick...) events go here
    private boolean mRelativeModeEnabled;

    @Override
    public boolean inRelativeMode() {
        return mRelativeModeEnabled;
    }

    @Override
    public void setRelativeMouseEnabled(boolean enabled) {
        if (enabled) {
            SDLActivity.getContentView().requestPointerCapture();
        } else {
            SDLActivity.getContentView().releasePointerCapture();
        }
        mRelativeModeEnabled = enabled;
    }

    @Override
    public void reclaimRelativeMouseModeIfNeeded() {
        if (mRelativeModeEnabled && !SDLActivity.isDeXMode()) {
            SDLActivity.getContentView().requestPointerCapture();
        }
    }

    @Override
    public boolean checkRelativeEvent(MotionEvent event) {
        return event.getSource() == InputDevice.SOURCE_MOUSE_RELATIVE;
    }

    @Override
    public float getEventX(MotionEvent event, int pointerIndex) {
        // Relative mouse in capture mode will only have relative for X/Y
        return event.getX(pointerIndex);
    }

    @Override
    public float getEventY(MotionEvent event, int pointerIndex) {
        // Relative mouse in capture mode will only have relative for X/Y
        return event.getY(pointerIndex);
    }
}
