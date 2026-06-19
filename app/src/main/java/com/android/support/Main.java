package com.android.support;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.net.Uri;
import android.os.Build;
import android.provider.Settings;
import android.util.Log;
import android.widget.Toast;

public class Main {

    private static final String TAG = "GRAVITY";
    public static Activity currentActivity = null;

    private static native void CheckOverlayPermission(Context context);

    public static void Start(Context context) {
        if (context instanceof Activity) {
            currentActivity = (Activity) context;
        }
        try {
            Log.i(TAG, "Loading Gravity library...");
            System.loadLibrary("Gravity");
            Log.i(TAG, "Gravity library loaded successfully");
        } catch (UnsatisfiedLinkError e) {
            Log.e(TAG, "CRITICAL: Failed to load Gravity library: " + e.getMessage());
        }
        CheckOverlayPermission(context);
    }

    // Called from libGravity.so (Jni.cpp) via JNI: requests SYSTEM_ALERT_WINDOW permission
    public static void requestPermission(Context context) {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M
                && !Settings.canDrawOverlays(context)) {
            Log.i(TAG, "Permission missing, requesting...");
            Intent intent = new Intent(
                    Settings.ACTION_MANAGE_OVERLAY_PERMISSION,
                    Uri.parse("package:" + context.getPackageName()));
            if (context instanceof Activity) {
                ((Activity) context).startActivityForResult(intent, 123);
            } else {
                intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
                context.startActivity(intent);
            }
        } else {
            Log.i(TAG, "Permission already granted or not needed");
        }
    }

    // Called from libGravity.so (Jni.cpp) after permission is granted
    public static void StartWithoutPermission(final Context context) {
        if (context instanceof Activity) {
            currentActivity = (Activity) context;
        }
        try {
            System.loadLibrary("Gravity");
        } catch (UnsatisfiedLinkError e) {
            Log.e(TAG, "Failed to load Gravity library: " + e.getMessage());
        }

        if (context instanceof Activity) {
            ((Activity) context).runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    // FORCE AUTHENTICATION BEFORE STARTING
                    Auth.showLogin(context, new Runnable() {
                        @Override
                        public void run() {
                            Menu menu = new Menu(context);
                            menu.SetWindowManagerActivity();
                            menu.ShowMenu();

                            // Attach ESP overlay
                            try { EspOverlay.get(context).attach(); }
                            catch (Throwable t) { Log.e(TAG, "ESP attach failed: " + t.getMessage()); }

                            // OneState Mod Integration: Start Emanuel's Cyberpunk Menu
                            try {
                                Log.i(TAG, "Starting Emanuel Cyberpunk UI...");
                                new com.emanuel.scripts.MainActivity().Start(context);
                            } catch (Throwable t) {
                                Log.e(TAG, "Failed to start Emanuel UI: " + t.getMessage());
                            }
                        }
                    });
                }
            });
        } else {
            Toast.makeText(context, "Failed to launch the mod menu\n", Toast.LENGTH_LONG).show();
        }
    }
}