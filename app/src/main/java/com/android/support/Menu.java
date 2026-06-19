//Please don't replace listeners with lambda!

package com.android.support;

import android.annotation.SuppressLint;
import android.app.Activity;
import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.res.AssetFileDescriptor;
import android.content.res.ColorStateList;
import android.graphics.BitmapFactory;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.LinearGradient;
import android.graphics.Paint;
import android.graphics.PixelFormat;
import android.graphics.PorterDuff;
import android.graphics.RadialGradient;
import android.graphics.Shader;
import android.graphics.Typeface;
import android.graphics.drawable.GradientDrawable;
import android.view.Choreographer;
import android.net.Uri;
import android.media.MediaPlayer;
import android.animation.ArgbEvaluator;
import android.animation.ValueAnimator;
import android.os.Build;
import android.os.Handler;
import android.text.Html;
import android.text.InputFilter;
import android.text.InputType;
import android.text.TextUtils;
import android.text.method.DigitsKeyListener;
import android.util.Base64;
import android.util.Log;
import android.util.TypedValue;
import android.view.Gravity;
import android.view.MotionEvent;
import android.view.View;
import android.view.ViewGroup;
import android.view.WindowManager;
import android.view.animation.AccelerateDecelerateInterpolator;
import android.view.animation.OvershootInterpolator;
import android.view.inputmethod.InputMethodManager;
import android.webkit.WebSettings;
import android.webkit.WebView;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.CompoundButton;
import android.widget.EditText;
import android.widget.FrameLayout;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.RadioButton;
import android.widget.RadioGroup;
import android.widget.RelativeLayout;
import android.widget.ScrollView;
import android.widget.SeekBar;
import android.widget.Spinner;
import android.widget.Switch;
import android.widget.TextView;
import android.widget.Toast;

import java.util.Arrays;
import java.util.LinkedList;
import java.util.List;
import java.util.Objects;

import static android.view.ViewGroup.LayoutParams.WRAP_CONTENT;
import static android.view.ViewGroup.LayoutParams.MATCH_PARENT;
import static android.view.WindowManager.LayoutParams.FLAG_TRANSLUCENT_STATUS;
import static android.widget.RelativeLayout.ALIGN_PARENT_LEFT;
import static android.widget.RelativeLayout.ALIGN_PARENT_RIGHT;

public class Menu {
    public static final String TAG = "Mod_Menu";

    // â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•
    // GRAVITY MOD - NEON CYBERPUNK THEME
    // â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•
    int TEXT_COLOR = Color.parseColor("#00FFFF");
    int TEXT_COLOR_2 = Color.parseColor("#C9D1D9");
    int BTN_COLOR = Color.parseColor("#080E1A");
    int MENU_BG_COLOR = Color.parseColor("#D8050510");
    int MENU_FEATURE_BG_COLOR = Color.parseColor("#C0030308");
    int MENU_WIDTH = 310;
    int MENU_HEIGHT = 230;
    int POS_X = 0;
    int POS_Y = 80;

    float MENU_CORNER = 16f;
    int ICON_SIZE = 50;
    float ICON_ALPHA = 1.0f;
    int ToggleON = Color.parseColor("#00FFC0");
    int ToggleOFF = Color.parseColor("#FF0055");
    int BtnON = Color.parseColor("#003322");
    int BtnOFF = Color.parseColor("#33001A");
    int CategoryBG = Color.parseColor("#0A1628");
    int SeekBarColor = Color.parseColor("#9D00FF");
    int SeekBarProgressColor = Color.parseColor("#00FFFF");
    int CheckBoxColor = Color.parseColor("#00FFFF");
    int RadioColor = Color.parseColor("#00FFFF");
    int CollapseColor = Color.parseColor("#0A1628");
    String NumberTxtColor = "#00FFC0";

    // RGB cycle colors used across animations
    private static final int[] RGB_COLORS = {
        Color.parseColor("#FF0055"),   // neon red
        Color.parseColor("#FF6600"),   // neon orange
        Color.parseColor("#FFE600"),   // neon yellow
        Color.parseColor("#00FF66"),   // neon green
        Color.parseColor("#00FFFF"),   // neon cyan
        Color.parseColor("#0088FF"),   // neon blue
        Color.parseColor("#9D00FF"),   // neon purple
        Color.parseColor("#FF00CC"),   // neon pink
        Color.parseColor("#FF0055"),   // back to red (loop)
    };

    RelativeLayout mCollapsed, mRootContainer;
    FrameLayout mExpanded;
    LinearLayout mods, mSettings, mCollapse;
    LinearLayout mExpandedInner;
    LinearLayout.LayoutParams scrlLLExpanded, scrlLL;
    WindowManager mWindowManager;
    WindowManager.LayoutParams vmParams;

    // === DOCK MODE â€” BAR EN BAS D'Ã‰CRAN ===
    // Au tap sur l'icÃ´ne, une barre horizontale apparaÃ®t en bas de l'Ã©cran :
    //  - La DOCK BAR affiche une pastille par catÃ©gorie (icÃ´ne + nom court).
    //  - Tap sur une pastille â†’ un PANNEAU remonte au-dessus du dock,
    //    contenant les features de cette catÃ©gorie (Switch / Button / SeekBar
    //    natifs, donc le curseur de vitesse est visible comme un vrai slider).
    //  - Tap sur la même pastille → ferme le panneau.
    //  - Tap sur l'icône flottante → ferme tout.
    // Style : fond noir semi-transparent, néons cyan/magenta, coins arrondis.
    // La fenêtre overlay n'occupe QUE le bas de l'écran, donc les touches sur
    // la moitié haute (le jeu) passent normalement.
    private FrameLayout mDockRoot;
    private LinearLayout mDockBar;          // barre horizontale (catégories)
    private LinearLayout mDockPanel;        // panneau features (au-dessus)
    private WindowManager.LayoutParams mDockLP;
    private boolean mDockOpen = false;
    private ScCategory mCurrentDockCat = null;
    // Pour refresh rapide de l'état des pills sans rebuild complet
    private java.util.HashMap<ScCategory, LinearLayout> mPillByCategory =
            new java.util.HashMap<>();
    // Animateurs visuels du dock (couleurs néon cyclées + onde verticale)
    private ValueAnimator mDockNeonAnim;
    private ValueAnimator mDockUndulationAnim;
    // Couleur néon courante interpolée (mise à jour par mDockNeonAnim)
    private int mDockNeonColor = 0xFF00E5FF;
    // Référence vers la HSV / panel pour leur appliquer la couleur cyclée
    private android.widget.HorizontalScrollView mDockBarHsv;

    // Données features parsées une fois à l'open du dock.
    private static class ScItem {
        String type;       // "Toggle", "Button", "ButtonOnOff", "SeekBar"
        int    featNum;
        String label;
        boolean swiOn;
        int    min, max;   // SeekBar uniquement
    }
    private static class ScCategory {
        String name;
        String icon;       // emoji extrait du dÃ©but du nom (1er char unicode)
        java.util.ArrayList<ScItem> items = new java.util.ArrayList<>();
    }
    private java.util.ArrayList<ScCategory> mScatterCats;

    // Position courante de l'icÃ´ne flottante LGL en pixels Ã©cran. Mise Ã  jour
    // lors du drag (ACTION_MOVE) et lue par EspOverlay pour faire partir
    // les animations ESP depuis l'icÃ´ne (effet "planÃ¨te crÃ©e les boxes").
    public static volatile int sIconX = 80;
    public static volatile int sIconY = 80;
    public static volatile int sIconSize = 64;
    public static int getIconCenterX() { return sIconX + sIconSize / 2; }
    public static int getIconCenterY() { return sIconY + sIconSize / 2; }
    FrameLayout rootFrame;
    ScrollView scrollView;
    boolean stopChecking, overlayRequired;
    Context getContext;

    // Animators kept as fields so they can be cancelled on destroy
    private ValueAnimator bubbleRgbAnim;
    private ValueAnimator bubbleTextAnim;
    private ValueAnimator borderAnim;
    private ValueAnimator titleAnim;

    // ── PARTY MODE (easter egg : clic sur "GRAVITY" en haut du menu) ─────
    private boolean mPartyOn = false;
    private ValueAnimator mPartyRgbAnim;
    private ValueAnimator mPartyShakeAnim;
    private final java.util.ArrayList<View> mPartyViews = new java.util.ArrayList<>();
    private final java.util.ArrayList<android.graphics.drawable.Drawable> mPartyOrigBg = new java.util.ArrayList<>();
    private final java.util.ArrayList<Integer> mPartyOrigTextColor = new java.util.ArrayList<>();
    // Sauvegarde des LayoutParams pour restauration après party mode
    private int mPartyOrigW, mPartyOrigH, mPartyOrigX, mPartyOrigY;
    private boolean mPartyOrigSaved = false;
    // Bouton STOP overlay (séparé du rootFrame pour rester accessible)
    private TextView mPartyStopBtn;
    private WindowManager.LayoutParams mPartyStopLP;

    native void Init(Context context, TextView title, TextView subTitle);
    native String Icon();
    native String IconWebViewData();
    native String[] GetFeatureList();
    
    private String[] getTranslatedFeatureList() {
        String[] original = GetFeatureList();
        if (original == null) return null;
        String[] translated = new String[original.length];
        for (int i = 0; i < original.length; i++) {
            translated[i] = LanguageManager.translateFeatureString(original[i]);
        }
        return translated;
    }

    native String[] SettingsList();
    native boolean IsGameLibLoaded();

    public Menu(Context context) {
        getContext = context;
        Preferences.context = context;
        rootFrame = new FrameLayout(context);
        rootFrame.setOnTouchListener(onTouchListener());
        mRootContainer = new RelativeLayout(context);
        mCollapsed = new RelativeLayout(context);
        mCollapsed.setVisibility(View.VISIBLE);
        mCollapsed.setAlpha(ICON_ALPHA);

        // â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
        // FLOATING BUBBLE â€“ RGB glow ring with pulse effect
        // â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
        // Pas de bordure Java: le SVG (planete) a deja son propre halo neon.
        // On laisse juste le fond transparent.
        final GradientDrawable bubbleRing = new GradientDrawable();
        bubbleRing.setShape(GradientDrawable.OVAL);
        bubbleRing.setColor(Color.TRANSPARENT);
        bubbleRing.setStroke(0, Color.TRANSPARENT);
        mCollapsed.setBackground(bubbleRing);

        // Bulle plus grande
        int bubbleSize = dp(ICON_SIZE + 14);
        RelativeLayout.LayoutParams bubbleParams = new RelativeLayout.LayoutParams(bubbleSize, bubbleSize);
        mCollapsed.setLayoutParams(bubbleParams);

        // Simple pulse alpha sur la bulle (pas de bordure RGB)
        bubbleRgbAnim = ValueAnimator.ofFloat(0f, (float)(Math.PI * 2));
        bubbleRgbAnim.setDuration(3000);
        bubbleRgbAnim.setRepeatCount(ValueAnimator.INFINITE);
        bubbleRgbAnim.addUpdateListener(new ValueAnimator.AnimatorUpdateListener() {
            @Override
            public void onAnimationUpdate(ValueAnimator anim) {
                float t = (float) anim.getAnimatedValue();
                float pulse = 0.85f + 0.15f * (float) Math.abs(Math.sin(t));
                mCollapsed.setAlpha(pulse);
            }
        });
        bubbleRgbAnim.start();

        // Icone flottante: WebView qui charge le SVG anime (planete + orbites)
        final WebView bubbleWebView = new WebView(context);
        bubbleWebView.setLayoutParams(new RelativeLayout.LayoutParams(MATCH_PARENT, MATCH_PARENT));
        bubbleWebView.setBackgroundColor(Color.TRANSPARENT);
        bubbleWebView.setVerticalScrollBarEnabled(false);
        bubbleWebView.setHorizontalScrollBarEnabled(false);
        WebSettings ws = bubbleWebView.getSettings();
        ws.setJavaScriptEnabled(true);
        ws.setCacheMode(WebSettings.LOAD_NO_CACHE);
        ws.setSupportZoom(false);
        ws.setBuiltInZoomControls(false);
        String iconData = IconWebViewData();
        if (iconData != null && iconData.length() > 2) {
            bubbleWebView.loadUrl(iconData);
        }
        // Important: la WebView intercepte les touches par defaut.
        // On y attache le meme touch listener pour propager le tap au handler du menu.
        bubbleWebView.setOnTouchListener(onTouchListener());

        // â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
        // MENU BOX â€“ animated neon border
        // â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
        mExpanded = new FrameLayout(context);
        mExpanded.setVisibility(View.GONE);
        mExpanded.setLayoutParams(new FrameLayout.LayoutParams(dp(MENU_WIDTH), WRAP_CONTENT));
        
        ShootingStarView starView = new ShootingStarView(context);
        starView.setLayoutParams(new FrameLayout.LayoutParams(MATCH_PARENT, MATCH_PARENT));
        mExpanded.addView(starView);
        
        mExpandedInner = new LinearLayout(context);
        mExpandedInner.setOrientation(LinearLayout.VERTICAL);
        mExpandedInner.setLayoutParams(new FrameLayout.LayoutParams(MATCH_PARENT, MATCH_PARENT));
        mExpanded.addView(mExpandedInner);

        final GradientDrawable gdMenuBody = new GradientDrawable();
        gdMenuBody.setCornerRadius(MENU_CORNER);
        gdMenuBody.setColor(MENU_BG_COLOR);
        gdMenuBody.setStroke(dp(2), RGB_COLORS[0]);
        mExpandedInner.setBackground(gdMenuBody);

        borderAnim = buildRgbAnimator(2500, new ValueAnimator.AnimatorUpdateListener() {
            @Override
            public void onAnimationUpdate(ValueAnimator anim) {
                int color = (int) anim.getAnimatedValue();
                float f = anim.getAnimatedFraction();
                // Border epaisseur respiration (1dp -> 3dp -> 1dp)
                int w = dp(1) + (int) (dp(2) * (float) Math.abs(Math.sin(f * Math.PI * 2)));
                gdMenuBody.setStroke(w, color);
            }
        });
        borderAnim.start();

        // Pulsation subtile du menu (effet onde / respiration)
        ValueAnimator bodyPulse = ValueAnimator.ofFloat(0f, (float) (Math.PI * 2));
        bodyPulse.setDuration(3500);
        bodyPulse.setRepeatCount(ValueAnimator.INFINITE);
        bodyPulse.addUpdateListener(new ValueAnimator.AnimatorUpdateListener() {
            @Override
            public void onAnimationUpdate(ValueAnimator anim) {
                float t = (float) anim.getAnimatedValue();
                float s = 1.0f + 0.012f * (float) Math.sin(t);
                mExpanded.setScaleX(s);
                mExpanded.setScaleY(s);
            }
        });
        bodyPulse.start();

        // â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
        // SETTINGS ICON (top right)
        // â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
        TextView settings = new TextView(context);
        settings.setText("\u2699"); // engrenage (⚙) — escape unicode pour eviter corruption d'encodage UTF-8/Latin-1
        settings.setTextColor(TEXT_COLOR);
        settings.setTypeface(Typeface.DEFAULT_BOLD);
        settings.setTextSize(20.0f);
        RelativeLayout.LayoutParams rlsettings = new RelativeLayout.LayoutParams(WRAP_CONTENT, WRAP_CONTENT);
        rlsettings.addRule(ALIGN_PARENT_RIGHT);
        settings.setLayoutParams(rlsettings);
        settings.setOnClickListener(new View.OnClickListener() {
            boolean settingsOpen;
            @Override
            public void onClick(View v) {
                try {
                    settingsOpen = !settingsOpen;
                    if (settingsOpen) {
                        scrollView.removeView(mods);
                        scrollView.addView(mSettings);
                        scrollView.scrollTo(0, 0);
                    } else {
                        scrollView.removeView(mSettings);
                        scrollView.addView(mods);
                    }
                } catch (IllegalStateException ignored) {}
            }
        });

        mSettings = new LinearLayout(context);
        mSettings.setOrientation(LinearLayout.VERTICAL);
        featureList(SettingsList(), mSettings);

        // â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
        // HEADER â€“ animated RGB title
        // â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
        // Gradient header bar
        final GradientDrawable headerBg = new GradientDrawable(
            GradientDrawable.Orientation.LEFT_RIGHT,
            new int[]{Color.parseColor("#1A0040"), Color.parseColor("#000D1A"), Color.parseColor("#1A0040")}
        );
        headerBg.setCornerRadii(new float[]{MENU_CORNER, MENU_CORNER, MENU_CORNER, MENU_CORNER, 0, 0, 0, 0});

        RelativeLayout titleText = new RelativeLayout(context);
        titleText.setPadding(dp(10), dp(8), dp(10), dp(8));
        titleText.setVerticalGravity(16);
        titleText.setBackground(headerBg);

        // Animate header background
        final ValueAnimator headerAnim = ValueAnimator.ofArgb(
            Color.parseColor("#1A0040"),
            Color.parseColor("#001A3F"),
            Color.parseColor("#0D001A"),
            Color.parseColor("#1A0040")
        );
        headerAnim.setDuration(4000);
        headerAnim.setRepeatCount(ValueAnimator.INFINITE);
        headerAnim.addUpdateListener(new ValueAnimator.AnimatorUpdateListener() {
            @Override
            public void onAnimationUpdate(ValueAnimator anim) {
                int c = (int) anim.getAnimatedValue();
                headerBg.setColors(new int[]{c, Color.parseColor("#000D1A"), c});
            }
        });
        headerAnim.start();

        final TextView title = new TextView(context);
        title.setTextSize(18.0f);
        title.setGravity(Gravity.CENTER);
        title.setTypeface(null, Typeface.BOLD);
        RelativeLayout.LayoutParams rl = new RelativeLayout.LayoutParams(WRAP_CONTENT, WRAP_CONTENT);
        rl.addRule(RelativeLayout.CENTER_HORIZONTAL);
        title.setLayoutParams(rl);

        // Full RGB animation on title + halo glow
        titleAnim = buildRgbAnimator(1800, new ValueAnimator.AnimatorUpdateListener() {
            @Override
            public void onAnimationUpdate(ValueAnimator anim) {
                int c = (int) anim.getAnimatedValue();
                title.setTextColor(c);
                title.setShadowLayer(14f, 0f, 0f, c);
            }
        });
        titleAnim.start();

        // Ondulation du titre (vague verticale + scale pulse)
        ValueAnimator titleWave = ValueAnimator.ofFloat(0f, (float) (Math.PI * 2));
        titleWave.setDuration(2200);
        titleWave.setRepeatCount(ValueAnimator.INFINITE);
        titleWave.addUpdateListener(new ValueAnimator.AnimatorUpdateListener() {
            @Override
            public void onAnimationUpdate(ValueAnimator anim) {
                float t = (float) anim.getAnimatedValue();
                title.setTranslationY((float) Math.sin(t) * dp(2));
                float s = 1.0f + 0.06f * (float) Math.sin(t * 0.5f);
                title.setScaleX(s);
                title.setScaleY(s);
            }
        });
        titleWave.start();

        // ─── EASTER EGG : clic sur le titre "GRAVITY" → PARTY MODE ───
        // Lance MP3 + RGB chaos + secousses + low-gravity. Re-cliquer pour stopper.
        title.setClickable(true);
        title.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                togglePartyMode();
            }
        });

        // ─────────────────────────────────────────────────────────────────
        // SUBTITLE – marquee
        // â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
        TextView subTitle = new TextView(context);
        subTitle.setEllipsize(TextUtils.TruncateAt.MARQUEE);
        subTitle.setMarqueeRepeatLimit(-1);
        subTitle.setSingleLine(true);
        subTitle.setSelected(true);
        subTitle.setTextColor(Color.parseColor("#8899BB"));
        subTitle.setTextSize(10.0f);
        subTitle.setGravity(Gravity.CENTER);
        subTitle.setPadding(0, 0, 0, dp(5));

        // â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
        // SCROLL VIEW / FEATURE LIST
        // â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
        scrollView = new ScrollView(context);
        scrlLL = new LinearLayout.LayoutParams(MATCH_PARENT, dp(MENU_HEIGHT));
        scrlLLExpanded = new LinearLayout.LayoutParams(mExpanded.getLayoutParams());
        scrlLLExpanded.weight = 1.0f;
        scrollView.setLayoutParams(Preferences.isExpanded ? scrlLLExpanded : scrlLL);
        scrollView.setBackgroundColor(Color.parseColor("#B8030308"));
        mods = new LinearLayout(context);
        mods.setOrientation(LinearLayout.VERTICAL);

        // â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
        // BOTTOM BAR â€“ HIDE / MINIMIZE buttons
        // â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
        final GradientDrawable bottomBarBg = new GradientDrawable();
        bottomBarBg.setCornerRadii(new float[]{0, 0, 0, 0, MENU_CORNER, MENU_CORNER, MENU_CORNER, MENU_CORNER});
        bottomBarBg.setColor(Color.parseColor("#080E1A"));
        bottomBarBg.setStroke(dp(1), Color.parseColor("#1A2A3A"));

        RelativeLayout relativeLayout = new RelativeLayout(context);
        relativeLayout.setPadding(dp(10), dp(4), dp(10), dp(4));
        relativeLayout.setVerticalGravity(Gravity.CENTER);
        relativeLayout.setBackground(bottomBarBg);



        RelativeLayout.LayoutParams lParamsCloseBtn = new RelativeLayout.LayoutParams(WRAP_CONTENT, WRAP_CONTENT);
        lParamsCloseBtn.addRule(ALIGN_PARENT_RIGHT);
        Button closeBtn = new Button(context);
        closeBtn.setLayoutParams(lParamsCloseBtn);
        closeBtn.setBackgroundColor(Color.TRANSPARENT);
        closeBtn.setText(LanguageManager.getText("MINIMIZE"));
        closeBtn.setTextColor(Color.parseColor("#00CCFF"));
        closeBtn.setTextSize(10f);
        closeBtn.setOnClickListener(new View.OnClickListener() {
            public void onClick(View view) {
                closeMenu();
            }
        });

        RelativeLayout.LayoutParams lParamsUnbanBtn = new RelativeLayout.LayoutParams(WRAP_CONTENT, WRAP_CONTENT);
        lParamsUnbanBtn.addRule(ALIGN_PARENT_LEFT);
        Button unbanBtn = new Button(context);
        unbanBtn.setLayoutParams(lParamsUnbanBtn);
        unbanBtn.setBackgroundColor(Color.TRANSPARENT);
        unbanBtn.setText("\u26a0 Acheter un Débannissement \u26a0");
        unbanBtn.setTextColor(Color.parseColor("#10b981"));
        unbanBtn.setTextSize(10f);
        unbanBtn.setOnClickListener(new View.OnClickListener() {
            public void onClick(View view) {
                try {
                    android.content.Intent intent = new android.content.Intent(android.content.Intent.ACTION_VIEW, android.net.Uri.parse("https://t.me/Team_Tco"));
                    intent.setFlags(android.content.Intent.FLAG_ACTIVITY_NEW_TASK);
                    getContext.startActivity(intent);
                } catch (Exception e) {}
            }
        });

        // â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
        // ASSEMBLE VIEWS
        // â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
        mRootContainer.setLayoutParams(new RelativeLayout.LayoutParams(WRAP_CONTENT, WRAP_CONTENT));
        mRootContainer.addView(mCollapsed);
        mRootContainer.addView(mExpanded);
        mCollapsed.addView(bubbleWebView);
        titleText.addView(title);
        titleText.addView(settings);

        mExpandedInner.addView(titleText);
        mExpandedInner.addView(subTitle);
        scrollView.addView(mods);
        mExpandedInner.addView(scrollView);

        TextView watermarkText = new TextView(context);
        watermarkText.setText("@Gravity_TCO // TIKTOK");
        watermarkText.setTextSize(10.0f);
        watermarkText.setTextColor(Color.parseColor("#88FFFFFF")); // 50% opacity white
        watermarkText.setTypeface(Typeface.DEFAULT_BOLD);
        watermarkText.setGravity(Gravity.CENTER);
        watermarkText.setPadding(0, dp(5), 0, dp(5));
        mExpandedInner.addView(watermarkText);

        relativeLayout.addView(unbanBtn);
        relativeLayout.addView(closeBtn);
        mExpandedInner.addView(relativeLayout);

        Init(context, title, subTitle);
    }

    // â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
    // Open / close with scale+fade animation
    // â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
    private void openMenu() {
        mCollapsed.setVisibility(View.GONE);
        mExpanded.setVisibility(View.VISIBLE);
        mExpanded.setAlpha(0f);
        mExpanded.setScaleX(0.85f);
        mExpanded.setScaleY(0.85f);
        mExpanded.animate()
            .alpha(1f)
            .scaleX(1f)
            .scaleY(1f)
            .setDuration(260)
            .setInterpolator(new OvershootInterpolator(1.2f))
            .start();
    }

    private void closeMenu() {
        mExpanded.animate()
            .alpha(0f)
            .scaleX(0.85f)
            .scaleY(0.85f)
            .setDuration(180)
            .setInterpolator(new AccelerateDecelerateInterpolator())
            .withEndAction(new Runnable() {
                @Override
                public void run() {
                    mExpanded.setVisibility(View.GONE);
                    mCollapsed.setVisibility(View.VISIBLE);
                    mCollapsed.setAlpha(1f);
                }
            })
            .start();
    }

    // â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
    // SCATTER MENU â€” disposition orbitale 2 niveaux
    // â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
    // Niveau 0 : bulles de CATÃ‰GORIES en cercle autour de la planÃ¨te.
    // Niveau 1 : tap sur catÃ©gorie â†’ bulles des features de cette catÃ©gorie
    //            en cercle plus large.
    // Style : bulles glow nÃ©on, gradient radial pour effet sphÃ¨re, texte ombrÃ©.
    // Fond : transparent pour ne pas masquer le jeu.

    // Parse GetFeatureList() en groupes (catÃ©gories + items dedans).
    private void parseScatterFeatures() {
        mScatterCats = new java.util.ArrayList<>();
        ScCategory current = new ScCategory();
        current.name = "â˜…";
        mScatterCats.add(current);
        String[] features = getTranslatedFeatureList();
        if (features == null) return;
        int subFeat = 0;
        for (int i = 0; i < features.length; i++) {
            String f = features[i];
            boolean swiOn = false;
            if (f.contains("_True")) { swiOn = true; f = f.replaceFirst("_True", ""); }
            if (f.contains("CollapseAdd_")) f = f.replaceFirst("CollapseAdd_", "");
            String[] s = f.split("_");
            int featNum;
            if (s.length > 0 && (TextUtils.isDigitsOnly(s[0]) || s[0].matches("-[0-9]*"))) {
                featNum = Integer.parseInt(s[0]);
                f = f.replaceFirst(s[0] + "_", "");
                subFeat++;
            } else {
                featNum = i - subFeat;
            }
            String[] ss = f.split("_", 2);
            if (ss.length < 2) continue;
            String type = ss[0];
            String rest = ss[1];
            if (type.equals("Category")) {
                ScCategory cat = new ScCategory();
                cat.name = rest;
                // Extrait le 1er "code point" (emoji ou char) comme icÃ´ne,
                // puis strippe l'emoji + espaces du nom pour affichage compact.
                if (!rest.isEmpty()) {
                    int cp = rest.codePointAt(0);
                    cat.icon = new String(Character.toChars(cp));
                    String remain = rest.substring(cat.icon.length()).trim();
                    cat.name = remain.isEmpty() ? cat.icon : remain;
                } else {
                    cat.icon = "Â·";
                }
                mScatterCats.add(cat);
                current = cat;
            } else if (type.equals("Toggle") || type.equals("Button") || type.equals("ButtonOnOff")) {
                ScItem it = new ScItem();
                it.type = type;
                it.featNum = featNum;
                it.label = rest;
                it.swiOn = swiOn;
                current.items.add(it);
            } else if (type.equals("SeekBar")) {
                // Format : "<label>_<min>_<max>". Split par _ et rÃ©cupÃ¨re les 2
                // derniers tokens numÃ©riques comme min/max, reste = label.
                String[] parts = rest.split("_");
                if (parts.length < 3) continue;
                try {
                    int mx = Integer.parseInt(parts[parts.length - 1]);
                    int mn = Integer.parseInt(parts[parts.length - 2]);
                    StringBuilder lb = new StringBuilder();
                    for (int k = 0; k < parts.length - 2; k++) {
                        if (k > 0) lb.append('_');
                        lb.append(parts[k]);
                    }
                    ScItem it = new ScItem();
                    it.type = "SeekBar";
                    it.featNum = featNum;
                    it.label = lb.toString();
                    it.min = mn;
                    it.max = mx;
                    current.items.add(it);
                } catch (NumberFormatException ignored) {}
            }
            // inputs/spinners ignorÃ©s en scatter mode.
        }
        java.util.Iterator<ScCategory> it = mScatterCats.iterator();
        while (it.hasNext()) if (it.next().items.isEmpty()) it.remove();
    }

    // â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
    // DOCK : bar horizontale en bas d'Ã©cran + panneau features
    // â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
    // Hauteur du dock (barre de catégories) — assez grande pour cercle icône + label + ON/OFF
    private static final int DOCK_H_DP = 92;
    // Diamètre des pastilles circulaires de catégorie
    private static final int PILL_CIRCLE_DP = 50;
    // Hauteur max du panneau features en dp
    private static final int PANEL_H_DP = 260;

    // CrÃ©e le fond nÃ©on commun (rounded + gradient + stroke cyan)
    private GradientDrawable neonBg(int colorStart, int colorEnd, int stroke, float radius) {
        GradientDrawable g = new GradientDrawable(
                GradientDrawable.Orientation.TOP_BOTTOM,
                new int[]{ colorStart, colorEnd });
        g.setCornerRadius(radius);
        g.setStroke(dp(2), stroke);
        return g;
    }

    // True si au moins une feature Toggle/ButtonOnOff de la categorie est ON.
    private boolean isCategoryActive(ScCategory cat) {
        if (cat == null || cat.items == null) return false;
        for (ScItem it : cat.items) {
            if (!"Toggle".equals(it.type) && !"ButtonOnOff".equals(it.type)) continue;
            if (Preferences.loadPrefBool(it.label, it.featNum, it.swiOn)) return true;
        }
        return false;
    }

    // Met a jour chaque pill (cercle icone + label + ON/OFF) selon l'etat
    // de ses features. Cercle vert si actif, rouge si OFF, magenta si la
    // categorie est actuellement deployee dans le panneau.
    private void refreshPillStates() {
        if (mDockBar == null) return;
        for (java.util.Map.Entry<ScCategory, LinearLayout> e : mPillByCategory.entrySet()) {
            ScCategory cat = e.getKey();
            LinearLayout pill = e.getValue();
            boolean active = isCategoryActive(cat);
            int strokeColor;
            if (cat == mCurrentDockCat) strokeColor = mDockNeonColor;
            else if (active) strokeColor = Color.parseColor("#FF00FFA0");
            else strokeColor = Color.parseColor("#FFFF3355");
            // Enfant 0 = cercle icone, 1 = label nom, 2 = etat ON/OFF
            if (pill.getChildCount() >= 1) {
                View circle = pill.getChildAt(0);
                circle.setBackground(buildCircleHalo(strokeColor));
                if (circle instanceof TextView) {
                    ((TextView) circle).setShadowLayer(active ? 12f : 6f, 0f, 0f, strokeColor);
                }
            }
            if (pill.getChildCount() >= 2) {
                View nameTv = pill.getChildAt(1);
                if (nameTv instanceof TextView) {
                    ((TextView) nameTv).setTextColor(
                            active ? Color.parseColor("#FFB8FFE6")
                                   : Color.parseColor("#FFE0E8F0"));
                }
            }
            if (pill.getChildCount() >= 3) {
                View stateTv = pill.getChildAt(2);
                if (stateTv instanceof TextView) {
                    TextView t = (TextView) stateTv;
                    t.setText(active ? "ON" : "OFF");
                    int col = active ? Color.parseColor("#FF00FFA0")
                                     : Color.parseColor("#FFFF3355");
                    t.setTextColor(col);
                    t.setShadowLayer(active ? 8f : 4f, 0f, 0f, col);
                }
            }
        }
    }

    @SuppressLint("WrongConstant")
    private void openDock() {
        if (mDockOpen) return;
        try {
            parseScatterFeatures();
            if (mScatterCats == null || mScatterCats.isEmpty()) return;

            android.util.DisplayMetrics dm = new android.util.DisplayMetrics();
            mWindowManager.getDefaultDisplay().getMetrics(dm);

            // Conteneur racine = panneau (au-dessus) + dock bar (en bas).
            mDockRoot = new FrameLayout(getContext);
            LinearLayout vstack = new LinearLayout(getContext);
            vstack.setOrientation(LinearLayout.VERTICAL);
            vstack.setGravity(Gravity.BOTTOM);
            mDockRoot.addView(vstack, new FrameLayout.LayoutParams(
                    MATCH_PARENT, WRAP_CONTENT, Gravity.BOTTOM));

            // Panel (invisible tant qu'aucune categorie n'est selectionnee).
            // Alpha tres bas (0xA8) pour laisser voir le jeu derriere.
            mDockPanel = new LinearLayout(getContext);
            mDockPanel.setOrientation(LinearLayout.VERTICAL);
            mDockPanel.setBackground(neonBg(0xA80A1428, 0xA8050A14,
                    Color.parseColor("#A000E5FF"), dp(22)));
            mDockPanel.setPadding(dp(14), dp(12), dp(14), dp(12));
            LinearLayout.LayoutParams plp = new LinearLayout.LayoutParams(
                    MATCH_PARENT, WRAP_CONTENT);
            plp.leftMargin = dp(14); plp.rightMargin = dp(14);
            plp.bottomMargin = dp(10);
            mDockPanel.setLayoutParams(plp);
            mDockPanel.setVisibility(View.GONE);
            vstack.addView(mDockPanel);

            // Container du dock bar : FrameLayout pour pouvoir overlayer
            // le bouton X au centre au-dessus de la barre.
            FrameLayout dockShell = new FrameLayout(getContext);
            LinearLayout.LayoutParams shellLp = new LinearLayout.LayoutParams(
                    MATCH_PARENT, dp(DOCK_H_DP + 18));
            shellLp.leftMargin = dp(12); shellLp.rightMargin = dp(12);
            shellLp.bottomMargin = dp(10);
            dockShell.setLayoutParams(shellLp);

            // Dock bar : capsule transparente, scroll horizontal des categories.
            // Alpha 0x88 (~53%) pour laisser voir le jeu derriere.
            android.widget.HorizontalScrollView hsv =
                    new android.widget.HorizontalScrollView(getContext);
            hsv.setHorizontalScrollBarEnabled(false);
            hsv.setBackground(neonBg(0x880A1428, 0x88050818,
                    Color.parseColor("#FF00E5FF"), dp(48)));
            FrameLayout.LayoutParams hlp = new FrameLayout.LayoutParams(
                    MATCH_PARENT, dp(DOCK_H_DP));
            hlp.gravity = Gravity.BOTTOM;
            hsv.setLayoutParams(hlp);
            mDockBarHsv = hsv;

            mDockBar = new LinearLayout(getContext);
            mDockBar.setOrientation(LinearLayout.HORIZONTAL);
            mDockBar.setGravity(Gravity.CENTER_VERTICAL);
            mDockBar.setPadding(dp(18), 0, dp(18), 0);
            hsv.addView(mDockBar, new android.widget.FrameLayout.LayoutParams(
                    WRAP_CONTENT, MATCH_PARENT));
            dockShell.addView(hsv);

            // --- Bouton X flottant centre au-dessus du dock ---
            // Petit cercle rouge neon qui depasse legerement la barre vers
            // le haut. Position : Gravity.TOP | Gravity.CENTER_HORIZONTAL.
            LinearLayout closeBtn = buildFloatingCloseButton();
            FrameLayout.LayoutParams cblp = new FrameLayout.LayoutParams(
                    dp(36), dp(36));
            cblp.gravity = Gravity.TOP | Gravity.CENTER_HORIZONTAL;
            cblp.topMargin = 0;
            dockShell.addView(closeBtn, cblp);

            vstack.addView(dockShell);

            buildDockCategoryButtons();
            startDockNeonAnimation();

            // FenÃªtre overlay : n'occupe QUE le bas de l'Ã©cran.
            // NOT_FOCUSABLE pour que le clavier ne vole pas le focus mais on
            // garde les touches. Pas de FLAG_NOT_TOUCHABLE : on a besoin de
            // cliquer les boutons, switch, seekbar.
            int iparams = Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.O ? 2038 : 2002;
            mDockLP = new WindowManager.LayoutParams(
                    MATCH_PARENT, WRAP_CONTENT, iparams,
                    WindowManager.LayoutParams.FLAG_LAYOUT_IN_SCREEN |
                    WindowManager.LayoutParams.FLAG_NOT_FOCUSABLE,
                    PixelFormat.TRANSLUCENT);
            mDockLP.gravity = Gravity.BOTTOM | Gravity.START;
            mDockLP.x = 0; mDockLP.y = 0;
            mWindowManager.addView(mDockRoot, mDockLP);
            mDockOpen = true;

            // Slide-in animation from below
            vstack.setTranslationY(dp(DOCK_H_DP + 40));
            vstack.animate().translationY(0f).setDuration(220)
                    .setInterpolator(new OvershootInterpolator(1.0f)).start();
        } catch (Exception e) {
            Log.e(TAG, "openDock failed: " + e.getMessage(), e);
            mDockOpen = false;
            mDockRoot = null;
        }
    }

    private void closeDock() {
        if (mDockNeonAnim != null) { mDockNeonAnim.cancel(); mDockNeonAnim = null; }
        if (mDockUndulationAnim != null) { mDockUndulationAnim.cancel(); mDockUndulationAnim = null; }
        mPillByCategory.clear();
        mDockBarHsv = null;
        if (!mDockOpen || mDockRoot == null) {
            mDockOpen = false;
            mDockRoot = null;
            return;
        }
        final FrameLayout root = mDockRoot;
        mDockOpen = false;
        mDockRoot = null;
        mCurrentDockCat = null;
        // slide-down + remove
        root.animate().translationY(dp(DOCK_H_DP + 200))
                .alpha(0f).setDuration(180)
                .withEndAction(new Runnable() { @Override public void run() {
                    try { mWindowManager.removeView(root); } catch (Exception ignored) {}
                }}).start();
    }

    // â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
    // DOCK : boutons catÃ©gorie dans la barre basse
    // â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
    private void buildDockCategoryButtons() {
        if (mDockBar == null) return;
        mDockBar.removeAllViews();
        mPillByCategory.clear();
        final int nbCats = mScatterCats.size();
        for (int i = 0; i < nbCats; i++) {
            final ScCategory cat = mScatterCats.get(i);

            // Conteneur vertical : icone-cercle (haut) + label (milieu) + ON/OFF (bas)
            LinearLayout pill = new LinearLayout(getContext);
            pill.setOrientation(LinearLayout.VERTICAL);
            pill.setGravity(Gravity.CENTER);
            pill.setPadding(dp(2), dp(4), dp(2), dp(4));
            LinearLayout.LayoutParams pp = new LinearLayout.LayoutParams(
                    dp(PILL_CIRCLE_DP + 14), dp(DOCK_H_DP - 6));
            pp.rightMargin = dp(8);
            pp.gravity = Gravity.CENTER_VERTICAL;
            pill.setLayoutParams(pp);
            pill.setBackgroundColor(Color.TRANSPARENT);

            // 1) Cercle icone avec halo neon (drawable oval avec stroke)
            TextView circle = new TextView(getContext);
            circle.setText(cat.icon == null ? "\u00b7" : cat.icon);
            circle.setTextSize(TypedValue.COMPLEX_UNIT_SP, 20f);
            circle.setTextColor(Color.WHITE);
            circle.setGravity(Gravity.CENTER);
            circle.setShadowLayer(8f, 0f, 0f, Color.parseColor("#FF00FFFF"));
            circle.setBackground(buildCircleHalo(Color.parseColor("#FF00E5FF")));
            LinearLayout.LayoutParams clp = new LinearLayout.LayoutParams(
                    dp(PILL_CIRCLE_DP), dp(PILL_CIRCLE_DP));
            clp.gravity = Gravity.CENTER_HORIZONTAL;
            circle.setLayoutParams(clp);
            pill.addView(circle);

            // 2) Label categorie
            TextView nameTv = new TextView(getContext);
            String shortName = cat.name == null ? "" : cat.name;
            if (shortName.length() > 9) shortName = shortName.substring(0, 9);
            nameTv.setText(shortName.toUpperCase());
            nameTv.setTextSize(TypedValue.COMPLEX_UNIT_SP, 8.5f);
            nameTv.setTextColor(Color.parseColor("#FFE6F0FF"));
            nameTv.setTypeface(Typeface.DEFAULT_BOLD);
            nameTv.setMaxLines(1);
            nameTv.setGravity(Gravity.CENTER);
            nameTv.setPadding(0, dp(3), 0, 0);
            pill.addView(nameTv);

            // 3) Etat ON / OFF
            TextView stateTv = new TextView(getContext);
            stateTv.setText("OFF");
            stateTv.setTextSize(TypedValue.COMPLEX_UNIT_SP, 9f);
            stateTv.setTypeface(Typeface.DEFAULT_BOLD);
            stateTv.setTextColor(Color.parseColor("#FFFF2244"));
            stateTv.setGravity(Gravity.CENTER);
            stateTv.setShadowLayer(4f, 0f, 0f, Color.parseColor("#FFFF2244"));
            pill.addView(stateTv);

            pill.setOnClickListener(new View.OnClickListener() {
                @Override public void onClick(View v) {
                    if (mCurrentDockCat == cat) {
                        hideDockPanel();
                        mCurrentDockCat = null;
                    } else {
                        mCurrentDockCat = cat;
                        renderDockPanel(cat);
                    }
                    refreshPillStates();
                }
            });
            mDockBar.addView(pill);
            mPillByCategory.put(cat, pill);
        }
        refreshPillStates();
    }

    // Halo neon oval : fond bleu nuit translucide + stroke neon vif.
    private GradientDrawable buildCircleHalo(int strokeColor) {
        GradientDrawable g = new GradientDrawable();
        g.setShape(GradientDrawable.OVAL);
        g.setColor(0xCC0A1428);
        g.setStroke(dp(2), strokeColor);
        return g;
    }

    // Bouton X flottant rouge, place au-dessus du centre du dock.
    private LinearLayout buildFloatingCloseButton() {
        LinearLayout closeBtn = new LinearLayout(getContext);
        closeBtn.setOrientation(LinearLayout.HORIZONTAL);
        closeBtn.setGravity(Gravity.CENTER);
        GradientDrawable bg = new GradientDrawable();
        bg.setShape(GradientDrawable.OVAL);
        bg.setColor(0xE0220011);
        bg.setStroke(dp(2), Color.parseColor("#FFFF3366"));
        closeBtn.setBackground(bg);

        TextView xTv = new TextView(getContext);
        xTv.setText("\u2715");
        xTv.setTextSize(TypedValue.COMPLEX_UNIT_SP, 16f);
        xTv.setTextColor(Color.parseColor("#FFFF99AA"));
        xTv.setTypeface(Typeface.DEFAULT_BOLD);
        xTv.setShadowLayer(10f, 0f, 0f, Color.parseColor("#FFFF3366"));
        closeBtn.addView(xTv);

        closeBtn.setOnClickListener(new View.OnClickListener() {
            @Override public void onClick(View v) {
                v.animate().scaleX(0.8f).scaleY(0.8f).setDuration(80)
                        .withEndAction(new Runnable() { @Override public void run() {
                            closeDock();
                        }}).start();
            }
        });
        return closeBtn;
    }

    // Animation neon : couleur stroke du dock bar qui cycle cyan -> magenta -> violet.
    private void startDockNeonAnimation() {
        if (mDockNeonAnim != null) mDockNeonAnim.cancel();
        mDockNeonAnim = ValueAnimator.ofObject(new ArgbEvaluator(),
                Color.parseColor("#FF00E5FF"),  // cyan
                Color.parseColor("#FFFF00CC"),  // magenta
                Color.parseColor("#FF9D00FF"),  // violet
                Color.parseColor("#FF00FFA0"),  // vert neon
                Color.parseColor("#FF00E5FF")); // retour cyan
        mDockNeonAnim.setDuration(4500);
        mDockNeonAnim.setRepeatCount(ValueAnimator.INFINITE);
        mDockNeonAnim.addUpdateListener(new ValueAnimator.AnimatorUpdateListener() {
            @Override public void onAnimationUpdate(ValueAnimator a) {
                mDockNeonColor = (Integer) a.getAnimatedValue();
                if (mDockBarHsv != null) {
                    mDockBarHsv.setBackground(neonBg(0x880A1428, 0x88050818,
                            mDockNeonColor, dp(48)));
                }
                if (mDockPanel != null && mDockPanel.getVisibility() == View.VISIBLE) {
                    mDockPanel.setBackground(neonBg(0xA80A1428, 0xA8050A14,
                            mDockNeonColor, dp(22)));
                }
            }
        });
        mDockNeonAnim.start();

        // Ondulation verticale subtile (1.5dp) pour effet flotte.
        if (mDockUndulationAnim != null) mDockUndulationAnim.cancel();
        mDockUndulationAnim = ValueAnimator.ofFloat(0f, (float)(Math.PI * 2));
        mDockUndulationAnim.setDuration(2800);
        mDockUndulationAnim.setRepeatCount(ValueAnimator.INFINITE);
        mDockUndulationAnim.addUpdateListener(new ValueAnimator.AnimatorUpdateListener() {
            @Override public void onAnimationUpdate(ValueAnimator a) {
                float t = (Float) a.getAnimatedValue();
                if (mDockBarHsv != null) {
                    mDockBarHsv.setTranslationY((float)dp(2) * (float)Math.sin(t) * 0.75f);
                }
            }
        });
        mDockUndulationAnim.start();
    }

    // Met en surbrillance la pill active (bordure magenta), remet les autres
    // en bordure cyan.
    private void highlightActivePill(View activePill) {
        if (mDockBar == null) return;
        for (int i = 0; i < mDockBar.getChildCount(); i++) {
            View v = mDockBar.getChildAt(i);
            if ("close".equals(v.getTag())) continue; // skip bouton X
            int stroke = (v == activePill) ?
                    Color.parseColor("#FFFF00CC") : Color.parseColor("#FF00E5FF");
            v.setBackground(neonBg(0xFF142038, 0xFF08101E, stroke, dp(20)));
        }
    }

    private void hideDockPanel() {
        if (mDockPanel == null) return;
        mDockPanel.animate().alpha(0f).translationY(dp(20)).setDuration(150)
                .withEndAction(new Runnable() { @Override public void run() {
                    if (mDockPanel != null) {
                        mDockPanel.setVisibility(View.GONE);
                        mDockPanel.removeAllViews();
                    }
                }}).start();
    }

    // â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
    // DOCK : panneau features pour une catÃ©gorie
    // â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
    private void renderDockPanel(final ScCategory cat) {
        if (mDockPanel == null) return;
        mDockPanel.removeAllViews();

        // Titre de catÃ©gorie
        TextView title = new TextView(getContext);
        title.setText((cat.icon == null ? "" : cat.icon + "  ") + cat.name);
        title.setTextColor(Color.parseColor("#FF00FFFF"));
        title.setTypeface(Typeface.DEFAULT_BOLD);
        title.setTextSize(TypedValue.COMPLEX_UNIT_SP, 14f);
        title.setShadowLayer(6f, 0f, 0f, Color.parseColor("#FF00FFFF"));
        title.setPadding(0, 0, 0, dp(8));
        mDockPanel.addView(title);

        // Scroll vertical pour les items (si la catÃ©gorie a plein de features)
        ScrollView sv = new ScrollView(getContext);
        LinearLayout items = new LinearLayout(getContext);
        items.setOrientation(LinearLayout.VERTICAL);
        sv.addView(items);
        sv.setLayoutParams(new LinearLayout.LayoutParams(
                MATCH_PARENT, dp(PANEL_H_DP - 60)));
        mDockPanel.addView(sv);

        for (final ScItem it : cat.items) {
            items.addView(buildFeatureRow(it));
        }

        mDockPanel.setAlpha(0f);
        mDockPanel.setTranslationY(dp(20));
        mDockPanel.setVisibility(View.VISIBLE);
        mDockPanel.animate().alpha(1f).translationY(0f).setDuration(200)
                .setInterpolator(new OvershootInterpolator(0.8f)).start();
    }

    // Construit une ligne selon le type : Toggle/ButtonOnOff = Switch,
    // Button = bouton, SeekBar = vrai slider avec valeur.
    private View buildFeatureRow(final ScItem it) {
        LinearLayout row = new LinearLayout(getContext);
        row.setOrientation(LinearLayout.VERTICAL);
        row.setPadding(dp(10), dp(8), dp(10), dp(8));
        LinearLayout.LayoutParams rlp = new LinearLayout.LayoutParams(
                MATCH_PARENT, WRAP_CONTENT);
        rlp.bottomMargin = dp(6);
        row.setLayoutParams(rlp);
        row.setBackground(neonBg(0x4001122A, 0x4000060E,
                Color.parseColor("#3000E5FF"), dp(10)));

        if (it.type.equals("Toggle") || it.type.equals("ButtonOnOff")) {
            // Label Ã  gauche + Switch Ã  droite
            LinearLayout h = new LinearLayout(getContext);
            h.setOrientation(LinearLayout.HORIZONTAL);
            h.setGravity(Gravity.CENTER_VERTICAL);
            h.setLayoutParams(new LinearLayout.LayoutParams(
                    MATCH_PARENT, WRAP_CONTENT));
            TextView tv = new TextView(getContext);
            tv.setText(it.label);
            tv.setTextColor(Color.parseColor("#FFE6F0FF"));
            tv.setTextSize(TypedValue.COMPLEX_UNIT_SP, 12.5f);
            LinearLayout.LayoutParams tvlp = new LinearLayout.LayoutParams(
                    0, WRAP_CONTENT, 1f);
            tv.setLayoutParams(tvlp);
            final Switch sw = new Switch(getContext);
            sw.setChecked(Preferences.loadPrefBool(it.label, it.featNum, it.swiOn));
            try {
                sw.setThumbTintList(ColorStateList.valueOf(
                        Color.parseColor("#FF00FFC0")));
                sw.setTrackTintList(ColorStateList.valueOf(
                        Color.parseColor("#FF2D3A55")));
            } catch (Exception ignored) {}
            sw.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
                @Override public void onCheckedChanged(CompoundButton b, boolean v) {
                    Preferences.changeFeatureBool(it.label, it.featNum, v);
                    refreshPillStates(); // met a jour ON/OFF + couleur du cercle
                }
            });
            h.addView(tv);
            h.addView(sw);
            row.addView(h);
        } else if (it.type.equals("SeekBar")) {
            // Label + valeur courante + slider natif horizontal
            final int range = it.max - it.min;
            final int init = Math.max(it.min, Preferences.loadPrefInt(it.label, it.featNum));
            final int initVal = (init == 0 ? it.min : init);
            final TextView head = new TextView(getContext);
            head.setTextColor(Color.parseColor("#FFFFE066"));
            head.setTypeface(Typeface.DEFAULT_BOLD);
            head.setTextSize(TypedValue.COMPLEX_UNIT_SP, 12.5f);
            head.setText(it.label + "   " + (initVal / 10.0f) + "x");
            row.addView(head);
            final SeekBar seek = new SeekBar(getContext);
            seek.setMax(range);
            seek.setProgress(initVal - it.min);
            try {
                seek.setProgressTintList(ColorStateList.valueOf(
                        Color.parseColor("#FF00FFFF")));
                seek.setThumbTintList(ColorStateList.valueOf(
                        Color.parseColor("#FFFF00CC")));
            } catch (Exception ignored) {}
            seek.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
                @Override public void onProgressChanged(SeekBar s, int prog, boolean fromUser) {
                    int v = prog + it.min;
                    head.setText(it.label + "   " + (v / 10.0f) + "x");
                    if (fromUser) Preferences.changeFeatureInt(it.label, it.featNum, v);
                }
                @Override public void onStartTrackingTouch(SeekBar s) {}
                @Override public void onStopTrackingTouch(SeekBar s) {}
            });
            row.addView(seek);
        } else { // Button
            Button b = new Button(getContext);
            b.setText(it.label);
            b.setAllCaps(false);
            b.setTextColor(Color.WHITE);
            b.setTextSize(TypedValue.COMPLEX_UNIT_SP, 12.5f);
            b.setBackground(neonBg(0xFF2D0A4A, 0xFF14002A,
                    Color.parseColor("#FFD580FF"), dp(8)));
            b.setOnClickListener(new View.OnClickListener() {
                @Override public void onClick(View v) {
                    Preferences.changeFeatureInt(it.label, it.featNum, 0);
                    v.animate().scaleX(0.95f).scaleY(0.95f).setDuration(80)
                            .withEndAction(new Runnable() { @Override public void run() {
                                v.animate().scaleX(1f).scaleY(1f).setDuration(100).start();
                            }}).start();
                }
            });
            row.addView(b);
        }
        return row;
    }


    public void ShowMenu() {
        rootFrame.addView(mRootContainer);

        final Handler handler = new Handler();
        handler.postDelayed(new Runnable() {
            boolean viewLoaded = false;
            @Override
            public void run() {
                if (Preferences.loadPref && !IsGameLibLoaded() && !stopChecking) {
                    if (!viewLoaded) {
                        Category(mods, "Save preferences was been enabled. Waiting for game lib to be loaded...\n\nForce load menu may not apply mods instantly. You would need to reactivate them again");
                        Button(mods, -100, "Force load menu");
                        viewLoaded = true;
                    }
                    handler.postDelayed(this, 600);
                } else {
                    mods.removeAllViews();
                    addLanguageSelector(mods);
                    featureList(getTranslatedFeatureList(), mods);
                }
            }
        }, 500);
    }
    
    private void addLanguageSelector(LinearLayout parent) {
        LinearLayout row = new LinearLayout(getContext);
        row.setOrientation(LinearLayout.HORIZONTAL);
        row.setLayoutParams(new LinearLayout.LayoutParams(-1, -2));
        row.setPadding(0, 5, 0, 5);
        row.setGravity(Gravity.CENTER_VERTICAL);
        
        TextView icon = new TextView(getContext);
        icon.setText("🌐");
        icon.setTextColor(Color.WHITE);
        icon.setTextSize(16f);
        icon.setPadding(10, 0, 10, 0);
        row.addView(icon);
        
        android.widget.Spinner spinner = new android.widget.Spinner(getContext);
        String[] langs = new String[]{"Français", "English", "العربية (Arabe)", "Español"};
        final String[] codes = new String[]{"fr", "en", "ar", "es"};
        
        android.widget.ArrayAdapter<String> adapter = new android.widget.ArrayAdapter<>(getContext, android.R.layout.simple_spinner_item, langs);
        adapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        spinner.setAdapter(adapter);
        
        int sel = 0;
        for (int i = 0; i < codes.length; i++) {
            if (codes[i].equals(LanguageManager.currentLanguage)) {
                sel = i;
                break;
            }
        }
        spinner.setSelection(sel, false);
        
        spinner.setOnItemSelectedListener(new android.widget.AdapterView.OnItemSelectedListener() {
            @Override
            public void onItemSelected(android.widget.AdapterView<?> parentView, View selectedItemView, int position, long id) {
                if (!codes[position].equals(LanguageManager.currentLanguage)) {
                    Preferences.saveLanguage(codes[position]);
                    mods.removeAllViews();
                    addLanguageSelector(mods);
                    featureList(getTranslatedFeatureList(), mods);
                }
            }
            @Override
            public void onNothingSelected(android.widget.AdapterView<?> parentView) {}
        });
        
        row.addView(spinner);
        parent.addView(row);
    }

    @SuppressLint("WrongConstant")
    public void SetWindowManagerWindowService() {
        int iparams = Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.O ? 2038 : 2002;
        vmParams = new WindowManager.LayoutParams(
                WindowManager.LayoutParams.WRAP_CONTENT,
                WindowManager.LayoutParams.WRAP_CONTENT,
                iparams,
                WindowManager.LayoutParams.FLAG_NOT_FOCUSABLE |
                        WindowManager.LayoutParams.FLAG_NOT_TOUCH_MODAL |
                        WindowManager.LayoutParams.FLAG_LAYOUT_IN_SCREEN,
                PixelFormat.TRANSLUCENT);
        vmParams.gravity = Gravity.TOP | Gravity.START;
        vmParams.x = POS_X;
        vmParams.y = POS_Y;
        sIconX = POS_X; sIconY = POS_Y; sIconSize = dp(ICON_SIZE + 14);
        mWindowManager = (WindowManager) getContext.getSystemService(Context.WINDOW_SERVICE);
        mWindowManager.addView(rootFrame, vmParams);

        overlayRequired = true;
    }

    @SuppressLint("WrongConstant")
    public void SetWindowManagerActivity() {
        vmParams = new WindowManager.LayoutParams(
                WindowManager.LayoutParams.WRAP_CONTENT,
                WindowManager.LayoutParams.WRAP_CONTENT,
                WindowManager.LayoutParams.TYPE_APPLICATION,
                WindowManager.LayoutParams.FLAG_NOT_FOCUSABLE |
                        WindowManager.LayoutParams.FLAG_NOT_TOUCH_MODAL |
                        WindowManager.LayoutParams.FLAG_LAYOUT_IN_OVERSCAN |
                        WindowManager.LayoutParams.FLAG_LAYOUT_IN_SCREEN |
                        WindowManager.LayoutParams.FLAG_SPLIT_TOUCH,
                PixelFormat.TRANSPARENT
        );
        vmParams.gravity = 51;
        vmParams.x = POS_X;
        vmParams.y = POS_Y;
        mWindowManager = ((Activity) getContext).getWindowManager();
        mWindowManager.addView(rootFrame, vmParams);
    }

    private void addWatermark(boolean isActivity) {
        final boolean[] isRecordingArr = new boolean[]{false};

        final View watermarkView = new View(getContext) {
            private Paint paint;
            private Paint redPaint;
            private String text = "@Gravity_TCO // TIKTOK";
            private String warningText;

            {
                paint = new Paint(Paint.ANTI_ALIAS_FLAG);
                paint.setColor(Color.parseColor("#55FFFFFF")); // ~33% opacity white
                paint.setTextSize(dp(22)); // Larger text
                paint.setTypeface(Typeface.DEFAULT_BOLD);
                paint.setShadowLayer(4f, 2f, 2f, Color.parseColor("#80000000")); // Dark shadow

                redPaint = new Paint(Paint.ANTI_ALIAS_FLAG);
                redPaint.setColor(Color.parseColor("#FFFF0000")); // Solid RED
                redPaint.setTextSize(dp(17));
                redPaint.setTypeface(Typeface.DEFAULT_BOLD);
                redPaint.setShadowLayer(5f, 2f, 2f, Color.parseColor("#FF000000")); // Black shadow

                String lang = java.util.Locale.getDefault().getLanguage();
                if (lang.equals("fr")) {
                    warningText = "ATTENTION: Les options Unban/Admin sont FAUSSES.\nSi on vous vend ce mod, c'est une ARNAQUE !";
                } else if (lang.equals("es")) {
                    warningText = "ATENCIÓN: Las opciones Unban/Admin son FALSAS.\n¡Si alguien te vende esto, es una ESTAFA!";
                } else if (lang.equals("ar")) {
                    warningText = "تحذير: خيارات المسؤول مزيفة.\nإذا تم بيع هذا لك، فهو عملية احتيال!";
                } else {
                    warningText = "WARNING: Unban/Admin features are FAKE.\nIf someone sells you this mod, it's a SCAM!";
                }
            }

            @Override
            protected void onDraw(Canvas canvas) {
                super.onDraw(canvas);
                if (!isRecordingArr[0]) return; // INVISIBLE TO LEGIT PLAYERS!

                int width = getWidth();
                int height = getHeight();
                
                float textWidth = paint.measureText(text) + dp(50);
                float textHeight = paint.getTextSize() + dp(80);
                
                canvas.save();
                canvas.rotate(-25, width / 2f, height / 2f);
                for (float x = -width; x < width * 2; x += textWidth) {
                    for (float y = -height; y < height * 2; y += textHeight) {
                        canvas.drawText(text, x, y, paint);
                    }
                }
                canvas.restore();

                // Draw RED SCAM WARNING
                String[] lines = warningText.split("\n");
                
                // Center
                float startY = height / 2f;
                for (int i = 0; i < lines.length; i++) {
                    canvas.drawText(lines[i], dp(10), startY + (i * dp(25)), redPaint);
                }
                
                // Bottom
                startY = height * 0.8f;
                for (int i = 0; i < lines.length; i++) {
                    canvas.drawText(lines[i], dp(10), startY + (i * dp(25)), redPaint);
                }
            }
        };

        int iparams = isActivity ? WindowManager.LayoutParams.TYPE_APPLICATION : (Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.O ? 2038 : 2002);
        final WindowManager.LayoutParams wmParams = new WindowManager.LayoutParams(
                WindowManager.LayoutParams.MATCH_PARENT,
                WindowManager.LayoutParams.MATCH_PARENT,
                iparams,
                WindowManager.LayoutParams.FLAG_NOT_FOCUSABLE |
                        WindowManager.LayoutParams.FLAG_NOT_TOUCHABLE |
                        WindowManager.LayoutParams.FLAG_LAYOUT_IN_SCREEN |
                        WindowManager.LayoutParams.FLAG_LAYOUT_NO_LIMITS,
                PixelFormat.TRANSLUCENT);
        wmParams.gravity = Gravity.CENTER;
        wmParams.alpha = 0.6f; // Android 12+ touch fix
        // DO NOT add view here! Wait for recording to start!

        // Poll for Screen Recording
        final android.os.Handler handler = new android.os.Handler(android.os.Looper.getMainLooper());
        Runnable checkRecordRunnable = new Runnable() {
            @Override
            public void run() {
                boolean recording = false;
                try {
                    android.hardware.display.DisplayManager dm = (android.hardware.display.DisplayManager) getContext.getSystemService(Context.DISPLAY_SERVICE);
                    if (dm != null) {
                        android.view.Display[] displays = dm.getDisplays();
                        for (android.view.Display display : displays) {
                            if (display.getDisplayId() != android.view.Display.DEFAULT_DISPLAY) {
                                recording = true;
                                break;
                            }
                        }
                    }
                } catch (Exception e) {}
                
                if (isRecordingArr[0] != recording) {
                    isRecordingArr[0] = recording;
                    if (recording) {
                        try { mWindowManager.addView(watermarkView, wmParams); } catch (Exception e) {}
                    } else {
                        try { mWindowManager.removeView(watermarkView); } catch (Exception e) {}
                    }
                }
                handler.postDelayed(this, 1000);
            }
        };
        handler.post(checkRecordRunnable);
    }

    private View.OnTouchListener onTouchListener() {
        return new View.OnTouchListener() {
            final View collapsedView = mCollapsed;
            final View expandedView = mExpanded;
            private float initialTouchX, initialTouchY;
            private int initialX, initialY;

            public boolean onTouch(View view, MotionEvent motionEvent) {
                switch (motionEvent.getAction()) {
                    case MotionEvent.ACTION_DOWN:
                        initialX = vmParams.x;
                        initialY = vmParams.y;
                        initialTouchX = motionEvent.getRawX();
                        initialTouchY = motionEvent.getRawY();
                        return true;
                    case MotionEvent.ACTION_UP:
                        int rawX = (int) (motionEvent.getRawX() - initialTouchX);
                        int rawY = (int) (motionEvent.getRawY() - initialTouchY);
                        mExpanded.setAlpha(1f);
                        mCollapsed.setAlpha(1f);
                        if (Math.abs(rawX) < 20 && Math.abs(rawY) < 20 && isViewCollapsed()) {
                            // SCATTER MODE â€” au tap simple, on ouvre/ferme l'overlay
                            // de bulles Ã©parpillÃ©es au lieu du menu LGL classique.
                            // Le menu classique reste accessible via long-press
                            // (>500ms) ci-dessous.
                            // REVERT V147 : menu classique LGL au tap (au lieu du dock).
                            // Le tap sur la bulle bascule entre mCollapsed et mExpanded
                            // exactement comme dans le menu LGL d'origine.
                            if (isViewCollapsed()) {
                                openMenu();
                            } else {
                                closeMenu();
                            }
                            // V156 : son "gravity_sound.mp3" au tap sur icône SUPPRIMÉ.
                            // Seul gravity_celeste.mp3 (party mode) joue désormais.
                        }
                        return true;
                    case MotionEvent.ACTION_MOVE:
                        mExpanded.setAlpha(0.5f);
                        mCollapsed.setAlpha(0.5f);
                        vmParams.x = initialX + ((int) (motionEvent.getRawX() - initialTouchX));
                        vmParams.y = initialY + ((int) (motionEvent.getRawY() - initialTouchY));
                        // Publier la position pour l'animation ESP.
                        sIconX = vmParams.x;
                        sIconY = vmParams.y;
                        mWindowManager.updateViewLayout(rootFrame, vmParams);
                        return true;
                    default:
                        return false;
                }
            }
        };
    }

    // â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
    // Helper: build a smooth RGB ValueAnimator
    // â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
    private ValueAnimator buildRgbAnimator(long duration, ValueAnimator.AnimatorUpdateListener listener) {
        ValueAnimator anim = ValueAnimator.ofArgb(RGB_COLORS);
        anim.setDuration(duration);
        anim.setRepeatCount(ValueAnimator.INFINITE);
        anim.setEvaluator(new ArgbEvaluator());
        anim.addUpdateListener(listener);
        return anim;
    }

    private void cancelAnimators() {
        if (bubbleRgbAnim != null) bubbleRgbAnim.cancel();
        if (bubbleTextAnim != null) bubbleTextAnim.cancel();
        if (borderAnim != null) borderAnim.cancel();
        if (titleAnim != null) titleAnim.cancel();
    }

    // â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
    // Feature list parser
    // â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
    private void featureList(String[] listFT, LinearLayout linearLayout) {
        int featNum, subFeat = 0;
        LinearLayout llBak = linearLayout;

        for (int i = 0; i < listFT.length; i++) {
            boolean switchedOn = false;
            String feature = listFT[i];
            if (feature.contains("_True")) {
                switchedOn = true;
                feature = feature.replaceFirst("_True", "");
            }
            linearLayout = llBak;
            if (feature.contains("CollapseAdd_")) {
                linearLayout = mCollapse;
                feature = feature.replaceFirst("CollapseAdd_", "");
            }
            String[] str = feature.split("_");
            if (TextUtils.isDigitsOnly(str[0]) || str[0].matches("-[0-9]*")) {
                featNum = Integer.parseInt(str[0]);
                feature = feature.replaceFirst(str[0] + "_", "");
                subFeat++;
            } else {
                featNum = i - subFeat;
            }
            String[] strSplit = feature.split("_");
            switch (strSplit[0]) {
                case "Toggle":
                    Switch(linearLayout, featNum, strSplit[1], switchedOn);
                    break;
                case "SeekBar":
                    SeekBar(linearLayout, featNum, strSplit[1], Integer.parseInt(strSplit[2]), Integer.parseInt(strSplit[3]));
                    break;
                case "Button":
                    Button(linearLayout, featNum, strSplit[1]);
                    break;
                case "ButtonOnOff":
                    ButtonOnOff(linearLayout, featNum, strSplit[1], switchedOn);
                    break;
                case "Spinner":
                    TextView(linearLayout, strSplit[1]);
                    Spinner(linearLayout, featNum, strSplit[1], strSplit[2]);
                    break;
                case "InputText":
                    InputText(linearLayout, featNum, strSplit[1]);
                    break;
                case "InputValue":
                    if (strSplit.length == 3)
                        InputNum(linearLayout, featNum, strSplit[2], Integer.parseInt(strSplit[1]));
                    if (strSplit.length == 2)
                        InputNum(linearLayout, featNum, strSplit[1], 0);
                    break;
                case "InputLValue":
                    if (strSplit.length == 3)
                        InputLNum(linearLayout, featNum, strSplit[2], Long.parseLong(strSplit[1]));
                    if (strSplit.length == 2)
                        InputLNum(linearLayout, featNum, strSplit[1], 0);
                    break;
                case "CheckBox":
                    CheckBox(linearLayout, featNum, strSplit[1], switchedOn);
                    break;
                case "RadioButton":
                    RadioButton(linearLayout, featNum, strSplit[1], strSplit[2]);
                    break;
                case "Collapse":
                    Collapse(linearLayout, strSplit[1], switchedOn);
                    subFeat++;
                    break;
                case "ButtonLink":
                    subFeat++;
                    ButtonLink(linearLayout, strSplit[1], strSplit[2]);
                    break;
                case "Category":
                    subFeat++;
                    Category(linearLayout, strSplit[1]);
                    break;
                case "RichTextView":
                    subFeat++;
                    TextView(linearLayout, strSplit[1]);
                    break;
                case "RichWebView":
                    subFeat++;
                    WebTextView(linearLayout, strSplit[1]);
                    break;
            }
        }
    }

    // â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
    // Switch / Toggle
    // â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
    private void Switch(LinearLayout linLayout, final int featNum, final String featName, boolean swiOn) {
        // Premium card container
        final FrameLayout card = new FrameLayout(getContext);
        LinearLayout.LayoutParams cardLp = new LinearLayout.LayoutParams(MATCH_PARENT, WRAP_CONTENT);
        cardLp.setMargins(dp(6), dp(3), dp(6), dp(3));
        card.setLayoutParams(cardLp);

        final boolean[] isOn = {Preferences.loadPrefBool(featName, featNum, swiOn)};
        final GradientDrawable cardBg = new GradientDrawable();
        cardBg.setCornerRadius(dp(10));
        cardBg.setColor(isOn[0] ? Color.parseColor("#0D1F0D") : Color.parseColor("#1A0510"));
        cardBg.setStroke(dp(1), isOn[0] ? Color.parseColor("#00FF88") : Color.parseColor("#FF0055"));
        card.setBackground(cardBg);

        LinearLayout row = new LinearLayout(getContext);
        row.setOrientation(LinearLayout.HORIZONTAL);
        row.setGravity(Gravity.CENTER_VERTICAL);
        row.setPadding(dp(10), dp(7), dp(10), dp(7));

        // Feature name
        final TextView label = new TextView(getContext);
        LinearLayout.LayoutParams llp = new LinearLayout.LayoutParams(0, WRAP_CONTENT, 1f);
        label.setLayoutParams(llp);
        label.setText(featName);
        label.setTextColor(Color.parseColor("#DDE8FF"));
        label.setTextSize(12f);
        label.setShadowLayer(isOn[0] ? 8f : 0f, 0f, 0f, Color.parseColor("#00FF88"));

        // Status badge
        final TextView badge = new TextView(getContext);
        badge.setText(isOn[0] ? " ON " : "OFF");
        badge.setTextSize(9f);
        badge.setTypeface(null, Typeface.BOLD);
        badge.setPadding(dp(5), dp(2), dp(5), dp(2));
        badge.setTextColor(Color.WHITE);
        final GradientDrawable badgeBg = new GradientDrawable();
        badgeBg.setCornerRadius(dp(20));
        badgeBg.setColor(isOn[0] ? Color.parseColor("#00CC66") : Color.parseColor("#CC0033"));
        badge.setBackground(badgeBg);

        final Switch switchR = new Switch(getContext);
        switchR.setChecked(isOn[0]);
        try {
            ColorStateList tint = new ColorStateList(
                new int[][]{ new int[]{android.R.attr.state_checked}, new int[]{} },
                new int[]{ Color.parseColor("#00FF88"), Color.parseColor("#FF0055") }
            );
            switchR.getThumbDrawable().setTintList(tint);
            switchR.getTrackDrawable().setTintList(tint);
        } catch (Exception ignored) {}
        switchR.setText("");
        switchR.setPadding(dp(4), 0, 0, 0);

        switchR.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            public void onCheckedChanged(CompoundButton compoundButton, boolean bool) {
                isOn[0] = bool;
                cardBg.setColor(bool ? Color.parseColor("#0D1F0D") : Color.parseColor("#1A0510"));
                cardBg.setStroke(dp(1), bool ? Color.parseColor("#00FF88") : Color.parseColor("#FF0055"));
                badge.setText(bool ? " ON " : "OFF");
                badgeBg.setColor(bool ? Color.parseColor("#00CC66") : Color.parseColor("#CC0033"));
                label.setShadowLayer(bool ? 10f : 0f, 0f, 0f, Color.parseColor("#00FF88"));
                // Bounce animation
                card.animate().scaleX(0.96f).scaleY(0.96f).setDuration(80).withEndAction(new Runnable() {
                    @Override public void run() {
                        card.animate().scaleX(1f).scaleY(1f).setDuration(150).start();
                    }
                }).start();
                Preferences.changeFeatureBool(featName, featNum, bool);
                switch (featNum) {
                    case -1:
                        Preferences.with(switchR.getContext()).writeBoolean(-1, bool);
                        if (!bool) Preferences.with(switchR.getContext()).clear();
                        break;
                    case -3:
                        Preferences.isExpanded = bool;
                        scrollView.setLayoutParams(bool ? scrlLLExpanded : scrlLL);
                        break;
                }
            }
        });

        row.addView(label);
        row.addView(badge);
        row.addView(switchR);
        card.addView(row);
        linLayout.addView(card);
    }

    // â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
    // SeekBar
    // â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
    private void SeekBar(LinearLayout linLayout, final int featNum, final String featName, final int min, int max) {
        int loadedProg = Preferences.loadPrefInt(featName, featNum);
        final int initVal = (loadedProg == 0) ? min : loadedProg;

        // Card container
        final LinearLayout card = new LinearLayout(getContext);
        LinearLayout.LayoutParams cardLp = new LinearLayout.LayoutParams(MATCH_PARENT, WRAP_CONTENT);
        cardLp.setMargins(dp(6), dp(4), dp(6), dp(4));
        card.setLayoutParams(cardLp);
        card.setOrientation(LinearLayout.VERTICAL);
        card.setPadding(dp(10), dp(8), dp(10), dp(8));
        GradientDrawable seekCard = new GradientDrawable();
        seekCard.setCornerRadius(dp(10));
        seekCard.setColor(Color.parseColor("#0A0E20"));
        seekCard.setStroke(dp(1), Color.parseColor("#0055CC"));
        card.setBackground(seekCard);

        // Label + value row
        LinearLayout headerRow = new LinearLayout(getContext);
        headerRow.setOrientation(LinearLayout.HORIZONTAL);
        headerRow.setGravity(Gravity.CENTER_VERTICAL);

        final TextView textView = new TextView(getContext);
        LinearLayout.LayoutParams tlp = new LinearLayout.LayoutParams(0, WRAP_CONTENT, 1f);
        textView.setLayoutParams(tlp);
        textView.setText(featName);
        textView.setTextColor(Color.parseColor("#BDD0FF"));
        textView.setTextSize(11.5f);

        // Value bubble
        final TextView valBubble = new TextView(getContext);
        valBubble.setText(String.valueOf(initVal));
        valBubble.setTextColor(Color.WHITE);
        valBubble.setTextSize(11f);
        valBubble.setTypeface(null, Typeface.BOLD);
        valBubble.setPadding(dp(8), dp(2), dp(8), dp(2));
        GradientDrawable valBg = new GradientDrawable();
        valBg.setCornerRadius(dp(20));
        valBg.setColor(Color.parseColor("#0055FF"));
        valBubble.setBackground(valBg);

        headerRow.addView(textView);
        headerRow.addView(valBubble);

        SeekBar seekBar = new SeekBar(getContext);
        seekBar.setPadding(0, dp(8), 0, dp(4));
        seekBar.setMax(max);
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) seekBar.setMin(min);
        seekBar.setProgress(initVal);
        seekBar.getThumb().setColorFilter(Color.parseColor("#0088FF"), PorterDuff.Mode.SRC_ATOP);
        seekBar.getProgressDrawable().setColorFilter(Color.parseColor("#0055FF"), PorterDuff.Mode.SRC_ATOP);
        seekBar.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            public void onStartTrackingTouch(SeekBar sb) {
                card.animate().scaleX(1.02f).scaleY(1.02f).setDuration(100).start();
            }
            public void onStopTrackingTouch(SeekBar sb) {
                card.animate().scaleX(1f).scaleY(1f).setDuration(150).start();
            }
            public void onProgressChanged(SeekBar sb, int i, boolean z) {
                int v = i < min ? min : i;
                sb.setProgress(v);
                Preferences.changeFeatureInt(featName, featNum, v);
                valBubble.setText(String.valueOf(v));
            }
        });
        card.addView(headerRow);
        card.addView(seekBar);
        linLayout.addView(card);
    }

    // â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
    // Button
    // â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
    private void Button(LinearLayout linLayout, final int featNum, final String featName) {
        final Button button = new Button(getContext);
        LinearLayout.LayoutParams layoutParams = new LinearLayout.LayoutParams(MATCH_PARENT, WRAP_CONTENT);
        layoutParams.setMargins(dp(8), dp(4), dp(8), dp(4));
        button.setLayoutParams(layoutParams);
        button.setTextColor(Color.WHITE);
        button.setAllCaps(false);
        button.setText(Html.fromHtml(featName));
        button.setTextSize(12f);
        button.setShadowLayer(6f, 0f, 0f, Color.parseColor("#0088FF"));

        // Gradient background: dark blue left → darker right
        final GradientDrawable btnBg = new GradientDrawable(
            GradientDrawable.Orientation.LEFT_RIGHT,
            new int[]{ Color.parseColor("#0A0E2A"), Color.parseColor("#050510") }
        );
        btnBg.setCornerRadius(dp(10));
        btnBg.setStroke(dp(1), Color.parseColor("#0055FF"));
        button.setBackground(btnBg);
        button.setPadding(dp(4), dp(10), dp(4), dp(10));

        // RGB border animation
        buildRgbAnimator(2800, new ValueAnimator.AnimatorUpdateListener() {
            @Override
            public void onAnimationUpdate(ValueAnimator anim) {
                int col = (int) anim.getAnimatedValue();
                btnBg.setStroke(dp(1), col);
                button.setShadowLayer(8f, 0f, 0f, col);
            }
        }).start();

        // Breathing opacity
        ValueAnimator breathe = ValueAnimator.ofFloat(0.85f, 1.0f);
        breathe.setDuration(1400);
        breathe.setRepeatCount(ValueAnimator.INFINITE);
        breathe.setRepeatMode(ValueAnimator.REVERSE);
        breathe.addUpdateListener(new ValueAnimator.AnimatorUpdateListener() {
            @Override
            public void onAnimationUpdate(ValueAnimator anim) {
                button.setAlpha((float) anim.getAnimatedValue());
            }
        });
        breathe.start();

        button.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {
                breathe.cancel();
                button.setAlpha(1f);
                // Flash white then restore
                button.setTextColor(Color.parseColor("#00FF88"));
                button.postDelayed(new Runnable() { @Override public void run() {
                    button.setTextColor(Color.WHITE);
                    breathe.start();
                }}, 250);
                v.animate().scaleX(0.92f).scaleY(0.92f).setDuration(70).withEndAction(new Runnable() {
                    @Override public void run() {
                        button.animate().scaleX(1f).scaleY(1f).setDuration(180)
                            .setInterpolator(new OvershootInterpolator(2f)).start();
                    }
                }).start();
                switch (featNum) {
                    case -6:
                        scrollView.removeView(mSettings);
                        scrollView.addView(mods);
                        break;
                    case -100:
                        stopChecking = true;
                        break;
                }
                Preferences.changeFeatureInt(featName, featNum, 0);
            }
        });
        linLayout.addView(button);
    }

    // â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
    // ButtonLink
    // â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
    private void ButtonLink(LinearLayout linLayout, final String featName, final String url) {
        final Button button = new Button(getContext);
        LinearLayout.LayoutParams layoutParams = new LinearLayout.LayoutParams(MATCH_PARENT, MATCH_PARENT);
        layoutParams.setMargins(dp(7), dp(5), dp(7), dp(5));
        button.setLayoutParams(layoutParams);
        button.setAllCaps(false);
        button.setTextColor(TEXT_COLOR_2);
        button.setText(Html.fromHtml(featName));
        button.setBackgroundColor(BTN_COLOR);
        button.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {
                Intent intent = new Intent(Intent.ACTION_VIEW);
                intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
                intent.setData(Uri.parse(url));
                getContext.startActivity(intent);
            }
        });
        linLayout.addView(button);
    }

    // â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
    // ButtonOnOff
    // â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
    private void ButtonOnOff(LinearLayout linLayout, final int featNum, String featName, boolean switchedOn) {
        final Button button = new Button(getContext);
        LinearLayout.LayoutParams layoutParams = new LinearLayout.LayoutParams(MATCH_PARENT, MATCH_PARENT);
        layoutParams.setMargins(dp(7), dp(5), dp(7), dp(5));
        button.setLayoutParams(layoutParams);
        button.setTextColor(Color.parseColor("#E0E8FF"));
        button.setAllCaps(false);

        final GradientDrawable gdOn = new GradientDrawable();
        gdOn.setCornerRadius(dp(8));
        gdOn.setColor(BtnON);
        gdOn.setStroke(dp(2), ToggleON);

        final GradientDrawable gdOff = new GradientDrawable();
        gdOff.setCornerRadius(dp(8));
        gdOff.setColor(BtnOFF);
        gdOff.setStroke(dp(1), ToggleOFF);

        final String finalfeatName = featName.replace("OnOff_", "");
        boolean isOn = Preferences.loadPrefBool(featName, featNum, switchedOn);
        if (isOn) {
            button.setText(Html.fromHtml("<b>" + finalfeatName + "</b>: <font color='#00FFC0'>ON</font>"));
            button.setBackground(gdOn);
            isOn = false;
        } else {
            button.setText(Html.fromHtml("<b>" + finalfeatName + "</b>: <font color='#FF0055'>OFF</font>"));
            button.setBackground(gdOff);
            isOn = true;
        }
        final boolean finalIsOn = isOn;
        button.setOnClickListener(new View.OnClickListener() {
            boolean isOn = finalIsOn;
            public void onClick(View v) {
                Preferences.changeFeatureBool(finalfeatName, featNum, isOn);
                button.animate().scaleX(0.95f).scaleY(0.95f).setDuration(70).withEndAction(new Runnable() {
                    @Override public void run() {
                        button.animate().scaleX(1f).scaleY(1f).setDuration(110).start();
                    }
                }).start();
                if (isOn) {
                    button.setText(Html.fromHtml("<b>" + finalfeatName + "</b>: <font color='#00FFC0'>ON</font>"));
                    button.setBackground(gdOn);
                    isOn = false;
                } else {
                    button.setText(Html.fromHtml("<b>" + finalfeatName + "</b>: <font color='#FF0055'>OFF</font>"));
                    button.setBackground(gdOff);
                    isOn = true;
                }
            }
        });
        linLayout.addView(button);
    }

    // â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
    // Spinner
    // â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
    private void Spinner(LinearLayout linLayout, final int featNum, final String featName, final String list) {
        Log.d(TAG, "spinner " + featNum + " " + featName + " " + list);
        final List<String> lists = new LinkedList<>(Arrays.asList(list.split(",")));

        LinearLayout linearLayout2 = new LinearLayout(getContext);
        LinearLayout.LayoutParams layoutParams2 = new LinearLayout.LayoutParams(MATCH_PARENT, WRAP_CONTENT);
        layoutParams2.setMargins(dp(7), dp(2), dp(7), dp(2));
        linearLayout2.setOrientation(LinearLayout.VERTICAL);
        linearLayout2.setBackgroundColor(BTN_COLOR);
        linearLayout2.setLayoutParams(layoutParams2);

        final Spinner spinner = new Spinner(getContext, Spinner.MODE_DROPDOWN);
        spinner.setLayoutParams(layoutParams2);
        spinner.getBackground().setColorFilter(1, PorterDuff.Mode.SRC_ATOP);
        ArrayAdapter aa = new ArrayAdapter(getContext, android.R.layout.simple_spinner_dropdown_item, lists);
        aa.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        spinner.setAdapter(aa);
        spinner.setSelection(Preferences.loadPrefInt(featName, featNum));
        spinner.setOnItemSelectedListener(new AdapterView.OnItemSelectedListener() {
            @Override
            public void onItemSelected(AdapterView<?> parentView, View selectedItemView, int position, long id) {
                Preferences.changeFeatureInt(spinner.getSelectedItem().toString(), featNum, position);
                ((TextView) parentView.getChildAt(0)).setTextColor(TEXT_COLOR_2);
            }
            @Override
            public void onNothingSelected(AdapterView<?> parent) {}
        });
        linearLayout2.addView(spinner);
        linLayout.addView(linearLayout2);
    }

    // â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
    // InputNum (int)
    // â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
    private void InputNum(LinearLayout linLayout, final int featNum, final String featName, final int maxValue) {
        LinearLayout linearLayout = new LinearLayout(getContext);
        LinearLayout.LayoutParams layoutParams = new LinearLayout.LayoutParams(MATCH_PARENT, MATCH_PARENT);
        layoutParams.setMargins(dp(7), dp(5), dp(7), dp(5));

        final Button button = new Button(getContext);
        int num = Preferences.loadPrefInt(featName, featNum);
        button.setText(Html.fromHtml(featName + ": <font color='" + NumberTxtColor + "'>" + num + "</font>"));
        button.setAllCaps(false);
        button.setLayoutParams(layoutParams);
        button.setBackgroundColor(BTN_COLOR);
        button.setTextColor(TEXT_COLOR_2);
        button.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                AlertDialog.Builder alertName = new AlertDialog.Builder(getContext);
                final EditText editText = new EditText(getContext);
                if (maxValue != 0) editText.setHint("Max value: " + maxValue);
                editText.setInputType(InputType.TYPE_CLASS_NUMBER);
                editText.setKeyListener(DigitsKeyListener.getInstance("0123456789-"));
                InputFilter[] FilterArray = new InputFilter[1];
                FilterArray[0] = new InputFilter.LengthFilter(10);
                editText.setFilters(FilterArray);
                editText.setOnFocusChangeListener(new View.OnFocusChangeListener() {
                    @Override
                    public void onFocusChange(View v, boolean hasFocus) {
                        InputMethodManager imm = (InputMethodManager) getContext.getSystemService(Context.INPUT_METHOD_SERVICE);
                        if (hasFocus) imm.toggleSoftInput(InputMethodManager.SHOW_FORCED, InputMethodManager.HIDE_IMPLICIT_ONLY);
                        else imm.toggleSoftInput(InputMethodManager.SHOW_IMPLICIT, 0);
                    }
                });
                editText.requestFocus();
                alertName.setTitle("Input number");
                LinearLayout layoutName = new LinearLayout(getContext);
                layoutName.setOrientation(LinearLayout.VERTICAL);
                layoutName.addView(editText);
                alertName.setView(layoutName);
                alertName.setPositiveButton("OK", new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog, int whichButton) {
                        int num;
                        try {
                            String inp = editText.getText().toString();
                            num = Integer.parseInt(inp.isEmpty() ? "0" : inp);
                            if (maxValue != 0 && num >= maxValue) num = maxValue;
                        } catch (NumberFormatException ex) {
                            num = maxValue != 0 ? maxValue : Integer.MAX_VALUE;
                        }
                        button.setText(Html.fromHtml(featName + ": <font color='" + NumberTxtColor + "'>" + num + "</font>"));
                        Preferences.changeFeatureInt(featName, featNum, num);
                        editText.setFocusable(false);
                    }
                });
                alertName.setNegativeButton("Cancel", new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog, int whichButton) {
                        InputMethodManager imm = (InputMethodManager) getContext.getSystemService(Context.INPUT_METHOD_SERVICE);
                        imm.toggleSoftInput(InputMethodManager.SHOW_IMPLICIT, 0);
                    }
                });
                if (overlayRequired) {
                    AlertDialog dialog = alertName.create();
                    Objects.requireNonNull(dialog.getWindow()).setType(Build.VERSION.SDK_INT >= 26 ? 2038 : 2002);
                    dialog.show();
                } else {
                    alertName.show();
                }
            }
        });
        linearLayout.addView(button);
        linLayout.addView(linearLayout);
    }

    // â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
    // InputLNum (long)
    // â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
    private void InputLNum(LinearLayout linLayout, final int featNum, final String featName, final long maxValue) {
        LinearLayout linearLayout = new LinearLayout(getContext);
        LinearLayout.LayoutParams layoutParams = new LinearLayout.LayoutParams(MATCH_PARENT, MATCH_PARENT);
        layoutParams.setMargins(dp(7), dp(5), dp(7), dp(5));

        final Button button = new Button(getContext);
        long lnum = Preferences.loadPrefLong(featName, featNum);
        button.setText(Html.fromHtml(featName + ": <font color='" + NumberTxtColor + "'>" + lnum + "</font>"));
        button.setAllCaps(false);
        button.setLayoutParams(layoutParams);
        button.setBackgroundColor(BTN_COLOR);
        button.setTextColor(TEXT_COLOR_2);
        button.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                AlertDialog.Builder alertName = new AlertDialog.Builder(getContext);
                final EditText editText = new EditText(getContext);
                if (maxValue != 0) editText.setHint("Max value: " + maxValue);
                editText.setInputType(InputType.TYPE_CLASS_NUMBER);
                editText.setKeyListener(DigitsKeyListener.getInstance("0123456789-"));
                InputFilter[] FilterArray = new InputFilter[1];
                FilterArray[0] = new InputFilter.LengthFilter(19);
                editText.setFilters(FilterArray);
                editText.setOnFocusChangeListener(new View.OnFocusChangeListener() {
                    @Override
                    public void onFocusChange(View v, boolean hasFocus) {
                        InputMethodManager imm = (InputMethodManager) getContext.getSystemService(Context.INPUT_METHOD_SERVICE);
                        if (hasFocus) imm.toggleSoftInput(InputMethodManager.SHOW_FORCED, InputMethodManager.HIDE_IMPLICIT_ONLY);
                        else imm.toggleSoftInput(InputMethodManager.SHOW_IMPLICIT, 0);
                    }
                });
                editText.requestFocus();
                alertName.setTitle("Input number");
                LinearLayout layoutName = new LinearLayout(getContext);
                layoutName.setOrientation(LinearLayout.VERTICAL);
                layoutName.addView(editText);
                alertName.setView(layoutName);
                alertName.setPositiveButton("OK", new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog, int whichButton) {
                        long num;
                        try {
                            String inp = editText.getText().toString();
                            num = Long.parseLong(inp.isEmpty() ? "0" : inp);
                            if (maxValue != 0 && num >= maxValue) num = maxValue;
                        } catch (NumberFormatException ex) {
                            num = maxValue != 0 ? maxValue : Long.MAX_VALUE;
                        }
                        button.setText(Html.fromHtml(featName + ": <font color='" + NumberTxtColor + "'>" + num + "</font>"));
                        Preferences.changeFeatureLong(featName, featNum, num);
                        editText.setFocusable(false);
                    }
                });
                alertName.setNegativeButton("Cancel", new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog, int whichButton) {
                        InputMethodManager imm = (InputMethodManager) getContext.getSystemService(Context.INPUT_METHOD_SERVICE);
                        imm.toggleSoftInput(InputMethodManager.SHOW_IMPLICIT, 0);
                    }
                });
                if (overlayRequired) {
                    AlertDialog dialog = alertName.create();
                    Objects.requireNonNull(dialog.getWindow()).setType(Build.VERSION.SDK_INT >= 26 ? 2038 : 2002);
                    dialog.show();
                } else {
                    alertName.show();
                }
            }
        });
        linearLayout.addView(button);
        linLayout.addView(linearLayout);
    }

    // â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
    // InputText
    // â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
    private void InputText(LinearLayout linLayout, final int featNum, final String featName) {
        LinearLayout linearLayout = new LinearLayout(getContext);
        LinearLayout.LayoutParams layoutParams = new LinearLayout.LayoutParams(MATCH_PARENT, MATCH_PARENT);
        layoutParams.setMargins(dp(7), dp(5), dp(7), dp(5));

        final Button button = new Button(getContext);
        String string = Preferences.loadPrefString(featName, featNum);
        button.setText(Html.fromHtml(featName + ": <font color='" + NumberTxtColor + "'>" + string + "</font>"));
        button.setAllCaps(false);
        button.setLayoutParams(layoutParams);
        button.setBackgroundColor(BTN_COLOR);
        button.setTextColor(TEXT_COLOR_2);
        button.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                AlertDialog.Builder alertName = new AlertDialog.Builder(getContext);
                final EditText editText = new EditText(getContext);
                editText.setOnFocusChangeListener(new View.OnFocusChangeListener() {
                    @Override
                    public void onFocusChange(View v, boolean hasFocus) {
                        InputMethodManager imm = (InputMethodManager) getContext.getSystemService(Context.INPUT_METHOD_SERVICE);
                        if (hasFocus) imm.toggleSoftInput(InputMethodManager.SHOW_FORCED, InputMethodManager.HIDE_IMPLICIT_ONLY);
                        else imm.toggleSoftInput(InputMethodManager.SHOW_IMPLICIT, 0);
                    }
                });
                editText.requestFocus();
                alertName.setTitle("Input text");
                LinearLayout layoutName = new LinearLayout(getContext);
                layoutName.setOrientation(LinearLayout.VERTICAL);
                layoutName.addView(editText);
                alertName.setView(layoutName);
                alertName.setPositiveButton("OK", new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog, int whichButton) {
                        String str = editText.getText().toString();
                        button.setText(Html.fromHtml(featName + ": <font color='" + NumberTxtColor + "'>" + str + "</font>"));
                        Preferences.changeFeatureString(featName, featNum, str);
                        editText.setFocusable(false);
                    }
                });
                alertName.setNegativeButton("Cancel", new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog, int whichButton) {
                        InputMethodManager imm = (InputMethodManager) getContext.getSystemService(Context.INPUT_METHOD_SERVICE);
                        imm.toggleSoftInput(InputMethodManager.SHOW_IMPLICIT, 0);
                    }
                });
                if (overlayRequired) {
                    AlertDialog dialog = alertName.create();
                    dialog.getWindow().setType(Build.VERSION.SDK_INT >= 26 ? 2038 : 2002);
                    dialog.show();
                } else {
                    alertName.show();
                }
            }
        });
        linearLayout.addView(button);
        linLayout.addView(linearLayout);
    }

    // â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
    // CheckBox
    // â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
    private void CheckBox(LinearLayout linLayout, final int featNum, final String featName, boolean switchedOn) {
        final CheckBox checkBox = new CheckBox(getContext);
        checkBox.setText(featName);
        checkBox.setTextColor(TEXT_COLOR_2);
        checkBox.setButtonTintList(ColorStateList.valueOf(CheckBoxColor));
        checkBox.setChecked(Preferences.loadPrefBool(featName, featNum, switchedOn));
        checkBox.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                Preferences.changeFeatureBool(featName, featNum, isChecked);
            }
        });
        linLayout.addView(checkBox);
    }

    // â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
    // RadioButton
    // â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
    @SuppressLint("SetTextI18n")
    private void RadioButton(LinearLayout linLayout, final int featNum, String featName, final String list) {
        final List<String> lists = new LinkedList<>(Arrays.asList(list.split(",")));
        final TextView textView = new TextView(getContext);
        textView.setText(featName + ":");
        textView.setTextColor(TEXT_COLOR_2);

        final RadioGroup radioGroup = new RadioGroup(getContext);
        radioGroup.setPadding(dp(10), dp(5), dp(10), dp(5));
        radioGroup.setOrientation(LinearLayout.VERTICAL);
        radioGroup.addView(textView);

        for (int i = 0; i < lists.size(); i++) {
            final RadioButton Radioo = new RadioButton(getContext);
            final String finalfeatName = featName, radioName = lists.get(i);
            View.OnClickListener first_radio_listener = new View.OnClickListener() {
                public void onClick(View v) {
                    textView.setText(Html.fromHtml(finalfeatName + ": <font color='" + NumberTxtColor + "'>" + radioName));
                    Preferences.changeFeatureInt(finalfeatName, featNum, radioGroup.indexOfChild(Radioo));
                }
            };
            System.out.println(lists.get(i));
            Radioo.setText(lists.get(i));
            Radioo.setTextColor(Color.LTGRAY);
            Radioo.setButtonTintList(ColorStateList.valueOf(RadioColor));
            Radioo.setOnClickListener(first_radio_listener);
            radioGroup.addView(Radioo);
        }

        int index = Preferences.loadPrefInt(featName, featNum);
        if (index > 0) {
            textView.setText(Html.fromHtml(featName + ": <font color='" + NumberTxtColor + "'>" + lists.get(index - 1)));
            ((RadioButton) radioGroup.getChildAt(index)).setChecked(true);
        }
        linLayout.addView(radioGroup);
    }

    // â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
    // Collapse
    // â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
    @SuppressLint("SetTextI18n")
    private void Collapse(LinearLayout linLayout, final String text, final boolean expanded) {
        LinearLayout.LayoutParams layoutParamsLL = new LinearLayout.LayoutParams(MATCH_PARENT, MATCH_PARENT);
        layoutParamsLL.setMargins(0, dp(5), 0, 0);

        LinearLayout collapse = new LinearLayout(getContext);
        collapse.setLayoutParams(layoutParamsLL);
        collapse.setVerticalGravity(16);
        collapse.setOrientation(LinearLayout.VERTICAL);

        final LinearLayout collapseSub = new LinearLayout(getContext);
        collapseSub.setVerticalGravity(16);
        collapseSub.setPadding(0, dp(5), 0, dp(5));
        collapseSub.setOrientation(LinearLayout.VERTICAL);
        collapseSub.setBackgroundColor(Color.parseColor("#1A2535"));
        collapseSub.setVisibility(View.GONE);
        mCollapse = collapseSub;

        // Animated collapse header
        final GradientDrawable collHdrBg = new GradientDrawable();
        collHdrBg.setColor(CollapseColor);
        collHdrBg.setStroke(dp(1), RGB_COLORS[0]);

        final TextView textView = new TextView(getContext);
        textView.setBackground(collHdrBg);
        textView.setText("â–½ " + text + " â–½");
        textView.setGravity(Gravity.CENTER);
        textView.setTextColor(TEXT_COLOR_2);
        textView.setTypeface(null, Typeface.BOLD);
        textView.setPadding(0, dp(12), 0, dp(12));

        // RGB border on collapse header
        final ValueAnimator collAnim = buildRgbAnimator(4000, new ValueAnimator.AnimatorUpdateListener() {
            @Override
            public void onAnimationUpdate(ValueAnimator anim) {
                collHdrBg.setStroke(dp(1), (int) anim.getAnimatedValue());
            }
        });
        collAnim.start();

        if (expanded) {
            collapseSub.setVisibility(View.VISIBLE);
            textView.setText("â–³ " + text + " â–³");
        }
        textView.setOnClickListener(new View.OnClickListener() {
            boolean isChecked = expanded;
            @Override
            public void onClick(View v) {
                boolean z = !isChecked;
                isChecked = z;
                if (z) {
                    collapseSub.setVisibility(View.VISIBLE);
                    textView.setText("â–³ " + text + " â–³");
                } else {
                    collapseSub.setVisibility(View.GONE);
                    textView.setText("â–½ " + text + " â–½");
                }
            }
        });
        collapse.addView(textView);
        collapse.addView(collapseSub);
        linLayout.addView(collapse);
    }

    // â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
    // Category
    // â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
    private void Category(LinearLayout linLayout, String text) {
        final LinearLayout ll = new LinearLayout(getContext);
        ll.setOrientation(LinearLayout.HORIZONTAL);
        ll.setGravity(Gravity.CENTER_VERTICAL);
        ll.setPadding(0, dp(2), 0, dp(2));

        // Gradient bg: dark purple → deep blue, animated
        final GradientDrawable catBg = new GradientDrawable(
            GradientDrawable.Orientation.LEFT_RIGHT,
            new int[]{Color.parseColor("#2A003F"), Color.parseColor("#00000A"), Color.parseColor("#001533")});
        catBg.setCornerRadius(dp(6));
        ll.setBackground(catBg);

        final ValueAnimator catAnim = ValueAnimator.ofArgb(
            Color.parseColor("#2A003F"),
            Color.parseColor("#001A50"),
            Color.parseColor("#1A0030"),
            Color.parseColor("#2A003F")
        );
        catAnim.setDuration(3000);
        catAnim.setRepeatCount(ValueAnimator.INFINITE);
        catAnim.addUpdateListener(new ValueAnimator.AnimatorUpdateListener() {
            @Override
            public void onAnimationUpdate(ValueAnimator anim) {
                catBg.setColors(new int[]{(int) anim.getAnimatedValue(), Color.parseColor("#00000A"), Color.parseColor("#001533")});
            }
        });
        catAnim.start();

        // Glowing left bar (3dp wide, full height, RGB animated)
        final View accent = new View(getContext);
        LinearLayout.LayoutParams ap = new LinearLayout.LayoutParams(dp(3), dp(36));
        ap.setMargins(dp(6), dp(4), dp(8), dp(4));
        accent.setLayoutParams(ap);
        final GradientDrawable accentBg = new GradientDrawable();
        accentBg.setCornerRadius(dp(2));
        accentBg.setColor(Color.parseColor("#FF0055"));
        accent.setBackground(accentBg);
        final ValueAnimator accentAnim = buildRgbAnimator(1800, new ValueAnimator.AnimatorUpdateListener() {
            @Override
            public void onAnimationUpdate(ValueAnimator anim) {
                accentBg.setColor((int) anim.getAnimatedValue());
            }
        });
        accentAnim.start();

        // Pulse scale animation on the accent bar
        ValueAnimator pulse = ValueAnimator.ofFloat(0.8f, 1.0f);
        pulse.setDuration(900);
        pulse.setRepeatCount(ValueAnimator.INFINITE);
        pulse.setRepeatMode(ValueAnimator.REVERSE);
        pulse.addUpdateListener(new ValueAnimator.AnimatorUpdateListener() {
            @Override
            public void onAnimationUpdate(ValueAnimator anim) {
                accent.setScaleY((float) anim.getAnimatedValue());
            }
        });
        pulse.start();

        TextView tv = new TextView(getContext);
        LinearLayout.LayoutParams tvp = new LinearLayout.LayoutParams(0, WRAP_CONTENT, 1f);
        tv.setLayoutParams(tvp);
        tv.setText(Html.fromHtml("▶  " + text));
        tv.setGravity(Gravity.CENTER_VERTICAL);
        tv.setTextColor(Color.WHITE);
        tv.setTypeface(null, Typeface.BOLD);
        tv.setPadding(0, dp(8), dp(8), dp(8));
        tv.setTextSize(12f);
        tv.setShadowLayer(12f, 0f, 0f, Color.parseColor("#AA00CCFF"));

        // Animate the category text glow color
        buildRgbAnimator(2500, new ValueAnimator.AnimatorUpdateListener() {
            @Override
            public void onAnimationUpdate(ValueAnimator anim) {
                int col = (int) anim.getAnimatedValue();
                tv.setShadowLayer(14f, 0f, 0f, col);
            }
        }).start();

        ll.addView(accent);
        ll.addView(tv);
        // Top separator line
        View sep = new View(getContext);
        LinearLayout.LayoutParams sepLp = new LinearLayout.LayoutParams(MATCH_PARENT, dp(1));
        sep.setLayoutParams(sepLp);
        sep.setBackgroundColor(Color.parseColor("#33FFFFFF"));
        linLayout.addView(sep);
        linLayout.addView(ll);
    }

    // â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
    // TextView / WebTextView
    // â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
    private void TextView(LinearLayout linLayout, String text) {
        TextView textView = new TextView(getContext);
        textView.setText(Html.fromHtml(text));
        textView.setTextColor(TEXT_COLOR_2);
        textView.setPadding(dp(10), dp(5), dp(10), dp(5));
        linLayout.addView(textView);
    }

    private void WebTextView(LinearLayout linLayout, String text) {
        WebView wView = new WebView(getContext);
        wView.loadData(text, "text/html", "utf-8");
        wView.setBackgroundColor(0x00000000);
        wView.setPadding(0, dp(5), 0, dp(5));
        wView.getSettings().setCacheMode(WebSettings.LOAD_NO_CACHE);
        linLayout.addView(wView);
    }

    // â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
    // Utilities
    // â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
    private boolean isViewCollapsed() {
        return rootFrame == null || mCollapsed.getVisibility() == View.VISIBLE;
    }

    private int convertDipToPixels(int i) {
        return (int) ((((float) i) * getContext.getResources().getDisplayMetrics().density) + 0.5f);
    }

    private int dp(int i) {
        return (int) TypedValue.applyDimension(TypedValue.COMPLEX_UNIT_DIP, (float) i, getContext.getResources().getDisplayMetrics());
    }

    public void setVisibility(int view) {
        if (rootFrame != null) rootFrame.setVisibility(view);
    }

    public void onDestroy() {
        cancelAnimators();
        if (rootFrame != null) mWindowManager.removeView(rootFrame);
    }

    // ═════════════════════════════════════════════════════════════════════
    // PARTY MODE (easter egg) — clic sur "GRAVITY" en haut du menu
    //   • Démarre/arrête le MP3 (toggle 201 réutilisé)
    //   • Active low-gravity côté natif (toggle 201 → patch_LowGravity)
    //   • RGB chaos sur TOUTES les vues du menu (background + textes)
    //   • Secousses/translations aléatoires + rotation du conteneur
    //   • Bouge le menu sur tout l'écran (drift sinusoidal large)
    // Re-cliquer le titre = stop, retour à l'état normal.
    // ═════════════════════════════════════════════════════════════════════
    private void togglePartyMode() {
        mPartyOn = !mPartyOn;
        if (mPartyOn) startParty();
        else          stopParty();
    }

    private void startParty() {
        Log.i(TAG, "🎉 PARTY MODE ON");
        // 1) Musique + low-gravity (passe par le toggle 201 invisible)
        try { 
            Preferences.changeFeatureBool("Dance Party", 201, true); 
            Preferences.startDanceMusic();
        } catch (Throwable ignored) {}

        // 2) Passer le menu en PLEIN ÉCRAN (sauve les params actuels pour restauration)
        if (vmParams != null && mWindowManager != null && rootFrame != null) {
            mPartyOrigW = vmParams.width;
            mPartyOrigH = vmParams.height;
            mPartyOrigX = vmParams.x;
            mPartyOrigY = vmParams.y;
            mPartyOrigSaved = true;
            vmParams.width  = MATCH_PARENT;
            vmParams.height = MATCH_PARENT;
            vmParams.x = 0;
            vmParams.y = 0;
            try { mWindowManager.updateViewLayout(rootFrame, vmParams); } catch (Throwable t) {
                Log.e(TAG, "party fullscreen update failed: " + t.getMessage());
            }
        }

        // 3) Collecte toutes les vues + SAUVEGARDE leurs couleurs originales
        mPartyViews.clear();
        mPartyOrigBg.clear();
        mPartyOrigTextColor.clear();
        if (mExpanded != null) collectPartyViews(mExpanded);
        for (View v : mPartyViews) {
            mPartyOrigBg.add(v.getBackground());
            mPartyOrigTextColor.add(v instanceof TextView ? ((TextView) v).getCurrentTextColor() : 0);
        }

        // 4) RGB cycle sur backgrounds + textColor
        if (mPartyRgbAnim != null) mPartyRgbAnim.cancel();
        mPartyRgbAnim = ValueAnimator.ofInt(0, 360);
        mPartyRgbAnim.setDuration(1600);
        mPartyRgbAnim.setRepeatCount(ValueAnimator.INFINITE);
        mPartyRgbAnim.addUpdateListener(new ValueAnimator.AnimatorUpdateListener() {
            @Override
            public void onAnimationUpdate(ValueAnimator a) {
                int hueBase = (int) a.getAnimatedValue();
                int n = mPartyViews.size();
                for (int i = 0; i < n; i++) {
                    View v = mPartyViews.get(i);
                    float[] hsv = { (hueBase + i * 23) % 360, 1f, 1f };
                    int col = Color.HSVToColor(hsv);
                    try { v.setBackgroundColor(0x66000000 | (col & 0xFFFFFF)); } catch (Throwable ignored) {}
                    if (v instanceof TextView) {
                        TextView tv = (TextView) v;
                        tv.setTextColor(col);
                        tv.setShadowLayer(12f, 0f, 0f, col);
                    }
                }
            }
        });
        mPartyRgbAnim.start();

        // 5) Shake/drift CHAOS sur mExpanded — utilise TOUT l'écran (le menu vole partout)
        if (mPartyShakeAnim != null) mPartyShakeAnim.cancel();
        mPartyShakeAnim = ValueAnimator.ofFloat(0f, (float)(Math.PI * 2));
        mPartyShakeAnim.setDuration(3500);
        mPartyShakeAnim.setRepeatCount(ValueAnimator.INFINITE);
        final android.util.DisplayMetrics dm = getContext.getResources().getDisplayMetrics();
        // amplitudes énormes : le menu balaye quasi tout l'écran
        final float ampX = dm.widthPixels  * 0.42f;
        final float ampY = dm.heightPixels * 0.38f;
        mPartyShakeAnim.addUpdateListener(new ValueAnimator.AnimatorUpdateListener() {
            @Override
            public void onAnimationUpdate(ValueAnimator a) {
                if (mExpanded == null) return;
                float t = (float) a.getAnimatedValue();
                // Lissajous chaotique multi-fréquences
                float dx = (float)(Math.sin(t * 1.7f) * ampX + Math.cos(t * 5.3f) * 60 + Math.sin(t * 13f) * 22);
                float dy = (float)(Math.cos(t * 1.3f) * ampY + Math.sin(t * 4.1f) * 50 + Math.cos(t * 11f) * 18);
                float rot = (float)(Math.sin(t * 1.2f) * 25.0f + Math.cos(t * 3.7f) * 8f);
                float s = 0.85f + 0.30f * (float) Math.sin(t * 4.5f);
                mExpanded.setTranslationX(dx);
                mExpanded.setTranslationY(dy);
                mExpanded.setRotation(rot);
                mExpanded.setScaleX(s);
                mExpanded.setScaleY(s);
            }
        });
        mPartyShakeAnim.start();

        // 6) NEON BORDER GLOW : bordure pulsante RGB tout autour de l'écran
        showPartyNeonBorder();

        // 7) STROBE FLASH : flashs colorés plein écran toutes les ~250ms
        startPartyStrobe();

        // 8) Zone tactile DISCRÈTE en haut-à-droite pour stopper (invisible)
        showPartyStopButton();
    }

    private void stopParty() {
        Log.i(TAG, "🎉 PARTY MODE OFF");
        try { Preferences.changeFeatureBool("Dance Party", 201, false); } catch (Throwable ignored) {}
        if (mPartyRgbAnim != null) { mPartyRgbAnim.cancel(); mPartyRgbAnim = null; }
        if (mPartyShakeAnim != null) { mPartyShakeAnim.cancel(); mPartyShakeAnim = null; }
        // Reset position/rotation/scale
        if (mExpanded != null) {
            mExpanded.setTranslationX(0f);
            mExpanded.setTranslationY(0f);
            mExpanded.setRotation(0f);
            mExpanded.setScaleX(1f);
            mExpanded.setScaleY(1f);
        }
        // RESTAURE les backgrounds + couleurs de texte originaux
        int n = mPartyViews.size();
        for (int i = 0; i < n; i++) {
            View v = mPartyViews.get(i);
            try {
                v.setBackground(i < mPartyOrigBg.size() ? mPartyOrigBg.get(i) : null);
            } catch (Throwable ignored) {}
            if (v instanceof TextView && i < mPartyOrigTextColor.size()) {
                TextView tv = (TextView) v;
                int origCol = mPartyOrigTextColor.get(i);
                tv.setTextColor(origCol);
                tv.setShadowLayer(0f, 0f, 0f, 0); // retire le halo party
            }
        }
        mPartyViews.clear();
        mPartyOrigBg.clear();
        mPartyOrigTextColor.clear();

        // Restaure les WindowManager.LayoutParams initiales (taille du menu d'avant)
        if (mPartyOrigSaved && vmParams != null && mWindowManager != null && rootFrame != null) {
            vmParams.width  = mPartyOrigW;
            vmParams.height = mPartyOrigH;
            vmParams.x = mPartyOrigX;
            vmParams.y = mPartyOrigY;
            try { mWindowManager.updateViewLayout(rootFrame, vmParams); } catch (Throwable t) {
                Log.e(TAG, "party restore update failed: " + t.getMessage());
            }
            mPartyOrigSaved = false;
        }

        // Retire les overlays party
        hidePartyStopButton();
        hidePartyNeonBorder();
        stopPartyStrobe();
    }

    // ── Zone tactile DISCRÈTE en haut-à-droite (invisible) ──────────────
    // Carré transparent de ~80dp dans le coin → tap = stop party
    private void showPartyStopButton() {
        if (mPartyStopBtn != null) return;
        mPartyStopBtn = new TextView(getContext);
        // Aucun texte, aucun fond → totalement invisible
        mPartyStopBtn.setBackgroundColor(Color.TRANSPARENT);
        mPartyStopBtn.setOnClickListener(new View.OnClickListener() {
            @Override public void onClick(View v) {
                mPartyOn = false;
                stopParty();
            }
        });

        int iparams = Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.O
                ? WindowManager.LayoutParams.TYPE_APPLICATION_OVERLAY
                : WindowManager.LayoutParams.TYPE_PHONE;
        mPartyStopLP = new WindowManager.LayoutParams(
                dp(80), dp(80), iparams,
                WindowManager.LayoutParams.FLAG_NOT_FOCUSABLE
                        | WindowManager.LayoutParams.FLAG_LAYOUT_IN_SCREEN,
                PixelFormat.TRANSLUCENT);
        mPartyStopLP.gravity = Gravity.TOP | Gravity.END;
        mPartyStopLP.x = 0;
        mPartyStopLP.y = 0;
        try { mWindowManager.addView(mPartyStopBtn, mPartyStopLP); }
        catch (Throwable t) { Log.e(TAG, "addStopBtn failed: " + t.getMessage()); }
    }

    private void hidePartyStopButton() {
        if (mPartyStopBtn != null) {
            try { mWindowManager.removeView(mPartyStopBtn); } catch (Throwable ignored) {}
            mPartyStopBtn = null;
            mPartyStopLP = null;
        }
    }

    // ── Bordure néon RGB pulsante autour de tout l'écran ───────────────
    private View mPartyNeonBorder;
    private WindowManager.LayoutParams mPartyNeonLP;
    private ValueAnimator mPartyNeonAnim;
    private void showPartyNeonBorder() {
        if (mPartyNeonBorder != null) return;
        final android.graphics.drawable.GradientDrawable border = new android.graphics.drawable.GradientDrawable();
        border.setShape(android.graphics.drawable.GradientDrawable.RECTANGLE);
        border.setColor(Color.TRANSPARENT);
        border.setStroke(dp(8), Color.parseColor("#FF00E5FF"));

        View v = new View(getContext);
        v.setBackground(border);
        mPartyNeonBorder = v;

        int iparams = Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.O
                ? WindowManager.LayoutParams.TYPE_APPLICATION_OVERLAY
                : WindowManager.LayoutParams.TYPE_PHONE;
        mPartyNeonLP = new WindowManager.LayoutParams(
                MATCH_PARENT, MATCH_PARENT, iparams,
                WindowManager.LayoutParams.FLAG_NOT_FOCUSABLE
                        | WindowManager.LayoutParams.FLAG_NOT_TOUCHABLE
                        | WindowManager.LayoutParams.FLAG_LAYOUT_IN_SCREEN
                        | WindowManager.LayoutParams.FLAG_LAYOUT_NO_LIMITS,
                PixelFormat.TRANSLUCENT);
        try { mWindowManager.addView(mPartyNeonBorder, mPartyNeonLP); }
        catch (Throwable t) { Log.e(TAG, "addNeonBorder failed: " + t.getMessage()); }

        if (mPartyNeonAnim != null) mPartyNeonAnim.cancel();
        mPartyNeonAnim = ValueAnimator.ofInt(0, 360);
        mPartyNeonAnim.setDuration(1200);
        mPartyNeonAnim.setRepeatCount(ValueAnimator.INFINITE);
        mPartyNeonAnim.addUpdateListener(new ValueAnimator.AnimatorUpdateListener() {
            @Override
            public void onAnimationUpdate(ValueAnimator a) {
                int hue = (int) a.getAnimatedValue();
                float[] hsv = { hue, 1f, 1f };
                int col = Color.HSVToColor(hsv);
                int w = (int)(dp(6) + Math.abs(Math.sin(hue * Math.PI / 180.0)) * dp(10));
                border.setStroke(w, col);
            }
        });
        mPartyNeonAnim.start();
    }
    private void hidePartyNeonBorder() {
        if (mPartyNeonAnim != null) { mPartyNeonAnim.cancel(); mPartyNeonAnim = null; }
        if (mPartyNeonBorder != null) {
            try { mWindowManager.removeView(mPartyNeonBorder); } catch (Throwable ignored) {}
            mPartyNeonBorder = null;
            mPartyNeonLP = null;
        }
    }

    // ── Strobe : flashs colorés rapides plein écran ────────────────────
    private View mPartyStrobe;
    private WindowManager.LayoutParams mPartyStrobeLP;
    private Handler mPartyStrobeH;
    private Runnable mPartyStrobeR;
    private void startPartyStrobe() {
        if (mPartyStrobe != null) return;
        mPartyStrobe = new View(getContext);
        mPartyStrobe.setBackgroundColor(0x00000000);

        int iparams = Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.O
                ? WindowManager.LayoutParams.TYPE_APPLICATION_OVERLAY
                : WindowManager.LayoutParams.TYPE_PHONE;
        mPartyStrobeLP = new WindowManager.LayoutParams(
                MATCH_PARENT, MATCH_PARENT, iparams,
                WindowManager.LayoutParams.FLAG_NOT_FOCUSABLE
                        | WindowManager.LayoutParams.FLAG_NOT_TOUCHABLE
                        | WindowManager.LayoutParams.FLAG_LAYOUT_IN_SCREEN
                        | WindowManager.LayoutParams.FLAG_LAYOUT_NO_LIMITS,
                PixelFormat.TRANSLUCENT);
        try { mWindowManager.addView(mPartyStrobe, mPartyStrobeLP); }
        catch (Throwable t) { Log.e(TAG, "addStrobe failed: " + t.getMessage()); }

        mPartyStrobeH = new Handler();
        mPartyStrobeR = new Runnable() {
            int i = 0;
            final int[] cols = {
                0x33FF00FF, 0x3300FFFF, 0x33FFFF00, 0x3300FF66,
                0x33FF3366, 0x33FFFFFF, 0x33000000, 0x336600FF
            };
            @Override public void run() {
                if (mPartyStrobe == null) return;
                mPartyStrobe.setBackgroundColor(cols[i++ % cols.length]);
                mPartyStrobeH.postDelayed(this, 110);
            }
        };
        mPartyStrobeH.post(mPartyStrobeR);
    }
    private void stopPartyStrobe() {
        if (mPartyStrobeH != null && mPartyStrobeR != null) {
            mPartyStrobeH.removeCallbacks(mPartyStrobeR);
        }
        mPartyStrobeH = null;
        mPartyStrobeR = null;
        if (mPartyStrobe != null) {
            try { mWindowManager.removeView(mPartyStrobe); } catch (Throwable ignored) {}
            mPartyStrobe = null;
            mPartyStrobeLP = null;
        }
    }

    // Récursif : récupère tous les enfants à colorier pour le party mode.
    // On limite la profondeur pour éviter d'animer 500 sous-vues.
    private void collectPartyViews(View root) {
        if (root == null || mPartyViews.size() > 80) return;
        mPartyViews.add(root);
        if (root instanceof ViewGroup) {
            ViewGroup vg = (ViewGroup) root;
            for (int i = 0; i < vg.getChildCount(); i++) {
                collectPartyViews(vg.getChildAt(i));
            }
        }
    }
}

