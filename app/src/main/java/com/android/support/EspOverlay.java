package com.android.support;

import android.annotation.SuppressLint;
import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.PixelFormat;
import android.graphics.Rect;
import android.os.Build;
import android.os.Handler;
import android.os.Looper;
import android.util.DisplayMetrics;
import android.util.Log;
import android.view.Gravity;
import android.view.View;
import android.view.ViewGroup;
import android.view.Choreographer;
import android.view.WindowManager;

/**
 * Transparent fullscreen overlay that draws ESP information on top of the game.
 *
 * - Built on top of {@link WindowManager.LayoutParams.TYPE_APPLICATION_OVERLAY}
 *   (or {@link WindowManager.LayoutParams.TYPE_PHONE} on legacy devices), the
 *   same window type used by the floating mod menu.
 * - Reads ped data each tick from {@link EspBridge#getData()} and paints
 *   bounding boxes + distance text using a {@link Canvas}.
 *
 * Singleton because we only ever want one ESP overlay attached.
 */
public final class EspOverlay {

    private static final String TAG = "GravityESP";
    private static final int FRAME_PERIOD_MS = 16; // 60 Hz : synchro framerate Unity

    private static EspOverlay sInstance;

    public static synchronized EspOverlay get(Context context) {
        if (sInstance == null) sInstance = new EspOverlay(context.getApplicationContext());
        return sInstance;
    }

    private final Context mAppContext;
    private final WindowManager mWm;
    private DrawView mView;
    private boolean mAttached = false;
    // Choreographer = sync vsync exact (60/90/120Hz selon écran). On invalidate
    // la View à chaque frame d'écran pour que la projection W2S côté Java
    // utilise toujours la VP matrix la plus fraîche (capturée par le hook
    // Camera.OnPreCull en C++). Résultat : zéro décalage caméra/box.
    private final Choreographer.FrameCallback mFrameCallback = new Choreographer.FrameCallback() {
        @Override public void doFrame(long frameTimeNanos) {
            if (mView != null) mView.invalidate();
            if (mAttached) Choreographer.getInstance().postFrameCallback(this);
        }
    };

    private EspOverlay(Context appCtx) {
        mAppContext = appCtx;
        mWm = (WindowManager) appCtx.getSystemService(Context.WINDOW_SERVICE);
    }

    /** Adds the overlay to the WindowManager if not already attached. */
    @SuppressLint("ClickableViewAccessibility")
    public void attach() {
        if (mAttached) return;
        try {
            mView = new DrawView(mAppContext);
            int type;
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
                type = WindowManager.LayoutParams.TYPE_APPLICATION_OVERLAY;
            } else {
                //noinspection deprecation
                type = WindowManager.LayoutParams.TYPE_PHONE;
            }
            WindowManager.LayoutParams lp = new WindowManager.LayoutParams(
                    ViewGroup.LayoutParams.MATCH_PARENT,
                    ViewGroup.LayoutParams.MATCH_PARENT,
                    type,
                    WindowManager.LayoutParams.FLAG_NOT_FOCUSABLE
                            | WindowManager.LayoutParams.FLAG_NOT_TOUCHABLE
                            | WindowManager.LayoutParams.FLAG_LAYOUT_IN_SCREEN
                            | WindowManager.LayoutParams.FLAG_LAYOUT_NO_LIMITS,
                    PixelFormat.TRANSLUCENT);
            lp.gravity = Gravity.TOP | Gravity.START;
            mWm.addView(mView, lp);
            mAttached = true;

            DisplayMetrics dm = mAppContext.getResources().getDisplayMetrics();
            EspBridge.setScreenSize(dm.widthPixels, dm.heightPixels);

            Choreographer.getInstance().postFrameCallback(mFrameCallback);
            Log.i(TAG, "EspOverlay attached " + dm.widthPixels + "x" + dm.heightPixels);
        } catch (Throwable t) {
            Log.e(TAG, "EspOverlay attach failed: " + t.getMessage(), t);
        }
    }

    public void detach() {
        if (!mAttached) return;
        try {
            Choreographer.getInstance().removeFrameCallback(mFrameCallback);
            mWm.removeView(mView);
        } catch (Throwable ignored) { }
        mAttached = false;
        mView = null;
    }

    /** Inner View that does the actual Canvas drawing. */
    private static final class DrawView extends View {
        private final Paint mBoxPaint;
        private final Paint mBoxOutline;
        private final Paint mTextPaint;
        private final Paint mTextShadow;
        private final Paint mLinkPaint;     // ligne icône → box
        private final Paint mGlowPaint;     // halo de la box pendant l'apparition
        private final Paint mIconAura;      // halo qui pulse autour de l'icône LGL
        private final Rect mTmpRect = new Rect();

        // Variables réutilisées pour la fluidité (évite garbage-collector freezes)
        private final Paint mHealthBgPaint;
        private final Paint mHealthBarPaint;
        private final android.graphics.Path mMarkerPath;
        private final Paint mCrosshairCirclePaint;
        private final Paint mCrosshairCircleOutline;
        private final Paint mCrosshairPaint;
        private final Paint mCrosshairOutline;
        private final Paint mCrosshairDotPaint;

        // Animation : on track les boxes vues à la dernière frame pour détecter
        // les "nouvelles" et leur jouer une anim d'apparition (scale + glow +
        // ligne qui jaillit de l'icône LGL). Clé = signature stable basée sur
        // la position écran arrondie (on n'a pas d'ID stable côté Java).
        private final java.util.HashMap<Long, Long> mFirstSeenMs = new java.util.HashMap<>();
        private static final long INTRO_DURATION_MS = 700; // durée anim apparition

        DrawView(Context c) {
            super(c);
            mBoxPaint = new Paint(Paint.ANTI_ALIAS_FLAG);
            mBoxPaint.setStyle(Paint.Style.STROKE);
            mBoxPaint.setStrokeWidth(2.0f);
            mBoxPaint.setColor(Color.parseColor("#FF0055"));

            mBoxOutline = new Paint(Paint.ANTI_ALIAS_FLAG);
            mBoxOutline.setStyle(Paint.Style.STROKE);
            mBoxOutline.setStrokeWidth(4.0f);
            mBoxOutline.setColor(Color.parseColor("#80000000"));

            mTextPaint = new Paint(Paint.ANTI_ALIAS_FLAG);
            mTextPaint.setColor(Color.parseColor("#00FFFF"));
            mTextPaint.setTextSize(22.0f);
            mTextPaint.setFakeBoldText(true);

            mTextShadow = new Paint(Paint.ANTI_ALIAS_FLAG);
            mTextShadow.setColor(Color.parseColor("#FF000000"));
            mTextShadow.setTextSize(22.0f);
            mTextShadow.setFakeBoldText(true);

            mLinkPaint = new Paint(Paint.ANTI_ALIAS_FLAG);
            mLinkPaint.setStyle(Paint.Style.STROKE);
            mLinkPaint.setStrokeWidth(2.5f);
            mLinkPaint.setColor(Color.parseColor("#AA00FFFF"));
            // Pointillé qui défile en animant le phase via setPathEffect.
            mLinkPaint.setStrokeCap(Paint.Cap.ROUND);

            mGlowPaint = new Paint(Paint.ANTI_ALIAS_FLAG);
            mGlowPaint.setStyle(Paint.Style.STROKE);
            mGlowPaint.setStrokeWidth(6.0f);
            mGlowPaint.setColor(Color.parseColor("#FF00FFFF"));

            mIconAura = new Paint(Paint.ANTI_ALIAS_FLAG);
            mIconAura.setStyle(Paint.Style.STROKE);
            mIconAura.setStrokeWidth(3.0f);
            mIconAura.setColor(Color.parseColor("#9000FFFF"));

            // Initialisation unique des objets de dessin réutilisables
            mHealthBgPaint = new Paint();
            mHealthBgPaint.setColor(Color.BLACK);
            mHealthBgPaint.setStyle(Paint.Style.FILL);

            mHealthBarPaint = new Paint();
            mHealthBarPaint.setStyle(Paint.Style.FILL);

            mMarkerPath = new android.graphics.Path();

            mCrosshairCirclePaint = new Paint(Paint.ANTI_ALIAS_FLAG);
            mCrosshairCirclePaint.setColor(Color.GREEN);
            mCrosshairCirclePaint.setStrokeWidth(2.0f);
            mCrosshairCirclePaint.setStyle(Paint.Style.STROKE);

            mCrosshairCircleOutline = new Paint(Paint.ANTI_ALIAS_FLAG);
            mCrosshairCircleOutline.setColor(Color.BLACK);
            mCrosshairCircleOutline.setStrokeWidth(4.0f);
            mCrosshairCircleOutline.setStyle(Paint.Style.STROKE);

            mCrosshairPaint = new Paint(Paint.ANTI_ALIAS_FLAG);
            mCrosshairPaint.setColor(Color.GREEN);
            mCrosshairPaint.setStrokeWidth(3.0f);
            mCrosshairPaint.setStyle(Paint.Style.STROKE);

            mCrosshairOutline = new Paint(Paint.ANTI_ALIAS_FLAG);
            mCrosshairOutline.setColor(Color.BLACK);
            mCrosshairOutline.setStrokeWidth(5.0f);
            mCrosshairOutline.setStyle(Paint.Style.STROKE);

            mCrosshairDotPaint = new Paint(Paint.ANTI_ALIAS_FLAG);
            mCrosshairDotPaint.setColor(Color.GREEN);
            mCrosshairDotPaint.setStyle(Paint.Style.FILL);

            setWillNotDraw(false);
        }

        // Couleurs par type de ped (rendu ESP).
        //  PLAYER  = rouge vif (cible prioritaire)
        //  POLICE  = bleu      (forces de l'ordre)
        //  NPC     = gris      (PNJ peu intéressants)
        //  UNKNOWN = magenta   (non classé, fallback)
        private int colorForType(int type) {
            switch (type) {
                case EspBridge.TYPE_PLAYER: return android.graphics.Color.parseColor("#FF2828"); // rouge vif
                case EspBridge.TYPE_POLICE: return android.graphics.Color.parseColor("#3DA5FF"); // bleu
                case EspBridge.TYPE_NPC:    return android.graphics.Color.parseColor("#A0A0A0"); // gris
                default:                    return android.graphics.Color.parseColor("#FF00FF"); // magenta fallback
            }
        }

        private String labelForType(int type) {
            switch (type) {
                case EspBridge.TYPE_PLAYER: return "PLAYER";
                case EspBridge.TYPE_POLICE: return "COP";
                case EspBridge.TYPE_NPC:    return "NPC";
                default:                    return "PED";
            }
        }

        // Clé stable d'un ped basée sur son INDEX dans le snapshot natif.
        // Le tableau retourné par EspBridge.getData() conserve son ordre tant
        // que le scan IL2CPP (refresh à 2Hz) ne se relance pas. C'est bien plus
        // stable qu'une clé position-based : pas de re-démarrage de l'anim
        // chaque fois que le ped bouge un peu = pas de clignotement.
        private long pedKey(int index) {
            return (long) index;
        }

        // Buffer matrix réutilisé chaque frame (évite alloc).
        private float[] mVpBuf = new float[18];

        // Projection World->Screen avec la VP courante (column-major).
        // Retourne true si le point est devant la caméra. Remplit out[0]=screenX,
        // out[1]=screenY, out[2]=clipW (sert à filtrer derrière la caméra).
        private boolean worldToScreen(float[] vp, int pixW, int pixH,
                                      float wx, float wy, float wz, float[] out) {
            float cx = vp[0]*wx + vp[4]*wy + vp[8] *wz + vp[12];
            float cy = vp[1]*wx + vp[5]*wy + vp[9] *wz + vp[13];
            float cw = vp[3]*wx + vp[7]*wy + vp[11]*wz + vp[15];
            if (cw <= 0.001f) { out[2] = cw; return false; }
            float ndcX = cx / cw;
            float ndcY = cy / cw;
            out[0] = (ndcX * 0.5f + 0.5f) * pixW;
            out[1] = (1.0f - (ndcY * 0.5f + 0.5f)) * pixH;
            out[2] = cw;
            return true;
        }

        @Override protected void onDraw(Canvas canvas) {
            super.onDraw(canvas);
            if (!EspBridge.isEnabled()) return;
            float[] data;
            float[] vpAndDims;
            try {
                data = EspBridge.getData();
                vpAndDims = EspBridge.getViewProjection();
            } catch (Throwable t) {
                return;
            }
            if (data == null || data.length < 1) return;
            if (vpAndDims == null || vpAndDims.length < 18) return;
            int n = (int) data[0];
            if (n <= 0) return;
            int stride = EspBridge.STRIDE;

            // VP matrix + dimensions caméra Unity.
            int camW = (int) vpAndDims[16];
            int camH = (int) vpAndDims[17];
            if (camW <= 0 || camH <= 0) return;
            // Facteur de mise à l'échelle caméra Unity → résolution écran Android.
            float viewW = getWidth();
            float viewH = getHeight();
            if (viewW <= 0 || viewH <= 0) return;
            float scaleX = viewW / camW;
            float scaleY = viewH / camH;

            mBoxPaint.setColor(0xFFFF1A1A);
            mBoxPaint.setAlpha(255);
            mBoxOutline.setColor(0xAA000000);
            mTextPaint.setColor(0xFFFFFFFF);
            mTextShadow.setColor(0xFF000000);

            boolean dotMode;
            try { dotMode = EspBridge.isDotMode(); } catch (Throwable t) { dotMode = false; }
            boolean lineOn, boxOn, distOn, markerOn, dynColor;
            try {
                lineOn = EspBridge.isLineEnabled();
                boxOn = EspBridge.isBoxEnabled();
                distOn = EspBridge.isDistanceEnabled();
                markerOn = EspBridge.isMarkerEnabled();
                dynColor = EspBridge.isDynamicColor();
            } catch (Throwable t) {
                lineOn = true; boxOn = true; distOn = true; markerOn = false; dynColor = false;
            }

            String hud = "ESP " + n;
            canvas.drawText(hud, 41, 51, mTextShadow);
            canvas.drawText(hud, 40, 50, mTextPaint);

            // Icône LGL center pour les lignes ESP
            int iconCX = Menu.getIconCenterX();
            int iconCY = Menu.getIconCenterY();

            // Buffer réutilisé pour chaque projection.
            float[] feetOut = new float[3];
            float[] headOut = new float[3];

            // Couleur dynamique basée sur un cycle RGB
            long nowMs = System.currentTimeMillis();
            float hueCycle = (nowMs % 3000) / 3000.0f;

            String[] names = null;
            try { names = EspBridge.getPedNames(); } catch (Throwable t) {}

            for (int i = 0; i < n; i++) {
                int o = 1 + i * stride;
                if (o + (stride - 1) >= data.length) break;
                float feetWorldX = data[o + 0];
                float feetWorldY = data[o + 1];
                float feetWorldZ = data[o + 2];
                float dist = data[o + 3];
                boolean isLocal = data[o + 4] > 0.5f;
                if (isLocal) continue;

                float headWorldY = feetWorldY + 1.8f;
                float health = 1.0f;
                int pedType = EspBridge.TYPE_UNKNOWN;

                // Projection temps réel synchronisée avec le vsync d'Android (sans lag caméra)
                boolean okF = worldToScreen(vpAndDims, camW, camH, feetWorldX, feetWorldY, feetWorldZ, feetOut);
                boolean okH = worldToScreen(vpAndDims, camW, camH, feetWorldX, headWorldY, feetWorldZ, headOut);
                if (!okF || !okH) continue;

                // Conversion vers les coordonnées de l'overlay de l'écran Android
                float feetScreenX = feetOut[0] * scaleX;
                float feetScreenY = feetOut[1] * scaleY;
                float headScreenX = headOut[0] * scaleX;
                float headScreenY = headOut[1] * scaleY;

                float midX = (feetScreenX + headScreenX) * 0.5f;
                float topY = Math.min(feetScreenY, headScreenY);
                float botY = Math.max(feetScreenY, headScreenY);
                float h = botY - topY;
                if (h < 8.0f) h = 50.0f;
                float marginY = h * 0.10f;
                topY -= marginY;
                botY += marginY * 0.5f;
                h = botY - topY;
                float w = h * 0.45f;
                float left = midX - w * 0.5f;
                float top  = topY;
                float right = left + w;
                float bottom = botY;

                // Couleur dynamique ou fixe
                int pedColor = colorForType(pedType);
                if (dynColor) {
                    float hue = (hueCycle + i * 0.1f) % 1.0f;
                    pedColor = android.graphics.Color.HSVToColor(new float[]{hue * 360f, 1f, 1f});
                }
                mBoxPaint.setColor(pedColor);

                // ESP LIGNE : relie l'icône LGL au centre du ped
                if (lineOn) {
                    float cx = (left + right) * 0.5f;
                    float cy = (top + bottom) * 0.5f;
                    mLinkPaint.setColor(pedColor);
                    mLinkPaint.setAlpha(160);
                    canvas.drawLine(iconCX, iconCY, cx, cy, mLinkPaint);
                }

                // ESP BOX
                if (boxOn) {
                    canvas.drawRect(left, top, right, bottom, mBoxOutline);
                    canvas.drawRect(left, top, right, bottom, mBoxPaint);
                }

                // ESP BARRE DE VIE (Health Bar)
                boolean healthOn;
                try { healthOn = EspBridge.isHealthEnabled(); } catch (Throwable t) { healthOn = false; }
                if (healthOn && health >= 0f) {
                    float barWidth = 5.0f;
                    float barGap = 5.0f;
                    float barLeft = left - barWidth - barGap;
                    float barRight = barLeft + barWidth;
                    
                    // Contour noir de la barre
                    canvas.drawRect(barLeft - 1, top - 1, barRight + 1, bottom + 1, mHealthBgPaint);
                    
                    // Barre de vie (vert -> rouge)
                    int healthColor;
                    if (health > 0.5f) {
                        healthColor = Color.rgb((int)((1.0f - health) * 2.0f * 255), 255, 0);
                    } else {
                        healthColor = Color.rgb(255, (int)(health * 2.0f * 255), 0);
                    }
                    mHealthBarPaint.setColor(healthColor);
                    
                    float barTop = bottom - (h * health);
                    canvas.drawRect(barLeft, barTop, barRight, bottom, mHealthBarPaint);
                }

                // ESP SKELETON
                boolean skeletonOn;
                try { skeletonOn = EspBridge.isSkeletonEnabled(); } catch (Throwable t) { skeletonOn = false; }
                if (skeletonOn) {
                    float[][] bones = new float[16][2];
                    boolean[] bonesOk = new boolean[16];
                    for (int b = 0; b < 16; b++) {
                        float bx = data[o + 5 + b * 3 + 0];
                        float by = data[o + 5 + b * 3 + 1];
                        float bz = data[o + 5 + b * 3 + 2];
                        if (bx == 0f && by == 0f && bz == 0f) {
                            bonesOk[b] = false;
                        } else {
                            float[] bOut = new float[3];
                            boolean ok = worldToScreen(vpAndDims, camW, camH, bx, by, bz, bOut);
                            if (ok) {
                                bones[b][0] = bOut[0] * scaleX;
                                bones[b][1] = bOut[1] * scaleY;
                                bonesOk[b] = true;
                            } else {
                                bonesOk[b] = false;
                            }
                        }
                    }
                    int[][] pairs = {
                        {0, 1}, {1, 2}, {2, 3}, // spine
                        {1, 4}, {4, 5}, {5, 6}, // left arm
                        {1, 7}, {7, 8}, {8, 9}, // right arm
                        {3, 10}, {10, 11}, {11, 12}, // left leg
                        {3, 13}, {13, 14}, {14, 15} // right leg
                    };
                    for (int p = 0; p < pairs.length; p++) {
                        int b1 = pairs[p][0], b2 = pairs[p][1];
                        if (bonesOk[b1] && bonesOk[b2]) {
                            canvas.drawLine(bones[b1][0], bones[b1][1], bones[b2][0], bones[b2][1], mBoxPaint);
                        }
                    }
                }

                // ESP MARKER (triangle au-dessus)
                if (markerOn) {
                    float cx = (left + right) * 0.5f;
                    float cy = top - 8f;
                    mMarkerPath.reset();
                    mMarkerPath.moveTo(cx, cy - 12f);
                    mMarkerPath.lineTo(cx - 8f, cy + 4f);
                    mMarkerPath.lineTo(cx + 8f, cy + 4f);
                    mMarkerPath.close();
                    mBoxPaint.setStyle(android.graphics.Paint.Style.FILL);
                    canvas.drawPath(mMarkerPath, mBoxPaint);
                    mBoxPaint.setStyle(android.graphics.Paint.Style.STROKE);
                }

                if (dotMode) {
                    float cx = (left + right) * 0.5f;
                    float cy = (top + bottom) * 0.5f;
                    float r = (right - left) * 0.15f;
                    if (r < 4.0f) r = 4.0f;
                    if (r > 14.0f) r = 14.0f;
                    android.graphics.Paint fill = mBoxPaint;
                    fill.setStyle(android.graphics.Paint.Style.FILL);
                    canvas.drawCircle(cx, cy, r, fill);
                    fill.setStyle(android.graphics.Paint.Style.STROKE);
                    canvas.drawCircle(cx, cy, r, mBoxOutline);
                }

                // ESP PSEUDO
                String nameLabel = (names != null && i < names.length && names[i] != null) ? names[i] : "";
                if (boxOn && !nameLabel.isEmpty()) {
                    mTextPaint.setTextSize(20.0f);
                    mTextShadow.setTextSize(20.0f);
                    float tx = (left + right) * 0.5f - mTextPaint.measureText(nameLabel) * 0.5f;
                    float ty = top - 8.0f;
                    canvas.drawText(nameLabel, tx + 1, ty + 1, mTextShadow);
                    canvas.drawText(nameLabel, tx, ty, mTextPaint);
                }

                // ESP DISTANCE
                if (distOn) {
                    String label = ((int) dist) + "m";
                    float tx, ty;
                    if (dotMode) {
                        float cx = (left + right) * 0.5f;
                        float cy = (top + bottom) * 0.5f;
                        float r = (right - left) * 0.15f;
                        if (r < 4.0f) r = 4.0f;
                        tx = cx + r + 4.0f;
                        ty = cy + 8.0f;
                    } else {
                        mTextPaint.setTextSize(18.0f);
                        mTextShadow.setTextSize(18.0f);
                        tx = (left + right) * 0.5f - mTextPaint.measureText(label) * 0.5f;
                        ty = bottom + 20.0f;
                    }
                    canvas.drawText(label, tx + 1, ty + 1, mTextShadow);
                    canvas.drawText(label, tx, ty, mTextPaint);
                }
            }

            // DESSIN DU RETICULE DE VISÉE (CROSSHAIR)
            boolean crosshairOn = false;
            try { crosshairOn = EspBridge.isCrosshairEnabled(); } catch (Throwable t) {}
            if (crosshairOn) {
                float cx = viewW * 0.5f;
                float cy = viewH * 0.5f;
                
                // Dessin du cercle réticule modifiable
                boolean circleOn = false;
                int circleRadius = 50;
                try {
                    circleOn = EspBridge.isCrosshairCircleEnabled();
                    circleRadius = EspBridge.getCrosshairCircleRadius();
                } catch (Throwable t) {}
                
                if (circleOn) {
                    mCrosshairCirclePaint.setStrokeWidth(2.0f);
                    mCrosshairCircleOutline.setStrokeWidth(4.0f);
                    canvas.drawCircle(cx, cy, circleRadius, mCrosshairCircleOutline);
                    canvas.drawCircle(cx, cy, circleRadius, mCrosshairCirclePaint);
                }
                float size = 20.0f; // taille des branches
                float gap = 5.0f;   // espace central
                
                // Contour noir
                canvas.drawLine(cx - size, cy, cx - gap, cy, mCrosshairOutline);
                canvas.drawLine(cx + gap, cy, cx + size, cy, mCrosshairOutline);
                canvas.drawLine(cx, cy - size, cx, cy - gap, mCrosshairOutline);
                canvas.drawLine(cx, cy + gap, cx, cy + size, mCrosshairOutline);
                
                // Branches vertes
                canvas.drawLine(cx - size, cy, cx - gap, cy, mCrosshairPaint);
                canvas.drawLine(cx + gap, cy, cx + size, cy, mCrosshairPaint);
                canvas.drawLine(cx, cy - size, cx, cy - gap, mCrosshairPaint);
                canvas.drawLine(cx, cy + gap, cx, cy + size, mCrosshairPaint);
                
                // Point central
                canvas.drawCircle(cx, cy, 2.0f, mCrosshairDotPaint);
            }
        }
    }
}
