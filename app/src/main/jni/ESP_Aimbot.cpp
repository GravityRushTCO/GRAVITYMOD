// ESP + AIMBOT - Implémentation principale
#include "ESP_Aimbot.h"
#include "KittyMemory/KittyMemory.hpp"
#include "dobby.h"
#include "il2cpp_bridge.h"
#include <dlfcn.h>
#include <cstring>
#include <algorithm>
#include <string>

// ==============================================
// VARIABLES GLOBALES
// ==============================================
ESPConfig g_ESPConfig;
AimbotConfig g_AimbotConfig;

// Offsets (à vérifier avec le dump Frida)
uintptr_t g_BaseAddress = 0;
static uintptr_t g_GameClient = 0;

void ESP_SetBaseAddress(uintptr_t base) {
    g_BaseAddress = base;
    LOGI("ESP base address set: 0x%lx", base);
}

// EGL function pointers originaux
static EGLBoolean (*orig_eglSwapBuffers)(EGLDisplay, EGLSurface) = nullptr;

// ==============================================
// RENDERER - Implémentation
// ==============================================
void ESPRenderer::Initialize() {
    if (m_bInitialized) return;
    
    // Shaders GLES2 simples pour le rendu 2D
    const char* vertShader = R"(
        attribute vec4 aPosition;
        uniform mat4 uMVPMatrix;
        void main() {
            gl_Position = uMVPMatrix * aPosition;
        }
    )";
    
    const char* fragShader = R"(
        precision mediump float;
        uniform vec4 uColor;
        void main() {
            gl_FragColor = uColor;
        }
    )";
    
    // Compiler les shaders
    auto CompileShader = [](GLenum type, const char* src) -> GLuint {
        GLuint shader = glCreateShader(type);
        glShaderSource(shader, 1, &src, nullptr);
        glCompileShader(shader);
        return shader;
    };
    
    GLuint vs = CompileShader(GL_VERTEX_SHADER, vertShader);
    GLuint fs = CompileShader(GL_FRAGMENT_SHADER, fragShader);
    
    m_textProgram = glCreateProgram();
    glAttachShader(m_textProgram, vs);
    glAttachShader(m_textProgram, fs);
    glLinkProgram(m_textProgram);
    
    glDeleteShader(vs);
    glDeleteShader(fs);
    
    m_bInitialized = true;
    LOGI("ESP Renderer initialized");
}

void ESPRenderer::Shutdown() {
    if (m_textProgram) {
        glDeleteProgram(m_textProgram);
        m_textProgram = 0;
    }
    m_bInitialized = false;
}

void ESPRenderer::SetupOrtho() {
    glGetIntegerv(GL_VIEWPORT, (GLint*)&m_iWidth); // Récupère width
    // HACK: récupère la taille réelle
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
    m_iWidth = viewport[2];
    m_iHeight = viewport[3];
    
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void ESPRenderer::RestoreGL() {
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
}

void ESPRenderer::RenderFrame() {
    if (!g_ESPConfig.bEnabled) return;
    
    SetupOrtho();
    
    auto& players = PlayerManager::Instance().GetPlayers();
    PlayerData* local = PlayerManager::Instance().GetLocalPlayer();
    
    for (auto& p : players) {
        if (!p.bValid || p.bIsLocal || p.bIsDead) continue;
        if (p.fDistance > g_ESPConfig.fMaxDistance) continue;
        
        // Calculer la position écran
        CameraUtils::Instance().WorldToScreen(p.vPosition, p.vScreenPos);
        CameraUtils::Instance().WorldToScreen(p.vHeadPos, p.vScreenHead);
        
        if (p.vScreenPos.z <= 0.0f) continue; // Derrière la caméra
        
        // Hauteur de la boîte
        p.fBoxHeight = p.vScreenPos.y - p.vScreenHead.y;
        p.fBoxWidth = p.fBoxHeight * 0.4f;
        
        float x = p.vScreenHead.x - p.fBoxWidth / 2;
        float y = p.vScreenHead.y;
        float w = p.fBoxWidth;
        float h = p.fBoxHeight;
        
        Color boxCol = p.bVisible ? g_ESPConfig.colBoxVisible : g_ESPConfig.colBox;
        
        // === BOX ESP ===
        if (g_ESPConfig.bBox) {
            DrawBox(x, y, w, h, boxCol, 2.0f);
            // Contour fin
            DrawBox(x-1, y-1, w+2, h+2, Color(0,0,0,0.5f), 1.0f);
        }
        
        // === BARRE DE VIE ===
        if (g_ESPConfig.bHealth) {
            float barX = x - 6.0f;
            float barY = y;
            float barW = 3.0f;
            float barH = h;
            DrawHealthBar(barX, barY, barW, barH, p.fHealth, p.fMaxHealth);
        }
        
        // === BARRE D'ARMURE ===
        if (g_ESPConfig.bArmor && p.fMaxArmor > 0) {
            float barX = x - 10.0f;
            float barY = y;
            float barW = 3.0f;
            float barH = h;
            DrawHealthBar(barX, barY, barW, barH, p.fArmor, p.fMaxArmor);
        }
        
        // === NOM ===
        if (g_ESPConfig.bName) {
            char buf[128];
            snprintf(buf, sizeof(buf), "%s [Lv.%d]", p.szName, p.iLevel);
            float tw = GetTextWidth(buf, 0.8f);
            DrawText(x + w/2 - tw/2, y - 16.0f, buf, g_ESPConfig.colName, 0.8f);
        }
        
        // === DISTANCE ===
        if (g_ESPConfig.bDistance) {
            char buf[32];
            snprintf(buf, sizeof(buf), "%.0fm", p.fDistance);
            DrawText(x + w/2 - GetTextWidth(buf, 0.7f)/2, y + h + 4.0f, buf, Color(0.8f, 0.8f, 0.8f, 1), 0.7f);
        }
        
        // === ARME ===
        if (g_ESPConfig.bWeapon && p.szWeapon[0]) {
            DrawText(x + w/2 - GetTextWidth(p.szWeapon, 0.7f)/2, y + h + 16.0f, p.szWeapon, Color(0.9f, 0.7f, 0.3f, 1), 0.7f);
        }
        
        // === LIGNE VERS L'ENNEMI ===
        if (g_ESPConfig.bLine) {
            float cx = m_iWidth / 2.0f;
            float cy = m_iHeight;
            DrawLine(cx, cy, p.vScreenHead.x, p.vScreenHead.y, g_ESPConfig.colLine, 1.5f);
        }
        
        // === CERCLE SUR LA TÊTE ===
        if (g_ESPConfig.bHead) {
            DrawCircle(p.vScreenHead.x, p.vScreenHead.y, w * 0.6f, g_ESPConfig.colHead, 16);
        }
        
        // === SQUELETTE ===
        if (g_ESPConfig.bSkeleton) {
            // TODO: Implémenter avec les vrais offsets des os
        }
    }
    
    RestoreGL();
}

void ESPRenderer::DrawBox(float x, float y, float w, float h, Color col, float thickness) {
    // Ligne haut
    DrawLine(x, y, x + w, y, col, thickness);
    // Ligne bas
    DrawLine(x, y + h, x + w, y + h, col, thickness);
    // Ligne gauche
    DrawLine(x, y, x, y + h, col, thickness);
    // Ligne droite
    DrawLine(x + w, y, x + w, y + h, col, thickness);
}

void ESPRenderer::DrawFilledBox(float x, float y, float w, float h, Color col) {
    GLfloat vertices[] = {
        x, y,
        x + w, y,
        x, y + h,
        x + w, y + h
    };
    
    glUseProgram(m_textProgram);
    GLint posLoc = glGetAttribLocation(m_textProgram, "aPosition");
    GLint colorLoc = glGetUniformLocation(m_textProgram, "uColor");
    
    // Matrice ortho simple
    GLfloat mvp[16] = {
        2.0f/m_iWidth, 0, 0, 0,
        0, -2.0f/m_iHeight, 0, 0,
        0, 0, 1, 0,
        -1, 1, 0, 1
    };
    
    glUniformMatrix4fv(glGetUniformLocation(m_textProgram, "uMVPMatrix"), 1, GL_FALSE, mvp);
    glUniform4f(colorLoc, col.r, col.g, col.b, col.a * 0.3f);
    
    glVertexAttribPointer(posLoc, 2, GL_FLOAT, GL_FALSE, 0, vertices);
    glEnableVertexAttribArray(posLoc);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glDisableVertexAttribArray(posLoc);
}

void ESPRenderer::DrawLine(float x1, float y1, float x2, float y2, Color col, float thickness) {
    GLfloat vertices[] = { x1, y1, x2, y2 };
    
    glUseProgram(m_textProgram);
    GLint posLoc = glGetAttribLocation(m_textProgram, "aPosition");
    GLint colorLoc = glGetUniformLocation(m_textProgram, "uColor");
    
    GLfloat mvp[16] = {
        2.0f/m_iWidth, 0, 0, 0,
        0, -2.0f/m_iHeight, 0, 0,
        0, 0, 1, 0,
        -1, 1, 0, 1
    };
    
    glUniformMatrix4fv(glGetUniformLocation(m_textProgram, "uMVPMatrix"), 1, GL_FALSE, mvp);
    glUniform4f(colorLoc, col.r, col.g, col.b, col.a);
    glLineWidth(thickness);
    
    glVertexAttribPointer(posLoc, 2, GL_FLOAT, GL_FALSE, 0, vertices);
    glEnableVertexAttribArray(posLoc);
    glDrawArrays(GL_LINES, 0, 2);
    glDisableVertexAttribArray(posLoc);
}

void ESPRenderer::DrawCircle(float x, float y, float radius, Color col, int segments) {
    std::vector<GLfloat> verts;
    for (int i = 0; i <= segments; i++) {
        float angle = 2.0f * M_PI * i / segments;
        verts.push_back(x + radius * cosf(angle));
        verts.push_back(y + radius * sinf(angle));
    }
    
    glUseProgram(m_textProgram);
    GLint posLoc = glGetAttribLocation(m_textProgram, "aPosition");
    GLint colorLoc = glGetUniformLocation(m_textProgram, "uColor");
    
    GLfloat mvp[16] = {
        2.0f/m_iWidth, 0, 0, 0,
        0, -2.0f/m_iHeight, 0, 0,
        0, 0, 1, 0,
        -1, 1, 0, 1
    };
    
    glUniformMatrix4fv(glGetUniformLocation(m_textProgram, "uMVPMatrix"), 1, GL_FALSE, mvp);
    glUniform4f(colorLoc, col.r, col.g, col.b, col.a);
    glLineWidth(1.5f);
    
    glVertexAttribPointer(posLoc, 2, GL_FLOAT, GL_FALSE, 0, verts.data());
    glEnableVertexAttribArray(posLoc);
    glDrawArrays(GL_LINE_LOOP, 0, segments + 1);
    glDisableVertexAttribArray(posLoc);
}

void ESPRenderer::DrawText(float x, float y, const char* text, Color col, float scale) {
    // TODO: Implémenter avec bitmap font
    // Pour l'instant, placeholder
    (void)x; (void)y; (void)text; (void)col; (void)scale;
}

void ESPRenderer::DrawHealthBar(float x, float y, float w, float h, float health, float maxHealth) {
    if (maxHealth <= 0) return;
    
    float ratio = health / maxHealth;
    if (ratio < 0) ratio = 0;
    if (ratio > 1) ratio = 1;
    
    // Fond noir
    DrawFilledBox(x, y, w, h, Color(0, 0, 0, 0.6f));
    
    // Barre de vie (vert -> jaune -> rouge)
    Color healthCol;
    if (ratio > 0.6f)
        healthCol = Color(0, 1, 0, 1); // Vert
    else if (ratio > 0.3f)
        healthCol = Color(1, 1, 0, 1); // Jaune
    else
        healthCol = Color(1, 0, 0, 1); // Rouge
    
    float fillH = h * ratio;
    DrawFilledBox(x, y + h - fillH, w, fillH, healthCol);
    
    // Bordure
    DrawBox(x, y, w, h, Color(0, 0, 0, 0.8f), 1.0f);
}

float ESPRenderer::GetTextWidth(const char* text, float scale) {
    // Approximation: ~8px par caractère
    return strlen(text) * 8.0f * scale;
}

#ifndef GL_MODELVIEW_MATRIX
#define GL_MODELVIEW_MATRIX   0x0BA6
#define GL_PROJECTION_MATRIX  0x0BA7
#endif

void CameraUtils::UpdateViewMatrix() {
    m_iWidth = ESPRenderer::Instance().GetScreenWidth();
    m_iHeight = ESPRenderer::Instance().GetScreenHeight();
    
    if (!g_il2cpp.ready) {
        g_il2cpp.Init();
    }
    if (!g_il2cpp.ready) return;
    
    void* cam = g_il2cpp.GetMainCamera();
    if (!cam) return;
    
    g_il2cpp.GetWorldToCameraMatrix(cam, m_viewMatrix.m);
    g_il2cpp.GetProjectionMatrix(cam, m_projMatrix.m);
    
    float* mv = m_viewMatrix.m;
    m_camPos.x = -(mv[0]*mv[12] + mv[1]*mv[13] + mv[2]*mv[14]);
    m_camPos.y = -(mv[4]*mv[12] + mv[5]*mv[13] + mv[6]*mv[14]);
    m_camPos.z = -(mv[8]*mv[12] + mv[9]*mv[13] + mv[10]*mv[14]);
}

bool CameraUtils::WorldToScreen(const Vector3& worldPos, Vector3& screenPos) {
    // Transformation monde -> écran avec la matrice MVP
    Vector4 clip;
    const float* mv = m_viewMatrix.m;
    const float* mp = m_projMatrix.m;
    
    // MVP = Projection * View * World
    // Simplifié: on combine manuellement
    
    // View transform
    float vx = mv[0]*worldPos.x + mv[4]*worldPos.y + mv[8]*worldPos.z + mv[12];
    float vy = mv[1]*worldPos.x + mv[5]*worldPos.y + mv[9]*worldPos.z + mv[13];
    float vz = mv[2]*worldPos.x + mv[6]*worldPos.y + mv[10]*worldPos.z + mv[14];
    float vw = mv[3]*worldPos.x + mv[7]*worldPos.y + mv[11]*worldPos.z + mv[15];
    
    // Projection transform
    clip.x = mp[0]*vx + mp[4]*vy + mp[8]*vz + mp[12]*vw;
    clip.y = mp[1]*vx + mp[5]*vy + mp[9]*vz + mp[13]*vw;
    clip.z = mp[2]*vx + mp[6]*vy + mp[10]*vz + mp[14]*vw;
    clip.w = mp[3]*vx + mp[7]*vy + mp[11]*vz + mp[15]*vw;
    
    if (clip.w < 0.001f) {
        screenPos.z = -1.0f;
        return false;
    }
    
    // Normalized Device Coordinates -> Screen
    float ndcX = clip.x / clip.w;
    float ndcY = clip.y / clip.w;
    
    screenPos.x = (ndcX + 1.0f) * 0.5f * m_iWidth;
    screenPos.y = (1.0f - ndcY) * 0.5f * m_iHeight;
    screenPos.z = clip.w; // Profondeur pour le tri
    
    return true;
}

// ==============================================
// PLAYER MANAGER - Utilise les offsets de l'Assembly-CSharp
// ==============================================
// Offsets vérifiés depuis Assembly-CSharp / Scellecs.Morpeh
#define OFFSET_GAMECLIENT       0xAB8E1A0  // TODO: trouver GameClient static
#define OFFSET_PLAYERS_FEATURE  0xB8        // _playersFeature dans GameClient
#define OFFSET_PLAYER_FILTER    0x20        // Filter dans PlayersFeature
#define OFFSET_ENTITY_POS       0x60        // Position component
#define OFFSET_ENTITY_HEALTH    0x70        // Health component
#define OFFSET_ENTITY_NAME      0x80        // Name component

void PlayerManager::Update() {
    m_Players.clear();
    if (!g_BaseAddress) return;
    
    // Lire GameClient
    uintptr_t gc = *(uintptr_t*)(g_BaseAddress + OFFSET_GAMECLIENT);
    if (!gc) return;
    
    // Lire PlayersFeature
    uintptr_t pf = *(uintptr_t*)(gc + OFFSET_PLAYERS_FEATURE);
    if (!pf) return;
    
    // Lire le filtre d'entités (Morpeh Filter)
    uintptr_t filter = *(uintptr_t*)(pf + OFFSET_PLAYER_FILTER);
    if (!filter) return;
    
    // Structure Filter Morpeh:
    // +0x00: int32_t worldId
    // +0x04: int32_t typeId  
    // +0x08: int32_t entitiesCount
    // +0x10: EntityId* entities (pointeur vers tableau d'IDs)
    int count = *(int32_t*)(filter + 0x08);
    uintptr_t entities = *(uintptr_t*)(filter + 0x10);
    if (!entities || count <= 0 || count > 1000) return;
    
    // Récupérer le World ECS pour résoudre les entités
    uintptr_t world = *(uintptr_t*)(filter + 0x00);
    
    for (int i = 0; i < count; i++) {
        int32_t entityId = *(int32_t*)(entities + i * 4);
        if (entityId < 0) continue;
        
        // Résoudre l'entité: World.entities[entityId]
        // Structure World: +0x18 = EntityData* entities
        uintptr_t entityData = *(uintptr_t*)(world + 0x18);
        if (!entityData) continue;
        
        // EntityData: +0x00 = archetype, +0x08 = components[]
        // Simplifié: on lit directement les composants à des offsets fixes
        uintptr_t entity = entityData + entityId * 0x100; // taille approximative
        
        PlayerData p;
        p.bValid = true;
        
        // Position (Vector3)
        float* pos = (float*)(entity + OFFSET_ENTITY_POS);
        p.vPosition = Vector3(pos[0], pos[1], pos[2]);
        p.vHeadPos = Vector3(pos[0], pos[1] + 1.8f, pos[2]); // tête ~1.8m au-dessus
        
        // Santé
        p.fHealth = *(float*)(entity + OFFSET_ENTITY_HEALTH);
        p.fMaxHealth = 100.0f;
        p.bIsDead = (p.fHealth <= 0);
        
        // Nom (chaîne UTF-16 ou UTF-8)
        const char* name = (const char*)(entity + OFFSET_ENTITY_NAME);
        strncpy(p.szName, name, 63);
        p.szName[63] = 0;
        
        // Distance à la caméra
        Vector3 camPos = CameraUtils::Instance().GetCameraPosition();
        p.fDistance = camPos.Distance(p.vPosition);
        
        // Vérifier si c'est le joueur local
        p.bIsLocal = false; // TODO: comparer avec l'ID local
        
        m_Players.push_back(p);
    }
    
    LOGI("PlayerManager: %d players found", (int)m_Players.size());
}

PlayerData* PlayerManager::GetLocalPlayer() {
    return &m_LocalPlayer;
}

PlayerData* PlayerManager::FindBestTarget(const Vector3& camPos, const Vector3& camForward, float fov, float maxDist) {
    PlayerData* best = nullptr;
    float bestScore = 999999.0f;
    
    for (auto& p : m_Players) {
        if (!p.bValid || p.bIsLocal || p.bIsDead) continue;
        if (p.fDistance > maxDist) continue;
        
        // Calculer l'angle entre la direction de la caméra et le joueur
        Vector3 dirToPlayer = p.vHeadPos - camPos;
        float dist = dirToPlayer.Length();
        dirToPlayer = dirToPlayer * (1.0f / dist);
        
        float dot = camForward.Dot(dirToPlayer);
        float angle = acosf(dot) * (180.0f / M_PI);
        
        if (angle > fov / 2.0f) continue;
        
        // Score: distance + angle
        float score = dist + angle * 2.0f;
        if (score < bestScore) {
            bestScore = score;
            best = &p;
        }
    }
    
    return best;
}

// ==============================================
// HOOK: eglSwapBuffers
// ==============================================
EGLBoolean Hooked_eglSwapBuffers(EGLDisplay dpy, EGLSurface surface) {
    // Rendre l'ESP avant le swap
    ESPRenderer::Instance().RenderFrame();
    
    // Appeler la fonction originale
    return orig_eglSwapBuffers(dpy, surface);
}

// ==============================================
// AIMBOT UPDATE
// ==============================================
void Aimbot_OnUpdate() {
    if (!g_AimbotConfig.bEnabled) return;
    
    CameraUtils::Instance().UpdateViewMatrix();
    PlayerManager::Instance().Update();
    
    Vector3 camPos = CameraUtils::Instance().GetCameraPosition();
    Vector3 camForward(0, 0, 1); // TODO: Lire la rotation réelle
    
    PlayerData* target = PlayerManager::Instance().FindBestTarget(
        camPos, camForward, 
        g_AimbotConfig.fFov, 
        g_AimbotConfig.fMaxDistance
    );
    
    if (!target) return;
    
    // TODO: Appliquer l'angle de visée
    // Vector3 aimAngle = CalcAngle(camPos, target->vHeadPos);
    // SetCameraAngles(aimAngle);
    
    // Trigger bot
    if (g_AimbotConfig.bTriggerBot) {
        // TODO: Simuler l'appui sur le bouton de tir
    }
}

// ==============================================
// INSTALL / REMOVE HOOKS
// ==============================================
void InstallHooks() {
    LOGI("Installing ESP/Aimbot hooks...");
    
    // Hook eglSwapBuffers via Dobby
    void* egl = dlopen("libEGL.so", RTLD_NOW);
    if (egl) {
        orig_eglSwapBuffers = (decltype(orig_eglSwapBuffers))dlsym(egl, "eglSwapBuffers");
        if (orig_eglSwapBuffers) {
            int ret = DobbyHook((void*)orig_eglSwapBuffers, 
                               (void*)Hooked_eglSwapBuffers, 
                               (void**)&orig_eglSwapBuffers);
            LOGI("DobbyHook eglSwapBuffers ret=%d", ret);
        }
        dlclose(egl);
    }
    
    ESPRenderer::Instance().Initialize();
    LOGI("ESP/Aimbot hooks installed");
}

void RemoveHooks() {
    if (orig_eglSwapBuffers) {
        DobbyDestroy((void*)orig_eglSwapBuffers);
    }
    ESPRenderer::Instance().Shutdown();
    LOGI("ESP/Aimbot hooks removed");
}
