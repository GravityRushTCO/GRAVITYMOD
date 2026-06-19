#include "VipMenu.h"
#include "../Aimbot.h"
#include "../Esp.h"
#include "EmbeddedTextures.h"
#include "PlanetData.h"
#include "VipHacks.h"
#include <jni.h>
#include <math.h>
#include "../imgui/imgui.h"

// Variables defined in Main.cpp
extern bool g_VipSpeedRun;
extern bool g_VipBigJump;
extern bool g_VipWallHack;
extern bool g_VipNoRecoil;
extern bool g_VipSuperRecoil;
extern bool g_VipStaminaInfinie;
extern bool g_VipMoveToVehicle;
extern bool g_VipSpeedOfMovement;
extern bool g_DragCheckpointToPlayer;
extern float g_DragCheckpointDelay;
extern float g_HeistFarmDelay;

extern float g_VipVehicleSpeed;
extern float g_VipVehicleAngle;
extern bool g_VipVehicleMaxBrake;
extern float g_VipVehicleForwardForce;
extern bool g_VipVehicleNoDamage;
extern bool g_VipVehicleInfFuel;
extern float g_VipVehicleSlipping;
extern int g_VipVehicleWheelSize;

bool g_VipUnlocked = false;
char g_VipPinCode[16] = "";
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Extern states
extern bool g_GodModeEnabled;
extern bool g_GreenZoneBypass;
extern bool g_NoClipEnabled;
extern bool g_TpCarteToggle;
extern bool g_NoReloadEnabled;
extern "C" bool g_DeviceIdFaker;
extern bool g_IgnoreAllCollisions;
extern bool g_ForceDriverSeat;
extern bool g_AutoCheckpointEnabled;
extern bool g_AutoCheckpointWithDelay;
extern bool g_AimVisibleOnly;
extern bool g_VehicleWallhack;
extern bool g_SuperSpeedEnabled;
extern bool g_FlyEnabled;
extern "C" void Esp_ForceRapatrierMarqueurs();
extern bool g_FarmMoneyEnabled;
extern bool g_TpEnemy10sEnabled;
extern bool g_MeuDestinoEnabled;
extern float g_SpeedMultiplier;
extern float g_CameraFov;
extern bool g_CameraFovEnabled;
extern float g_AutoFollowDistance;
extern float g_AutoFollowHeight;
extern int g_BonePriority;
extern bool g_VisibilityCheck;
extern float g_AimLockSmoothness;

extern "C" {
extern "C" void Changes(JNIEnv *env, jclass clazz, jobject obj, jint id,
                        jstring str, jint intVal, jlong longVal, jboolean on,
                        jstring str2);
}

void TriggerChange(int id, bool on = false, int intVal = 0) {
  Changes(nullptr, nullptr, nullptr, id, nullptr, intVal, 0, on, nullptr);
}

struct TeleportPoint {
  const char *name;
  float x, y, z;
};

#include "ParsedCoordinates.h"
#include "../json.hpp"

extern nlohmann::json g_DynamicConfig;
extern bool g_ConfigLoaded;

static bool IsFeatureVisible(int id) {
  if (!g_ConfigLoaded) return true;
  try {
    if (g_DynamicConfig.contains("tabs") && g_DynamicConfig["tabs"].is_array()) {
      for (const auto& tab : g_DynamicConfig["tabs"]) {
        if (tab.contains("items") && tab["items"].is_array()) {
          for (const auto& item : tab["items"]) {
            if (item.contains("id") && item["id"].get<int>() == id) {
              return true;
            }
          }
        }
      }
      return false;
    }
  } catch (...) {}
  return true;
}

// Static constants
const float VipMenu::NOTCH_W = 38.f;
const float VipMenu::NOTCH_H = 120.f;

void VipMenu::init() {
  // Initializer
}

static float g_notchYOffset = 0.f;
static bool g_isDraggingNotch = false;
static float g_dragStartY = 0.f;
static float g_dragStartOffset = 0.f;

bool VipMenu::onTouch(int action, float x, float y) {
  m_tx = x;
  m_ty = y;

  float sw = (float)g_GL.W();
  float sh = (float)g_GL.H();

  // Compute the panel X position from animation (slides in from left)
  float panelX = sw - m_w * easeInOutCubic(m_openAnim);
  // Notch is always visible at the right edge of the panel
  float notchX = panelX - NOTCH_W;
  float notchY = (sh - NOTCH_H) * 0.25f + g_notchYOffset;

  if (!m_open) {
    // Tap on notch opens the menu
    if (action == 0) {
      if (x >= notchX && x <= notchX + NOTCH_W && y >= notchY && y <= notchY + NOTCH_H) {
        m_open = true;
        m_openPhase = 0.f;
        return true;
      }
    }
    return false;
  }

  // Menu is open
  // Tap on notch closes the menu
  if (action == 0 && x >= notchX && x <= notchX + NOTCH_W && y >= notchY && y <= notchY + NOTCH_H) {
    m_open = false;
    return true;
  }
  // Tap outside panel closes the menu
  if (action == 0 && x < panelX - NOTCH_W) {
    m_open = false;
    return true;
  }

  if (action == 0) { // DOWN
    m_touchDown = true;
    m_click = false; // Only click on UP
    m_isScrolling = false;
    m_draggingSlider = false;
    m_lastTy = y;
  } else if (action == 1) { // UP
    m_touchDown = false;
    if (!m_isScrolling && !m_draggingSlider) m_click = true;
    else m_click = false;
    m_isScrolling = false;
    m_draggingSlider = false;
  } else if (action == 2) { // MOVE
    if (m_touchDown) {
      float dy = y - m_lastTy;
      if (fabsf(dy) > 5.f) m_isScrolling = true;
      // Content area starts after header (120px) and tabs (50px)
      float contentTop = m_y + 112.f;
      if (m_isScrolling && m_tx >= m_x && m_tx <= m_x + m_w &&
          m_ty > contentTop && m_ty < sh) {
        m_scrollY[m_tab] += dy;
        if (m_scrollY[m_tab] > 0) m_scrollY[m_tab] = 0;
      }
    }
    m_lastTy = y;
  }

  if (m_isScrolling) return true;
  if (x >= panelX && x <= panelX + m_w) return true;
  return false;
}

void VipMenu::initBgStars() {
  for (int i = 0; i < 60; i++) {
    m_bgStars[i].x = m_x + (rand() % (int)m_w);
    m_bgStars[i].y = m_y + (rand() % (int)m_h);
    m_bgStars[i].vx = ((rand() % 100) / 100.f - 0.5f) * 0.5f;
    m_bgStars[i].vy = ((rand() % 100) / 100.f + 0.1f) * 1.5f;
    m_bgStars[i].size = (rand() % 100) / 100.f * 2.f + 1.f;
    m_bgStars[i].life =
        (rand() % 100) / 100.f * M_PI * 2.f; // Used for twinkle phase

    int ct = rand() % 3;
    if (ct == 0) {
      m_bgStars[i].r = 0.4f;
      m_bgStars[i].g = 0.8f;
      m_bgStars[i].b = 1.f;
    } else if (ct == 1) {
      m_bgStars[i].r = 0.8f;
      m_bgStars[i].g = 0.4f;
      m_bgStars[i].b = 1.f;
    } else {
      m_bgStars[i].r = 1.f;
      m_bgStars[i].g = 1.f;
      m_bgStars[i].b = 1.f;
    }
  }
  m_bgStarsInit = true;
}

void VipMenu::updateBgStars() {
  for (int i = 0; i < 60; i++) {
    m_bgStars[i].x += m_bgStars[i].vx;
    m_bgStars[i].y += m_bgStars[i].vy;
    m_bgStars[i].life += 0.05f;

    if (m_bgStars[i].y > m_y + m_h || m_bgStars[i].x < m_x ||
        m_bgStars[i].x > m_x + m_w) {
      m_bgStars[i].x = m_x + (rand() % (int)m_w);
      m_bgStars[i].y = m_y;
    }
  }
}

void VipMenu::drawBgStars() {
  for (int i = 0; i < 60; i++) {
    float twinkle = (sinf(m_bgStars[i].life) + 1.f) * 0.5f;
    GColor col = {m_bgStars[i].r, m_bgStars[i].g, m_bgStars[i].b,
                  0.2f + 0.8f * twinkle};
    g_GL.circleFill(m_bgStars[i].x, m_bgStars[i].y, m_bgStars[i].size,
                    col * m_openAnim);
  }
}

void VipMenu::initOrbs() {
  for (int i = 0; i < 8; i++) {
    m_orbs[i].angle = (M_PI * 2.f / 8.f) * i;
    m_orbs[i].radius = 45.f + (rand() % 15);
    m_orbs[i].speed =
        ((rand() % 100) / 100.f * 0.04f + 0.02f) * (i % 2 == 0 ? 1 : -1);
    m_orbs[i].size = 2.f + (rand() % 3);
    m_orbs[i].r = (rand() % 100) / 100.f;
    m_orbs[i].g = (rand() % 100) / 100.f;
    m_orbs[i].b = 1.f;
  }
  m_orbsInit = true;
}

void VipMenu::drawBubble() {
  // Legacy - no longer called. Kept as stub.
}

void VipMenu::drawOpenAnimation()  {}
void VipMenu::drawBackground()     {}
void VipMenu::drawNebula()         {}
void VipMenu::drawHeader()         {}
void VipMenu::drawTabs()           {}
void VipMenu::initDebris()         {}
void VipMenu::updateDebris()       {}
void VipMenu::updateOrbs()         {}
void VipMenu::drawDebris()         {}
void VipMenu::drawOrbs()           {}


void VipMenu::render() {
  if (!m_texLoaded) {
    m_texLoaded = true;
    m_texPlanet = g_GL.loadRGBA(Planet_png, 0, Planet_png_len, GL_LINEAR);
    m_texSpaceBg = g_GL.loadRGBA(g_SpaceBgTex_DATA, g_SpaceBgTex_W, g_SpaceBgTex_H);
    m_texPanel = g_GL.loadRGBA(g_UIPanelTex_DATA, g_UIPanelTex_W, g_UIPanelTex_H);
  }

  float sw = (float)g_GL.W();
  float sh = (float)g_GL.H();

  // Landscape: panel is full screen height, 420px wide
  m_h = sh;
  m_y = 0.f;
  // Clamp width to 50% of screen at most
  m_w = sw * 0.4f;
  if (m_w < 350.f) m_w = 350.f;
  if (m_w > 520.f) m_w = 520.f;

  m_anim += 0.016f;

  // Animate open/close
  if (m_open) {
    if (m_openPhase < 1.f) {
      m_openPhase += 0.10f;
      if (m_openPhase > 1.f) m_openPhase = 1.f;
    }
  } else {
    if (m_openPhase > 0.f) {
      m_openPhase -= 0.10f;
      if (m_openPhase < 0.f) m_openPhase = 0.f;
    }
  }
  m_openAnim = easeInOutCubic(m_openPhase);

  // Sliding panel X: starts at -m_w (off screen left), slides to 0
  m_x = sw - m_w * m_openAnim;

  // Notch removed - no longer drawing the old left-edge toggle button

  if (m_openAnim < 0.01f) {
    m_click = false;
    return; // Fully closed, nothing else to draw
  }

  // Draw sliding panel
  drawPanel();
  drawPanelHeader();
  drawPanelTabs();

  // Scissored content area
  g_GL.flush();
  glEnable(GL_SCISSOR_TEST);
  float contentTop = m_y + 112.f; // header (96) + tabs (44)
  float glScissorY = sh - (contentTop + (sh - contentTop)); // bottom of screen
  glScissor((GLint)m_x, (GLint)0, (GLsizei)(m_w), (GLsizei)(int)(sh - contentTop));

  drawContent();

  g_GL.flush();
  glDisable(GL_SCISSOR_TEST);

  m_click = false;
}


static GColor lerpColor(GColor a, GColor b, float t) {
    return { a.r + (b.r - a.r) * t, a.g + (b.g - a.g) * t, a.b + (b.b - a.b) * t, a.a + (b.a - a.a) * t };
}

// Node particle system
struct NodeParticle { float x, y, vx, vy; };
#define NUM_NODES 150
static NodeParticle g_Nodes[NUM_NODES];
static bool g_NodesInit = false;


// ============================================================
// NOTCH - Left edge pull tab (always visible, even when panel is closed)
// ============================================================
void VipMenu::drawNotch() {
  float sh = (float)g_GL.H();
  float notchX = m_x - NOTCH_W;
  float notchY = (sh - NOTCH_H) * 0.25f + g_notchYOffset;
  float r = 8.f;

  // Shadow behind notch for depth
  g_GL.roundRect(notchX - 2.f, notchY + 2.f, NOTCH_W, NOTCH_H, r, GColor::hex(0x000000, 0.4f));

  // Animated gradient color
  float animSin = (sinf(m_anim * 2.f) + 1.f) * 0.5f;
  GColor cTop = lerpColor(GColor::hex(0x0B1D4A, 0.95f), GColor::hex(0x3B1054, 0.95f), animSin);
  g_GL.roundRect(notchX, notchY, NOTCH_W, NOTCH_H, r, cTop);

  // GRAVITY vertical text - centered
  const char* txt = "GRAVITY";
  float textSize = 0.85f;
  float totalTextH = 7 * 14.f;
  float ty = notchY + (NOTCH_H - totalTextH) / 2.f + 12.f; 
  for (int i = 0; txt[i] != '\0'; i++) {
      char buf[2] = {txt[i], 0};
      float tw = g_GL.textW(buf, textSize);
      g_GL.text(buf, notchX + (NOTCH_W - tw) / 2.f, ty, textSize, GColor::hex(0xFFFFFF, 0.9f), false);
      ty += 14.f;
  }
}

// ============================================================
// PANEL BACKGROUND - Full height frosted glass
// ============================================================
void VipMenu::drawPanel() {
  float sh = (float)g_GL.H();

  // Animated blue-purple gradient base
  float animSin = (sinf(m_anim * 1.5f) + 1.f) * 0.5f;
  GColor cTop = lerpColor(GColor::hex(0x090D2B, 0.98f * m_openAnim), GColor::hex(0x1B0B3B, 0.98f * m_openAnim), animSin);
  GColor cBot = lerpColor(GColor::hex(0x110524, 0.98f * m_openAnim), GColor::hex(0x041A3B, 0.98f * m_openAnim), animSin);
  g_GL.gradV(m_x, m_y, m_w, m_h, cTop, cBot);

  // Subtle left-edge separator
  g_GL.rect(m_x, m_y, 1.f, m_h, P::CYAN().alpha(0.25f * m_openAnim));

  // Subtle node particles inside panel (low density, very faint)
  if (!g_NodesInit) {
    for (int i = 0; i < NUM_NODES; i++) {
      g_Nodes[i].x = (float)(rand() % 1000) / 1000.f;
      g_Nodes[i].y = (float)(rand() % 1000) / 1000.f;
      g_Nodes[i].vx = ((float)(rand() % 1000) / 1000.f - 0.5f) * 0.0008f;
      g_Nodes[i].vy = ((float)(rand() % 1000) / 1000.f - 0.5f) * 0.0008f;
    }
    g_NodesInit = true;
  }
  for (int i = 0; i < NUM_NODES; i++) {
    // Touch repulsion
    float px_rep = m_x + g_Nodes[i].x * m_w;
    float py_rep = m_y + g_Nodes[i].y * m_h;
    if (m_touchDown) {
        float dx = px_rep - m_tx;
        float dy = py_rep - m_ty;
        float distSq = dx * dx + dy * dy;
        if (distSq < 40000.f && distSq > 0.1f) {
            float force = (40000.f - distSq) / 40000.f * 0.005f;
            g_Nodes[i].vx += (dx / sqrtf(distSq)) * force;
            g_Nodes[i].vy += (dy / sqrtf(distSq)) * force;
        }
    }
    
    // Friction to stabilize
    g_Nodes[i].vx = g_Nodes[i].vx * 0.95f + ((float)(i % 10) / 10.f - 0.5f) * 0.00005f;
    g_Nodes[i].vy = g_Nodes[i].vy * 0.95f + ((float)((i+5) % 10) / 10.f - 0.5f) * 0.00005f;

    g_Nodes[i].x += g_Nodes[i].vx;
    g_Nodes[i].y += g_Nodes[i].vy;
    if (g_Nodes[i].x < 0.f) g_Nodes[i].x = 1.f;
    if (g_Nodes[i].x > 1.f) g_Nodes[i].x = 0.f;
    if (g_Nodes[i].y < 0.f) g_Nodes[i].y = 1.f;
    if (g_Nodes[i].y > 1.f) g_Nodes[i].y = 0.f;

    float px = m_x + g_Nodes[i].x * m_w;
    float py = m_y + g_Nodes[i].y * m_h;
    for (int j = i + 1; j < NUM_NODES; j++) {
      float px2 = m_x + g_Nodes[j].x * m_w;
      float py2 = m_y + g_Nodes[j].y * m_h;
      float dist = sqrtf((px-px2)*(px-px2)+(py-py2)*(py-py2));
      if (dist < 90.f) {
        g_GL.line(px, py, px2, py2, 0.8f, P::CYAN().alpha((1.f - dist/90.f) * 0.08f * m_openAnim));
      }
    }
    g_GL.circleFill(px, py, 1.2f, P::CYAN().alpha(0.12f * m_openAnim));
  }
}

// ============================================================
// PANEL HEADER - iOS-style title bar
// ============================================================
void VipMenu::drawPanelHeader() {
  float safeTop = 32.f;
  float headerH = 44.f + safeTop; // 76.f
  float py = m_y;

  // Header darker strip
  g_GL.rect(m_x, py, m_w, headerH, GColor::hex(0x000000, 0.4f * m_openAnim));

  // Bottom separator line
  g_GL.rect(m_x, py + headerH - 1.f, m_w, 1.f, GColor::hex(0x2A3040, m_openAnim));

  // Title - "GRAVITY MOD" left-aligned like iOS, with floating animation
  float floatY = sinf(m_anim * 3.f) * 3.f;
  float tx = m_x + 20.f;
  g_GL.text("GRAVITY", tx, py + safeTop + 8.f + floatY, 1.4f, P::TEXT().alpha(m_openAnim), false);
  float titleW = g_GL.textW("GRAVITY", 1.4f);
  g_GL.text("MOD", tx + titleW + 6.f, py + safeTop + 8.f + floatY, 1.4f, P::CYAN().alpha(m_openAnim), false);

  // Subtitle

  // Close button (top right) - small X icon
  float cbSize = 36.f; // Bigger
  float cx = m_x + m_w - 75.f; // More to the left
  float cy2 = py + safeTop + 4.f;
  
  GColor rgbClose = {
       (sinf(m_anim * 2.0f) + 1.f) * 0.5f,
       (sinf(m_anim * 2.0f + 2.09f) + 1.f) * 0.5f,
       (sinf(m_anim * 2.0f + 4.18f) + 1.f) * 0.5f,
       m_openAnim
  };
  
  bool hoverClose = (m_touchDown && m_tx >= cx && m_tx <= cx + cbSize && m_ty >= cy2 && m_ty <= cy2 + cbSize);
  g_GL.roundRect(cx, cy2, cbSize, cbSize, 8.f, GColor::hex(hoverClose ? 0x3A3A3A : 0x1a1a2a, 0.9f * m_openAnim));
  g_GL.roundBorder(cx, cy2, cbSize, cbSize, 8.f, 2.f, rgbClose);
  
  float txW = g_GL.textW("X", 1.4f);
  g_GL.text("X", cx + (cbSize - txW) / 2.f, cy2 + 25.f, 1.4f, rgbClose, false);

  if (m_click && m_tx >= cx && m_tx <= cx + cbSize && m_ty >= cy2 && m_ty <= cy2 + cbSize) {
    m_open = false;
    m_click = false;
  }
}

// ============================================================
// PANEL TABS - Horizontal tab bar (iOS control strip style)
// ============================================================
void VipMenu::drawPanelTabs() {
  const char *tabs[] = {"\xf0\x9f\x9b\xa1 DEFENSE", "\xf0\x9f\x8e\xaf COMBAT", "\xe2\x9a\xa1 MOVE", "\xf0\x9f\x93\x8d TELEPORT", "\xf0\x9f\x91\x91 VIP", "\xe2\x9a\x99 PARAMS"};
  int tabCount = 6;
  float safeTop = 32.f;
  float tabBarY = m_y + 44.f + safeTop; // 76.f
  float tabBarH = 36.f;

  // Tab bar background
  g_GL.rect(m_x, tabBarY, m_w, tabBarH, GColor::hex(0x0A0E16, 0.6f * m_openAnim));
  g_GL.rect(m_x, tabBarY + tabBarH - 1.f, m_w, 1.f, GColor::hex(0x2A3040, m_openAnim));

  float tabW = m_w / tabCount;

  // Tab change animation
  if (m_prevTab != m_tab) {
    m_tabAnim += 0.12f;
    if (m_tabAnim >= 1.f) {
      m_tabAnim = 1.f;
      m_prevTab = m_tab;
    }
  } else {
    m_tabAnim = 1.f;
  }

  for (int i = 0; i < tabCount; i++) {
    float tx = m_x + i * tabW;
    float ty = tabBarY;
    bool isActive = (m_tab == i);

    // Tap to select
    if (m_click && m_tx >= tx && m_tx <= tx + tabW && m_ty >= ty && m_ty <= ty + tabBarH) {
      if (m_tab != i) {
        m_prevTab = m_tab;
        m_tab = i;
        m_tabAnim = 0.f;
        m_scrollY[i] = 0;
      }
      m_click = false;
    }

    // Active indicator underline
    if (isActive) {
      g_GL.rect(tx + 4.f, tabBarY + tabBarH - 3.f, tabW - 8.f, 3.f, P::CYAN().alpha(m_openAnim));
    }

    // Tab label
    float labelW = g_GL.textW(tabs[i], 0.85f);
    float labelX = tx + (tabW - labelW) / 2.f;
    GColor labelCol = isActive ? P::CYAN().alpha(m_openAnim) : P::DIM().alpha(0.7f * m_openAnim);
    
    if (i == 4) { // VIP tab
        labelCol = {
           (sinf(m_anim * 2.0f) + 1.f) * 0.5f,
           (sinf(m_anim * 2.0f + 2.09f) + 1.f) * 0.5f,
           (sinf(m_anim * 2.0f + 4.18f) + 1.f) * 0.5f,
           m_openAnim
        };
    }
    g_GL.text(tabs[i], labelX, ty + 10.f, 0.85f, labelCol, false);
  }
}

// ============================================================
// SECTION HEADER - "FEATURES" / "SETTINGS" style grey caps
// ============================================================
void VipMenu::doSectionHeader(const char *label, float &cy) {
  float contentTop = m_y + 112.f;
  if (cy < contentTop) { cy += 30.f; return; }

  float sx = m_x + 16.f;
  cy += 26.f; // Baseline shift so it doesn't overlap header
  g_GL.text(label, sx, cy, 0.9f, P::CYAN().alpha(0.8f * m_openAnim), false);
  cy += 16.f;
}

// ============================================================
// TOGGLE ROW - iOS-style pill toggle
// ============================================================
bool VipMenu::doToggle(const char *label, float &cy, bool &val) {
  float contentTop = m_y + 112.f;
  float rowH = 42.f;
  float sx = m_x + 16.f;
  float rowW = m_w - 32.f;

  if (cy + rowH < contentTop || cy > m_y + m_h) {
    cy += rowH;
    return false;
  }

  // Row background with subtle separator at bottom
  if (m_touchDown && m_tx >= sx && m_tx <= sx + rowW && m_ty >= cy && m_ty <= cy + rowH) {
    g_GL.rect(sx, cy, rowW, rowH, GColor::hex(0x1E2430, 0.6f * m_openAnim));
  }
  g_GL.rect(sx + 8.f, cy + rowH - 1.f, rowW - 16.f, 1.f, GColor::hex(0x2A3040, 0.5f * m_openAnim));

  // Label
  g_GL.text(label, sx + 8.f, cy + 28.f, 1.05f, P::TEXT().alpha(m_openAnim), false);

  // iOS-style toggle pill (40x24px)
  float pillW = 40.f, pillH = 22.f;
  float pillX = m_x + m_w - 16.f - pillW;
  float pillY = cy + (rowH - pillH) / 2.f;

  GColor pillBg = val ? GColor::hex(0x0A84FF, 0.9f * m_openAnim)   // iOS blue ON
                      : GColor::hex(0x3A3A3C, 0.8f * m_openAnim);  // Grey OFF
  g_GL.roundRect(pillX, pillY, pillW, pillH, pillH / 2.f, pillBg);

  // Thumb circle
  float thumbR = (pillH / 2.f) - 2.f;
  float thumbX = val ? (pillX + pillW - thumbR * 2.f - 2.f) : (pillX + 2.f);
  g_GL.circleFill(thumbX + thumbR, pillY + pillH / 2.f, thumbR, GColor::hex(0xFFFFFF, m_openAnim), 20);

  bool changed = false;
  if (m_click && !m_draggingSlider && m_tx >= sx && m_tx <= sx + rowW && m_ty >= cy && m_ty <= cy + rowH) {
    val = !val;
    changed = true;
    m_click = false;
  }

  cy += rowH;
  return changed;
}

// ============================================================
// BUTTON ROW - Full width iOS-style action button
// ============================================================
bool VipMenu::doButton(const char *label, float &cy) {
  float contentTop = m_y + 112.f;
  float rowH = 42.f;
  float sx = m_x + 16.f;
  float rowW = m_w - 32.f;

  if (cy + rowH < contentTop || cy > m_y + m_h) {
    cy += rowH;
    return false;
  }

  bool pressed = m_touchDown && m_tx >= sx && m_tx <= sx + rowW && m_ty >= cy && m_ty <= cy + rowH;
  GColor btnCol = pressed ? GColor::hex(0x0A84FF, 0.4f * m_openAnim)
                          : GColor::hex(0x1E2430, 0.6f * m_openAnim);
  g_GL.roundRect(sx, cy + 4.f, rowW, rowH - 8.f, 8.f, btnCol);

  float labelW = g_GL.textW(label, 1.05f);
  // Left aligned like the rest
  g_GL.text(label, sx + 8.f, cy + 27.f, 1.05f,
            P::CYAN().alpha(m_openAnim), false);

  bool clicked = false;
  if (m_click && !m_draggingSlider && m_tx >= sx && m_tx <= sx + rowW && m_ty >= cy && m_ty <= cy + rowH) {
    clicked = true;
    m_click = false;
  }

  cy += rowH;
  return clicked;
}

// ============================================================
// SLIDER ROW - Clean iOS-style slider
// ============================================================
bool VipMenu::doSlider(const char *label, float &cy, float &val, float min,
                       float max, const char *format) {
  float contentTop = m_y + 112.f;
  float rowH = 56.f;
  float sx = m_x + 16.f;
  float rowW = m_w - 32.f;

  if (cy + rowH < contentTop || cy > m_y + m_h) {
    cy += rowH;
    return false;
  }

  g_GL.rect(sx + 8.f, cy + rowH - 1.f, rowW - 16.f, 1.f, GColor::hex(0x2A3040, 0.5f * m_openAnim));

  // Label + value on same line
  char valBuf[32];
  snprintf(valBuf, sizeof(valBuf), format, val);
  g_GL.text(label, sx + 8.f, cy + 22.f, 1.05f, P::TEXT().alpha(m_openAnim), false);
  float valW = g_GL.textW(valBuf, 1.05f);
  g_GL.text(valBuf, m_x + m_w - 16.f - valW, cy + 22.f, 1.05f, P::DIM().alpha(m_openAnim), false);

  // Track
  float trackX = sx + 8.f;
  float trackY = cy + 34.f;
  float trackW = rowW - 16.f;
  float trackH = 4.f;

  g_GL.roundRect(trackX, trackY, trackW, trackH, 2.f, GColor::hex(0x3A3A3C, 0.8f * m_openAnim));

  float pct = (val - min) / (max - min);
  pct = pct < 0.f ? 0.f : pct > 1.f ? 1.f : pct;
  g_GL.roundRect(trackX, trackY, trackW * pct, trackH, 2.f, GColor::hex(0x0A84FF, m_openAnim));

  // Thumb
  float thumbR = 10.f;
  float thumbX = trackX + trackW * pct;
  g_GL.circleFill(thumbX, trackY + trackH / 2.f, thumbR, GColor::hex(0xFFFFFF, m_openAnim), 20);

  bool changed = false;
  bool inBounds = (m_ty >= contentTop && m_ty <= m_y + m_h);
  if (inBounds && m_touchDown && m_tx >= trackX - 20.f && m_tx <= trackX + trackW + 20.f &&
      m_ty >= cy + 10.f && m_ty <= cy + rowH) {
    float p = (m_tx - trackX) / trackW;
    p = p < 0.f ? 0.f : p > 1.f ? 1.f : p;
    val = min + p * (max - min);
    changed = true;
    m_isScrolling = false;
    m_draggingSlider = true;
  }

  cy += rowH;
  return changed;
}

// ============================================================
// SPINNER ROW - iOS-style value picker
// ============================================================
bool VipMenu::doSpinner(const char *label, float &cy, int &val,
                        const char **items, int count) {
  float contentTop = m_y + 112.f;
  float rowH = 42.f;
  float sx = m_x + 16.f;
  float rowW = m_w - 32.f;

  if (cy + rowH < contentTop || cy > m_y + m_h) {
    cy += rowH;
    return false;
  }

  g_GL.rect(sx + 8.f, cy + rowH - 1.f, rowW - 16.f, 1.f, GColor::hex(0x2A3040, 0.5f * m_openAnim));

  g_GL.text(label, sx + 8.f, cy + 28.f, 1.05f, P::TEXT().alpha(m_openAnim), false);

  // Right side: < value >
  float btnW = 26.f;
  float rightX = m_x + m_w - 16.f;

  g_GL.text(">", rightX - btnW + 4.f, cy + 28.f, 1.05f, P::DIM().alpha(m_openAnim), false);
  float vw = g_GL.textW(items[val], 1.05f);
  g_GL.text(items[val], rightX - btnW - 8.f - vw, cy + 28.f, 1.05f, P::CYAN().alpha(m_openAnim), false);
  g_GL.text("<", rightX - btnW - 8.f - vw - btnW, cy + 28.f, 1.05f, P::DIM().alpha(m_openAnim), false);

  bool changed = false;
  bool inBounds = (m_ty >= contentTop && m_ty <= m_y + m_h);
  if (inBounds && m_click && m_ty >= cy && m_ty <= cy + rowH) {
    if (m_tx >= rightX - btnW && m_tx <= rightX) {
      val = (val + 1) % count; changed = true; m_click = false;
    } else if (m_tx >= rightX - btnW - 8.f - vw - btnW && m_tx <= rightX - btnW - 8.f - vw) {
      val = (val - 1 + count) % count; changed = true; m_click = false;
    }
  }

  cy += rowH;
  return changed;
}

// ============================================================
// DRAW CONTENT - Full-width single column, iOS sections
// ============================================================
void VipMenu::drawContent() {
  // Content starts right below the tab bar
  float contentTop = m_y + 112.f;
  float cy = contentTop + m_scrollY[m_tab];

  // Tab-change slide animation
  if (m_prevTab != m_tab && m_tabAnim < 1.f) {
    cy += (1.f - easeInOutCubic(m_tabAnim)) * 30.f;
  }

  if (m_tab == 0) { // DEFENSE
    doSectionHeader("PROTECTIONS", cy);
    if (IsFeatureVisible(13)) {
      if (doToggle("God Mode", cy, g_GodModeEnabled))
        TriggerChange(13, g_GodModeEnabled);
    } else if (g_GodModeEnabled) {
      g_GodModeEnabled = false;
      TriggerChange(13, false);
    }

    if (IsFeatureVisible(205)) {
      if (doToggle("Pas de Rechargement", cy, g_NoReloadEnabled))
        TriggerChange(205, g_NoReloadEnabled);
    } else if (g_NoReloadEnabled) {
      g_NoReloadEnabled = false;
      TriggerChange(205, false);
    }

    if (IsFeatureVisible(98)) {
      if (doToggle("Tirer en Zone Verte", cy, g_GreenZoneBypass))
        TriggerChange(98, g_GreenZoneBypass);
    } else if (g_GreenZoneBypass) {
      g_GreenZoneBypass = false;
      TriggerChange(98, false);
    }

    extern bool g_VipStaminaInfinie;
    if (IsFeatureVisible(505)) {
      if (doToggle("Stamina Infinie", cy, g_VipStaminaInfinie))
        TriggerChange(505, g_VipStaminaInfinie);
    } else if (g_VipStaminaInfinie) {
      g_VipStaminaInfinie = false;
      TriggerChange(505, false);
    }

  } else if (m_tab == 1) { // COMBAT
    doSectionHeader("AIMBOT FEATURES", cy);
    extern bool Aimbot_IsEnabled();
    bool aimEn = Aimbot_IsEnabled();
    if (IsFeatureVisible(120)) {
      if (doToggle("Aimbot Global", cy, aimEn))
        TriggerChange(120, aimEn);
    } else if (aimEn) {
      aimEn = false;
      TriggerChange(120, false);
    }

    if (aimEn) {
      bool wb = !g_AimVisibleOnly;
      if (IsFeatureVisible(132)) {
        if (doToggle("Tirer a travers les murs", cy, wb)) {
          g_AimVisibleOnly = !wb;
          TriggerChange(132, wb);
        }
      } else if (wb) {
        g_AimVisibleOnly = true;
        TriggerChange(132, false);
      }

      if (IsFeatureVisible(184)) {
        if (doToggle("Cibles Visibles Uniquement", cy, g_VisibilityCheck))
          TriggerChange(184, g_VisibilityCheck);
      } else if (g_VisibilityCheck) {
        g_VisibilityCheck = false;
        TriggerChange(184, false);
      }

      const char *bones[] = {"Tete", "Cou", "Torse", "Bassin"};
      if (IsFeatureVisible(183)) {
        if (doSpinner("Priorite de Visee", cy, g_BonePriority, bones, 4))
          TriggerChange(183, false, g_BonePriority);
      }

      if (IsFeatureVisible(185)) {
        if (doSlider("AimLock Smoothness", cy, g_AimLockSmoothness, 1, 5,
                     "%.0f"))
          TriggerChange(185, false, (int)g_AimLockSmoothness);
      }

      extern bool g_AimLockCamera;
      if (IsFeatureVisible(186)) {
        if (doToggle("Bloquer Camera (AimLock)", cy, g_AimLockCamera))
          TriggerChange(186, g_AimLockCamera);
      } else if (g_AimLockCamera) {
        g_AimLockCamera = false;
        TriggerChange(186, false);
      }

      if (g_AimLockCamera) {
        extern bool g_FreecamLock;
        if (IsFeatureVisible(187)) {
          if (doToggle("Mode Freecam Lock (360)", cy, g_FreecamLock))
            TriggerChange(187, g_FreecamLock);
        } else if (g_FreecamLock) {
          g_FreecamLock = false;
          TriggerChange(187, false);
        }
      }
    }

    if (IsFeatureVisible(193)) {
      if (doSlider("Camera FOV", cy, g_CameraFov, 60, 140, "%.0f"))
        TriggerChange(193, false, (int)g_CameraFov);
    }

    doSectionHeader("ESP FEATURES", cy);
    bool espEn = Esp_IsEnabled();
    if (IsFeatureVisible(121)) {
      if (doToggle("ESP Global", cy, espEn))
        TriggerChange(121, espEn);
    } else if (espEn) {
      espEn = false;
      TriggerChange(121, false);
    }

    if (espEn) {
      // FOV Circle en tete de liste
      bool fovC = Esp_IsCrosshairCircleEnabled();
      if (IsFeatureVisible(242)) {
        if (doToggle("FOV Circle", cy, fovC))
          TriggerChange(242, fovC);
      } else if (fovC) {
        TriggerChange(242, false);
      }

      float radius = (float)Esp_GetCrosshairCircleRadius();
      if (IsFeatureVisible(243)) {
        if (doSlider("Taille FOV Circle", cy, radius, 50, 80, "%.0f px"))
          TriggerChange(243, false, (int)radius);
      }

      bool line = Esp_IsLineEnabled();
      if (IsFeatureVisible(194)) {
        if (doToggle("Lignes (Snaplines)", cy, line))
          TriggerChange(194, line);
      } else if (line) {
        TriggerChange(194, false);
      }

      bool box = Esp_IsBoxEnabled();
      if (IsFeatureVisible(195)) {
        if (doToggle("Boites 2D", cy, box))
          TriggerChange(195, box);
      } else if (box) {
        TriggerChange(195, false);
      }

      bool hp = Esp_IsHealthEnabled();
      if (IsFeatureVisible(241)) {
        if (doToggle("Barres de Vie", cy, hp))
          TriggerChange(241, hp);
      } else if (hp) {
        TriggerChange(241, false);
      }

      bool dist = Esp_IsDistanceEnabled();
      if (IsFeatureVisible(196)) {
        if (doToggle("Distance", cy, dist))
          TriggerChange(196, dist);
      } else if (dist) {
        TriggerChange(196, false);
      }

      bool name = Esp_IsMarkerEnabled();
      if (IsFeatureVisible(197)) {
        if (doToggle("Noms / Markers", cy, name))
          TriggerChange(197, name);
      } else if (name) {
        TriggerChange(197, false);
      }

      bool dyn = Esp_IsDynamicColor();
      if (IsFeatureVisible(198)) {
        if (doToggle("Couleurs Dynamiques", cy, dyn))
          TriggerChange(198, dyn);
      } else if (dyn) {
        TriggerChange(198, false);
      }
    }

  } else if (m_tab == 2) { // MOVE
    doSectionHeader("MOUVEMENT JOUEUR", cy);

    if (IsFeatureVisible(109)) {
      if (doToggle("NoClip / Vol Murs", cy, g_NoClipEnabled))
        TriggerChange(109, g_NoClipEnabled);
    } else if (g_NoClipEnabled) {
      g_NoClipEnabled = false;
      TriggerChange(109, false);
    }

    if (IsFeatureVisible(308)) {
      if (doToggle("Vol Libre (Fly Mode)", cy, g_FlyEnabled))
        TriggerChange(308, g_FlyEnabled);
    } else if (g_FlyEnabled) {
      g_FlyEnabled = false;
      TriggerChange(308, false);
    }

    extern float g_FlySpeedForward;
    extern float g_FlySpeedVertical;
    if (g_FlyEnabled) {
      if (doSlider("Vitesse Vol (Avant/Arriere)", cy, g_FlySpeedForward, 1.0f, 50.0f, "%.0f")) {}
      if (doSlider("Vitesse Vol (Haut/Bas)", cy, g_FlySpeedVertical, 1.0f, 50.0f, "%.0f")) {}
    }

    static float speedLevel = 1.0f;
    if (IsFeatureVisible(143)) {
      if (doSlider("Vitesse Global (x1-x10)", cy, speedLevel, 1.0f, 10.0f, "x%.0f")) {
        g_SpeedMultiplier = speedLevel;
        TriggerChange(143, false, (int)(speedLevel * 10.0f));
      }
    }

    doSectionHeader("FEATURES VEHICULE", cy);

    extern bool g_VehicleNoClipEnabled;
    if (IsFeatureVisible(110)) {
      if (doToggle("NoClip Voiture / Passe-Muraille", cy, g_VehicleNoClipEnabled))
        TriggerChange(110, g_VehicleNoClipEnabled);
    } else if (g_VehicleNoClipEnabled) {
      g_VehicleNoClipEnabled = false;
      TriggerChange(110, false);
    }

    static bool autoFollow = false;
    extern bool Esp_IsAutoFollowActive();
    autoFollow = Esp_IsAutoFollowActive();
    if (IsFeatureVisible(300)) {
      if (doToggle("Auto-Follow Cible", cy, autoFollow))
        TriggerChange(300, autoFollow);
    } else if (autoFollow) {
      TriggerChange(300, false);
    }

    if (IsFeatureVisible(305)) {
      if (doButton("Changer / Verrouiller Cible", cy)) {
        TriggerChange(305);
      }
    }

    static bool autoFollowCar = false;
    extern bool Esp_IsAutoFollowCarActive();
    autoFollowCar = Esp_IsAutoFollowCarActive();
    if (IsFeatureVisible(301)) {
      if (doToggle("Auto-Follow en Voiture", cy, autoFollowCar))
        TriggerChange(301, autoFollowCar);
    } else if (autoFollowCar) {
      TriggerChange(301, false);
    }

    if (IsFeatureVisible(302)) {
      if (doSlider("Distance Auto-Follow", cy, g_AutoFollowDistance, 0.0f, 10.0f,
                   "%.0fm"))
        TriggerChange(302, false, (int)g_AutoFollowDistance);
    }
    if (IsFeatureVisible(303)) {
      if (doSlider("Hauteur Auto-Follow", cy, g_AutoFollowHeight, -5.0f, 10.0f,
                   "%.0fm"))
        TriggerChange(303, false, (int)g_AutoFollowHeight);
    }

    if (IsFeatureVisible(304)) {
      if (doButton("TP Voiture vers Cible", cy)) {
        TriggerChange(304);
      }
    }

    static bool stickyCarLocal = false;
    extern bool g_StickyCarEnabled;
    stickyCarLocal = g_StickyCarEnabled;
    if (IsFeatureVisible(306)) {
      if (doToggle("Coller Voiture sur Cible", cy, stickyCarLocal))
        TriggerChange(306, stickyCarLocal);
    } else if (stickyCarLocal) {
      g_StickyCarEnabled = false;
      TriggerChange(306, false);
    }

  } else if (m_tab == 3) { // TELEPORT
    doSectionHeader("WAYPOINT UTILS", cy);
    if (IsFeatureVisible(228)) {
      if (doToggle("TP Carte Auto (Click)", cy, g_TpCarteToggle))
        TriggerChange(228, g_TpCarteToggle);
    } else if (g_TpCarteToggle) {
      g_TpCarteToggle = false;
      TriggerChange(228, false);
    }

    if (IsFeatureVisible(400)) {
      if (doToggle("Rapatrier Marqueurs/Jobs", cy, g_DragCheckpointToPlayer))
        TriggerChange(400, g_DragCheckpointToPlayer);
    } else if (g_DragCheckpointToPlayer) {
      g_DragCheckpointToPlayer = false;
      TriggerChange(400, false);
    }

    // Delai toujours visible (permet de pre-configurer avant activation)
    if (IsFeatureVisible(401)) {
      if (doSlider("Delai Rapatriement (sec)", cy, g_DragCheckpointDelay, 0.0f, 10.0f, "%.1fs"))
        TriggerChange(401, false, (int)(g_DragCheckpointDelay * 10.0f));
    }

    doSectionHeader("TELEPORTS PAR CATEGORIE", cy);
    
    std::vector<const char*> visibleCatNames;
    std::vector<int> visibleCatIndices;
    if (IsFeatureVisible(700)) { visibleCatNames.push_back("Metiers (Jobs)"); visibleCatIndices.push_back(0); }
    if (IsFeatureVisible(701)) { visibleCatNames.push_back("Points d'interet"); visibleCatIndices.push_back(1); }
    if (IsFeatureVisible(702)) { visibleCatNames.push_back("Territoires"); visibleCatIndices.push_back(2); }
    if (IsFeatureVisible(703)) { visibleCatNames.push_back("Activites (Raids)"); visibleCatIndices.push_back(3); }
    if (IsFeatureVisible(704)) { visibleCatNames.push_back("Armureries"); visibleCatIndices.push_back(4); }
    if (IsFeatureVisible(705)) { visibleCatNames.push_back("Magasins Vetements"); visibleCatIndices.push_back(5); }
    if (IsFeatureVisible(706)) { visibleCatNames.push_back("Prison"); visibleCatIndices.push_back(6); }
    if (IsFeatureVisible(707)) { visibleCatNames.push_back("Graffitis (Sprays)"); visibleCatIndices.push_back(7); }

    if (visibleCatNames.empty()) {
      g_GL.text("Aucune categorie disponible", m_x + 24.f, cy + 10.f, 1.0f, P::RED().alpha(m_openAnim), false);
      cy += 30.f;
    } else {
      static int localSelectedIdx = 0;
      if (localSelectedIdx >= (int)visibleCatNames.size()) localSelectedIdx = 0;

      doSpinner("Categorie Teleport", cy, localSelectedIdx, visibleCatNames.data(), (int)visibleCatNames.size());
      int selectedCat = visibleCatIndices[localSelectedIdx];

      static int selectedJobIdx = 0;
      static int selectedPoiIdx = 0;
      static int selectedTerrIdx = 0;
      static int selectedActIdx = 0;
      static int selectedGunIdx = 0;
      static int selectedClothIdx = 0;
      static int selectedPrisonIdx = 0;
      static int selectedSprayIdx = 0;

      if (selectedCat == 0) {
        static const char *jobNames[] = {"Chauffeur de Bus",
                                         "Convoyeur de fonds",
                                         "Mecanicien",
                                         "Livreur",
                                         "Bus Express",
                                         "Bucheron",
                                         "Mineur",
                                         "Taxi",
                                         "Routier",
                                         "Auto-ecole",
                                         "Livraison Medicaments",
                                         "Collecteur de Gemmes"};
        doSpinner("Selectionner Metier", cy, selectedJobIdx, jobNames, 12);
        if (doButton("Teleporter au Metier", cy)) {
          Esp_QueueTeleport(k_Jobs[selectedJobIdx].x, k_Jobs[selectedJobIdx].y,
                            k_Jobs[selectedJobIdx].z);
        }
      } else if (selectedCat == 1) {
        static const char *poiNames[] = {"Integration Sociale",
                                         "Hopital",
                                         "Marche Vehicule",
                                         "Destruction Vehicule",
                                         "Concessionnaire Standard",
                                         "Concessionnaire Premium",
                                         "Bar",
                                         "Theatre",
                                         "Eglise",
                                         "Commissariat",
                                         "Quartier General",
                                         "Tour de Controle",
                                         "Clinique",
                                         "Plateforme d'echange",
                                         "Salle de Jeux",
                                         "Contrats de Syndicat"};
        doSpinner("Selectionner Point d'interet", cy, selectedPoiIdx, poiNames,
                  16);
        if (doButton("Teleporter au Point d'interet", cy)) {
          Esp_QueueTeleport(k_Pois[selectedPoiIdx].x, k_Pois[selectedPoiIdx].y,
                            k_Pois[selectedPoiIdx].z);
        }
      } else if (selectedCat == 2) {
        static const char *terrNames[] = {"Walk of Fame", "Entrepot Cargo",
                                          "Tour de Diffusion"};
        doSpinner("Selectionner Territoire", cy, selectedTerrIdx, terrNames, 3);
        if (doButton("Teleporter au Territoire", cy)) {
          Esp_QueueTeleport(k_Territories[selectedTerrIdx].x,
                            k_Territories[selectedTerrIdx].y,
                            k_Territories[selectedTerrIdx].z);
        }
      } else if (selectedCat == 3) {
        static const char *actNames[] = {
            "Braquage du Port", "Raid Arsenal (Armee)", "Raid Arsenal (Gangs)"};
        doSpinner("Selectionner Activite", cy, selectedActIdx, actNames, 3);
        if (doButton("Teleporter a l'Activite", cy)) {
          Esp_QueueTeleport(k_Activities[selectedActIdx].x,
                            k_Activities[selectedActIdx].y,
                            k_Activities[selectedActIdx].z);
        }
      } else if (selectedCat == 4) {
        static const char *gunshopNames[] = {"GUN SHOP 1", "GUN SHOP 2",
                                             "GUN SHOP 3", "GUN SHOP 4",
                                             "GUN SHOP 5", "GUN SHOP 6"};
        doSpinner("Selectionner Armurerie", cy, selectedGunIdx, gunshopNames,
                  6);
        if (doButton("Teleporter a l'Armurerie", cy)) {
          Esp_QueueTeleport(k_Gunshops[selectedGunIdx].x,
                            k_Gunshops[selectedGunIdx].y,
                            k_Gunshops[selectedGunIdx].z);
        }
      } else if (selectedCat == 5) {
        static const char *clothingNames[] = {
            "CLOTHING STORE 1", "CLOTHING STORE 2", "CLOTHING STORE 3",
            "CLOTHING STORE 4", "CLOTHING STORE 5", "CLOTHING STORE 6",
            "CLOTHING STORE 7"};
        doSpinner("Selectionner Magasin", cy, selectedClothIdx, clothingNames,
                  7);
        if (doButton("Teleporter au Magasin", cy)) {
          Esp_QueueTeleport(k_Clothing[selectedClothIdx].x,
                            k_Clothing[selectedClothIdx].y,
                            k_Clothing[selectedClothIdx].z);
        }
      } else if (selectedCat == 6) {
        static const char *prisonNames[] = {
            "Enregistrement Prison 1", "Enregistrement Prison 2",
            "Enregistrement Prison 3", "Evasion Prison"};
        doSpinner("Selectionner Option Prison", cy, selectedPrisonIdx,
                  prisonNames, 4);
        if (doButton("Executer Action Prison", cy)) {
          Esp_QueueTeleport(k_Prison[selectedPrisonIdx].x,
                            k_Prison[selectedPrisonIdx].y,
                            k_Prison[selectedPrisonIdx].z);
        }
      } else if (selectedCat == 7) {
        static const char *sprayNames[111] = {nullptr};
        if (sprayNames[0] == nullptr) {
          for (int i = 0; i < 111; i++) {
            sprayNames[i] = k_Sprays[i].name;
          }
        }
        doSpinner("Selectionner Spray", cy, selectedSprayIdx, sprayNames, 111);
        if (doButton("Teleporter au Spray", cy)) {
          Esp_QueueTeleport(k_Sprays[selectedSprayIdx].x,
                            k_Sprays[selectedSprayIdx].y,
                            k_Sprays[selectedSprayIdx].z);
        }
      }
    }

  } else if (m_tab == 4) { // VIP
    extern bool g_VipUnlocked;
    extern char g_VipPinCode[16];

    if (!g_VipUnlocked) {
      // Draw VIP Lock screen / Keypad
      g_GL.roundRect(m_x + 16, cy, m_w - 32, 90, 12.f, P::CARD().alpha(0.8f));
      g_GL.text("ACCES VIP VERROUILLE", m_x + 40, cy + 18, 1.8f, P::CYAN(),
                false);
      g_GL.text("Entrez le code PIN pour deverrouiller", m_x + 40, cy + 50,
                1.4f, P::TEXT(), false);
      cy += 105;

      char pinDots[32] = "";
      int pinLen = strlen(g_VipPinCode);
      for (int i = 0; i < pinLen; i++) {
        strcat(pinDots, "* ");
      }
      if (pinLen == 0)
        strcpy(pinDots, "Entrez le PIN");

      float pinW = g_GL.textW(pinDots, 2.0f);
      g_GL.text(pinDots, m_x + (m_w - pinW) / 2.f, cy + 10, 2.0f, P::CYAN(),
                false);
      cy += 55;

      // Draw standard PIN pad: 3 columns x 4 rows
      // Col width: 80, Row height: 50. Total grid width ~240, centered.
      float gridW = 260.f;
      float startGridX = m_x + (m_w - gridW) / 2.f;
      float cellW = 80.f;
      float cellH = 50.f;
      float gap = 10.f;

      const char *keys[12] = {"1", "2", "3", "4", "5", "6",
                              "7", "8", "9", "C", "0", "OK"};

      for (int r = 0; r < 4; r++) {
        for (int c = 0; c < 3; c++) {
          int idx = r * 3 + c;
          float kx = startGridX + c * (cellW + gap);
          float ky = cy + r * (cellH + gap);

          // Render button
          g_GL.roundRect(kx, ky, cellW, cellH, 8.f, P::CARD().alpha(0.6f));
          float kw = g_GL.textW(keys[idx], 1.6f);
          g_GL.text(keys[idx], kx + (cellW - kw) / 2.f, ky + 15.f, 1.6f,
                    P::TEXT(), false);

          bool inBounds = (m_ty >= m_y + 130 && m_ty <= m_y + m_h - 15);
          if (inBounds && m_click && m_tx >= kx && m_tx <= kx + cellW &&
              m_ty >= ky && m_ty <= ky + cellH) {
            m_click = false; // consume click
            if (strcmp(keys[idx], "C") == 0) {
              g_VipPinCode[0] = '\0';
            } else if (strcmp(keys[idx], "OK") == 0) {
              if (strcmp(g_VipPinCode, "2702") == 0) {
                g_VipUnlocked = true;
                g_VipPinCode[0] = '\0';
              } else {
                g_VipPinCode[0] = '\0';
              }
            } else {
              int currLen = strlen(g_VipPinCode);
              if (currLen < 8) {
                g_VipPinCode[currLen] = keys[idx][0];
                g_VipPinCode[currLen + 1] = '\0';
              }
            }
          }
        }
      }
      cy += 4 * (cellH + gap) + 20.f;
    } else {
      // === VIP DÉVERROUILLÉ : Features exclusives fonctionnelles ===
      doSectionHeader("JOUEUR VIP", cy);

      doSectionHeader("VEHICULE VIP", cy);

      // Vehicle Speed (VehicleViewParams +0x78, Lua Y1/Y2)
      if (IsFeatureVisible(510)) {
        if (doSlider("Vitesse Moteur (max)", cy, g_VipVehicleSpeed, 0.0f, 99999.0f, "%.0f"))
          TriggerChange(510, false, (int)g_VipVehicleSpeed);
      }

      // Vehicle Angle (VehicleViewParams +0xAC, Lua Y3)
      if (IsFeatureVisible(511)) {
        if (doSlider("Angle de Braquage", cy, g_VipVehicleAngle, 0.0f, 99.0f, "%.0f°"))
          TriggerChange(511, false, (int)g_VipVehicleAngle);
      }

      // Max Brake (VehicleViewParams +0x88 = 999999, Lua Y4)
      if (IsFeatureVisible(512)) {
        if (doToggle("Max Brake Force", cy, g_VipVehicleMaxBrake))
          TriggerChange(512, g_VipVehicleMaxBrake);
      } else if (g_VipVehicleMaxBrake) {
        g_VipVehicleMaxBrake = false;
        TriggerChange(512, false);
      }

      // Forward Force (WheelViewParams +0x24, Lua Y5)
      if (IsFeatureVisible(513)) {
        if (doSlider("Forward Force Roues", cy, g_VipVehicleForwardForce, 0.0f, 15.0f, "%.0f"))
          TriggerChange(513, false, (int)g_VipVehicleForwardForce);
      }

      // No Damage (VehicleViewParams +0x12C = 999999, Lua Y6)
      if (IsFeatureVisible(514)) {
        if (doToggle("Voiture Indestructible", cy, g_VipVehicleNoDamage))
          TriggerChange(514, g_VipVehicleNoDamage);
      } else if (g_VipVehicleNoDamage) {
        g_VipVehicleNoDamage = false;
        TriggerChange(514, false);
      }

      // Fuel Infini (Lua Y7)
      if (IsFeatureVisible(515)) {
        if (doToggle("Carburant Infini", cy, g_VipVehicleInfFuel))
          TriggerChange(515, g_VipVehicleInfFuel);
      } else if (g_VipVehicleInfFuel) {
        g_VipVehicleInfFuel = false;
        TriggerChange(515, false);
      }

      // Slipping (WheelViewParams +0x2C, Lua Y8)
      if (IsFeatureVisible(516)) {
        if (doSlider("Adherence Roues (Slipping)", cy, g_VipVehicleSlipping, 0.0f, 9999.0f, "%.0f"))
          TriggerChange(516, false, (int)g_VipVehicleSlipping);
      }

      // Wheel Size (WheelViewParams +0x40 DWORD, Lua YY3)
      if (IsFeatureVisible(517)) {
        if (doSlider("Taille des Roues", cy, (float &)g_VipVehicleWheelSize, 0.0f, 2000000000.0f, "%.0f"))
          TriggerChange(517, false, g_VipVehicleWheelSize);
      }

      doSectionHeader("COSMETIQUES", cy);

      // We should check dynamic catalog sub-tabs instead of forcing default cosmetics options
      // if those catalog sections are not enabled by the admin
      if (IsFeatureVisible(602)) { // Weapons Catalog
        static const char *weaponNames[] = {"Par defaut", "AK47 Scarlet",
                                             "M4A1 Scorpio", "Deagle Gold",
                                             "MP5 Yumi", "KTR Dragon"};
        extern int g_WeaponReplaceVal;
        if (doSpinner("Remplacement Arme", cy, g_WeaponReplaceVal, weaponNames, 6))
          TriggerChange(260, false, g_WeaponReplaceVal);
      }

      if (IsFeatureVisible(601)) { // Vehicles Catalog
        static const char *vehicleNames[] = {"Par defaut", "Moto", "Lafera",
                                              "Police", "Cube", "Nissan", "Hummer"};
        extern int g_VehicleReplaceVal;
        if (doSpinner("Remplacement Vehicule", cy, g_VehicleReplaceVal, vehicleNames, 7))
          TriggerChange(262, false, g_VehicleReplaceVal);
      }

      if (IsFeatureVisible(600)) { // Skins Catalog
        static const char *skinNames[] = {"Par defaut", "Triade (F)", "Baddie (F)",
                                           "Grim Spec", "Capo", "Street", "Tiger"};
        extern int g_SkinReplaceVal;
        if (doSpinner("Remplacement Skin", cy, g_SkinReplaceVal, skinNames, 7))
          TriggerChange(261, false, g_SkinReplaceVal);
      }
    }


  } else if (m_tab == 5) { // MISC
    doSectionHeader("SETTINGS", cy);
    // Toggle Device ID : ON = active + regenere le faux ID, OFF = ID reel
    if (IsFeatureVisible(153)) {
      if (doToggle("Regenerer Device ID (On/Off)", cy, g_DeviceIdFaker))
        TriggerChange(153, g_DeviceIdFaker);
    } else if (g_DeviceIdFaker) {
      g_DeviceIdFaker = false;
      TriggerChange(153, false);
    }
  }

  // Bounds check for scroll
  float contentHeight = cy - (m_y + 112 + m_scrollY[m_tab]);
  float visibleHeight = m_h - 140;
  float maxScroll = visibleHeight - contentHeight;
  if (maxScroll > 0)
    maxScroll = 0;
  if (m_scrollY[m_tab] < maxScroll)
    m_scrollY[m_tab] = maxScroll;
}

extern "C" JNIEXPORT jboolean JNICALL
Java_com_chillbase_games_ChillbaseActivity_nativeTouch(JNIEnv *env,
                                                       jclass clazz,
                                                       jint action, jfloat x,
                                                       jfloat y, jfloat viewW,
                                                       jfloat viewH) {
  float scaled_x = x;
  float scaled_y = y;
  float rw = (float)g_GL.W();
  float rh = (float)g_GL.H();

  if (viewW > 0.f && viewH > 0.f && rw > 0.f && rh > 0.f) {
    scaled_x = x * (rw / viewW);
    scaled_y = y * (rh / viewH);
  }

  if (ImGui::GetCurrentContext() != nullptr) {
    ImGuiIO& io = ImGui::GetIO();
    io.AddMousePosEvent(scaled_x, scaled_y);
    if (action == 0) io.AddMouseButtonEvent(0, true); // ACTION_DOWN
    if (action == 1) io.AddMouseButtonEvent(0, false); // ACTION_UP
    if (io.WantCaptureMouse) {
      return JNI_TRUE;
    }
  }

  // The old VipMenu interface is dead, so we just return false 
  // to let the game handle the touch if ImGui didn't capture it.
  return JNI_FALSE;
}

void VipMenu::drawEspOverlay() {
  if (!Esp_IsEnabled())
    return;

  float cx = g_GL.W() / 2.f;
  float cy = g_GL.H() / 2.f;

  if (Esp_IsCrosshairEnabled()) {
    g_GL.line(cx - 15, cy, cx + 15, cy, 2.f, P::RED());
    g_GL.line(cx, cy - 15, cx, cy + 15, 2.f, P::RED());
  }

  if (Esp_IsCrosshairCircleEnabled()) {
    float r = (float)Esp_GetCrosshairCircleRadius();
    g_GL.circleOutline(cx, cy, r, 2.f, P::TEXT(), 36);
  }

  static std::vector<EspPed> peds;
  peds = Esp_GetSnapshot();
  float lineOriginY = (float)g_GL.H();

  extern bool Aimbot_IsTargetLocked();
  extern int Aimbot_GetTargetId();
  bool aimbotLocked = Aimbot_IsTargetLocked();

  float bestScrDist = 999999.f;
  const EspPed *bestTarget = nullptr;
  for (const auto &p : peds) {
    if (p.isLocal || !p.onScreen)
      continue;
    float dx = p.screenX - cx;
    float dy = p.screenY - cy;
    float d = dx * dx + dy * dy;
    if (d < bestScrDist) {
      bestScrDist = d;
      bestTarget = &p;
    }
  }

  for (const auto &p : peds) {
    if (!p.onScreen || p.isLocal)
      continue;

    bool isLockedTarget = aimbotLocked &&
                          bestTarget &&
                          (p.actorPtr == bestTarget->actorPtr) &&
                          (bestTarget == &p);
    GColor col;
    if (isLockedTarget) {
      float pulse = (sinf(m_anim * 4.f) + 1.f) * 0.5f;
      col = GColor{0.1f, 1.f, 0.3f + pulse * 0.2f, 1.f};
    } else if (Esp_IsDynamicColor()) {
      float hue = m_anim + p.distance * 0.01f;
      col = GColor{(sinf(hue) + 1.f) * .5f, (sinf(hue + 2.f) + 1.f) * .5f,
                   (sinf(hue + 4.f) + 1.f) * .5f, 1.f};
    } else {
      if (p.pedType == ESP_PED_PLAYER)
        col = P::PURPLE();
      else if (p.pedType == ESP_PED_NPC)
        col = GColor{0.3f, 1.f, 0.3f, 1.f};
      else if (p.pedType == ESP_PED_POLICE)
        col = P::CYAN();
      else
        col = P::GOLD();
    }

    float h = p.screenY - p.screenHeadY;
    float w = h / 2.2f;

    if (Esp_IsLineEnabled()) {
      g_GL.line(cx, lineOriginY, p.screenX, p.screenY,
                isLockedTarget ? 3.f : 1.5f,
                col.alpha(isLockedTarget ? 0.9f : 0.5f));
    }

    if (Esp_IsBoxEnabled()) {
      g_GL.roundBorder(p.screenX - w / 2.f, p.screenHeadY, w, h, 4.f,
                       isLockedTarget ? 2.5f : 1.5f, col);
      if (isLockedTarget)
        g_GL.glow(p.screenX - w / 2.f, p.screenHeadY, w, h, 12.f,
                  col.alpha(0.35f));
    }

    if (Esp_IsDotMode()) {
      g_GL.circleFill(p.screenX, p.screenY - h / 2.f,
                      isLockedTarget ? 7.f : 4.f, col);
    }

    if (Esp_IsHealthEnabled() && p.health >= 0.f) {
      float hpPadding = 3.f;
      float hpBarW = 4.f;
      float hpX = p.screenX - w / 2.f - 8.f;
      float hpY = p.screenHeadY + hpPadding;
      float hpHeight = h - (hpPadding * 2.f);
      if (hpHeight > 0.f) {
        g_GL.rect(hpX, hpY, hpBarW, hpHeight, GColor::hex(0x000000, 0.8f));

        float clampedHealth = p.health;
        if (clampedHealth < 0.f)
          clampedHealth = 0.f;
        if (clampedHealth > 1.f)
          clampedHealth = 1.f;

        float hpColR = 1.f - clampedHealth;
        float hpColG = clampedHealth;
        float hpFillH = hpHeight * clampedHealth;
        g_GL.rect(hpX + 1.f, hpY + (hpHeight - hpFillH) + 1.f, hpBarW - 2.f,
                  hpFillH - 2.f, GColor{hpColR, hpColG, 0.f, 1.f});
      }
    }

    float textY = p.screenY + 5.f;
    if (Esp_IsDistanceEnabled()) {
      char distBuf[64];
      snprintf(distBuf, sizeof(distBuf), isLockedTarget ? "%.1fm" : "%.1fm",
               p.distance);
      g_GL.text(distBuf, p.screenX, textY, 1.2f,
                isLockedTarget ? GColor{0.3f, 1.f, 0.4f, 1.f} : P::TEXT());
      textY += 18.f;
    }
    if (Esp_IsMarkerEnabled() && p.name[0] != '\0') {
      g_GL.text(p.name, p.screenX, textY, 1.2f,
                isLockedTarget ? GColor{0.3f, 1.f, 0.4f, 1.f} : P::GOLD());
    }
  }
}
