# Add project specific ProGuard rules here.
# By default, the flags in this file are appended to flags specified
# in [sdk]/tools/proguard/proguard-android.txt
# You can edit the include path and order by changing the proguardFiles
# directive in build.gradle.
#
# For more details, see
#   http://developer.android.com/guide/developing/tools/proguard.html

# Add any project specific keep options here:

# If your project uses WebView with JS, uncomment the following
# and specify the fully qualified class name to the JavaScript interface
# class:
#-keepclassmembers class fqcn.of.javascript.interface.for.webview {
#   public *;
#}

-keep,includedescriptorclasses,allowoptimization class org.libsdl.app.SDLInputConnection {
    # JNI to SDL interface
    void nativeCommitText(java.lang.String, int);
    void nativeGenerateScancodeForUnichar(char);
}

-keep,includedescriptorclasses,allowoptimization class org.libsdl.app.SDLActivity {
    # JNI to SDL interface
    java.lang.String nativeGetVersion();
    int nativeSetupJNI();
    int nativeRunMain(java.lang.String, java.lang.String, java.lang.Object );
    void nativeLowMemory();
    void nativeSendQuit();
    void nativeQuit();
    void nativePause();
    void nativeResume();
    void nativeFocusChanged(boolean);
    void onNativeDropFile(java.lang.String);
    void nativeSetScreenResolution(int, int, int, int, float);
    void onNativeResize();
    void onNativeKeyDown(int);
    void onNativeKeyUp(int);
    boolean onNativeSoftReturnKey();
    void onNativeKeyboardFocusLost();
    void onNativeMouse(int, int, float, float, boolean);
    void onNativeTouch(int, int, int, float, float, float);
    void onNativeAccel(float, float, float);
    void onNativeClipboardChanged();
    void onNativeSurfaceCreated();
    void onNativeSurfaceChanged();
    void onNativeSurfaceDestroyed();
    java.lang.String nativeGetHint(java.lang.String);
    boolean nativeGetHintBoolean(java.lang.String, boolean);
    void nativeSetenv(java.lang.String, java.lang.String);
    void onNativeOrientationChanged(int);
    void nativeAddTouch(int, java.lang.String);
    void nativePermissionResult(int, boolean);
    void onNativeLocaleChanged();
    # SDL to JNI interface
    boolean supportsRelativeMouse();
    void setWindowStyle(boolean);
    boolean isScreenKeyboardShown();
    android.util.DisplayMetrics getDisplayDPI();
    java.lang.String clipboardGetText();
    boolean clipboardHasText();
    void clipboardSetText(java.lang.String);
    int createCustomCursor(int[], int, int, int, int);
    void destroyCustomCursor(int);
    android.content.Context getContext();
    boolean getManifestEnvironmentVariables();
    android.view.Surface getNativeSurface();
    void initTouch();
    boolean isAndroidTV();
    boolean isChromebook();
    boolean isDeXMode();
    boolean isTablet();
    void manualBackButton();
    int messageboxShowMessageBox(int, java.lang.String, java.lang.String, int[], int[], java.lang.String[], int[]);
    void minimizeWindow();
    int openURL(java.lang.String);
    void requestPermission(java.lang.String, int);
    int showToast(java.lang.String, int, int, int, int);
    boolean sendMessage(int, int);
    boolean setActivityTitle(java.lang.String);
    boolean setCustomCursor(int);
    void setOrientation(int, int, boolean, java.lang.String);
    boolean setRelativeMouseEnabled(boolean);
    boolean setSystemCursor(int);
    boolean shouldMinimizeOnFocusLoss();
    boolean showTextInput(int, int, int, int);
}

-keep,includedescriptorclasses,allowoptimization class org.libsdl.app.HIDDeviceManager {
    # JNI to SDL interface
    void HIDDeviceRegisterCallback();
    void HIDDeviceReleaseCallback();
    void HIDDeviceConnected(int, java.lang.String, int, int, java.lang.String, int, java.lang.String, java.lang.String, int, int, int, int);
    void HIDDeviceOpenPending(int);
    void HIDDeviceOpenResult(int, boolean);
    void HIDDeviceDisconnected(int);
    void HIDDeviceInputReport(int, byte[]);
    void HIDDeviceFeatureReport(int, byte[]);
    # SDL to JNI interface
    boolean initialize(boolean, boolean);
    boolean openDevice(int);
    int sendOutputReport(int, byte[]);
    int sendFeatureReport(int, byte[]);
    boolean getFeatureReport(int, byte[]);
    void closeDevice(int);
}

-keep,includedescriptorclasses,allowoptimization class org.libsdl.app.SDLAudioManager {
    # SDL to JNI interface
    int[] getAudioOutputDevices();
    int[] getAudioInputDevices();
    int[] audioOpen(int, int, int, int, int);
    void audioWriteFloatBuffer(float[]);
    void audioWriteShortBuffer(short[]);
    void audioWriteByteBuffer(byte[]);
    void audioClose();
    int[] captureOpen(int, int, int, int, int);
    int captureReadFloatBuffer(float[], boolean);
    int captureReadShortBuffer(short[], boolean);
    int captureReadByteBuffer(byte[], boolean);
    void captureClose();
    void audioSetThreadPriority(boolean, int);
    # JNI to SDL interface
    int nativeSetupJNI();
    void removeAudioDevice(boolean, int);
    void addAudioDevice(boolean, int);
}

-keep,includedescriptorclasses,allowoptimization class org.libsdl.app.SDLControllerManager {
    # SDL to JNI interface
    void pollInputDevices();
    void pollHapticDevices();
    void hapticRun(int, float, int);
    void hapticRumble(int, float, float, int);
    void hapticStop(int);
    # JNI to SDL interface
    int nativeSetupJNI();
    int nativeAddJoystick(int, java.lang.String, java.lang.String, int, int, int, int, int, int, boolean);
    int nativeRemoveJoystick(int);
    int nativeAddHaptic(int, java.lang.String);
    int nativeRemoveHaptic(int);
    int onNativePadDown(int, int);
    int onNativePadUp(int, int);
    void onNativeJoy(int, int, float);
    void onNativeHat(int, int, int, int);
}
