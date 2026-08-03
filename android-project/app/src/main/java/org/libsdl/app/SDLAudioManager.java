package org.libsdl.app;

import android.content.Context;
import android.media.AudioDeviceCallback;
import android.media.AudioDeviceInfo;
import android.media.AudioManager;
import android.os.Build;
import android.util.Log;

public class SDLAudioManager {
    protected static final String TAG = "SDLAudio";

    protected static AudioManager mAudioManager;

    protected static AudioDeviceCallback mAudioDeviceCallback;

    private static void addAudioDevices(AudioDeviceInfo[] devices) {
        for (AudioDeviceInfo deviceInfo : devices) {
            boolean isCapture = deviceInfo.isSink();
            if (!isCapture || deviceInfo.getType() != AudioDeviceInfo.TYPE_TELEPHONY) {
                addAudioDevice(isCapture, deviceInfo.getId());
            }
        }
    }

    public static void create(Context context) {
        if (Build.VERSION.SDK_INT >= 23 /* Android 6.0 (M) */) {
            // create the AudioDeviceCallback
            mAudioDeviceCallback = new AudioDeviceCallback() {
                @Override
                public void onAudioDevicesAdded(AudioDeviceInfo[] addedDevices) {
                    SDLAudioManager.addAudioDevices(addedDevices);
                }

                @Override
                public void onAudioDevicesRemoved(AudioDeviceInfo[] removedDevices) {
                    for (AudioDeviceInfo deviceInfo : removedDevices) {
                        removeAudioDevice(deviceInfo.isSink(), deviceInfo.getId());
                    }
                }
            };
            // register the AudioDeviceCallback
            mAudioManager = (AudioManager) context.getSystemService(Context.AUDIO_SERVICE);
            mAudioManager.registerAudioDeviceCallback(mAudioDeviceCallback, null);
        }
    }

    public static void destroy() {
        if (Build.VERSION.SDK_INT >= 23 /* Android 6.0 (M) */) {
            // unregisterAudioDeviceCallback
            if (mAudioManager != null) {
                mAudioManager.unregisterAudioDeviceCallback(mAudioDeviceCallback);
                mAudioManager = null;
                mAudioDeviceCallback = null;
            }
        }
    }

    /**
     * This method is called by SDL using JNI.
     */
    public static void audioDetectDevices() {
        if (Build.VERSION.SDK_INT >= 23 /* Android 6.0 (M) */) {
            // assert(mAudioManager != null);
            AudioDeviceInfo[] devices = mAudioManager.getDevices(AudioManager.GET_DEVICES_ALL);
            SDLAudioManager.addAudioDevices(devices);
        }
    }

    /** This method is called by SDL using JNI. */
    public static void audioSetThreadPriority(boolean iscapture, int device_id) {
        try {

            /* Set thread name */
            Thread.currentThread().setName((iscapture ? "SDLAudioC" : "SDLAudioP") + device_id);

            /* Set thread priority */
            android.os.Process.setThreadPriority(android.os.Process.THREAD_PRIORITY_AUDIO);

        } catch (Exception e) {
            Log.v(TAG, "modify thread properties failed " + e.toString());
        }
    }

    public static native void nativeSetupJNI();

    public static native void removeAudioDevice(boolean isCapture, int deviceId);

    public static native void addAudioDevice(boolean isCapture, int deviceId);

}
