#include "GravityGL.h"
#include "EmbeddedFont.h"
#include <android/log.h>
#include <math.h>
#include <string.h>

#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, "GravityGL", __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, "GravityGL", __VA_ARGS__)

GravityGL g_GL;

// Minimal font for demonstration (ASCII 32 to 126)
const uint8_t GravityGL::kFont[95][8] = {
    {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}, // space
    {0x18,0x3C,0x3C,0x18,0x18,0x00,0x18,0x00}, // !
    {0x66,0x66,0x24,0x00,0x00,0x00,0x00,0x00}, // "
    {0x66,0xFF,0x66,0x66,0xFF,0x66,0x00,0x00}, // #
    {0x18,0x3E,0x60,0x3C,0x06,0x7C,0x18,0x00}, // $
    {0x62,0x66,0x0C,0x18,0x30,0x66,0x46,0x00}, // %
    {0x3C,0x66,0x3C,0x38,0x67,0x66,0x3F,0x00}, // &
    {0x06,0x0C,0x18,0x00,0x00,0x00,0x00,0x00}, // '
    {0x0C,0x18,0x30,0x30,0x30,0x18,0x0C,0x00}, // (
    {0x30,0x18,0x0C,0x0C,0x0C,0x18,0x30,0x00}, // )
    {0x00,0x66,0x3C,0xFF,0x3C,0x66,0x00,0x00}, // *
    {0x00,0x18,0x18,0x7E,0x18,0x18,0x00,0x00}, // +
    {0x00,0x00,0x00,0x00,0x00,0x18,0x18,0x30}, // ,
    {0x00,0x00,0x00,0x7E,0x00,0x00,0x00,0x00}, // -
    {0x00,0x00,0x00,0x00,0x00,0x18,0x18,0x00}, // .
    {0x06,0x0C,0x18,0x30,0x60,0xC0,0x80,0x00}, // /
    {0x3C,0x66,0x66,0x66,0x66,0x66,0x3C,0x00}, // 0
    {0x18,0x38,0x18,0x18,0x18,0x18,0x7E,0x00}, // 1
    {0x3C,0x66,0x06,0x1C,0x30,0x66,0x7E,0x00}, // 2
    {0x3C,0x66,0x06,0x1C,0x06,0x66,0x3C,0x00}, // 3
    {0x1C,0x3C,0x6C,0xCC,0xFE,0x0C,0x1E,0x00}, // 4
    {0x7E,0x60,0x7C,0x06,0x06,0x66,0x3C,0x00}, // 5
    {0x3C,0x66,0x60,0x7C,0x66,0x66,0x3C,0x00}, // 6
    {0x7E,0x66,0x0C,0x18,0x30,0x30,0x30,0x00}, // 7
    {0x3C,0x66,0x66,0x3C,0x66,0x66,0x3C,0x00}, // 8
    {0x3C,0x66,0x66,0x3E,0x06,0x66,0x3C,0x00}, // 9
    {0x00,0x18,0x18,0x00,0x00,0x18,0x18,0x00}, // :
    {0x00,0x18,0x18,0x00,0x00,0x18,0x18,0x30}, // ;
    {0x0C,0x18,0x30,0x60,0x30,0x18,0x0C,0x00}, // <
    {0x00,0x00,0x7E,0x00,0x7E,0x00,0x00,0x00}, // =
    {0x30,0x18,0x0C,0x06,0x0C,0x18,0x30,0x00}, // >
    {0x3C,0x66,0x06,0x0C,0x18,0x00,0x18,0x00}, // ?
    {0x3C,0x66,0x6E,0x6E,0x60,0x66,0x3C,0x00}, // @
    {0x3C,0x66,0x66,0x7E,0x66,0x66,0x66,0x00}, // A
    {0x7C,0x66,0x66,0x7C,0x66,0x66,0x7C,0x00}, // B
    {0x3C,0x66,0x60,0x60,0x60,0x66,0x3C,0x00}, // C
    {0x78,0x6C,0x66,0x66,0x66,0x6C,0x78,0x00}, // D
    {0x7E,0x60,0x60,0x7C,0x60,0x60,0x7E,0x00}, // E
    {0x7E,0x60,0x60,0x7C,0x60,0x60,0x60,0x00}, // F
    {0x3C,0x66,0x60,0x6E,0x66,0x66,0x3E,0x00}, // G
    {0x66,0x66,0x66,0x7E,0x66,0x66,0x66,0x00}, // H
    {0x3C,0x18,0x18,0x18,0x18,0x18,0x3C,0x00}, // I
    {0x06,0x06,0x06,0x06,0x06,0x66,0x3C,0x00}, // J
    {0x66,0x6C,0x78,0x70,0x78,0x6C,0x66,0x00}, // K
    {0x60,0x60,0x60,0x60,0x60,0x60,0x7E,0x00}, // L
    {0x63,0x77,0x7F,0x6B,0x63,0x63,0x63,0x00}, // M
    {0x66,0x76,0x7E,0x7E,0x6E,0x66,0x66,0x00}, // N
    {0x3C,0x66,0x66,0x66,0x66,0x66,0x3C,0x00}, // O
    {0x7C,0x66,0x66,0x7C,0x60,0x60,0x60,0x00}, // P
    {0x3C,0x66,0x66,0x66,0x66,0x7C,0x3A,0x00}, // Q
    {0x7C,0x66,0x66,0x7C,0x78,0x6C,0x66,0x00}, // R
    {0x3C,0x66,0x60,0x3C,0x06,0x66,0x3C,0x00}, // S
    {0x7E,0x18,0x18,0x18,0x18,0x18,0x18,0x00}, // T
    {0x66,0x66,0x66,0x66,0x66,0x66,0x3C,0x00}, // U
    {0x66,0x66,0x66,0x66,0x3C,0x3C,0x18,0x00}, // V
    {0x63,0x63,0x63,0x6B,0x7F,0x77,0x63,0x00}, // W
    {0x66,0x66,0x3C,0x18,0x3C,0x66,0x66,0x00}, // X
    {0x66,0x66,0x66,0x3C,0x18,0x18,0x18,0x00}, // Y
    {0x7E,0x06,0x0C,0x18,0x30,0x60,0x7E,0x00}, // Z
    {0x3C,0x30,0x30,0x30,0x30,0x30,0x3C,0x00}, // [
    {0x80,0xC0,0x60,0x30,0x18,0x0C,0x06,0x00}, // \.
    {0x3C,0x0C,0x0C,0x0C,0x0C,0x0C,0x3C,0x00}, // ]
    {0x18,0x3C,0x66,0x00,0x00,0x00,0x00,0x00}, // ^
    {0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0x00}, // _
    {0x18,0x0C,0x06,0x00,0x00,0x00,0x00,0x00}, // `
    {0x00,0x00,0x3C,0x06,0x3E,0x66,0x3E,0x00}, // a
    {0x60,0x60,0x7C,0x66,0x66,0x66,0x7C,0x00}, // b
    {0x00,0x00,0x3C,0x60,0x60,0x60,0x3C,0x00}, // c
    {0x06,0x06,0x3E,0x66,0x66,0x66,0x3E,0x00}, // d
    {0x00,0x00,0x3C,0x66,0x7E,0x60,0x3C,0x00}, // e
    {0x1C,0x30,0x7C,0x30,0x30,0x30,0x30,0x00}, // f
    {0x00,0x00,0x3E,0x66,0x66,0x3E,0x06,0x3C}, // g
    {0x60,0x60,0x7C,0x66,0x66,0x66,0x66,0x00}, // h
    {0x18,0x00,0x38,0x18,0x18,0x18,0x3C,0x00}, // i
    {0x06,0x00,0x0E,0x06,0x06,0x06,0x06,0x3C}, // j
    {0x60,0x60,0x66,0x6C,0x78,0x6C,0x66,0x00}, // k
    {0x38,0x18,0x18,0x18,0x18,0x18,0x3C,0x00}, // l
    {0x00,0x00,0x6C,0xFE,0xAA,0xAA,0xAA,0x00}, // m
    {0x00,0x00,0x7C,0x66,0x66,0x66,0x66,0x00}, // n
    {0x00,0x00,0x3C,0x66,0x66,0x66,0x3C,0x00}, // o
    {0x00,0x00,0x7C,0x66,0x66,0x7C,0x60,0x60}, // p
    {0x00,0x00,0x3E,0x66,0x66,0x3E,0x06,0x06}, // q
    {0x00,0x00,0x7C,0x66,0x60,0x60,0x60,0x00}, // r
    {0x00,0x00,0x3E,0x60,0x3C,0x06,0x7C,0x00}, // s
    {0x30,0x30,0x7C,0x30,0x30,0x34,0x18,0x00}, // t
    {0x00,0x00,0x66,0x66,0x66,0x66,0x3E,0x00}, // u
    {0x00,0x00,0x66,0x66,0x66,0x3C,0x18,0x00}, // v
    {0x00,0x00,0x63,0x6B,0x7F,0x77,0x63,0x00}, // w
    {0x00,0x00,0x66,0x3C,0x18,0x3C,0x66,0x00}, // x
    {0x00,0x00,0x66,0x66,0x66,0x3E,0x06,0x3C}, // y
    {0x00,0x00,0x7E,0x0C,0x18,0x30,0x7E,0x00}, // z
    {0x0E,0x18,0x18,0x70,0x18,0x18,0x0E,0x00}, // {
    {0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x00}, // |
    {0x70,0x18,0x18,0x0E,0x18,0x18,0x70,0x00}, // }
    {0x3A,0x6C,0x00,0x00,0x00,0x00,0x00,0x00}, // ~
};

static const char* vShaderSrc = 
    "attribute vec2 aPos;\n"
    "attribute vec2 aUV;\n"
    "attribute vec4 aCol;\n"
    "varying vec2 vUV;\n"
    "varying vec4 vCol;\n"
    "void main() {\n"
    "  gl_Position = vec4(aPos, 0.0, 1.0);\n"
    "  vUV = aUV;\n"
    "  vCol = aCol;\n"
    "}\n";

static const char* fShaderSrc = 
    "precision mediump float;\n"
    "varying vec2 vUV;\n"
    "varying vec4 vCol;\n"
    "uniform sampler2D uTex;\n"
    "void main() {\n"
    "  gl_FragColor = vCol * texture2D(uTex, vUV);\n"
    "}\n";

static GLuint loadShader(GLenum type, const char* src) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &src, NULL);
    glCompileShader(shader);
    GLint compiled;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
    if (!compiled) {
        GLint infoLen = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);
        if (infoLen > 1) {
            char* infoLog = new char[infoLen];
            glGetShaderInfoLog(shader, infoLen, NULL, infoLog);
            LOGE("Error compiling shader:\n%s", infoLog);
            delete[] infoLog;
        }
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}

bool GravityGL::compileShaders() {
    GLuint vShader = loadShader(GL_VERTEX_SHADER, vShaderSrc);
    GLuint fShader = loadShader(GL_FRAGMENT_SHADER, fShaderSrc);
    if (!vShader || !fShader) return false;

    m_prog = glCreateProgram();
    glAttachShader(m_prog, vShader);
    glAttachShader(m_prog, fShader);
    glLinkProgram(m_prog);

    GLint linked;
    glGetProgramiv(m_prog, GL_LINK_STATUS, &linked);
    if (!linked) {
        LOGE("Error linking program");
        glDeleteProgram(m_prog);
        return false;
    }
    
    glUseProgram(m_prog);
    glUniform1i(glGetUniformLocation(m_prog, "uTex"), 0);
    return true;
}

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"
#include "OrbitronData.h"

void GravityGL::uploadFont() {
    const int texW = 512;
    const int texH = 512;
    std::vector<uint8_t> temp_bitmap(texW * texH);
    
    // We need our own array matching the struct to let stb bake, then we copy back.
    stbtt_bakedchar cdata[96];
    
    stbtt_BakeFontBitmap((const unsigned char*)Orbitron_ttf, 0, 32.0f, temp_bitmap.data(), texW, texH, 32, 96, cdata);
    
    // Copy baked chars to our struct
    for(int i=0; i<96; ++i) {
        m_cdata[i].x0 = cdata[i].x0;
        m_cdata[i].y0 = cdata[i].y0;
        m_cdata[i].x1 = cdata[i].x1;
        m_cdata[i].y1 = cdata[i].y1;
        m_cdata[i].xoff = cdata[i].xoff;
        m_cdata[i].yoff = cdata[i].yoff;
        m_cdata[i].xadvance = cdata[i].xadvance;
    }

    // Convert 1-byte alpha to 4-byte RGBA (White + Alpha)
    std::vector<uint32_t> pixels(texW * texH);
    for (int i = 0; i < texW * texH; i++) {
        uint8_t a = temp_bitmap[i];
        pixels[i] = (a << 24) | 0x00FFFFFF; // White text, baked alpha
    }
    
    glGenTextures(1, &m_fontTex);
    glBindTexture(GL_TEXTURE_2D, m_fontTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texW, texH, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
    // Use Linear filtering for smooth font rendering
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

bool GravityGL::init(int w, int h) {
    m_w = w;
    m_h = h;
    
    if (m_prog) return true; // Already initialized

    if (!compileShaders()) return false;
    uploadFont();
    
    // Dummy texture for solid color rendering
    glGenTextures(1, &m_dummyTex);
    glBindTexture(GL_TEXTURE_2D, m_dummyTex);
    uint32_t whitePix = 0xFFFFFFFF;
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, &whitePix);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glGenBuffers(1, &m_vbo);
    return true;
}

void GravityGL::destroy() {
    if (m_prog) {
        glDeleteProgram(m_prog);
        m_prog = 0;
    }
}

void GravityGL::beginFrame() {
    m_t += 0.016f; // approx 60fps
    m_verts.clear();
    m_curTex = 0xFFFFFFFF;
    
    glViewport(0, 0, m_w, m_h);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glUseProgram(m_prog);
}

void GravityGL::flush() {
    if (m_verts.empty()) return;

    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, m_verts.size() * sizeof(V), m_verts.data(), GL_DYNAMIC_DRAW);

    GLint aPos = glGetAttribLocation(m_prog, "aPos");
    GLint aUV = glGetAttribLocation(m_prog, "aUV");
    GLint aCol = glGetAttribLocation(m_prog, "aCol");

    glEnableVertexAttribArray(aPos);
    glEnableVertexAttribArray(aUV);
    glEnableVertexAttribArray(aCol);

    glVertexAttribPointer(aPos, 2, GL_FLOAT, GL_FALSE, sizeof(V), (void*)offsetof(V, x));
    glVertexAttribPointer(aUV, 2, GL_FLOAT, GL_FALSE, sizeof(V), (void*)offsetof(V, u));
    glVertexAttribPointer(aCol, 4, GL_FLOAT, GL_FALSE, sizeof(V), (void*)offsetof(V, r));

    glDrawArrays(GL_TRIANGLES, 0, m_verts.size());

    glDisableVertexAttribArray(aPos);
    glDisableVertexAttribArray(aUV);
    glDisableVertexAttribArray(aCol);

    m_verts.clear();
}

void GravityGL::endFrame() {
    flush();
}

void GravityGL::quad(float x, float y, float w, float h, float u0, float v0, float u1, float v1,
                     GColor tl, GColor tr, GColor bl, GColor br, GLuint tex) {
    GLuint targetTex = (tex == 0) ? m_dummyTex : tex;
    if (targetTex != m_curTex) {
        flush();
        m_curTex = targetTex;
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, targetTex);
    }

    float nx0 = nx(x), ny0 = ny(y);
    float nx1 = nx(x + w), ny1 = ny(y + h);

    // Triangle 1
    m_verts.push_back({nx0, ny0, u0, v0, tl.r, tl.g, tl.b, tl.a});
    m_verts.push_back({nx1, ny0, u1, v0, tr.r, tr.g, tr.b, tr.a});
    m_verts.push_back({nx0, ny1, u0, v1, bl.r, bl.g, bl.b, bl.a});

    // Triangle 2
    m_verts.push_back({nx1, ny0, u1, v0, tr.r, tr.g, tr.b, tr.a});
    m_verts.push_back({nx1, ny1, u1, v1, br.r, br.g, br.b, br.a});
    m_verts.push_back({nx0, ny1, u0, v1, bl.r, bl.g, bl.b, bl.a});
}

void GravityGL::rect(float x, float y, float w, float h, GColor c) {
    quad(x, y, w, h, 0, 0, 1, 1, c, c, c, c, 0);
}

void GravityGL::gradH(float x, float y, float w, float h, GColor l, GColor r) {
    quad(x, y, w, h, 0, 0, 1, 1, l, r, l, r, 0);
}

void GravityGL::gradV(float x, float y, float w, float h, GColor t, GColor b) {
    quad(x, y, w, h, 0, 0, 1, 1, t, t, b, b, 0);
}

void GravityGL::roundRect(float x, float y, float w, float h, float r, GColor c) {
    // Basic implementation (rect with circles at corners)
    rect(x + r, y, w - 2*r, h, c);
    rect(x, y + r, r, h - 2*r, c);
    rect(x + w - r, y + r, r, h - 2*r, c);
    circleFill(x + r, y + r, r, c);
    circleFill(x + w - r, y + r, r, c);
    circleFill(x + r, y + h - r, r, c);
    circleFill(x + w - r, y + h - r, r, c);
}

void GravityGL::roundBorder(float x, float y, float w, float h, float r, float th, GColor c) {
    // Top, bottom, left, right edges
    rect(x + r, y, w - 2*r, th, c); // top
    rect(x + r, y + h - th, w - 2*r, th, c); // bottom
    rect(x, y + r, th, h - 2*r, c); // left
    rect(x + w - th, y + r, th, h - 2*r, c); // right

    // Corners (90 degree arcs)
    arcOutline(x + r, y + r, r, th, M_PI, M_PI*1.5f, c); // top-left
    arcOutline(x + w - r, y + r, r, th, M_PI*1.5f, M_PI*2.f, c); // top-right
    arcOutline(x + r, y + h - r, r, th, M_PI*0.5f, M_PI, c); // bottom-left
    arcOutline(x + w - r, y + h - r, r, th, 0.f, M_PI*0.5f, c); // bottom-right
}

void GravityGL::line(float x0, float y0, float x1, float y1, float th, GColor c) {
    float dx = x1 - x0, dy = y1 - y0;
    float len = sqrtf(dx*dx + dy*dy);
    if (len == 0.f) return;
    float nx = -dy / len * th * 0.5f;
    float ny = dx / len * th * 0.5f;

    // Draw as a quad
    float u0 = 0, v0 = 0, u1 = 1, v1 = 1;
    if (m_curTex != m_dummyTex) {
        flush();
        m_curTex = m_dummyTex;
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_dummyTex);
    }
    
    m_verts.push_back({this->nx(x0 + nx), this->ny(y0 + ny), u0, v0, c.r, c.g, c.b, c.a});
    m_verts.push_back({this->nx(x1 + nx), this->ny(y1 + ny), u1, v0, c.r, c.g, c.b, c.a});
    m_verts.push_back({this->nx(x0 - nx), this->ny(y0 - ny), u0, v1, c.r, c.g, c.b, c.a});

    m_verts.push_back({this->nx(x1 + nx), this->ny(y1 + ny), u1, v0, c.r, c.g, c.b, c.a});
    m_verts.push_back({this->nx(x1 - nx), this->ny(y1 - ny), u1, v1, c.r, c.g, c.b, c.a});
    m_verts.push_back({this->nx(x0 - nx), this->ny(y0 - ny), u0, v1, c.r, c.g, c.b, c.a});
}

void GravityGL::circleFill(float cx, float cy, float r, GColor c, int s) {
    if (m_curTex != m_dummyTex) {
        flush();
        m_curTex = m_dummyTex;
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_dummyTex);
    }
    float ncx = nx(cx), ncy = ny(cy);
    for (int i = 0; i < s; i++) {
        float a0 = (float)i / s * 2.f * M_PI;
        float a1 = (float)(i+1) / s * 2.f * M_PI;
        m_verts.push_back({ncx, ncy, 0, 0, c.r, c.g, c.b, c.a});
        m_verts.push_back({nx(cx + cosf(a0)*r), ny(cy + sinf(a0)*r), 0, 0, c.r, c.g, c.b, c.a});
        m_verts.push_back({nx(cx + cosf(a1)*r), ny(cy + sinf(a1)*r), 0, 0, c.r, c.g, c.b, c.a});
    }
}

void GravityGL::circleOutline(float cx, float cy, float r, float th, GColor c, int s) {
    if (m_curTex != m_dummyTex) {
        flush();
        m_curTex = m_dummyTex;
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_dummyTex);
    }
    float r1 = r - th*0.5f, r2 = r + th*0.5f;
    for (int i = 0; i < s; i++) {
        float a0 = (float)i / s * 2.f * M_PI;
        float a1 = (float)(i+1) / s * 2.f * M_PI;
        
        float x0i = cx + cosf(a0)*r1, y0i = cy + sinf(a0)*r1;
        float x0o = cx + cosf(a0)*r2, y0o = cy + sinf(a0)*r2;
        float x1i = cx + cosf(a1)*r1, y1i = cy + sinf(a1)*r1;
        float x1o = cx + cosf(a1)*r2, y1o = cy + sinf(a1)*r2;

        m_verts.push_back({nx(x0i), ny(y0i), 0, 0, c.r, c.g, c.b, c.a});
        m_verts.push_back({nx(x0o), ny(y0o), 0, 0, c.r, c.g, c.b, c.a});
        m_verts.push_back({nx(x1i), ny(y1i), 0, 0, c.r, c.g, c.b, c.a});

        m_verts.push_back({nx(x0o), ny(y0o), 0, 0, c.r, c.g, c.b, c.a});
        m_verts.push_back({nx(x1o), ny(y1o), 0, 0, c.r, c.g, c.b, c.a});
        m_verts.push_back({nx(x1i), ny(y1i), 0, 0, c.r, c.g, c.b, c.a});
    }
}

void GravityGL::arcOutline(float cx, float cy, float r, float th, float startAngle, float endAngle, GColor c, int s) {
    if (m_curTex != m_dummyTex) {
        flush();
        m_curTex = m_dummyTex;
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_dummyTex);
    }
    float r1 = r - th*0.5f, r2 = r + th*0.5f;
    float range = endAngle - startAngle;
    for (int i = 0; i < s; i++) {
        float a0 = startAngle + (float)i / s * range;
        float a1 = startAngle + (float)(i+1) / s * range;
        
        float x0i = cx + cosf(a0)*r1, y0i = cy + sinf(a0)*r1;
        float x0o = cx + cosf(a0)*r2, y0o = cy + sinf(a0)*r2;
        float x1i = cx + cosf(a1)*r1, y1i = cy + sinf(a1)*r1;
        float x1o = cx + cosf(a1)*r2, y1o = cy + sinf(a1)*r2;

        m_verts.push_back({nx(x0i), ny(y0i), 0, 0, c.r, c.g, c.b, c.a});
        m_verts.push_back({nx(x0o), ny(y0o), 0, 0, c.r, c.g, c.b, c.a});
        m_verts.push_back({nx(x1i), ny(y1i), 0, 0, c.r, c.g, c.b, c.a});

        m_verts.push_back({nx(x0o), ny(y0o), 0, 0, c.r, c.g, c.b, c.a});
        m_verts.push_back({nx(x1o), ny(y1o), 0, 0, c.r, c.g, c.b, c.a});
        m_verts.push_back({nx(x1i), ny(y1i), 0, 0, c.r, c.g, c.b, c.a});
    }
}

void GravityGL::glow(float x, float y, float w, float h, float sp, GColor c) {
    // Pseudo-glow with gradients
    GColor end = c; end.a = 0.f;
    gradH(x - sp, y, sp, h, end, c); // left
    gradH(x + w, y, sp, h, c, end); // right
    gradV(x, y - sp, w, sp, end, c); // top
    gradV(x, y + h, w, sp, c, end); // bottom
}

void GravityGL::text(const char* s, float x, float y, float sc, GColor c, bool shadow) {
    sc *= 0.75f; // Global text zoom reduction for premium feel
    
    if (shadow) {
        GColor shadowCol = GColor::hex(0x000000, c.a * 0.9f);
        text(s, x + 1.5f, y + 1.5f, sc / 0.75f, shadowCol, false); // Pass original sc so it scales properly inside the recursive call
    }
    
    float cx = x;
    int len = strlen(s);
    for (int i = 0; i < len; i++) {
        char ch = s[i];
        if (ch >= 32 && ch <= 126) {
            int idx = ch - 32;
            BakedChar* bc = &m_cdata[idx];
            
            float u0 = (float)bc->x0 / 512.f;
            float v0 = (float)bc->y0 / 512.f;
            float u1 = (float)bc->x1 / 512.f;
            float v1 = (float)bc->y1 / 512.f;
            
            float rx = roundf(cx + bc->xoff * sc);
            float ry = roundf(y + bc->yoff * sc);
            float rw = roundf((bc->x1 - bc->x0) * sc);
            float rh = roundf((bc->y1 - bc->y0) * sc);
            
            quad(rx, ry, rw, rh, u0, v0, u1, v1, c, c, c, c, m_fontTex);
            
            cx += bc->xadvance * sc;
        } else {
            // Unprintable
            cx += 10.0f * sc;
        }
    }
}

float GravityGL::textW(const char* s, float sc) {
    sc *= 0.75f; // Global text zoom reduction
    float w = 0.f;
    int len = strlen(s);
    for (int i = 0; i < len; i++) {
        char ch = s[i];
        if (ch >= 32 && ch <= 126) {
            w += m_cdata[ch - 32].xadvance * sc;
        } else {
            w += 10.0f * sc;
        }
    }
    return w;
}

void GravityGL::panel(float x, float y, float w, float h, float r) {
    roundRect(x, y, w, h, r, P::PANEL());
    roundBorder(x, y, w, h, r, 1.f, P::CYAN().alpha(0.4f));
}

void GravityGL::toggle(float x, float y, bool on, float anim) {
    GColor bg = on ? P::GREEN().alpha(0.8f) : P::DIM().alpha(0.5f);
    roundRect(x, y, 40, 20, 10, bg);
    float kx = x + 2 + (on ? 20 * anim : 0);
    circleFill(kx + 8, y + 10, 8, P::TEXT());
}

void GravityGL::progressBar(float x, float y, float w, float h, float v, GColor fg) {
    rect(x, y, w, h, P::BG());
    rect(x, y, w * v, h, fg);
    roundBorder(x, y, w, h, 2, 1, P::DIM());
}

void GravityGL::separator(float x, float y, float w, GColor c) {
    gradH(x, y, w/2, 1, c.alpha(0), c);
    gradH(x + w/2, y, w/2, 1, c, c.alpha(0));
}

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

GLuint GravityGL::loadRGBA(const uint8_t* data, int w, int h, GLint filter) {
    // If w and h are passed as 0, assume data is a PNG/JPEG buffer and size is in 'h' (hacky but works) or we use a separate method.
    // Let's make it robust:
    GLuint tex = 0;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    
    if (w == 0 && h > 0) {
        // Decode PNG from memory (h = length of buffer)
        int imgW, imgH, channels;
        unsigned char *pixels = stbi_load_from_memory(data, h, &imgW, &imgH, &channels, 4);
        if (pixels) {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, imgW, imgH, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
            stbi_image_free(pixels);
        } else {
            LOGE("Failed to decode image buffer");
        }
    } else {
        // Raw RGBA
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    }
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    return tex;
}

void GravityGL::quadRotated(float cx, float cy, float w, float h, float angle, float u0, float v0, float u1, float v1, GColor c, GLuint tex) {
    GLuint targetTex = (tex == 0) ? m_dummyTex : tex;
    if (targetTex != m_curTex) {
        flush();
        m_curTex = targetTex;
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, targetTex);
    }
    


    float hw = w / 2.f;
    float hh = h / 2.f;
    float cA = cosf(angle);
    float sA = sinf(angle);

    auto rotX = [&](float x, float y) { return cx + (x * cA - y * sA); };
    auto rotY = [&](float x, float y) { return cy + (x * sA + y * cA); };

    float x0 = rotX(-hw, -hh), y0 = rotY(-hw, -hh); // tl
    float x1 = rotX(hw, -hh), y1 = rotY(hw, -hh);   // tr
    float x2 = rotX(-hw, hh), y2 = rotY(-hw, hh);   // bl
    float x3 = rotX(hw, hh), y3 = rotY(hw, hh);     // br

    m_verts.push_back({nx(x0), ny(y0), u0, v0, c.r, c.g, c.b, c.a});
    m_verts.push_back({nx(x1), ny(y1), u1, v0, c.r, c.g, c.b, c.a});
    m_verts.push_back({nx(x2), ny(y2), u0, v1, c.r, c.g, c.b, c.a});

    m_verts.push_back({nx(x1), ny(y1), u1, v0, c.r, c.g, c.b, c.a});
    m_verts.push_back({nx(x3), ny(y3), u1, v1, c.r, c.g, c.b, c.a});
    m_verts.push_back({nx(x2), ny(y2), u0, v1, c.r, c.g, c.b, c.a});
}

void GravityGL::sphere3D(float cx, float cy, float radius, float angleY, GColor tint, GLuint tex) {
    GLuint targetTex = (tex == 0) ? m_dummyTex : tex;
    if (targetTex != m_curTex) {
        flush();
        m_curTex = targetTex;
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, targetTex);
    }
    
    int lats = 20;
    int longs = 20;
    
    for (int i = 0; i < lats; i++) {
        float lat0 = M_PI * (-0.5f + (float)(i) / lats);
        float z0  = sinf(lat0);
        float zr0 =  cosf(lat0);
        
        float lat1 = M_PI * (-0.5f + (float)(i+1) / lats);
        float z1 = sinf(lat1);
        float zr1 = cosf(lat1);
        
        for (int j = 0; j < longs; j++) {
            float lng0 = 2 * M_PI * (float)(j) / longs;
            float lng1 = 2 * M_PI * (float)(j+1) / longs;
            
            // Texture coordinates
            float u0 = (float)j / longs;
            float u1 = (float)(j+1) / longs;
            float v0 = (float)i / lats;
            float v1 = (float)(i+1) / lats;
            
            // Apply rotation Y to longitude angles
            float rotated_lng0 = lng0 + angleY;
            float rotated_lng1 = lng1 + angleY;
            
            float x00 = cosf(rotated_lng0) * zr0;
            float y00 = sinf(rotated_lng0) * zr0;
            
            float x10 = cosf(rotated_lng1) * zr0;
            float y10 = sinf(rotated_lng1) * zr0;
            
            float x01 = cosf(rotated_lng0) * zr1;
            float y01 = sinf(rotated_lng0) * zr1;
            
            float x11 = cosf(rotated_lng1) * zr1;
            float y11 = sinf(rotated_lng1) * zr1;
            
            // Only draw front facing polygons (simple backface culling based on Z)
            // Z is y00, y10, etc in our rotation (since we rotate around Y axis, let's treat Y as depth here)
            // Wait, standard: X = cos(lng)*cos(lat), Y = sin(lat), Z = sin(lng)*cos(lat)
            // Let's re-map:
            auto getPt = [&](float lng, float z, float zr) {
                float rx = cosf(lng + angleY) * zr;
                float ry = z;
                float rz = sinf(lng + angleY) * zr;
                return std::make_tuple(rx, ry, rz);
            };
            
            auto p00 = getPt(lng0, z0, zr0);
            auto p10 = getPt(lng1, z0, zr0);
            auto p01 = getPt(lng0, z1, zr1);
            auto p11 = getPt(lng1, z1, zr1);
            
            // If average Z < 0, it's on the back of the sphere, don't draw it!
            float avgZ = (std::get<2>(p00) + std::get<2>(p10) + std::get<2>(p01) + std::get<2>(p11)) / 4.f;
            if (avgZ < 0) continue;
            
            // Project to 2D (orthographic)
            float sx00 = cx + std::get<0>(p00) * radius; float sy00 = cy - std::get<1>(p00) * radius;
            float sx10 = cx + std::get<0>(p10) * radius; float sy10 = cy - std::get<1>(p10) * radius;
            float sx01 = cx + std::get<0>(p01) * radius; float sy01 = cy - std::get<1>(p01) * radius;
            float sx11 = cx + std::get<0>(p11) * radius; float sy11 = cy - std::get<1>(p11) * radius;
            
            // Draw quad
            m_verts.push_back({nx(sx00), ny(sy00), u0, v0, tint.r, tint.g, tint.b, tint.a});
            m_verts.push_back({nx(sx10), ny(sy10), u1, v0, tint.r, tint.g, tint.b, tint.a});
            m_verts.push_back({nx(sx01), ny(sy01), u0, v1, tint.r, tint.g, tint.b, tint.a});
            
            m_verts.push_back({nx(sx10), ny(sy10), u1, v0, tint.r, tint.g, tint.b, tint.a});
            m_verts.push_back({nx(sx11), ny(sy11), u1, v1, tint.r, tint.g, tint.b, tint.a});
            m_verts.push_back({nx(sx01), ny(sy01), u0, v1, tint.r, tint.g, tint.b, tint.a});
        }
    }
}
