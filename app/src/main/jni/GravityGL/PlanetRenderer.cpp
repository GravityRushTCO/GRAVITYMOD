#include "PlanetRenderer.h"
#include <android/log.h>

#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, "PlanetRnd", __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, "PlanetRnd", __VA_ARGS__)

static const char* planetVertexShaderSource = 
    "#version 300 es\n"
    "layout(location = 0) in vec2 aPos;\n"
    "void main() {\n"
    "    gl_Position = vec4(aPos, 0.0, 1.0);\n"
    "}\n";

static const char* planetFragmentShaderSource = 
    "#version 300 es\n"
    "precision highp float;\n"
    "out vec4 FragColor;\n"
    "uniform vec2 u_resolution;\n"
    "uniform float u_time;\n"
    "\n"
    "mat2 rot(float a) {\n"
    "    float s = sin(a), c = cos(a);\n"
    "    return mat2(c, -s, s, c);\n"
    "}\n"
    "\n"
    "// 3D Noise for planet surface\n"
    "float noise(vec3 p) {\n"
    "    vec3 i = floor(p);\n"
    "    vec3 f = fract(p);\n"
    "    f = f*f*(3.0-2.0*f);\n"
    "    \n"
    "    vec2 uv = (i.xy + vec2(37.0, 17.0)*i.z) + f.xy;\n"
    "    vec2 rg = fract(sin((floor(uv) + vec2(0.0,1.0))*1e-3)*1e5);\n"
    "    return mix(rg.x, rg.y, f.y);\n"
    "}\n"
    "\n"
    "float fbm(vec3 p) {\n"
    "    float f = 0.0;\n"
    "    f += 0.5000*noise(p); p = p*2.02;\n"
    "    f += 0.2500*noise(p); p = p*2.03;\n"
    "    f += 0.1250*noise(p); p = p*2.01;\n"
    "    f += 0.0625*noise(p);\n"
    "    return f/0.9375;\n"
    "}\n"
    "\n"
    "void main() {\n"
    "    vec2 uv = (gl_FragCoord.xy - 0.5 * u_resolution.xy) / u_resolution.y;\n"
    "    \n"
    "    // Sphere properties\n"
    "    float r = 0.45; // Radius (smaller to fit comfortably)\n"
    "    float d = length(uv);\n"
    "    \n"
    "    vec4 col = vec4(0.0);\n"
    "    \n"
    "    if (d > r) {\n"
    "        // Completely transparent background, no square\n"
    "        discard;\n"
    "    } else {\n"
    "        // 3D Sphere mapping\n"
    "        float z = sqrt(r*r - d*d);\n"
    "        vec3 p = vec3(uv, z);\n"
    "        vec3 n = normalize(p);\n"
    "        \n"
    "        // Rotate planet\n"
    "        n.xz *= rot(u_time * 0.5);\n"
    "        n.xy *= rot(0.2);\n"
    "        \n"
    "        // Surface generation\n"
    "        float nVal = fbm(n * 5.0);\n"
    "        \n"
    "        // Colors (Neon Cyberpunk/Universal)\n"
    "        vec3 ocean = vec3(0.05, 0.0, 0.2);\n"
    "        vec3 land = vec3(0.2, 0.8, 1.0) * nVal;\n"
    "        vec3 surface = mix(ocean, land, smoothstep(0.4, 0.6, nVal));\n"
    "        \n"
    "        // Lighting\n"
    "        vec3 lightDir = normalize(vec3(1.0, 1.0, 1.0));\n"
    "        float diff = max(dot(normalize(p), lightDir), 0.0);\n"
    "        surface *= diff + 0.2; // Ambient\n"
    "        \n"
    "        // Ring (Equator wireframe/glow)\n"
    "        float ringDist = abs(n.y);\n"
    "        float ringAlpha = smoothstep(0.05, 0.0, ringDist);\n"
    "        surface += vec3(1.0, 0.2, 0.8) * ringAlpha * (sin(u_time * 5.0 - n.x*10.0)*0.5+0.5);\n"
    "        \n"
    "        // Anti-aliasing edge\n"
    "        float alpha = smoothstep(r, r - 0.01, d);\n"
    "        col = vec4(surface, alpha);\n"
    "    }\n"
    "    \n"
    "    FragColor = col;\n"
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

PlanetRenderer::PlanetRenderer() {}
PlanetRenderer::~PlanetRenderer() {
    if (m_fbo) glDeleteFramebuffers(1, &m_fbo);
    if (m_fboTexture) glDeleteTextures(1, &m_fboTexture);
    if (m_vao) glDeleteVertexArrays(1, &m_vao);
    if (m_vbo) glDeleteBuffers(1, &m_vbo);
    if (m_shaderProgram) glDeleteProgram(m_shaderProgram);
}

void PlanetRenderer::init(int width, int height) {
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
        LOGE("Planet Framebuffer is not complete!");
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    
    m_initialized = true;
}

void PlanetRenderer::setupShaders() {
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &planetVertexShaderSource, NULL);
    glCompileShader(vertexShader);

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &planetFragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    m_shaderProgram = glCreateProgram();
    glAttachShader(m_shaderProgram, vertexShader);
    glAttachShader(m_shaderProgram, fragmentShader);
    glLinkProgram(m_shaderProgram);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
}

void PlanetRenderer::setupQuad() {
    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);

    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
}

void PlanetRenderer::render(float time) {
    if (!m_initialized) return;

    GLint last_fbo;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &last_fbo);
    GLint last_viewport[4];
    glGetIntegerv(GL_VIEWPORT, last_viewport);

    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
    glViewport(0, 0, m_width, m_height);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(m_shaderProgram);
    glUniform2f(glGetUniformLocation(m_shaderProgram, "u_resolution"), (float)m_width, (float)m_height);
    glUniform1f(glGetUniformLocation(m_shaderProgram, "u_time"), time);

    // Enable blending for transparency
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glBindVertexArray(m_vao);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);

    glBindFramebuffer(GL_FRAMEBUFFER, last_fbo);
    glViewport(last_viewport[0], last_viewport[1], last_viewport[2], last_viewport[3]);
}
