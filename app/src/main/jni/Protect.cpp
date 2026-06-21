#include <android/log.h>
#include <string.h>
#include <sys/ptrace.h>
#include <unistd.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <dirent.h>
#include <fcntl.h>
#include <string>
#include "HttpUtils.h"
#include "GravityServerConfig.h"

#define LOG_TAG "Protect"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

// ─────────────────────────────────────────────────────────────────
//  Zéroïsation sécurisée (anti-optimisation compilateur)
// ─────────────────────────────────────────────────────────────────
void secure_zeroize(void *v, size_t n) {
    volatile unsigned char *p = (volatile unsigned char *)v;
    while (n--) *p++ = 0;
}

// ─────────────────────────────────────────────────────────────────
//  Lockdown : sortie forcée du processus (intraçable)
// ─────────────────────────────────────────────────────────────────
void trigger_lockdown() {
    LOGE("[PROTECT] Kill switch activated — terminating.");
    // Double sortie : signal + exit direct pour contourner les handlers
    raise(SIGKILL);
    _exit(1);
}

// ─────────────────────────────────────────────────────────────────
//  Détection 1 : /proc/self/status → TracerPid
// ─────────────────────────────────────────────────────────────────
static bool is_traced_via_status() {
    char line[256];
    FILE *f = fopen("/proc/self/status", "r");
    if (!f) return false;
    while (fgets(line, sizeof(line), f)) {
        if (strncmp(line, "TracerPid:", 10) == 0) {
            int pid = atoi(line + 10);
            fclose(f);
            return (pid != 0);
        }
    }
    fclose(f);
    return false;
}

// ─────────────────────────────────────────────────────────────────
//  Détection 2 : scan /proc/self/maps pour Frida
// ─────────────────────────────────────────────────────────────────
static bool is_frida_present() {
    const char *FRIDA_SIGS[] = {
        "frida-agent", "linjector", "frida-gadget", "gum-js-loop", nullptr
    };
    char line[512];
    FILE *f = fopen("/proc/self/maps", "r");
    if (!f) return false;
    while (fgets(line, sizeof(line), f)) {
        for (int i = 0; FRIDA_SIGS[i] != nullptr; i++) {
            if (strstr(line, FRIDA_SIGS[i])) { fclose(f); return true; }
        }
    }
    fclose(f);
    return false;
}

// ─────────────────────────────────────────────────────────────────
//  Détection 3 : sockets Frida via /proc/self/fd
// ─────────────────────────────────────────────────────────────────
static bool is_frida_socket_present() {
    char path[256], target[256];
    DIR *dir = opendir("/proc/self/fd");
    if (!dir) return false;
    struct dirent *entry;
    while ((entry = readdir(dir)) != nullptr) {
        snprintf(path, sizeof(path), "/proc/self/fd/%s", entry->d_name);
        ssize_t len = readlink(path, target, sizeof(target) - 1);
        if (len > 0) {
            target[len] = '\0';
            if (strstr(target, "linjector") || strstr(target, "frida")) {
                closedir(dir);
                return true;
            }
        }
    }
    closedir(dir);
    return false;
}

// ─────────────────────────────────────────────────────────────────
//  KILL SWITCH DISTANT — poll /api/killswitch toutes les 30s
//
//  Côté serveur, retourner :
//    { "kill": false }  →  rien
//    { "kill": true }   →  fermeture immédiate
//
//  Tu peux activer ça depuis ton dashboard Lovable en 1 clic.
// ─────────────────────────────────────────────────────────────────
static void on_killswitch_response(const std::string& resp) {
    if (resp.empty()) return;
    // Cherche "kill":true dans la réponse JSON (simple string search, pas de parsing)
    if (resp.find("\"kill\":true") != std::string::npos ||
        resp.find("\"kill\": true") != std::string::npos) {
        LOGE("[PROTECT] Remote kill switch received from server.");
        trigger_lockdown();
    }
}

static void poll_killswitch_once() {
    std::string url = std::string(GRAVITY_SERVER_PUBLIC_URL) + "/api/killswitch";
    HttpUtils::GetAsync(url, on_killswitch_response);
}

// ─────────────────────────────────────────────────────────────────
//  Thread principal de surveillance
// ─────────────────────────────────────────────────────────────────
void *monitor_system(void *arg) {
    int tick = 0;
    while (1) {
        // Anti-debug checks — toutes les 750ms
        if (is_traced_via_status()) {
            LOGI("[PROTECT] Debugger detected via TracerPid.");
            trigger_lockdown();
        }
        if (is_frida_present()) {
            LOGI("[PROTECT] Frida detected via /proc/maps.");
            trigger_lockdown();
        }
        if (is_frida_socket_present()) {
            LOGI("[PROTECT] Frida detected via socket fd.");
            trigger_lockdown();
        }

        // Kill switch distant — toutes les ~30s (40 × 750ms)
        if (tick % 40 == 0) {
            poll_killswitch_once();
        }
        tick++;

        usleep(750000);
    }
    return nullptr;
}

// ─────────────────────────────────────────────────────────────────
//  Initialisation — appeler depuis JNI_OnLoad
// ─────────────────────────────────────────────────────────────────
void InitProtection() {
    // Premier poll immédiat au démarrage
    poll_killswitch_once();

    pthread_t tid;
    pthread_create(&tid, nullptr, monitor_system, nullptr);
    pthread_detach(tid);
    LOGI("[PROTECT] Protection + kill switch thread started.");
}