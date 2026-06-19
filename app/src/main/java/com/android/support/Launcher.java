package com.android.support;

import android.app.ActivityManager;
import android.app.Service;
import android.content.Intent;
import android.os.Handler;
import android.os.IBinder;
import android.util.Log;
import android.view.View;

public class Launcher extends Service {

    static {
        try {
            System.loadLibrary("Gravity");
        } catch (UnsatisfiedLinkError e) {
            Log.e("GRAVITY", "Launcher class: Failed to load library: " + e.getMessage());
        }
    }

    Menu menu;

    //When this Class is called the code in this function will be executed
    @Override
    public void onCreate() {
        super.onCreate();

        menu = new Menu(this);
        menu.SetWindowManagerWindowService();
        menu.ShowMenu();

        // Attach ESP overlay (stays inert until toggled ON via menu).
        try {
            EspOverlay.get(this).attach();
        } catch (Throwable t) {
            Log.e("GRAVITY", "ESP attach failed: " + t.getMessage());
        }

        //Create a handler for this Class
        final Handler handler = new Handler();
        handler.post(new Runnable() {
            public void run() {
               Thread();
                handler.postDelayed(this, 1000);
            }
        });
    }

    @Override
    public IBinder onBind(Intent intent) {
        return null;
    }

    //Check if we are still in the game. If now our menu and menu button will dissapear
    private boolean isNotInGame() {
        ActivityManager.RunningAppProcessInfo runningAppProcessInfo = new ActivityManager.RunningAppProcessInfo();
        ActivityManager.getMyMemoryState(runningAppProcessInfo);
        return runningAppProcessInfo.importance != 100;
    }

    private void Thread() {
        // Force visible for testing
        menu.setVisibility(View.VISIBLE);
    }

    //Destroy our View
    public void onDestroy() {
        super.onDestroy();
        menu.onDestroy();
    }

    //Same as above so it wont crash in the background and therefore use alot of Battery life
    public void onTaskRemoved(Intent intent) {
        super.onTaskRemoved(intent);
        try {
            Thread.sleep(100);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
        stopSelf();
    }

    //Override our Start Command so the Service doesnt try to recreate itself when the App is closed
    public int onStartCommand(Intent intent, int i, int i2) {
        return Service.START_NOT_STICKY;
    }
}
