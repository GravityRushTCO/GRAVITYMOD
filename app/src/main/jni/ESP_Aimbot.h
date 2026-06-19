// ESP + AIMBOT COMPLET - OneState RP
// Rendu overlay, Box ESP, Skeleton, Health, Distance, Auto-Aim
#pragma once

#include <jni.h>
#include <android/log.h>
#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <vector>
#include <string>
#include <cmath>
#include <pthread.h>

#define LOG_TAG "ONESTATE_ESP"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

// ==============================================
// STRUCTURES MATH
// ==============================================
struct Vector2 { float x, y; };
struct Vector4 { float x, y, z, w; };

struct Vector3 {
    float x, y, z;
    Vector3() : x(0), y(0), z(0) {}
    Vector3(float _x, float _y, float _z) : x(_x), y(_y), z(_z) {}
    Vector3 operator-(const Vector3& o) const { return Vector3(x-o.x, y-o.y, z-o.z); }
    Vector3 operator+(const Vector3& o) const { return Vector3(x+o.x, y+o.y, z+o.z); }
    Vector3 operator*(float f) const { return Vector3(x*f, y*f, z*f); }
    float Dot(const Vector3& o) const { return x*o.x + y*o.y + z*o.z; }
    float Length() const { return sqrtf(x*x + y*y + z*z); }
    float Distance(const Vector3& o) const { return (*this - o).Length(); }
};

struct Matrix4x4 {
    float m[16];
    Matrix4x4() { memset(m, 0, sizeof(m)); m[0]=m[5]=m[10]=m[15]=1.0f; }
};

// Couleurs RGBA
struct Color {
    float r, g, b, a;
    Color() : r(1), g(1), b(1), a(1) {}
    Color(float _r, float _g, float _b, float _a) : r(_r), g(_g), b(_b), a(_a) {}
};

// ==============================================
// CONFIG ESP
// ==============================================
struct ESPConfig {
    bool bEnabled = false;
    bool bBox = true;           // Boîtes 2D
    bool bName = true;          // Noms
    bool bHealth = true;        // Barre de vie
    bool bDistance = true;      // Distance
    bool bSkeleton = false;     // Squelette
    bool bLine = false;         // Ligne vers l'ennemi
    bool bHead = false;         // Cercle sur la tête
    bool bWeapon = true;        // Arme équipée
    bool bArmor = true;         // Armure
    bool bSnapLines = false;    // Lignes de visée
    
    float fMaxDistance = 500.0f; // Distance max d'affichage
    
    // Couleurs
    Color colBox = Color(1, 0, 0, 1);        // Rouge
    Color colBoxVisible = Color(0, 1, 0, 1);  // Vert (visible)
    Color colName = Color(1, 1, 1, 1);        // Blanc
    Color colHealth = Color(0, 1, 0, 1);      // Vert
    Color colSkeleton = Color(1, 1, 0, 1);    // Jaune
    Color colLine = Color(1, 0.5f, 0, 1);     // Orange
    Color colHead = Color(1, 0, 0, 1);        // Rouge
};

// ==============================================
// CONFIG AIMBOT
// ==============================================
struct AimbotConfig {
    bool bEnabled = false;
    bool bSilentAim = false;     // Silent aim
    bool bTriggerBot = false;    // Tir automatique
    bool bNoSpread = true;       // Pas de dispersion
    bool bAutoFire = false;      // Tir automatique continu
    bool bAutoWall = false;      // Tir à travers les murs
    bool bVisibilityCheck = true;// Vérifier si visible
    
    float fFov = 180.0f;         // Champ de vision
    float fSmooth = 1.0f;        // Lissage (1=instantané)
    int iTargetBone = 0;         // 0=tête, 1=cou, 2=torse, 3=hanche
    int iTargetPriority = 0;     // 0=distance, 1=FOV, 2=santé
    
    float fMaxDistance = 300.0f; // Distance max
};

// ==============================================
// DONNÉES JOUEUR POUR ESP
// ==============================================
struct PlayerData {
    bool bValid = false;
    bool bIsLocal = false;
    bool bIsDead = false;
    bool bVisible = false;
    bool bIsFriend = false;
    
    char szName[64];
    float fHealth;
    float fMaxHealth;
    float fArmor;
    float fMaxArmor;
    float fDistance;
    int iLevel;
    char szWeapon[32];
    
    Vector3 vPosition;
    Vector3 vHeadPos;
    Vector3 vScreenPos;      // Position écran (pieds)
    Vector3 vScreenHead;     // Position écran (tête)
    float fBoxHeight;
    float fBoxWidth;
    
    // Pour squelette
    Vector3 vBones[20];      // Positions os
    Vector3 vScreenBones[20];// Positions écran os
};

// ==============================================
// RENDERER OPENGL ES
// ==============================================
class ESPRenderer {
public:
    static ESPRenderer& Instance() {
        static ESPRenderer inst;
        return inst;
    }
    
    void Initialize();
    void Shutdown();
    void RenderFrame();
    void DrawBox(float x, float y, float w, float h, Color col, float thickness = 2.0f);
    void DrawFilledBox(float x, float y, float w, float h, Color col);
    void DrawLine(float x1, float y1, float x2, float y2, Color col, float thickness = 1.0f);
    void DrawCircle(float x, float y, float radius, Color col, int segments = 32);
    void DrawText(float x, float y, const char* text, Color col, float scale = 1.0f);
    void DrawHealthBar(float x, float y, float w, float h, float health, float maxHealth);
    float GetTextWidth(const char* text, float scale = 1.0f);
    
    int GetScreenWidth() const { return m_iWidth; }
    int GetScreenHeight() const { return m_iHeight; }
    
private:
    ESPRenderer() {}
    void SetupOrtho();
    void RestoreGL();
    
    int m_iWidth = 0, m_iHeight = 0;
    bool m_bInitialized = false;
    GLuint m_textProgram;
    // TODO: Ajouter texture atlas pour les fonts
};

// ==============================================
// WORLD TO SCREEN
// ==============================================
class CameraUtils {
public:
    static CameraUtils& Instance() {
        static CameraUtils inst;
        return inst;
    }
    
    bool WorldToScreen(const Vector3& worldPos, Vector3& screenPos);
    void UpdateViewMatrix();
    Matrix4x4 GetViewMatrix() const { return m_viewMatrix; }
    Matrix4x4 GetProjMatrix() const { return m_projMatrix; }
    Vector3 GetCameraPosition() const { return m_camPos; }
    
private:
    CameraUtils() {}
    Matrix4x4 m_viewMatrix;
    Matrix4x4 m_projMatrix;
    Vector3 m_camPos;
    int m_iWidth, m_iHeight;
};

// ==============================================
// GESTIONNAIRE DE JOUEURS
// ==============================================
class PlayerManager {
public:
    static PlayerManager& Instance() {
        static PlayerManager inst;
        return inst;
    }
    
    void Update();
    std::vector<PlayerData>& GetPlayers() { return m_Players; }
    PlayerData* GetLocalPlayer();
    PlayerData* FindBestTarget(const Vector3& camPos, const Vector3& camForward, float fov, float maxDist);
    
private:
    PlayerManager() {}
    std::vector<PlayerData> m_Players;
    PlayerData m_LocalPlayer;
};

// ==============================================
// HOOKS
// ==============================================
void InstallHooks();
void RemoveHooks();

// Hook eglSwapBuffers pour le rendu ESP
EGLBoolean Hooked_eglSwapBuffers(EGLDisplay dpy, EGLSurface surface);

// Hook pour l'aimbot (appelé dans la boucle de jeu)
void Aimbot_OnUpdate();

// ==============================================
// VARIABLES GLOBALES
// ==============================================
extern ESPConfig g_ESPConfig;
extern AimbotConfig g_AimbotConfig;

// Initialisation
void ESP_SetBaseAddress(uintptr_t base);
