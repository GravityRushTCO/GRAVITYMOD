#include "HexagonBackground.h"
#include <android/log.h>

#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, "HexagonBg", __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, "HexagonBg", __VA_ARGS__)

static const char* bgVertexShaderSource = 
    "#version 300 es\n"
    "layout(location = 0) in vec2 aPos;\n"
    "void main() {\n"
    "    gl_Position = vec4(aPos, 0.0, 1.0);\n"
    "}\n";

static const char* bgFragmentShaderSource = 
    "#version 300 es\n"
    "precision highp float;\n"
    "out vec4 FragColor;\n"
    "uniform vec2 u_resolution;\n"
    "uniform float u_time;\n"
    "uniform vec3 u_touchData;\n"
    "uniform vec3 u_colorA;\n"
    "uniform vec3 u_colorB;\n"
    "\n"
    "float udRoundBox( vec2 p, vec2 b, float r ) {\n"
    "    return length(max(abs(p)-b+r,0.0)) - r;\n"
    "}\n"
    "\n"
    "float hash12(vec2 p) {\n"
    "    vec3 p3  = fract(vec3(p.xyx) * .1031);\n"
    "    p3 += dot(p3, p3.yzx + 33.33);\n"
    "    return fract((p3.x + p3.y) * p3.z);\n"
    "}\n"
    "\n"
    "float animSegment(vec2 p, vec2 a, vec2 b, float timeOffset) {\n"
    "    float localTime = fract(u_time * 0.15 + timeOffset * 0.5);\n" // Slower loop, scaled offsets
    "    float writeProgress = smoothstep(0.0, 0.2, localTime);\n"
    "    float eraseProgress = smoothstep(0.7, 0.9, localTime);\n"
    "    vec2 currentA = mix(a, b, eraseProgress);\n"
    "    vec2 currentB = mix(a, b, writeProgress);\n"
    "    if (writeProgress <= 0.0 || eraseProgress >= 1.0) return 1000.0;\n"
    "    vec2 ba = currentB - currentA;\n"
    "    vec2 pa = p - currentA;\n"
    "    float len = dot(ba,ba);\n"
    "    if (len < 0.0001) return length(pa);\n"
    "    float h = clamp( dot(pa,ba)/len, 0.0, 1.0 );\n"
    "    return length( pa - ba*h );\n"
    "}\n"
    "\n"
    "void main() {\n"
    "    vec2 p = gl_FragCoord.xy - u_resolution.xy * 0.5;\n"
    "    vec2 b = u_resolution.xy * 0.5 - vec2(4.0);\n" 
    "    float r = 24.0;\n"
    "    float d = udRoundBox(p, b, r);\n"
    "    \n"
    "    float t_grad = fract((gl_FragCoord.x / u_resolution.x) - u_time * 0.5);\n"
    "    vec3 neonColor = mix(u_colorA, u_colorB, t_grad);\n"
    "    \n"
    "    vec3 fwCol = vec3(0.0);\n"
    "    if (d < 0.0) {\n"
    "        vec2 cp = (p / u_resolution.y) * 12.0;\n"
    "        \n"
    "        float dWord = 1000.0;\n"
    "        float tOff = 0.0;\n"
    "        #define DRAW(a,b) dWord = min(dWord, animSegment(cp, a, b, tOff)); tOff -= 0.03;\n"
    "        \n"
    "        // G (-4.5)\n"
    "        DRAW(vec2(-4.0, 1.0), vec2(-5.0, 1.0));\n"
    "        DRAW(vec2(-5.0, 1.0), vec2(-5.0, -1.0));\n"
    "        DRAW(vec2(-5.0, -1.0), vec2(-4.0, -1.0));\n"
    "        DRAW(vec2(-4.0, -1.0), vec2(-4.0, 0.0));\n"
    "        DRAW(vec2(-4.0, 0.0), vec2(-4.5, 0.0));\n"
    "        tOff -= 0.05;\n"
    "        // R (-3.0)\n"
    "        DRAW(vec2(-3.5, -1.0), vec2(-3.5, 1.0));\n"
    "        DRAW(vec2(-3.5, 1.0), vec2(-2.5, 1.0));\n"
    "        DRAW(vec2(-2.5, 1.0), vec2(-2.5, 0.0));\n"
    "        DRAW(vec2(-2.5, 0.0), vec2(-3.5, 0.0));\n"
    "        DRAW(vec2(-3.5, 0.0), vec2(-2.5, -1.0));\n"
    "        tOff -= 0.05;\n"
    "        // A (-1.5)\n"
    "        DRAW(vec2(-2.0, -1.0), vec2(-1.5, 1.0));\n"
    "        DRAW(vec2(-1.5, 1.0), vec2(-1.0, -1.0));\n"
    "        DRAW(vec2(-1.8, 0.0), vec2(-1.2, 0.0));\n"
    "        tOff -= 0.05;\n"
    "        // V (0.0)\n"
    "        DRAW(vec2(-0.5, 1.0), vec2(0.0, -1.0));\n"
    "        DRAW(vec2(0.0, -1.0), vec2(0.5, 1.0));\n"
    "        tOff -= 0.05;\n"
    "        // I (1.5)\n"
    "        DRAW(vec2(1.5, 1.0), vec2(1.5, -1.0));\n"
    "        tOff -= 0.05;\n"
    "        // T (3.0)\n"
    "        DRAW(vec2(2.5, 1.0), vec2(3.5, 1.0));\n"
    "        DRAW(vec2(3.0, 1.0), vec2(3.0, -1.0));\n"
    "        tOff -= 0.05;\n"
    "        // Y (4.5)\n"
    "        DRAW(vec2(4.0, 1.0), vec2(4.5, 0.0));\n"
    "        DRAW(vec2(5.0, 1.0), vec2(4.5, 0.0));\n"
    "        DRAW(vec2(4.5, 0.0), vec2(4.5, -1.0));\n"
    "        \n"
    "        float scanCycle = u_time * 0.2;\n"
    "        float scanX = fract(scanCycle) * 30.0 - 15.0;\n"
    "        float state = mod(floor(scanCycle), 2.0);\n"
    "        float textMask = mix(step(cp.x, scanX), step(scanX, cp.x), state);\n"
    "        float laser = smoothstep(0.05, 0.0, abs(cp.x - scanX));\n"
    "        float wordGlow = (smoothstep(0.06, 0.0, dWord) * 1.5 + 0.005 / (dWord + 0.001)) * textMask;\n"
    "        wordGlow += laser * smoothstep(0.4, 0.0, dWord) * 0.2;\n"
    "        vec3 wordCol = mix(u_colorA, u_colorB, sin(u_time * 2.0 - cp.x * 0.5) * 0.5 + 0.5);\n"
    "        fwCol += wordCol * wordGlow * (0.6 + u_touchData.z * 1.0) + (wordCol * laser * 0.05);\n"
    "        \n"
    "        // Floating firefly particles\n"
    "        // Floating firefly particles\n"
    "        for (int i = 0; i < 6; i++) {\n"
    "            float fi = float(i);\n"
    "            vec2 fpos = vec2(sin(u_time*(0.11+fi*0.04)+fi*4.3)*4.5, fract(u_time*(0.09+fi*0.03)+fi*0.618)*8.0-4.0);\n"
    "            fpos.x += sin(fpos.y*2.0+u_time+fi*1.7)*0.7;\n"
    "            float dP = length(cp-fpos);\n"
    "            float tw = sin(u_time*(2.5+fi)+fi*5.0)*0.5+0.5;\n"
    "            fwCol += mix(u_colorA, u_colorB, fract(fi*0.37))*(0.004/(dP*2.0+0.04))*tw;\n"
    "        }\n"
    "        \n"
    "        // Wavy energy border lines\n"
    "        float dLines = 1000.0;\n"
    "        vec2 cpW = vec2(cp.x, cp.y + sin(cp.x*2.5+u_time*4.0)*0.12);\n"
    "        vec2 cpW2 = vec2(cp.x, cp.y + cos(cp.x*1.5-u_time*2.5)*0.18);\n"
    "        dLines = min(dLines, animSegment(cpW,  vec2(-5.5, 3.5),  vec2(0.0,  3.5),   0.0));\n"
    "        dLines = min(dLines, animSegment(cpW,  vec2(0.0,  3.5),  vec2(5.5,  3.5),  -0.1));\n"
    "        dLines = min(dLines, animSegment(cpW2, vec2(-5.5, 3.2),  vec2(5.5,  3.2),  -0.2));\n"
    "        dLines = min(dLines, animSegment(cpW,  vec2(5.5, -3.5),  vec2(0.0, -3.5),  -0.3));\n"
    "        dLines = min(dLines, animSegment(cpW,  vec2(0.0, -3.5),  vec2(-5.5,-3.5),  -0.4));\n"
    "        dLines = min(dLines, animSegment(cpW2, vec2(5.5, -3.2),  vec2(-5.5,-3.2),  -0.5));\n"
    "        float linesGlow = smoothstep(0.025,0.0,dLines)+0.002/(dLines+0.001);\n"
    "        fwCol += wordCol * linesGlow * (0.4 + u_touchData.z * 1.0);\n"
    "    }\n"
    "    \n"
    "    float border = smoothstep(2.0, 0.0, abs(d));\n"
    "    float borderGlow = smoothstep(8.0, 0.0, abs(d));\n"
    "    float fill = smoothstep(1.0, 0.0, d);\n"
    "    \n"
    "    vec3 bg = vec3(0.0, 0.0, 0.0); // PURE BLACK\n"
    "    \n"
    "    vec3 finalColor = neonColor * border * 1.8 + neonColor * 0.9 * borderGlow;\n"
    "    if (d < 0.0) {\n"
    "        finalColor = mix(finalColor, bg + clamp(fwCol, 0.0, 1.5), 1.0);\n"
    "    }\n"
    "    \n"
    "    float alpha = fill * 1.0 + border + borderGlow * 0.6;\n"
    "    FragColor = vec4(finalColor, clamp(alpha, 0.0, 1.0));\n"
    "}\n";

// Full screen quad
static const float quadVertices[] = {
    -1.0f,  1.0f,
    -1.0f, -1.0f,
     1.0f, -1.0f,
    -1.0f,  1.0f,
     1.0f, -1.0f,
     1.0f,  1.0f
};

HexagonBackground::HexagonBackground() {}
HexagonBackground::~HexagonBackground() {
    if (m_fbo) glDeleteFramebuffers(1, &m_fbo);
    if (m_fboTexture) glDeleteTextures(1, &m_fboTexture);
    if (m_vao) glDeleteVertexArrays(1, &m_vao);
    if (m_vbo) glDeleteBuffers(1, &m_vbo);
    if (m_shaderProgram) glDeleteProgram(m_shaderProgram);
}

void HexagonBackground::init(int width, int height) {
    if (m_initialized && m_width == width && m_height == height) return;
    
    m_width = width;
    m_height = height;

    if (!m_initialized) {
        setupShaders();
        setupQuad();
    }

    if (m_fbo == 0) glGenFramebuffers(1, &m_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

    if (m_fboTexture == 0) glGenTextures(1, &m_fboTexture);
    glBindTexture(GL_TEXTURE_2D, m_fboTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_fboTexture, 0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        LOGE("Background Framebuffer is not complete!");
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    
    m_initialized = true;
}

void HexagonBackground::setupShaders() {
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &bgVertexShaderSource, NULL);
    glCompileShader(vertexShader);

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &bgFragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    m_shaderProgram = glCreateProgram();
    glAttachShader(m_shaderProgram, vertexShader);
    glAttachShader(m_shaderProgram, fragmentShader);
    glLinkProgram(m_shaderProgram);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
}

void HexagonBackground::setupQuad() {
    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);

    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
}

void HexagonBackground::render(float time, float touchX, float touchY) {
    if (!m_initialized) return;

    GLint last_fbo;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &last_fbo);
    GLint last_viewport[4];
    glGetIntegerv(GL_VIEWPORT, last_viewport);

    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
    glViewport(0, 0, m_width, m_height);

    static float lastTouchX = -1000.0f;
    static float lastTouchY = -1000.0f;
    static float lastTime = 0.0f;
    static float touchAlpha = 0.0f;
    
    float dt = time - lastTime;
    lastTime = time;
    
    if (touchX >= 0.0f) {
        lastTouchX = touchX;
        lastTouchY = touchY;
        touchAlpha = 1.0f;
    } else {
        touchAlpha -= dt * 0.5f; // Fades out over 2 seconds
        if (touchAlpha < 0.0f) touchAlpha = 0.0f;
    }

    glUseProgram(m_shaderProgram);
    glUniform2f(glGetUniformLocation(m_shaderProgram, "u_resolution"), (float)m_width, (float)m_height);
    glUniform1f(glGetUniformLocation(m_shaderProgram, "u_time"), time);
    glUniform3f(glGetUniformLocation(m_shaderProgram, "u_touchData"), lastTouchX, lastTouchY, touchAlpha);
    
    // Pass dynamic global theme colors
    extern float g_GradColorA[3];
    extern float g_GradColorB[3];
    glUniform3f(glGetUniformLocation(m_shaderProgram, "u_colorA"), g_GradColorA[0], g_GradColorA[1], g_GradColorA[2]);
    glUniform3f(glGetUniformLocation(m_shaderProgram, "u_colorB"), g_GradColorB[0], g_GradColorB[1], g_GradColorB[2]);

    glBindVertexArray(m_vao);
    GLboolean blendEnabled;
    glGetBooleanv(GL_BLEND, &blendEnabled);
    glDisable(GL_BLEND);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    if (blendEnabled) glEnable(GL_BLEND);
    glBindVertexArray(0);

    glBindFramebuffer(GL_FRAMEBUFFER, last_fbo);
    glViewport(last_viewport[0], last_viewport[1], last_viewport[2], last_viewport[3]);
}
