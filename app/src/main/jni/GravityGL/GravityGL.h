#pragma once
#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <cstdint>
#include <vector>

struct GColor {
  float r, g, b, a;
  static GColor hex(uint32_t h, float a = 1.f) {
    return {((h >> 16) & 0xFF) / 255.f, ((h >> 8) & 0xFF) / 255.f,
            (h & 0xFF) / 255.f, a};
  }
  GColor alpha(float na) const { return {r, g, b, na}; }
  GColor operator*(float m) const { return {r, g, b, a * m}; }
};
namespace P {
static inline GColor CYAN() { return GColor::hex(0x00DCFF, 1.0f); }
static inline GColor PURPLE() { return GColor::hex(0x00BFFF, 1.0f); } // Map purple to Deep Sky Blue
static inline GColor GOLD() { return GColor::hex(0x87CEEB, 1.0f); } // Map gold to Sky Blue
static inline GColor GREEN() { return GColor::hex(0x33FFCC, 1.0f); } // Cyan-ish green
static inline GColor RED() { return GColor::hex(0xFF3366, 1.0f); }
static inline GColor BG() { return GColor::hex(0x0A0D1A, 0.82f); } // 0.04, 0.05, 0.10, 0.82
static inline GColor PANEL() { return GColor::hex(0x0F1426, 0.65f); } // 0.06, 0.08, 0.15, 0.65
static inline GColor CARD() { return GColor::hex(0x141F38, 0.45f); } // 0.08, 0.12, 0.22, 0.45
static inline GColor TEXT() { return GColor::hex(0xEBF5FF, 1.0f); } // 0.92, 0.96, 1.00
static inline GColor DIM() { return GColor::hex(0x738C96, 1.0f); } // 0.45, 0.55, 0.65
static inline GColor BORDER() { return GColor::hex(0x00D9FF, 0.75f); } // 0.00, 0.85, 1.00, 0.75
static inline GColor NET() { return GColor::hex(0x00D9FF, 0.6f); }
static inline GColor MAGENTA() { return GColor::hex(0x00FFFF, 1.0f); } // Map magenta to Cyan
} // namespace P
class GravityGL {
public:
  bool init(int w, int h);
  void destroy();
  void beginFrame();
  void endFrame();
  void setSize(int w, int h) {
    m_w = w;
    m_h = h;
  }
  void flush();
  void rect(float x, float y, float w, float h, GColor c);
  void gradH(float x, float y, float w, float h, GColor l, GColor r);
  void gradV(float x, float y, float w, float h, GColor t, GColor b);
  void roundRect(float x, float y, float w, float h, float r, GColor c);
  void roundBorder(float x, float y, float w, float h, float r, float th,
                   GColor c);
  void line(float x0, float y0, float x1, float y1, float th, GColor c);
  void circleFill(float cx, float cy, float r, GColor c, int s = 32);
  void circleOutline(float cx, float cy, float r, float th, GColor c,
                     int s = 32);
  void arcOutline(float cx, float cy, float r, float th, float startAngle, float endAngle, GColor c, int s = 16);
  void glow(float x, float y, float w, float h, float sp, GColor c);
  void text(const char *s, float x, float y, float sc, GColor c,
            bool shadow = true);
  float textW(const char *s, float sc);
  void panel(float x, float y, float w, float h, float r = 10.f);
  void toggle(float x, float y, bool on, float anim = 1.f);
  void progressBar(float x, float y, float w, float h, float v, GColor fg);
  void separator(float x, float y, float w, GColor c);
  GLuint loadRGBA(const uint8_t* data, int w, int h, GLint filter = GL_LINEAR);
  void quad(float x, float y, float w, float h, float u0, float v0, float u1,
            float v1, GColor tl, GColor tr, GColor bl, GColor br, GLuint tex);
  void quadRotated(float cx, float cy, float w, float h, float angle, float u0,
                   float v0, float u1, float v1, GColor c, GLuint tex);
  void sphere3D(float cx, float cy, float radius, float angleY, GColor tint, GLuint tex);
  int W() const { return m_w; }
  int H() const { return m_h; }
  float T() const { return m_t; }

private:
  EGLDisplay m_dpy = EGL_NO_DISPLAY;
  EGLSurface m_sur = EGL_NO_SURFACE;
  EGLContext m_ctx = EGL_NO_CONTEXT;
  GLuint m_prog = 0, m_fontTex = 0, m_vbo = 0, m_dummyTex = 0;
  int m_w = 1080, m_h = 1920;
  float m_t = 0.f;
  struct V {
    float x, y, u, v, r, g, b, a;
  };
  std::vector<V> m_verts;
  GLuint m_curTex = 0;
  bool compileShaders();
  void uploadFont();
  float nx(float x) const { return 2.f * x / m_w - 1.f; }
  float ny(float y) const { return -2.f * y / m_h + 1.f; }
  
  // Font data structures for stb_truetype
  struct BakedChar {
    unsigned short x0, y0, x1, y1;
    float xoff, yoff, xadvance;
  };
  BakedChar m_cdata[96]; // ASCII 32..126
  
  static const uint8_t kFont[95][8];
};
extern GravityGL g_GL;
