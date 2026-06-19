#include "ModelRenderer.h"
#include <android/log.h>
#include <math.h>

#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, "ModelRenderer", __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, "ModelRenderer", __VA_ARGS__)

static const char* vertexShaderSource = 
    "#version 300 es\n"
    "layout(location = 0) in vec3 aPos;\n"
    "out vec3 FragPos;\n"
    "uniform mat4 model;\n"
    "uniform mat4 view;\n"
    "uniform mat4 projection;\n"
    "void main() {\n"
    "    FragPos = vec3(model * vec4(aPos, 1.0));\n"
    "    gl_Position = projection * view * model * vec4(aPos, 1.0);\n"
    "}\n";

static const char* fragmentShaderSource = 
    "#version 300 es\n"
    "precision mediump float;\n"
    "in vec3 FragPos;\n"
    "out vec4 FragColor;\n"
    "uniform float time;\n"
    "uniform vec3 holoColor;\n"
    "void main() {\n"
    "    // Compute flat face normal using derivatives\n"
    "    vec3 dX = dFdx(FragPos);\n"
    "    vec3 dY = dFdy(FragPos);\n"
    "    vec3 normal = normalize(cross(dX, dY));\n"
    "    \n"
    "    // Fresnel outline glow (viewDir points towards camera, i.e. -Z in view space)\n"
    "    vec3 viewDir = normalize(vec3(0.0, 0.0, 1.0));\n"
    "    float fresnel = 1.0 - max(dot(normal, viewDir), 0.0);\n"
    "    fresnel = pow(fresnel, 2.5) * 1.6;\n"
    "    \n"
    "    // Moving scanline effect\n"
    "    float scanline = sin(FragPos.y * 22.0 - time * 4.0) * 0.5 + 0.5;\n"
    "    scanline = pow(scanline, 4.0) * 0.35;\n"
    "    \n"
    "    // Grid pattern\n"
    "    float grid = sin(FragPos.x * 35.0) * sin(FragPos.z * 35.0) * 0.5 + 0.5;\n"
    "    grid = pow(grid, 3.0) * 0.2;\n"
    "    \n"
    "    float intensity = fresnel + scanline + grid + 0.15;\n"
    "    FragColor = vec4(holoColor * intensity, 0.35 + fresnel * 0.55);\n"
    "}\n";

// Cube vertices (Pos only, 3 floats per vertex)
static const float vertices[] = {
    // Face Avant
    -0.5f, -0.5f, -0.5f,
     0.5f, -0.5f, -0.5f,
     0.5f,  0.5f, -0.5f,
     0.5f,  0.5f, -0.5f,
    -0.5f,  0.5f, -0.5f,
    -0.5f, -0.5f, -0.5f,
    // Face Arrière
    -0.5f, -0.5f,  0.5f,
     0.5f, -0.5f,  0.5f,
     0.5f,  0.5f,  0.5f,
     0.5f,  0.5f,  0.5f,
    -0.5f,  0.5f,  0.5f,
    -0.5f, -0.5f,  0.5f,
    // Face Gauche
    -0.5f,  0.5f,  0.5f,
    -0.5f,  0.5f, -0.5f,
    -0.5f, -0.5f, -0.5f,
    -0.5f, -0.5f, -0.5f,
    -0.5f, -0.5f,  0.5f,
    -0.5f,  0.5f,  0.5f,
    // Face Droite
     0.5f,  0.5f,  0.5f,
     0.5f,  0.5f, -0.5f,
     0.5f, -0.5f, -0.5f,
     0.5f, -0.5f, -0.5f,
     0.5f, -0.5f,  0.5f,
     0.5f,  0.5f,  0.5f,
    // Face Bas
    -0.5f, -0.5f, -0.5f,
     0.5f, -0.5f, -0.5f,
     0.5f, -0.5f,  0.5f,
     0.5f, -0.5f,  0.5f,
    -0.5f, -0.5f,  0.5f,
    -0.5f, -0.5f, -0.5f,
    // Face Haut
    -0.5f,  0.5f, -0.5f,
     0.5f,  0.5f, -0.5f,
     0.5f,  0.5f,  0.5f,
     0.5f,  0.5f,  0.5f,
    -0.5f,  0.5f,  0.5f,
    -0.5f,  0.5f, -0.5f
};

ModelRenderer::ModelRenderer() {}
ModelRenderer::~ModelRenderer() {
    if (m_fbo) glDeleteFramebuffers(1, &m_fbo);
    if (m_fboTexture) glDeleteTextures(1, &m_fboTexture);
    if (m_rbo) glDeleteRenderbuffers(1, &m_rbo);
    if (m_vao) glDeleteVertexArrays(1, &m_vao);
    if (m_vbo) glDeleteBuffers(1, &m_vbo);
    if (m_shaderProgram) glDeleteProgram(m_shaderProgram);
}

void ModelRenderer::init(int width, int height) {
    if (m_initialized && m_width == width && m_height == height) return;
    
    m_width = width;
    m_height = height;

    if (!m_initialized) {
        setupShaders();
        setupBuffers();
    }

    // Setup FBO
    if (m_fbo == 0) glGenFramebuffers(1, &m_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

    // Create texture
    if (m_fboTexture == 0) glGenTextures(1, &m_fboTexture);
    glBindTexture(GL_TEXTURE_2D, m_fboTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_fboTexture, 0);

    // Create depth buffer (Renderbuffer)
    if (m_rbo == 0) glGenRenderbuffers(1, &m_rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, m_rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_rbo);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        LOGE("Framebuffer is not complete!");
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    
    m_initialized = true;
}

void ModelRenderer::setupShaders() {
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    m_shaderProgram = glCreateProgram();
    glAttachShader(m_shaderProgram, vertexShader);
    glAttachShader(m_shaderProgram, fragmentShader);
    glLinkProgram(m_shaderProgram);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
}

void ModelRenderer::setupBuffers() {
    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);

    glBindVertexArray(m_vao);

    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
}

// Simple mat4 helper for projection/view/model
static void makeIdentity(float* m) {
    for (int i=0; i<16; i++) m[i] = (i%5 == 0) ? 1.0f : 0.0f;
}

static void makePerspective(float fov, float aspect, float nearP, float farP, float* m) {
    makeIdentity(m);
    float tanHalfFov = tanf(fov / 2.0f);
    m[0] = 1.0f / (aspect * tanHalfFov);
    m[5] = 1.0f / tanHalfFov;
    m[10] = -(farP + nearP) / (farP - nearP);
    m[11] = -1.0f;
    m[14] = -(2.0f * farP * nearP) / (farP - nearP);
    m[15] = 0.0f;
}

static void rotateY(float angle, float* m) {
    float c = cosf(angle);
    float s = sinf(angle);
    makeIdentity(m);
    m[0] = c; m[2] = s;
    m[8] = -s; m[10] = c;
}

static void translate(float x, float y, float z, float* m) {
    makeIdentity(m);
    m[12] = x; m[13] = y; m[14] = z;
}

static void scale(float x, float y, float z, float* m) {
    makeIdentity(m);
    m[0] = x; m[5] = y; m[10] = z;
}

static void multiply(const float* a, const float* b, float* r) {
    float tmp[16];
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            tmp[j * 4 + i] = 0.0f;
            for (int k = 0; k < 4; k++) {
                tmp[j * 4 + i] += a[k * 4 + i] * b[j * 4 + k];
            }
        }
    }
    for (int i = 0; i < 16; i++) r[i] = tmp[i];
}

void ModelRenderer::render(float time, int modelType, int selectionIndex) {
    if (!m_initialized) return;

    // Save state
    GLint last_fbo;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &last_fbo);
    GLint last_viewport[4];
    glGetIntegerv(GL_VIEWPORT, last_viewport);
    GLboolean depth_enabled = glIsEnabled(GL_DEPTH_TEST);
    GLboolean cull_enabled = glIsEnabled(GL_CULL_FACE);
    GLboolean blend_enabled = glIsEnabled(GL_BLEND);
    GLint blend_src, blend_dst;
    glGetIntegerv(GL_BLEND_SRC_ALPHA, &blend_src);
    glGetIntegerv(GL_BLEND_DST_ALPHA, &blend_dst);

    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
    glViewport(0, 0, m_width, m_height);

    glEnable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Clear background to transparent so hologram floats on the menu
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(m_shaderProgram);

    // Set time uniform
    glUniform1f(glGetUniformLocation(m_shaderProgram, "time"), time);

    // Set hologram color based on category
    float r = 0.0f, g = 0.8f, b = 1.0f; // Default cyan
    if (modelType == MODEL_WEAPON) {
        // Cyan-Blue holo
        r = 0.0f; g = 0.8f; b = 1.0f;
    } else if (modelType == MODEL_VEHICLE) {
        // Purple-Magenta holo
        r = 0.8f; g = 0.0f; b = 1.0f;
    } else if (modelType == MODEL_SKIN) {
        // Pink-Rose holo
        r = 1.0f; g = 0.0f; b = 0.5f;
    }
    glUniform3f(glGetUniformLocation(m_shaderProgram, "holoColor"), r, g, b);

    float projection[16];
    makePerspective(45.0f * 3.14159f / 180.0f, (float)m_width / (float)m_height, 0.1f, 100.0f, projection);
    glUniformMatrix4fv(glGetUniformLocation(m_shaderProgram, "projection"), 1, GL_FALSE, projection);
    
    float view[16];
    translate(0.0f, -0.1f, -2.5f, view);
    glUniformMatrix4fv(glGetUniformLocation(m_shaderProgram, "view"), 1, GL_FALSE, view);

    // Base spin transformation for the whole object
    float baseModel[16];
    rotateY(time * 1.5f, baseModel);

    glBindVertexArray(m_vao);

    auto drawPart = [&](float tx, float ty, float tz, float sx, float sy, float sz) {
        float partModel[16];
        float tMat[16], sMat[16], finalMat[16];
        translate(tx, ty, tz, tMat);
        scale(sx, sy, sz, sMat);
        multiply(tMat, sMat, partModel);
        multiply(baseModel, partModel, finalMat);
        glUniformMatrix4fv(glGetUniformLocation(m_shaderProgram, "model"), 1, GL_FALSE, finalMat);
        glDrawArrays(GL_TRIANGLES, 0, 36);
    };

    if (modelType == MODEL_WEAPON) {
        // Fusil d'assaut / Pistolet
        drawPart(0.0f, 0.0f, 0.0f, 0.5f, 0.12f, 0.08f);       // Corps principal
        drawPart(0.4f, 0.03f, 0.0f, 0.6f, 0.05f, 0.05f);      // Canon
        drawPart(-0.25f, -0.12f, 0.0f, 0.12f, 0.2f, 0.08f);  // Crosse
        drawPart(0.15f, -0.15f, 0.0f, 0.08f, 0.25f, 0.06f);   // Chargeur
        drawPart(0.0f, 0.09f, 0.0f, 0.15f, 0.06f, 0.04f);     // Lunette / Viseur
    }
    else if (modelType == MODEL_VEHICLE) {
        // Voiture
        drawPart(0.0f, -0.05f, 0.0f, 0.85f, 0.18f, 0.42f);     // Châssis
        drawPart(-0.05f, 0.1f, 0.0f, 0.45f, 0.16f, 0.35f);     // Habitacle
        drawPart(0.28f, -0.1f, 0.22f, 0.15f, 0.15f, 0.08f);    // Roue AVG
        drawPart(0.28f, -0.1f, -0.22f, 0.15f, 0.15f, 0.08f);   // Roue AVD
        drawPart(-0.28f, -0.1f, 0.22f, 0.15f, 0.15f, 0.08f);   // Roue ARG
        drawPart(-0.28f, -0.1f, -0.22f, 0.15f, 0.15f, 0.08f);  // Roue ARD
    }
    else if (modelType == MODEL_SKIN) {
        // Androïde humanoïde
        drawPart(0.0f, 0.5f, 0.0f, 0.18f, 0.18f, 0.18f);      // Tête
        drawPart(0.0f, 0.15f, 0.0f, 0.3f, 0.45f, 0.16f);      // Buste
        drawPart(-0.22f, 0.18f, 0.0f, 0.09f, 0.35f, 0.09f);   // Bras Gauche
        drawPart(0.22f, 0.18f, 0.0f, 0.09f, 0.35f, 0.09f);    // Bras Droit
        drawPart(-0.09f, -0.26f, 0.0f, 0.1f, 0.42f, 0.1f);    // Jambe Gauche
        drawPart(0.09f, -0.26f, 0.0f, 0.1f, 0.42f, 0.1f);     // Jambe Droit
    }
    else {
        // Par défaut: Un simple cube de base qui tourne
        glUniformMatrix4fv(glGetUniformLocation(m_shaderProgram, "model"), 1, GL_FALSE, baseModel);
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }

    glBindVertexArray(0);

    // Restore state
    glBindFramebuffer(GL_FRAMEBUFFER, last_fbo);
    glViewport(last_viewport[0], last_viewport[1], last_viewport[2], last_viewport[3]);
    if (!depth_enabled) glDisable(GL_DEPTH_TEST);
    if (!blend_enabled) glDisable(GL_BLEND);
    else glBlendFunc(blend_src, blend_dst);
    if (cull_enabled) glEnable(GL_CULL_FACE);
}
