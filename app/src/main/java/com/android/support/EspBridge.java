package com.android.support;

/**
 * JNI bridge to native ESP module (Esp.cpp).
 * Public methods are called from {@link EspOverlay} via JNI.
 */
public final class EspBridge {
    private EspBridge() {}

    public static native void setScreenSize(int width, int height);
    public static native void setEnabled(boolean enabled);
    public static native boolean isEnabled();

    /**
     * Retourne un float[] : [count, then per-ped 5 floats:
     *   worldX, worldY, worldZ, distance, isLocal(0/1)]
     * Java fait la projection W2S lui-même au moment du onDraw avec la VP
     * la plus récente (cf. {@link #getViewProjection()}). Ça synchronise
     * les boxes au vsync = aucun décalage caméra/box quand on bouge la
     * caméra.
     */
    public static native float[] getData();

    /**
     * Retourne la matrice View*Projection (column-major, 16 floats) capturée
     * au dernier Camera.OnPreCull + [pixelW, pixelH] = 18 floats au total.
     * Null si aucune frame Unity n'a encore été capturée.
     */
    public static native float[] getViewProjection();

    /**
     * Returns a String[] avec les noms GameObject des peds (parallèle à getData()).
     * index i de getPedNames() correspond au ped i de getData().
     * Retourne null si aucun ped.
     */
    public static native String[] getPedNames();

    /** True si le toggle 152 (Dot Mode) est activé : dessine un point au
     *  centre de chaque ped au lieu d'une box. */
    public static native boolean isDotMode();

    /** ESP options (toggles 194-198) */
    public static native boolean isLineEnabled();
    public static native boolean isBoxEnabled();
    public static native boolean isDistanceEnabled();
    public static native boolean isMarkerEnabled();
    public static native boolean isDynamicColor();
    public static native boolean isCrosshairEnabled();
    public static native boolean isCrosshairCircleEnabled();
    public static native int getCrosshairCircleRadius();
    public static native boolean isHealthEnabled();
    public static native boolean isSkeletonEnabled();

    public static final int STRIDE = 53;
    public static final int TYPE_UNKNOWN = 0;
    public static final int TYPE_LOCAL   = 1;
    public static final int TYPE_PLAYER  = 2;
    public static final int TYPE_NPC     = 3;
    public static final int TYPE_POLICE  = 4;
}
