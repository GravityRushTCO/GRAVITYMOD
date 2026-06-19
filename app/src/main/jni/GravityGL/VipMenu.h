#pragma once
#include "GravityGL.h"

// Shooting star particle
struct Star {
    float x, y, vx, vy, life, maxLife, size;
    float r, g, b;
};

// Floating debris particle for galaxy BG
struct Debris {
    float x, y, size, speed, angle, alpha;
    float r, g, b;
};

// Orbital ring particle (bubble)
struct OrbParticle {
    float angle, radius, speed;
    float r, g, b, a;
    float size;
};

// Splash/explosion particle for menu open
struct SplashParticle {
    float x, y, vx, vy;
    float life, maxLife;
    float size;
    float r, g, b;
};

// Ring segment for planet visual
struct RingSegment {
    float angle, width;
    float r, g, b, a;
};

class VipMenu {
public:
    static VipMenu& get() { static VipMenu instance; return instance; }

    void init();
    void render();
    void drawEspOverlay();
    bool onTouch(int action, float x, float y);

private:
    VipMenu() = default;

    bool m_open = false;

    // Slide-out panel: anchored to left edge, full screen height
    float m_w = 420.f;   // Width of the slide-out panel
    float m_h = 0.f;     // Set dynamically to screen height
    float m_x = 0.f;     // X position (animated: -m_w when closed, 0 when open)
    float m_y = 0.f;     // Always 0 (full height)

    // Notch (left-edge tab to open)
    float m_notchX = 0.f; // Dynamic: when closed = 0, when open = m_w
    static const float NOTCH_W;
    static const float NOTCH_H;

    // Dragging (scroll inside menu)
    bool m_dragging = false;
    float m_dragDx = 0.f, m_dragDy = 0.f;
    float m_dragStartX = 0.f, m_dragStartY = 0.f;
    bool  m_dragConfirmed = false;

    int m_tab = 0;
    int m_prevTab = -1;
    float m_tabAnim = 0.f;

    bool m_touchDown = false;
    float m_tx = 0, m_ty = 0;
    bool m_click = false;
    bool m_draggingSlider = false;

    // Per-tab scroll offsets
    float m_scrollY[6] = {0,0,0,0,0,0};
    bool m_isScrolling = false;
    float m_lastTy = 0;

    // Animation state
    float m_anim = 0.f;       // Global time counter
    float m_openAnim = 0.f;   // 0..1 open transition (garage door slide)
    float m_openPhase = 0.f;

    // Galaxy background stars
    Star m_bgStars[40];
    bool m_bgStarsInit = false;

    // Debris particles
    Debris m_debris[20];
    bool m_debrisInit = false;

    // Bubble orbital particles (kept for compat)
    OrbParticle m_orbs[8];
    bool m_orbsInit = false;

    // Planet angle (not used for bubble anymore but kept for compat)
    float m_planetAngle = 0.f;
    float m_pulseAngle  = 0.f;
    float m_nebulaAngle = 0.f;
    float m_moonAngle   = 0.f;
    float m_ringSpinAngle = 0.f;

    // Splash particles on open
    SplashParticle m_splash[80];
    bool m_splashFired = false;
    float m_splashTime = 0.f;

    float m_shockwaveR = 0.f;
    float m_shockwaveA = 0.f;

    // Embedded textures
    GLuint m_texPlanet  = 0;
    GLuint m_texSpaceBg = 0;
    GLuint m_texPanel   = 0;
    bool   m_texLoaded  = false;

    // VipUnlocked state
    bool m_vipUnlocked = false;
    char m_vipPin[16]  = "";

    void drawNotch();         // The left-edge pull tab
    void drawPanel();         // Sliding panel background
    void drawPanelHeader();   // Title + close button at top
    void drawPanelTabs();     // Horizontal tab bar under header
    void drawContent();       // Scrollable content

    void drawBackground();
    void drawNebula();
    void drawBgStars();
    void drawHeader();
    void drawTabs();
    void drawOpenAnimation();
    void initBgStars();
    void initDebris();
    void initOrbs();
    void updateBgStars();
    void updateDebris();
    void updateOrbs();
    void drawDebris();
    void drawOrbs();
    void drawBubble(); // legacy – no longer called

    void doSectionHeader(const char* label, float& y);
    bool doToggle (const char* label, float& y, bool& val);
    bool doButton (const char* label, float& y);
    bool doSlider (const char* label, float& y, float& val, float min, float max, const char* format = "%.1f");
    bool doSpinner(const char* label, float& y, int& val, const char** items, int count);

    // Helpers
    static float lerp(float a, float b, float t) { return a + (b - a) * t; }
    static float clamp01(float v) { return v < 0.f ? 0.f : v > 1.f ? 1.f : v; }
    static float easeOutBack(float t) {
        const float c1 = 1.70158f, c3 = c1 + 1.f;
        return 1.f + c3 * (t - 1.f)*(t - 1.f)*(t - 1.f) + c1 * (t - 1.f)*(t - 1.f);
    }
    static float easeOutElastic(float t) {
        if (t <= 0.f) return 0.f;
        if (t >= 1.f) return 1.f;
        const float c4 = (2.f * 3.14159f) / 3.f;
        return powf(2.f, -10.f * t) * sinf((t * 10.f - 0.75f) * c4) + 1.f;
    }
    static float easeInOutCubic(float t) {
        return t < 0.5f ? 4.f*t*t*t : 1.f - powf(-2.f*t+2.f, 3.f)/2.f;
    }
};
