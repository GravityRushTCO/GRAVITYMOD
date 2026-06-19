package com.emanuel.scripts;

import android.app.Activity;
import android.content.Context;
import android.graphics.PixelFormat;
import android.os.Build;
import android.os.Handler;
import android.view.MotionEvent;
import android.view.View;
import android.view.WindowManager;
import android.provider.Settings;

public class MainActivity {
    public static WindowManager manager;
    public static WindowManager.LayoutParams vParams;
    public static View vTouch;

    static {
        System.loadLibrary("EsVip");
    }

    public static void Start(final Context context) {
        if (!(context instanceof Activity)) return;
        final Activity activity = (Activity) context;
        
        activity.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                manager = activity.getWindowManager();
                vParams = getAttributes(false);
                WindowManager.LayoutParams wParams = getAttributes(true);

                GLES3JNIView display = new GLES3JNIView(context);
                vTouch = new View(context);

                manager.addView(vTouch, vParams);
                manager.addView(display, wParams);

                vTouch.setOnTouchListener(new View.OnTouchListener() {
                    @Override
                    public boolean onTouch(View v, MotionEvent event) {
                        float x = event.getRawX();
                        float y = event.getRawY();
                        switch (event.getAction()) {
                            case MotionEvent.ACTION_DOWN:
                            case MotionEvent.ACTION_MOVE:
                                GLES3JNIView.MotionEventClick(true, x, y);
                                break;
                            case MotionEvent.ACTION_UP:
                                GLES3JNIView.MotionEventClick(false, x, y);
                                break;
                        }
                        // Forward the touch event to the game activity so the game is never blocked!
                        try {
                            activity.dispatchTouchEvent(event);
                        } catch (Exception e) {}
                        
                        return false;
                    }
                });

                // Auto-refresh loop or similar logic from smali (MainActivity$2)
                final Handler handler = new Handler();
                handler.postDelayed(new Runnable() {
                    @Override
                    public void run() {
                        // The smali had logic here to update window rect or visibility
                        // We can add it if needed.
                        handler.postDelayed(this, 20);
                    }
                }, 20);
            }
        });
    }

    public static WindowManager.LayoutParams getAttributes(boolean isWindow) {
        WindowManager.LayoutParams params = new WindowManager.LayoutParams(
                WindowManager.LayoutParams.MATCH_PARENT,
                WindowManager.LayoutParams.MATCH_PARENT,
                WindowManager.LayoutParams.TYPE_APPLICATION,
                WindowManager.LayoutParams.FLAG_NOT_FOCUSABLE |
                        WindowManager.LayoutParams.FLAG_NOT_TOUCH_MODAL |
                        WindowManager.LayoutParams.FLAG_LAYOUT_IN_SCREEN |
                        WindowManager.LayoutParams.FLAG_FULLSCREEN |
                        WindowManager.LayoutParams.FLAG_LAYOUT_INSET_DECOR,
                PixelFormat.TRANSLUCENT
        );

        if (isWindow) {
            params.flags |= WindowManager.LayoutParams.FLAG_NOT_TOUCHABLE;
        }

        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.P) {
            params.layoutInDisplayCutoutMode = WindowManager.LayoutParams.LAYOUT_IN_DISPLAY_CUTOUT_MODE_SHORT_EDGES;
        }

        params.gravity = android.view.Gravity.TOP | android.view.Gravity.LEFT;
        params.x = 0;
        params.y = 0;

        return params;
    }
}
