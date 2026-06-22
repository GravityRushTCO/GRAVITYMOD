#include "LicenseSystem.h"
#include "BuildVersion.h"
#include "HttpUtils.h"
#include "json.hpp"
#include <android/log.h>
#include <chrono>
#include <fstream>
#include <mutex>
#include <thread>


#include "Includes/obfuscate.h"

#define LOG_TAG "GravityLicense"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

using json = nlohmann::json;

// Obfuscated Supabase endpoint: "https://prywroemrjeidhvcxgrr.supabase.co"
static const std::string SUPABASE_URL =
    (const char *)OBFUSCATE("https://prywroemrjeidhvcxgrr.supabase.co");

__attribute__((weak)) void
ImGuiMenu_ProcessRemoteCommands(const std::string &response) {
  // Stub pour libPayload.so. Sera écrasé par ImGuiMenu.cpp dans libCore.so
}

namespace License {
// Obfuscated Supabase key
const std::string SUPABASE_ANON_KEY =
    (const char *)OBFUSCATE("sb_publishable_QxuT8DZIxauXJwligR5WIA_aVknjmdD");

static std::atomic<State> g_State{State::PENDING};
static std::string g_HWID = "";
static std::string g_SavePath = "";
static std::string g_StatusMessage =
    "Vérification de l'autorisation de l'appareil...";
static float g_Progress = 0.0f;
static std::mutex g_Mutex;
static std::atomic<bool> g_Unlocked{false};
static std::string g_ExpirationDate = "Jamais";
static std::atomic<bool> g_NewVersionAvailable{false};
static std::string g_LatestVersionStr = "";

bool IsNewVersionAvailable() { return g_NewVersionAvailable.load(); }

const char *GetLatestVersion() {
  std::lock_guard<std::mutex> lock(g_Mutex);
  static char buffer[128];
  strncpy(buffer, g_LatestVersionStr.c_str(), sizeof(buffer) - 1);
  buffer[sizeof(buffer) - 1] = '\0';
  return buffer;
}

void SetStatus(State s, const std::string &msg) {
  std::lock_guard<std::mutex> lock(g_Mutex);
  g_State = s;
  g_StatusMessage = msg;
}

void SetExpirationDate(const std::string &dateStr) {
  std::lock_guard<std::mutex> lock(g_Mutex);
  g_ExpirationDate = dateStr;
}

const char *GetExpirationDate() {
  std::lock_guard<std::mutex> lock(g_Mutex);
  static char buffer[128];
  strncpy(buffer, g_ExpirationDate.c_str(), sizeof(buffer) - 1);
  buffer[sizeof(buffer) - 1] = '\0';
  return buffer;
}

std::string GetRemainingTime() {
  std::lock_guard<std::mutex> lock(g_Mutex);
  std::string expiresStr = g_ExpirationDate;

  if (expiresStr.empty() || expiresStr == "Jamais" ||
      expiresStr.find("Illimit") != std::string::npos ||
      expiresStr == "Illimité / Inconnu") {
    return "Illimité";
  }

  int year = 0, month = 0, day = 0, hour = 0, minute = 0, second = 0;
  int parsed = sscanf(expiresStr.c_str(), "%d-%d-%d %d:%d:%d", &year, &month,
                      &day, &hour, &minute, &second);
  if (parsed < 3) {
    parsed = sscanf(expiresStr.c_str(), "%d-%d-%dT%d:%d:%d", &year, &month,
                    &day, &hour, &minute, &second);
  }

  if (parsed < 3) {
    return expiresStr;
  }

  std::tm expTime = {};
  expTime.tm_year = year - 1900;
  expTime.tm_mon = month - 1;
  expTime.tm_mday = day;
  expTime.tm_hour = hour;
  expTime.tm_min = minute;
  expTime.tm_sec = second;

  std::time_t expTimeT = timegm(&expTime);
  if (expTimeT == -1) {
    expTimeT = std::mktime(&expTime);
  }

  std::time_t now = std::time(nullptr);
  double diffSeconds = std::difftime(expTimeT, now);

  if (diffSeconds <= 0) {
    return "Expiré";
  }

  int days = (int)(diffSeconds / (24 * 3600));
  int hours = (int)(((int)diffSeconds % (24 * 3600)) / 3600);
  int minutes = (int)(((int)diffSeconds % 3600) / 60);

  std::string res = "";
  if (days > 0) {
    res += std::to_string(days) + "j ";
  }
  if (days > 0 || hours > 0) {
    res += std::to_string(hours) + "h ";
  }
  res += std::to_string(minutes) + "m";
  return res;
}

void SaveKey(const std::string &key) {
  if (g_SavePath.empty())
    return;
  std::ofstream out(g_SavePath);
  if (out.is_open()) {
    out << key;
    out.close();
  }
}

std::string LoadKey() {
  if (g_SavePath.empty())
    return "";
  std::ifstream in(g_SavePath);
  std::string key;
  if (in.is_open()) {
    std::getline(in, key);
    in.close();
  }
  return key;
}

State GetState() { return g_State.load(); }

bool IsLocked() { return !g_Unlocked.load(); }

void SetUnlocked(bool unlocked) { g_Unlocked.store(unlocked); }

const char *GetStatusMessage() {
  std::lock_guard<std::mutex> lock(g_Mutex);
  return g_StatusMessage.c_str();
}

float GetCheckingProgress() { return g_Progress; }

void SubmitKey(const std::string &key) {
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
        std::string url = SUPABASE_URL + (const char *)OBFUSCATE(
                                             "/rest/v1/rpc/verify_license");

        json bodyJson;
        bodyJson["p_key"] = g_HWID;
        bodyJson["p_hwid"] = g_HWID;
        std::string jsonBody = bodyJson.dump();

        // Note: headers Supabase auto-injectés par DoHttpRequest pour toute URL
        // supabase.co
        HttpUtils::PostAsync(url, jsonBody, [](const std::string &response) {
          s_licenseInFlight.store(false);

          if (response.empty()) {
            s_networkFailCount++;
            if (s_networkFailCount > 2) {
              SetStatus(State::OFFLINE,
                        "Connexion perdue avec le serveur de licence");
              g_Unlocked.store(
                  false); // VERROUILLE le menu en cas de perte Wi-Fi
            }
            return;
          }

          s_networkFailCount = 0;

          json resJson = json::parse(response, nullptr, false);
          if (resJson.is_discarded() || resJson.contains("code")) {
            // Supabase error usually contains a "code" or "message" at root if
            // not a valid RPC response
            std::string errMsg =
                resJson.is_discarded()
                    ? response
                    : resJson.value("message", "Erreur serveur");
            SetStatus(State::INVALID, "API Error: " + errMsg);
            LOGE("Supabase HTTP Error: %s", errMsg.c_str());
            return;
          }

          std::string status = resJson.value("status", "");
          std::string message = resJson.value("message", "");
          std::string expires = resJson.value("expires_at", "");

          if (!expires.empty()) {
            SetExpirationDate(expires);
          } else {
            SetExpirationDate("Illimité / Inconnu");
          }

          if (status == "valid") {
            SetStatus(State::VALID, message.empty()
                                        ? "Bienvenue — Gravity VIP actif"
                                        : message);
            g_Unlocked.store(true); // DÉVERROUILLE le menu
            LOGI("License: VALID — menu deverouillé");
          } else if (status == "banned") {
            SetStatus(State::BANNED,
                      message.empty()
                          ? "APPAREIL BANNI, merci de renouveler votre "
                            "abonnement, si vous recevez ce message suite a "
                            "une tentative de modification de l'apk votre "
                            "appareil ne sera jamais debanni."
                          : message);
            g_Unlocked.store(false);
            LOGI("License: BANNED");
          } else if (status == "expired") {
            SetStatus(State::BANNED,
                      message.empty()
                          ? "VOTRE ABONNEMENT EST EXPIRÉ. Contactez "
                            "@Gravity_TCO pour renouveler."
                          : message);
            g_Unlocked.store(false);
            LOGI("License: EXPIRED");
          } else if (status == "pending") {
            SetStatus(State::PENDING, message.empty()
                                          ? "APPAREIL EN ATTENTE D'APPROBATION"
                                          : message);
            g_Unlocked.store(false);
            LOGI("License: PENDING");
          } else {
            SetStatus(State::INVALID,
                      message.empty()
                          ? "APPAREIL NON AUTORISÉ — Contactez @Gravity_TCO"
                          : message);
            g_Unlocked.store(false);
            LOGI("License: INVALID (status=%s)", status.c_str());
          }
        });
      }

      // Poll player_commands — une seule requête à la fois
      if (g_Unlocked.load() && !s_cmdInFlight.exchange(true)) {
        std::string cmdUrl =
            SUPABASE_URL +
            (const char *)OBFUSCATE("/rest/v1/player_commands?hwid=eq.") +
            g_HWID + (const char *)OBFUSCATE("&ack=eq.false");
        HttpUtils::GetAsync(cmdUrl, [](const std::string &response) {
          s_cmdInFlight.store(false);
          if (!response.empty())
            ::ImGuiMenu_ProcessRemoteCommands(response);
        });
      } else if (!g_Unlocked.load()) {
        s_cmdInFlight.store(false);
      }

      // Poll version_check — une fois toutes les 10 boucles (30s)
      if (g_Unlocked.load()) {
        static int versionCheckTicks = 0;
        versionCheckTicks++;
        if (versionCheckTicks >= 10) {
          versionCheckTicks = 0;
          std::string configUrl =
              SUPABASE_URL +
              (const char *)OBFUSCATE(
                  "/rest/v1/app_config?key=eq.version&select=value");
          HttpUtils::GetAsync(configUrl, [](const std::string &response) {
            if (!response.empty()) {
              json configJson = json::parse(response, nullptr, false);
              if (!configJson.is_discarded() && configJson.is_array() &&
                  !configJson.empty()) {
                std::string latestVersion = configJson[0].value("value", "");
                if (!latestVersion.empty() &&
                    latestVersion != GRAVITY_OTA_VERSION) {
                  g_NewVersionAvailable.store(true);
                  std::lock_guard<std::mutex> lock(g_Mutex);
                  g_LatestVersionStr = latestVersion;
                }
              }
            }
          });
        }
      }

      std::this_thread::sleep_for(std::chrono::seconds(3));
    }
  }).detach();
}

void Init(const std::string &appDataDir, const std::string &hwid) {
  g_HWID = hwid;
  g_SavePath = appDataDir + "gravity_license.dat";

  SetStatus(State::CHECKING, "Authentification de l'appareil...");

  // On laisse la boucle de polling appeler verify_license (RPC)
  // C'est la fonction SQL qui va se charger de l'enregistrement initial.
  StartPollingLoop();
}

} // namespace License
