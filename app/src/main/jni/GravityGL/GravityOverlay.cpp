#include "GravityGL.h"
#include "VipMenu.h"
#include "ImGuiMenu.h"
#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <dlfcn.h>
#include "Dobby/dobby.h"
#include <android/log.h>
#include "../imgui/imgui.h"
#include "../imgui/imgui_impl_opengl3.h"

#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, "GRAVITY_GL", __VA_ARGS__)

#ifndef GL_VERTEX_ARRAY_BINDING
#define GL_VERTEX_ARRAY_BINDING 0x85B5
#endif

typedef void (*PFNGLBINDVERTEXARRAYPROC)(GLuint array);
static PFNGLBINDVERTEXARRAYPROC local_glBindVertexArray = nullptr;
static bool vao_resolved = false;

static void resolve_vao() {
    if (vao_resolved) return;
    local_glBindVertexArray = (PFNGLBINDVERTEXARRAYPROC)eglGetProcAddress("glBindVertexArray");
    if (!local_glBindVertexArray) {
        local_glBindVertexArray = (PFNGLBINDVERTEXARRAYPROC)eglGetProcAddress("glBindVertexArrayOES");
    }
    vao_resolved = true;
}

static EGLBoolean (*orig_eglSwapBuffers)(EGLDisplay dpy, EGLSurface surface);
static bool imgui_initialized = false;

static EGLBoolean hook_eglSwapBuffers(EGLDisplay dpy, EGLSurface surface) {
    EGLint w = 1920, h = 1080;
    eglQuerySurface(dpy, surface, EGL_WIDTH, &w);
    eglQuerySurface(dpy, surface, EGL_HEIGHT, &h);
    
    static int s_swapCount = 0;
    if ((s_swapCount++ % 120) == 0) {
        static int dbgTick = 0;
        if ((dbgTick++ % 120) == 0) {
            LOGI("eglSwapBuffers running! W=%d H=%d", w, h);
        }
    }
    
    // Save GL state to avoid breaking Unity rendering
    GLint last_prog, last_vbo, last_blend_src, last_blend_dst, last_active_tex, last_tex;
    GLint last_viewport[4];
    GLint last_scissor[4];
    
    GLboolean blend_enabled = glIsEnabled(GL_BLEND);
    GLboolean depth_enabled = glIsEnabled(GL_DEPTH_TEST);
    GLboolean cull_enabled = glIsEnabled(GL_CULL_FACE);
    GLboolean scissor_enabled = glIsEnabled(GL_SCISSOR_TEST);
    
    glGetIntegerv(GL_CURRENT_PROGRAM, &last_prog);
    glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &last_vbo);
    glGetIntegerv(GL_BLEND_SRC_RGB, &last_blend_src);
    glGetIntegerv(GL_BLEND_DST_RGB, &last_blend_dst);
    glGetIntegerv(GL_VIEWPORT, last_viewport);
    glGetIntegerv(GL_SCISSOR_BOX, last_scissor);
    glGetIntegerv(GL_ACTIVE_TEXTURE, &last_active_tex);
    
    glActiveTexture(GL_TEXTURE0);
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_tex);
    
    // Isolate VAO
    resolve_vao();
    GLint last_vao = 0;
    if (local_glBindVertexArray) {
        glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &last_vao);
        local_glBindVertexArray(0);
    }
    
    // We must unbind Unity's Element Array Buffer because we use glDrawArrays which might be affected by VAO states
    GLint last_ebo;
    glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING, &last_ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    // Disable Scissor for UI
    glDisable(GL_SCISSOR_TEST);

    // Initialize & render existing GravityGL
    extern void Esp_SetScreenSize(int w, int h);
    Esp_SetScreenSize(w, h);
    g_GL.init(w, h);
    g_GL.beginFrame();
    VipMenu::get().drawEspOverlay();
    VipMenu::get().render();
    g_GL.endFrame();
    
    // === IMGUI INTEGRATION ===
    if (!imgui_initialized) {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.IniFilename = nullptr;
        ImGui::StyleColorsDark();
        
        ImFont* font = io.Fonts->AddFontFromFileTTF("/system/fonts/Roboto-Regular.ttf", 22.0f);
        if (font == nullptr) {
            ImFontConfig cfg;
            cfg.SizePixels = 22.0f;
            io.Fonts->AddFontDefault(&cfg);
        }
        
        ImGui_ImplOpenGL3_Init("#version 300 es");
        imgui_initialized = true;
    }
    
    ImGuiIO& io = ImGui::GetIO();
    io.DisplaySize = ImVec2((float)w, (float)h);
    
    ImGui_ImplOpenGL3_NewFrame();
    ImGui::NewFrame();
    
    // Rendu du nouveau menu 3D
    ImGuiMenu::get().render();
    
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    // ==========================
    
    // Restore GL state
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, last_ebo);
    
    if (local_glBindVertexArray) {
        local_glBindVertexArray(last_vao);
    }
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, last_tex);
    glActiveTexture(last_active_tex);
    
    glBindBuffer(GL_ARRAY_BUFFER, last_vbo);
    glUseProgram(last_prog);
    glBlendFunc(last_blend_src, last_blend_dst);
    glViewport(last_viewport[0], last_viewport[1], last_viewport[2], last_viewport[3]);
    glScissor(last_scissor[0], last_scissor[1], last_scissor[2], last_scissor[3]);
    
    if (!blend_enabled) glDisable(GL_BLEND);
    if (depth_enabled) glEnable(GL_DEPTH_TEST); else glDisable(GL_DEPTH_TEST);
    if (cull_enabled) glEnable(GL_CULL_FACE); else glDisable(GL_CULL_FACE);
    if (scissor_enabled) glEnable(GL_SCISSOR_TEST);

    return orig_eglSwapBuffers(dpy, surface);
}

extern "C" void setupGravityOverlay() {
    static bool overlaySetupDone = false;
    if (overlaySetupDone) return;
    overlaySetupDone = true;
    LOGI("setupGravityOverlay() called");
    void* libEGL = dlopen("libEGL.so", RTLD_LAZY);
    if (libEGL) {
        void* addr = dlsym(libEGL, "eglSwapBuffers");
        if (addr) {
            int hookRet = DobbyHook(addr, (dobby_dummy_func_t)hook_eglSwapBuffers, (dobby_dummy_func_t*)&orig_eglSwapBuffers);
            LOGI("eglSwapBuffers hooked at %p, ret=%d", addr, hookRet);
        } else {
            LOGI("eglSwapBuffers symbol not found");
        }
    } else {
        LOGI("libEGL.so not loaded");
    }
}
