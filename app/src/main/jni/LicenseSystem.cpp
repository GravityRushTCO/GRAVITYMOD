#include "LicenseSystem.h"
#include "HttpUtils.h"
#include <android/log.h>
#include <fstream>
#include <thread>
#include <mutex>
#include <chrono>
#include "json.hpp"

#define LOG_TAG "GravityLicense"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

using json = nlohmann::json;

static const std::string SUPABASE_URL = "https://prywroemrjeidhvcxgrr.supabase.co";
static const std::string SUPABASE_ANON_KEY = "sb_publishable_QxuT8DZIxauXJwligR5WIA_aVknjmdD";

__attribute__((weak)) void ImGuiMenu_ProcessRemoteCommands(const std::string& response) {
    // Stub pour libPayload.so. Sera écrasé par ImGuiMenu.cpp dans libCore.so
}

namespace License {

    static std::atomic<State> g_State{State::PENDING};
    static std::string g_HWID = "";
    static std::string g_SavePath = "";
    static std::string g_StatusMessage = "Vérification de l'autorisation de l'appareil...";
    static float g_Progress = 0.0f;
    static std::mutex g_Mutex;
    static std::atomic<bool> g_Unlocked{false};

    void SetStatus(State s, const std::string& msg) {
        std::lock_guard<std::mutex> lock(g_Mutex);
        g_State = s;
        g_StatusMessage = msg;
    }

    void SaveKey(const std::string& key) {
        if (g_SavePath.empty()) return;
        std::ofstream out(g_SavePath);
        if (out.is_open()) {
            out << key;
            out.close();
        }
    }

    std::string LoadKey() {
        if (g_SavePath.empty()) return "";
        std::ifstream in(g_SavePath);
        std::string key;
        if (in.is_open()) {
            std::getline(in, key);
            in.close();
        }
        return key;
    }

    State GetState() {
        return g_State.load();
    }

    bool IsLocked() {
        return !g_Unlocked.load();
    }

    void SetUnlocked(bool unlocked) {
        g_Unlocked.store(unlocked);
    }

    const char* GetStatusMessage() {
        std::lock_guard<std::mutex> lock(g_Mutex);
        return g_StatusMessage.c_str();
    }

    float GetCheckingProgress() {
        return g_Progress;
    }

    void SubmitKey(const std::string& key) {
        // Non utilisé dans le mode auto-HWID
    }

    static void StartPollingLoop() {
        std::thread([]() {
            static std::atomic<bool> s_licenseInFlight{false};
            static std::atomic<bool> s_cmdInFlight{false};
            static int s_networkFailCount = 0;

            while (true) {
                // Poll license status — une seule requête à la fois
                if (!g_Unlocked.load() && !s_licenseInFlight.exchange(true)) {
                    std::string url = SUPABASE_URL + "/rest/v1/rpc/verify_license";

                    json bodyJson;
                    bodyJson["p_key"]  = g_HWID;
                    bodyJson["p_hwid"] = g_HWID;
                    std::string jsonBody = bodyJson.dump();

                    // Note: headers Supabase auto-injectés par DoHttpRequest pour toute URL supabase.co
                    HttpUtils::PostAsync(url, jsonBody, [](const std::string& response) {
                        s_licenseInFlight.store(false);

                        if (response.empty()) {
                            s_networkFailCount++;
                            if (s_networkFailCount > 2) {
                                SetStatus(State::CHECKING, "Connexion perdue avec Gravity...");
                                g_Unlocked.store(false); // VERROUILLE le menu en cas de perte Wi-Fi
                            }
                            return;
                        }

                        s_networkFailCount = 0;

                        json resJson = json::parse(response, nullptr, false);
                        if (resJson.is_discarded()) {
                            SetStatus(State::INVALID, "Réponse serveur corrompue.");
                            LOGE("Supabase JSON parse error: %s", response.c_str());
                            return;
                        }

                        std::string status  = resJson.value("status",  "");
                        std::string message = resJson.value("message", "");

                        if (status == "valid") {
                            SetStatus(State::VALID, message.empty() ? "Bienvenue — Gravity VIP actif" : message);
                            g_Unlocked.store(true); // DÉVERROUILLE le menu
                            LOGI("License: VALID — menu deverouillé");
                        } else if (status == "banned") {
                            SetStatus(State::BANNED, message.empty() ? "APPAREIL BANNI, merci de renouveler votre abonnement, si vous recevez ce message suite a une tentative de modification de l'apk votre appareil ne sera jamais debanni." : message);
                            g_Unlocked.store(false);
                            LOGI("License: BANNED");
                        } else {
                            SetStatus(State::INVALID, message.empty() ? "APPAREIL NON AUTORISÉ — Contactez @Gravity_TCO" : message);
                            g_Unlocked.store(false);
                            LOGI("License: INVALID (status=%s)", status.c_str());
                        }
                    });
                }

                // Poll player_commands — une seule requête à la fois
                if (g_Unlocked.load() && !s_cmdInFlight.exchange(true)) {
                    std::string cmdUrl = SUPABASE_URL
                        + "/rest/v1/player_commands?hwid=eq." + g_HWID + "&ack=eq.false";
                    HttpUtils::GetAsync(cmdUrl, [](const std::string& response) {
                        s_cmdInFlight.store(false);
                        if (!response.empty())
                            ::ImGuiMenu_ProcessRemoteCommands(response);
                    });
                } else if (!g_Unlocked.load()) {
                    s_cmdInFlight.store(false);
                }

                std::this_thread::sleep_for(std::chrono::seconds(3));
            }
        }).detach();
    }

    void Init(const std::string& appDataDir, const std::string& hwid) {
        g_HWID = hwid;
        g_SavePath = appDataDir + "gravity_license.dat";
        
        SetStatus(State::CHECKING, "Authentification de l'appareil...");
        
        // On laisse la boucle de polling appeler verify_license (RPC)
        // C'est la fonction SQL qui va se charger de l'enregistrement initial.
        StartPollingLoop();
    }

} // namespace License
