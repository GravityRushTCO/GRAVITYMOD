package com.emanuel.scripts;

import android.content.Context;
import android.opengl.GLSurfaceView;
import android.view.SurfaceHolder;
import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

public class GLES3JNIView extends GLSurfaceView implements GLSurfaceView.Renderer {

    public GLES3JNIView(Context context) {
        super(context);
        setEGLConfigChooser(8, 8, 8, 8, 16, 0);
        getHolder().setFormat(-3); // PixelFormat.TRANSLUCENT
        setEGLContextClientVersion(3);
        setRenderer(this);
    }

    public static native void MotionEventClick(boolean isDown, float x, float y);
    public static native String getWindowRect();
    public static native void imgui_Shutdown();
    public static native void init();
    public static native void resize(int width, int height);
    public static native void setDeviceID(String deviceId);
    public static native void step();

    @Override
    protected void onDetachedFromWindow() {
        super.onDetachedFromWindow();
        imgui_Shutdown();
    }

    @Override
    public void onDrawFrame(GL10 gl) {
        step();
    }

    @Override
    public void onSurfaceChanged(GL10 gl, int width, int height) {
        resize(width, height);
    }

    @Override
    public void onSurfaceCreated(GL10 gl, EGLConfig config) {
        init();
    }
}
