#include "ImGuiMenu.h"
#include "../HttpUtils.h"
#include "../Hwid.h"
#include "../imgui/imgui.h"
#include "../json.hpp"
#include "Catalog.h"
#include "Esp.h"
#include "HexagonBackground.h"
#include "ModelRenderer.h"
#include "PlanetRenderer.h"
#include <atomic>
#include <signal.h>
#include <thread>
#include <unistd.h>

struct TeleportPoint {
  const char *name;
  float x, y, z;
};
#include "ParsedCoordinates.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#include <vector>

void stbi_write_func_callback(void *context, void *data, int size) {
  std::vector<unsigned char> *out = (std::vector<unsigned char> *)context;
  unsigned char *p = (unsigned char *)data;
  out->insert(out->end(), p, p + size);
}

static std::atomic<bool> g_ScreenCaptureBusy(false);

static void OnScreenSent(const std::string &resp) {
  g_ScreenCaptureBusy.store(false);
}

static std::string EncodeBase64(const std::vector<unsigned char> &data) {
  std::string b64 = "data:image/jpeg;base64,";
  static const char *b64_chars =
      "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
  int i = 0, j = 0;
  unsigned char char_array_3[3], char_array_4[4];
  for (size_t x = 0; x < data.size(); x++) {
    char_array_3[i++] = data[x];
    if (i == 3) {
      char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
      char_array_4[1] =
          ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
      char_array_4[2] =
          ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
      char_array_4[3] = char_array_3[2] & 0x3f;
      for (i = 0; i < 4; i++)
        b64 += b64_chars[char_array_4[i]];
      i = 0;
    }
  }
  if (i) {
    for (j = i; j < 3; j++)
      char_array_3[j] = '\0';
    char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
    char_array_4[1] =
        ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
    char_array_4[2] =
        ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
    for (j = 0; j < i + 1; j++)
      b64 += b64_chars[char_array_4[j]];
    while (i++ < 3)
      b64 += '=';
  }
  return b64;
}

void CaptureScreenAndSend() {
  if (g_ScreenCaptureBusy.load())
    return;

  GLint viewport[4];
  glGetIntegerv(GL_VIEWPORT, viewport);
  int width = viewport[2];
  int height = viewport[3];

  static std::vector<unsigned char> pixels;
  if (pixels.size() < (size_t)(width * height * 4)) {
    pixels.resize(width * height * 4);
  }
  glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());

  g_ScreenCaptureBusy.store(true);

  std::vector<unsigned char> *async_pixels =
      new std::vector<unsigned char>(pixels);
  std::string hwid = GetDeviceHWID();

  std::thread([async_pixels, width, height, hwid]() {
    std::vector<unsigned char> jpg_data;
    // Android OpenGL read pixels are bottom-up, stb expects top-down by
    // default, but we can flip it
    stbi_flip_vertically_on_write(1);
    stbi_write_jpg_to_func(stbi_write_func_callback, &jpg_data, width, height,
                           4, async_pixels->data(), 40);
    delete async_pixels;

    std::string b64 = EncodeBase64(jpg_data);

    nlohmann::json payload;
    payload["image"] = b64;
    HttpUtils::PostAsync(HttpUtils::GetServerUrl() +
                             "/api/screen?device=" + hwid,
                         payload.dump(), OnScreenSent);
  }).detach();
}

void Esp_QueueTeleport(float x, float y, float z);

extern "C" {
extern bool g_StickyCarEnabled;
extern bool g_VehicleNoClipEnabled;
}
#include <GLES2/gl2.h>
#include <jni.h>
#include <math.h>
#include <string>

extern JavaVM *g_GravityJVM;

void OpenURL(const char *url) {
  if (!g_GravityJVM)
    return;
  JNIEnv *env = nullptr;
  bool attached = false;
  if (g_GravityJVM->GetEnv((void **)&env, JNI_VERSION_1_6) == JNI_EDETACHED) {
    if (g_GravityJVM->AttachCurrentThread(&env, nullptr) == JNI_OK) {
      attached = true;
    }
  }
  if (!env)
    return;

  jclass activityThreadClass = env->FindClass("android/app/ActivityThread");
  if (activityThreadClass) {
    jmethodID currentAppMethod =
        env->GetStaticMethodID(activityThreadClass, "currentApplication",
                               "()Landroid/app/Application;");
    jobject context =
        env->CallStaticObjectMethod(activityThreadClass, currentAppMethod);
    if (context) {
      jclass intentClass = env->FindClass("android/content/Intent");
      jclass uriClass = env->FindClass("android/net/Uri");
      jmethodID uriParse = env->GetStaticMethodID(
          uriClass, "parse", "(Ljava/lang/String;)Landroid/net/Uri;");
      jmethodID intentInit = env->GetMethodID(
          intentClass, "<init>", "(Ljava/lang/String;Landroid/net/Uri;)V");
      jmethodID addFlags = env->GetMethodID(intentClass, "addFlags",
                                            "(I)Landroid/content/Intent;");
      jclass contextClass = env->FindClass("android/content/Context");
      jmethodID startActivity = env->GetMethodID(contextClass, "startActivity",
                                                 "(Landroid/content/Intent;)V");

      jstring urlString = env->NewStringUTF(url);
      jobject uri = env->CallStaticObjectMethod(uriClass, uriParse, urlString);
      jstring actionString = env->NewStringUTF("android.intent.action.VIEW");

      jobject intent =
          env->NewObject(intentClass, intentInit, actionString, uri);
      env->CallObjectMethod(intent, addFlags,
                            0x10000000); // FLAG_ACTIVITY_NEW_TASK

      env->CallVoidMethod(context, startActivity, intent);

      env->DeleteLocalRef(urlString);
      env->DeleteLocalRef(actionString);
      env->DeleteLocalRef(uri);
      env->DeleteLocalRef(intent);
      env->DeleteLocalRef(context);
    }
    env->DeleteLocalRef(activityThreadClass);
  }

  if (attached) {
    g_GravityJVM->DetachCurrentThread();
  }
}

extern jclass g_DialogHelperClass;

void OpenAndroidKeyboardForChat(const char *url) {
  if (!g_GravityJVM)
    return;
  JNIEnv *env = nullptr;
  bool attached = false;
  if (g_GravityJVM->GetEnv((void **)&env, JNI_VERSION_1_6) == JNI_EDETACHED) {
    if (g_GravityJVM->AttachCurrentThread(&env, nullptr) == JNI_OK)
      attached = true;
  }
  if (!env)
    return;

  jclass activityThreadClass = env->FindClass("android/app/ActivityThread");
  if (env->ExceptionCheck())
    env->ExceptionClear();

  jobject context = nullptr;
  if (activityThreadClass) {
    jmethodID currentAppMethod =
        env->GetStaticMethodID(activityThreadClass, "currentApplication",
                               "()Landroid/app/Application;");
    if (env->ExceptionCheck())
      env->ExceptionClear();
    if (currentAppMethod) {
      context =
          env->CallStaticObjectMethod(activityThreadClass, currentAppMethod);
      if (env->ExceptionCheck())
        env->ExceptionClear();
    }
    env->DeleteLocalRef(activityThreadClass);
  }

  // Essayer d'abord UnityPlayer.currentActivity (qui est sûr à 100% dans
  // OneState)
  jclass unityClass = env->FindClass("com/unity3d/player/UnityPlayer");
  if (env->ExceptionCheck())
    env->ExceptionClear();
  if (unityClass) {
    jfieldID curActField = env->GetStaticFieldID(unityClass, "currentActivity",
                                                 "Landroid/app/Activity;");
    if (env->ExceptionCheck())
      env->ExceptionClear();
    if (curActField) {
      jobject activeAct = env->GetStaticObjectField(unityClass, curActField);
      if (env->ExceptionCheck())
        env->ExceptionClear();
      if (activeAct) {
        if (context)
          env->DeleteLocalRef(context);
        context = activeAct;
      }
    }
    env->DeleteLocalRef(unityClass);
  } else {
    // Fallback sur Main.currentActivity
    jclass mainClass = env->FindClass("com/android/support/Main");
    if (env->ExceptionCheck())
      env->ExceptionClear();
    if (mainClass) {
      jfieldID curActField = env->GetStaticFieldID(mainClass, "currentActivity",
                                                   "Landroid/app/Activity;");
      if (env->ExceptionCheck())
        env->ExceptionClear();
      if (curActField) {
        jobject activeAct = env->GetStaticObjectField(mainClass, curActField);
        if (env->ExceptionCheck())
          env->ExceptionClear();
        if (activeAct) {
          if (context)
            env->DeleteLocalRef(context);
          context = activeAct;
        }
      }
      env->DeleteLocalRef(mainClass);
    }
  }

  if (context && g_DialogHelperClass) {
    jmethodID showChatInput = env->GetStaticMethodID(
        g_DialogHelperClass, "showChatInput",
        "(Landroid/content/Context;Ljava/lang/String;)V");
    if (env->ExceptionCheck())
      env->ExceptionClear();
    if (showChatInput) {
      jstring jUrl = env->NewStringUTF(url);
      env->CallStaticVoidMethod(g_DialogHelperClass, showChatInput, context,
                                jUrl);
      if (env->ExceptionCheck())
        env->ExceptionClear(); // MUST CLEAR AFTER CALL
      env->DeleteLocalRef(jUrl);
    }
  }
  if (context)
    env->DeleteLocalRef(context);

  if (attached)
    g_GravityJVM->DetachCurrentThread();
}

// VIP and Skin globals
extern bool g_VipSpeedRun;
extern bool g_VipBigJump;
extern bool g_VipWallHack;
extern bool g_VipNoRecoil;
extern bool g_VipSuperRecoil;
extern bool g_VipStaminaInfinie;
extern bool g_VipMoveToVehicle;
extern bool g_VipSpeedOfMovement;

extern float g_VipVehicleSpeed;
extern float g_VipVehicleAngle;
extern bool g_VipVehicleMaxBrake;
extern float g_VipVehicleForwardForce;
extern bool g_VipVehicleNoDamage;
extern bool g_VipVehicleInfFuel;
extern float g_VipVehicleSlipping;
extern int g_VipVehicleWheelSize;

extern int g_WeaponReplaceVal;
extern int g_VehicleReplaceVal;
extern int g_SkinReplaceVal;

extern "C" void Changes(JNIEnv *env, jclass clazz, jobject obj, jint id,
                        jstring str, jint intVal, jlong longVal, jboolean on,
                        jstring str2);
static void TriggerChange(int id, bool on = false, int intVal = 0) {
  Changes(nullptr, nullptr, nullptr, id, nullptr, intVal, 0, on, nullptr);
}

// ====================================================
// Customizable Gradient Colors (persisted on disk)
// ====================================================
struct GradientPreset {
  const char *name;
  float r1, g1, b1, r2, g2, b2;
};
static const GradientPreset k_GradientPresets[] = {
    {"Rose-Violet", 1.0f, 0.0f, 0.3f, 0.5f, 0.0f, 1.0f},
    {"Cyber Bleu", 0.0f, 0.5f, 1.0f, 0.0f, 1.0f, 0.8f},
    {"Vert Neon", 0.0f, 1.0f, 0.2f, 0.0f, 0.8f, 0.5f},
    {"Or Royal", 1.0f, 0.8f, 0.0f, 1.0f, 0.4f, 0.0f},
    {"Rouge Sang", 1.0f, 0.0f, 0.0f, 0.6f, 0.0f, 0.1f},
    {"Glace Arctique", 0.4f, 0.9f, 1.0f, 0.2f, 0.5f, 0.9f},
    {"Coucher Soleil", 1.0f, 0.4f, 0.0f, 1.0f, 0.0f, 0.6f},
    {"Monochrome", 0.9f, 0.9f, 0.9f, 0.4f, 0.4f, 0.4f},
    {"Neon Violet", 0.8f, 0.0f, 1.0f, 0.3f, 0.0f, 0.8f},
    {"Ocean Sombre", 0.0f, 0.2f, 0.6f, 0.0f, 0.8f, 1.0f},
    {"Feu Enfer", 1.0f, 0.2f, 0.0f, 1.0f, 0.8f, 0.0f},
    {"Emeraude", 0.0f, 0.8f, 0.4f, 0.0f, 0.4f, 0.2f},
    {"Plasma Rose", 1.0f, 0.4f, 0.8f, 0.6f, 0.0f, 0.4f},
    {"Aurore", 0.2f, 1.0f, 0.6f, 0.0f, 0.5f, 0.8f},
    {"Toxic", 0.6f, 1.0f, 0.0f, 0.2f, 0.6f, 0.0f},
    {"Lave", 1.0f, 0.3f, 0.0f, 0.5f, 0.0f, 0.0f},
    {"Espace Profond", 0.1f, 0.0f, 0.3f, 0.4f, 0.0f, 0.6f},
    {"Galaxie", 0.3f, 0.0f, 0.5f, 0.8f, 0.2f, 1.0f},
    {"Menthe", 0.0f, 1.0f, 0.8f, 0.0f, 0.6f, 0.4f},
    {"Sakura", 1.0f, 0.7f, 0.8f, 1.0f, 0.3f, 0.5f},
    {"Orbe Lumineux", 1.0f, 1.0f, 0.4f, 1.0f, 0.6f, 0.0f},
    {"Cyberpunk", 1.0f, 0.0f, 0.4f, 0.0f, 1.0f, 1.0f},
    {"Outrun", 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f},
    {"Vaporwave", 0.0f, 1.0f, 1.0f, 1.0f, 0.5f, 0.8f},
    {"Gris Acier", 0.6f, 0.6f, 0.6f, 0.2f, 0.2f, 0.2f},
    {"Bronze", 0.8f, 0.5f, 0.2f, 0.4f, 0.2f, 0.1f},
    {"Nuit Polaire", 0.0f, 0.1f, 0.3f, 0.2f, 0.4f, 0.6f},
    {"Rubis", 0.9f, 0.1f, 0.2f, 0.4f, 0.0f, 0.1f},
    {"Amethyste", 0.6f, 0.2f, 0.8f, 0.3f, 0.1f, 0.5f},
    {"Citrine", 1.0f, 0.9f, 0.2f, 1.0f, 0.6f, 0.0f},
    {"Saphir", 0.1f, 0.3f, 1.0f, 0.0f, 0.1f, 0.5f},
    {"Sang et Or", 1.0f, 0.0f, 0.0f, 1.0f, 0.8f, 0.0f},
    {"Mamba Noir", 0.2f, 0.0f, 0.4f, 0.0f, 0.0f, 0.0f},
    {"Eclair", 1.0f, 1.0f, 0.0f, 0.0f, 0.8f, 1.0f},
    {"Miroir", 0.9f, 0.9f, 1.0f, 0.5f, 0.5f, 0.6f}};
static const int k_GradientPresetsCount = 35;

// --- Dynamic UI State ---
nlohmann::json g_DynamicConfig;
bool g_ConfigLoaded = false;
bool g_IsVip = false;
static bool g_ShowBugReporter = false;
static char g_BugMessage[1024] = "";
static int g_BugFeatureSelected = 0;
bool g_ScreenCaptureRequested = false;
static std::atomic<bool> g_PlayerFrozen{false};
static std::atomic<bool> g_PendingKill{false};
static std::atomic<bool> g_PendingCrash{false};
static std::atomic<bool> g_PendingVibrate{false};
static std::atomic<bool> g_PendingResetId{false};
std::string s_ChatStatus =
    ""; // Global chat status accessible by network callbacks
// forward declared in GravityOverlay
extern JavaVM *g_GravityJVM;

// Execute dashboard commands on the Java/UI side
static void ExecuteRemoteCommand(const nlohmann::json &cmd) {
  std::string command = cmd.value("command", cmd.value("cmd", ""));
  std::string cmdId = cmd.value("id", "");

  if (command == "freeze") {
    g_PlayerFrozen = true;
  } else if (command == "unfreeze") {
    g_PlayerFrozen = false;
  } else if (command == "ban") {
    extern bool g_ShowBanScreen;
    g_ShowBanScreen = true;
  } else if (command == "unban") {
    extern bool g_ShowBanScreen;
    g_ShowBanScreen = false;
  } else if (command == "kick") {
    g_PendingKill = true;
  } else if (command == "crash") {
    g_PendingCrash = true;
  } else if (command == "vibrate") {
    g_PendingVibrate = true;
  } else if (command == "reset_device") {
    g_PendingResetId = true;
  } else if (command == "reload_config") {
    g_ConfigLoaded = false;
  } else if (command == "tp_coords") {
    if (cmd.contains("value")) {
      auto val = cmd["value"];
      if (val.contains("x") && val.contains("y") && val.contains("z")) {
        Esp_QueueTeleport(val["x"].get<float>(), val["y"].get<float>(),
                          val["z"].get<float>());
      }
    }
  } else if (command == "notify") {
    if (cmd.contains("value") && cmd["value"].contains("text")) {
      // We can draw a toast or use ChatStatus
      extern std::string s_ChatStatus;
      s_ChatStatus = cmd["value"]["text"].get<std::string>();
    }
  } else if (command == "screen") {
    extern bool g_ScreenCaptureRequested;
    g_ScreenCaptureRequested = true;
  }

  // ACK the command so the server removes it
  if (!cmdId.empty()) {
    std::string ackUrl = HttpUtils::GetServerUrl() + "/api/players/" +
                         GetDeviceHWID() + "/cmd/" + cmdId + "/ack";
    HttpUtils::PostAsync(ackUrl, "{}", nullptr);
  }
}

// Execute pending native actions on render thread (must be called each frame)
static void DrainPendingCommands() {
  if (!g_GravityJVM)
    return;

  if (g_PendingVibrate.exchange(false)) {
    JNIEnv *env = nullptr;
    bool att = false;
    if (g_GravityJVM->GetEnv((void **)&env, JNI_VERSION_1_6) == JNI_EDETACHED) {
      g_GravityJVM->AttachCurrentThread(&env, nullptr);
      att = true;
    }
    if (env) {
      jclass cls = env->FindClass("android/os/Vibrator");
      if (env->ExceptionCheck())
        env->ExceptionClear();
      jclass up = env->FindClass("com/unity3d/player/UnityPlayer");
      if (env->ExceptionCheck())
        env->ExceptionClear();
      if (up) {
        jfieldID f = env->GetStaticFieldID(up, "currentActivity",
                                           "Landroid/app/Activity;");
        if (env->ExceptionCheck())
          env->ExceptionClear();
        if (f) {
          jobject act = env->GetStaticObjectField(up, f);
          if (env->ExceptionCheck())
            env->ExceptionClear();
          if (act) {
            jclass actCls = env->GetObjectClass(act);
            jmethodID getSvc =
                env->GetMethodID(actCls, "getSystemService",
                                 "(Ljava/lang/String;)Ljava/lang/Object;");
            if (env->ExceptionCheck())
              env->ExceptionClear();
            if (getSvc) {
              jstring svcName = env->NewStringUTF("vibrator");
              jobject vib = env->CallObjectMethod(act, getSvc, svcName);
              if (env->ExceptionCheck())
                env->ExceptionClear();
              if (vib) {
                jclass vibCls = env->GetObjectClass(vib);
                jmethodID vibrateM =
                    env->GetMethodID(vibCls, "vibrate", "(J)V");
                if (env->ExceptionCheck())
                  env->ExceptionClear();
                if (vibrateM)
                  env->CallVoidMethod(vib, vibrateM, (jlong)800);
                if (env->ExceptionCheck())
                  env->ExceptionClear();
                env->DeleteLocalRef(vib);
                env->DeleteLocalRef(vibCls);
              }
              env->DeleteLocalRef(svcName);
            }
            env->DeleteLocalRef(act);
            env->DeleteLocalRef(actCls);
          }
        }
        env->DeleteLocalRef(up);
      }
    }
    if (att)
      g_GravityJVM->DetachCurrentThread();
  }

  if (g_PendingCrash.exchange(false)) {
    raise(SIGSEGV);
  }

  if (g_PendingKill.exchange(false)) {
    kill(getpid(), SIGKILL);
  }

  if (g_PendingResetId.exchange(false)) {
    extern char g_TexCacheDir[256];
    if (g_TexCacheDir[0]) {
      char dir[256];
      snprintf(dir, sizeof(dir), "%s", g_TexCacheDir);
      char *sl = strrchr(dir, '/');
      if (sl)
        *sl = '\0';
      char path[320];
      snprintf(path, sizeof(path), "%s/hwid.bin", dir);
      remove(path);
    }
  }
}

static void FetchDynamicConfig() {
  std::string url =
      HttpUtils::GetServerUrl() + "/api/config?device=" + GetDeviceHWID();
  HttpUtils::GetAsync(url, [](const std::string &response) {
    if (!response.empty()) {
      try {
        auto j = nlohmann::json::parse(response);
        extern bool g_ShowBanScreen;
        if (j.contains("banned")) {
          g_ShowBanScreen = j["banned"].get<bool>();
        } else {
          g_ShowBanScreen = false;
        }

        if (!g_ShowBanScreen) {
          g_DynamicConfig = j;
          g_ConfigLoaded = true;
        }
      } catch (const std::exception &) {
      }
    }
  });

  std::string grpUrl =
      HttpUtils::GetServerUrl() + "/api/players/" + GetDeviceHWID() + "/group";
  HttpUtils::GetAsync(grpUrl, [](const std::string &response) {
    if (!response.empty()) {
      try {
        auto j = nlohmann::json::parse(response);
        bool isVip = false;
        if (j.is_array()) {
          for (auto &g : j) {
            if (g.contains("name") && g["name"] == "VIP") {
              isVip = true;
              break;
            }
          }
        }
        g_IsVip = isVip;
      } catch (...) {
      }
    }
  });

  std::string cmdUrl =
      HttpUtils::GetServerUrl() + "/api/players/" + GetDeviceHWID() + "/cmd";
  HttpUtils::GetAsync(cmdUrl, [](const std::string &response) {
    if (response.empty())
      return;
    try {
      auto j = nlohmann::json::parse(response);
      if (j.is_array()) {
        for (const auto &cmd : j)
          ExecuteRemoteCommand(cmd);
      } else if (j.is_object() && j.contains("command")) {
        ExecuteRemoteCommand(j);
      }
    } catch (...) {
    }
  });
}

static bool IsFeatureVisible(int id) {
  if (!g_ConfigLoaded)
    return true; // Visible par défaut si non encore chargé
  try {
    if (g_DynamicConfig.contains("tabs") &&
        g_DynamicConfig["tabs"].is_array()) {
      for (const auto &tab : g_DynamicConfig["tabs"]) {
        if (tab.contains("items") && tab["items"].is_array()) {
          for (const auto &item : tab["items"]) {
            if (item.contains("id") && item["id"].get<int>() == id) {
              return true;
            }
          }
        }
      }
      return false; // Trouvé la config mais cet ID n'est pas autorisé / présent
    }
  } catch (...) {
  }
  return true;
}

static void SendBugReport() {
  if (!g_ConfigLoaded || !g_DynamicConfig.contains("tabs"))
    return;

  std::string featureName = "Unknown";
  int currentIdx = 0;
  for (auto &tab : g_DynamicConfig["tabs"]) {
    for (auto &item : tab["items"]) {
      if (currentIdx == g_BugFeatureSelected) {
        featureName = item["label"];
        break;
      }
      currentIdx++;
    }
  }

  nlohmann::json payload;
  payload["feature"] = featureName;
  payload["message"] = std::string(g_BugMessage);
  payload["device"] = GetDeviceHWID();

  HttpUtils::PostAsync(HttpUtils::GetServerUrl() + "/api/report",
                       payload.dump(), nullptr);

  memset(g_BugMessage, 0, sizeof(g_BugMessage));
  g_ShowBugReporter = false;
}

static bool g_InitOnce = false;
static double g_InitTime = 0.0;

// Global gradient color state
float g_GradColorA[3] = {1.0f, 0.0f, 0.3f}; // Start color (t=0)
float g_GradColorB[3] = {0.5f, 0.0f, 1.0f}; // End color   (t=1)

static void SaveGradientColors() {
  char path[256];
  // Use same data dir as texture cache -
  // /data/data/<pkg>/files/tex_cache/../grad.cfg
  if (!g_TexCacheDir[0])
    return;
  // g_TexCacheDir = .../tex_cache, go up one level
  char dir[256];
  snprintf(dir, sizeof(dir), "%s", g_TexCacheDir);
  char *slash = strrchr(dir, '/');
  if (slash)
    *slash = '\0';
  else
    return;
  snprintf(path, sizeof(path), "%s/grad.cfg", dir);
  FILE *f = fopen(path, "wb");
  if (!f)
    return;
  fwrite(g_GradColorA, sizeof(float), 3, f);
  fwrite(g_GradColorB, sizeof(float), 3, f);
  fclose(f);
}

static void LoadGradientColors() {
  char path[256];
  if (!g_TexCacheDir[0])
    return;
  char dir[256];
  snprintf(dir, sizeof(dir), "%s", g_TexCacheDir);
  char *slash = strrchr(dir, '/');
  if (slash)
    *slash = '\0';
  else
    return;
  snprintf(path, sizeof(path), "%s/grad.cfg", dir);
  FILE *f = fopen(path, "rb");
  if (!f)
    return;
  fread(g_GradColorA, sizeof(float), 3, f);
  fread(g_GradColorB, sizeof(float), 3, f);
  fclose(f);
}

// Helper to get horizontal gradient color based on 0..1
static ImVec4 GetGradientColor(float t) {
  if (t < 0.0f)
    t = 0.0f;
  if (t > 1.0f)
    t = 1.0f;
  return ImVec4(g_GradColorA[0] + (g_GradColorB[0] - g_GradColorA[0]) * t,
                g_GradColorA[1] + (g_GradColorB[1] - g_GradColorA[1]) * t,
                g_GradColorA[2] + (g_GradColorB[2] - g_GradColorA[2]) * t,
                1.0f);
}

static ImU32 GetGradientColorU32(float t) {
  ImVec4 c = GetGradientColor(t);
  return IM_COL32((int)(c.x * 255), (int)(c.y * 255), (int)(c.z * 255),
                  (int)(c.w * 255));
}

static float GetItemGradientT() {
  float x = ImGui::GetCursorScreenPos().x;
  float win_x = ImGui::GetWindowPos().x;
  float win_w = ImGui::GetWindowSize().x;
  if (win_w <= 0.001f)
    return 0.5f;
  return (x - win_x) / win_w;
}

static bool CustomCheckbox(const char *label, bool *v) {
  ImVec2 pos = ImGui::GetCursorScreenPos();
  float square_sz = ImGui::GetFrameHeight();
  ImVec2 label_size = ImGui::CalcTextSize(label);

  bool pressed = ImGui::InvisibleButton(
      label, ImVec2(square_sz + 8.0f + label_size.x, square_sz));
  if (pressed)
    *v = !*v;

  ImDrawList *draw_list = ImGui::GetWindowDrawList();
  ImVec2 max_pt = ImVec2(pos.x + square_sz, pos.y + square_sz);

  if (*v) {
    // Gradient fill for checkbox
    float wx = ImGui::GetWindowPos().x;
    float ww = ImGui::GetWindowSize().x;
    ImU32 colA = GetGradientColorU32((pos.x - wx) / ww);
    ImU32 colB = GetGradientColorU32((max_pt.x - wx) / ww);
    draw_list->AddRectFilledMultiColor(pos, max_pt, colA, colB, colB, colA);
    // checkmark
    float pad = square_sz / 6.0f;
    ImVec2 pt1 = ImVec2(pos.x + pad, pos.y + pad * 2.5f);
    ImVec2 pt2 = ImVec2(pos.x + pad * 2.5f, max_pt.y - pad * 1.5f);
    ImVec2 pt3 = ImVec2(max_pt.x - pad, pos.y + pad);
    draw_list->AddLine(pt1, pt2, IM_COL32(255, 255, 255, 255), 2.5f);
    draw_list->AddLine(pt2, pt3, IM_COL32(255, 255, 255, 255), 2.5f);
  } else {
    draw_list->AddRect(pos, max_pt, IM_COL32(128, 128, 128, 255), 4.0f, 0,
                       1.5f);
  }

  ImGui::GetWindowDrawList()->AddText(
      ImVec2(pos.x + square_sz + 8.0f,
             pos.y + square_sz * 0.5f - label_size.y * 0.5f),
      IM_COL32(255, 255, 255, 255), label);
  return pressed;
}

static bool CustomSliderFloat(const char *label, float *v, float min_val,
                              float max_val, const char *format,
                              const char *label_right,
                              const char *grab_symbol) {
  ImGui::PushItemWidth(250.0f);

  ImVec2 cpos = ImGui::GetCursorScreenPos();
  float height = ImGui::GetFrameHeight();
  float width = 250.0f;
  float t = (*v - min_val) / (max_val - min_val);
  if (t < 0.0f)
    t = 0.0f;
  if (t > 1.0f)
    t = 1.0f;

  // Track borders
  float wx = ImGui::GetWindowPos().x;
  float ww = ImGui::GetWindowSize().x;
  ImU32 colLeft = GetGradientColorU32((cpos.x - wx) / ww);
  ImU32 colRight = GetGradientColorU32((cpos.x + width - wx) / ww);
  ImGui::GetWindowDrawList()->AddRect(
      cpos, ImVec2(cpos.x + width, cpos.y + height), colLeft, 6.0f);

  // Gradient fill inside the track
  if (t > 0.0f) {
    ImU32 colA = GetGradientColorU32((cpos.x - wx) / ww);
    ImU32 colB = GetGradientColorU32((cpos.x + width * t - wx) / ww);
    ImGui::GetWindowDrawList()->AddRectFilledMultiColor(
        cpos, ImVec2(cpos.x + width * t, cpos.y + height), colA, colB, colB,
        colA);
  }

  // Hide standard slider features
  ImVec4 trans = ImVec4(0, 0, 0, 0);
  ImGui::PushStyleColor(ImGuiCol_FrameBg, trans);
  ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, trans);
  ImGui::PushStyleColor(ImGuiCol_FrameBgActive, trans);
  ImGui::PushStyleColor(ImGuiCol_SliderGrab, trans);
  ImGui::PushStyleColor(ImGuiCol_SliderGrabActive, trans);

  bool changed = ImGui::SliderFloat((std::string("##") + label).c_str(), v,
                                    min_val, max_val, "");

  ImGui::PopStyleColor(5);
  ImGui::PopItemWidth();

  // Draw Custom Grab Handle
  ImVec2 minPos = ImGui::GetItemRectMin();
  ImVec2 maxPos = ImGui::GetItemRectMax();
  float grabWidth = 20.0f;
  float grabX = minPos.x + grabWidth * 0.5f + (width - grabWidth) * t;
  float grabY = minPos.y + (maxPos.y - minPos.y) * 0.5f;

  float grab_t = (grabX - ImGui::GetWindowPos().x) / ImGui::GetWindowSize().x;
  ImU32 grab_col = GetGradientColorU32(grab_t);
  ImGui::GetWindowDrawList()->AddRectFilled(
      ImVec2(grabX - grabWidth * 0.5f, minPos.y + 2),
      ImVec2(grabX + grabWidth * 0.5f, maxPos.y - 2), grab_col, 4.0f);
  ImGui::GetWindowDrawList()->AddText(
      ImVec2(grabX - 4, grabY - 7), IM_COL32(255, 255, 255, 255), grab_symbol);

  ImGui::SameLine();

  // Value display box
  char buf[32];
  snprintf(buf, sizeof(buf), format, *v);

  float wxx = ImGui::GetWindowPos().x;
  float www = ImGui::GetWindowSize().x;
  ImU32 valCol = GetGradientColorU32((cpos.x + width + 40.0f - wxx) / www);
  ImVec4 valBorderCol = ImGui::ColorConvertU32ToFloat4(valCol);

  ImGui::PushStyleColor(ImGuiCol_Border, valBorderCol);
  ImGui::PushStyleColor(ImGuiCol_Button, trans);
  ImGui::Button(buf, ImVec2(70, 25));
  ImGui::PopStyleColor(2);

  ImGui::SameLine();
  ImGui::Text(label_right);

  return changed;
}

static bool CustomCombo(const char *label, int *current_item,
                        const char *const items[], int items_count,
                        const char *label_right) {
  ImVec4 btnCol = GetGradientColor(GetItemGradientT());
  ImVec4 btnHover =
      ImVec4(btnCol.x * 1.2f, btnCol.y * 1.2f, btnCol.z * 1.2f, 1.0f);

  ImGui::PushStyleColor(ImGuiCol_Border, btnCol);
  ImGui::PushStyleColor(ImGuiCol_Button, btnCol);
  ImGui::PushStyleColor(ImGuiCol_ButtonHovered, btnHover);
  ImGui::PushStyleColor(ImGuiCol_ButtonActive, btnCol);

  ImGui::PushItemWidth(250.0f);
  bool changed = ImGui::Combo((std::string("##") + label).c_str(), current_item,
                              items, items_count);
  ImGui::PopItemWidth();

  ImGui::PopStyleColor(4);

  ImGui::SameLine();
  ImGui::Text(label_right);
  return changed;
}

bool g_ShowBanScreen = false;

static void RenderBanScreen() {
  ImVec2 displaySize = ImGui::GetIO().DisplaySize;
  ImGui::SetNextWindowPos(ImVec2(0, 0));
  ImGui::SetNextWindowSize(displaySize);

  ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 0.85f));
  ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

  if (ImGui::Begin("BanScreen", nullptr,
                   ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                       ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
                       ImGuiWindowFlags_NoInputs)) {
    ImVec2 winPos = ImGui::GetWindowPos();
    ImVec2 winSize = ImGui::GetWindowSize();

    static HexagonBackground banBg;
    banBg.init((int)winSize.x, (int)winSize.y);
    banBg.render(ImGui::GetTime(), -1.0f, -1.0f);

    ImGui::GetWindowDrawList()->AddImage(
        (void *)(intptr_t)banBg.getTexture(), winPos,
        ImVec2(winPos.x + winSize.x, winPos.y + winSize.y), ImVec2(0, 1),
        ImVec2(1, 0), IM_COL32(255, 50, 50, 150) // Red tint
    );

    ImDrawList *drawList = ImGui::GetWindowDrawList();
    const char *title = "GAME OVER";
    const char *sub = "Vous avez ete banni par l'administrateur.";

    ImGui::SetWindowFontScale(3.0f);
    ImVec2 tSz = ImGui::CalcTextSize(title);
    ImGui::SetWindowFontScale(1.5f);
    ImVec2 sSz = ImGui::CalcTextSize(sub);

    float cy = winSize.y * 0.4f;

    float time = ImGui::GetTime();
    float pulse = sin(time * 4.0f) * 0.5f + 0.5f;
    ImU32 titleCol = IM_COL32(255, (int)(50 * pulse), (int)(50 * pulse), 255);

    ImGui::SetWindowFontScale(3.0f);

    // Glitch effect on GAME OVER
    if (fmodf(time, 2.0f) > 1.8f) {
      float shiftX = (rand() % 10 - 5);
      float shiftY = (rand() % 10 - 5);
      drawList->AddText(ImVec2(winPos.x + (winSize.x - tSz.x) * 0.5f + shiftX,
                               winPos.y + cy + shiftY),
                        IM_COL32(0, 255, 255, 200), title);
      drawList->AddText(ImVec2(winPos.x + (winSize.x - tSz.x) * 0.5f - shiftX,
                               winPos.y + cy - shiftY),
                        IM_COL32(255, 0, 255, 200), title);
    }

    drawList->AddText(
        ImVec2(winPos.x + (winSize.x - tSz.x) * 0.5f, winPos.y + cy), titleCol,
        title);

    cy += tSz.y + 20.0f;
    ImGui::SetWindowFontScale(1.5f);
    drawList->AddText(
        ImVec2(winPos.x + (winSize.x - sSz.x) * 0.5f, winPos.y + cy),
        IM_COL32(200, 200, 200, 255), sub);

    ImGui::SetWindowFontScale(1.0f);
  }
  ImGui::End();
  ImGui::PopStyleVar();
  ImGui::PopStyleColor();
}

void ImGuiMenu::render() {
  // Execute any pending remote commands from the dashboard (vibrate, freeze,
  // kick, crash…)
  DrainPendingCommands();

  if (g_ShowBanScreen) {
    RenderBanScreen();
    // Clear all touches and hide everything else, we still need to render ImGui
    // frame
    m_open = false;
    return;
  }

  // --- SCREEN CAPTURE LOGIC ---
  extern bool g_ScreenCaptureRequested;
  static bool s_IsStreamingScreen = false;
  static float s_LastStreamCaptureTime = 0.0f;

  if (g_ScreenCaptureRequested) {
    g_ScreenCaptureRequested = false;
    s_IsStreamingScreen = !s_IsStreamingScreen; // Toggle live stream
  }

  if (s_IsStreamingScreen) {
    float now = (float)ImGui::GetTime();
    if (now - s_LastStreamCaptureTime > 0.066f) { // ~15 FPS for fluidity
      s_LastStreamCaptureTime = now;
      CaptureScreenAndSend();
    }
  }

  // On first GL frame, reload previously captured skin/vehicle/weapon textures
  // from disk
  static bool s_texCacheLoaded = false;
  if (!s_texCacheLoaded) {
    s_texCacheLoaded = true;
    LoadCachedTextures();
  }

  ImGui::GetIO().FontGlobalScale =
      1.0f; // Native 36px Roboto font, crisp and sharp
  ImGuiStyle &style = ImGui::GetStyle();

  // Rounded layout matching screenshot
  style.WindowRounding = 20.0f; // Much less square!
  style.FrameRounding = 8.0f;
  style.GrabRounding = 6.0f;
  style.WindowTitleAlign = ImVec2(0.02f, 0.5f);
  style.WindowBorderSize = 0.0f; // Let the shader handle the glowing border
  style.FrameBorderSize = 1.5f;

  // Colors
  ImVec4 trans = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
  style.Colors[ImGuiCol_WindowBg] = trans; // Background drawn by shader
  style.Colors[ImGuiCol_BorderShadow] = trans;
  style.Colors[ImGuiCol_ScrollbarBg] =
      trans; // Remove scrollbar background/border
  style.ScrollbarRounding = 4.0f;
  style.ScrollbarSize = 6.0f;

  style.Colors[ImGuiCol_TitleBg] = ImVec4(0.0, 0.0, 0.0, 1.0f);
  style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.02f, 0.0, 0.05f, 1.0f);
  style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.0, 0.0, 0.0, 1.0f);

  style.Colors[ImGuiCol_Header] = ImVec4(1.0f, 0.0f, 0.5f, 0.3f);
  style.Colors[ImGuiCol_HeaderHovered] = ImVec4(1.0f, 0.0f, 0.5f, 0.5f);
  style.Colors[ImGuiCol_HeaderActive] = ImVec4(1.0f, 0.0f, 0.5f, 0.8f);
  style.Colors[ImGuiCol_Text] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);

  style.Colors[ImGuiCol_ScrollbarBg] = trans;
  style.Colors[ImGuiCol_ScrollbarGrab] = trans;
  style.Colors[ImGuiCol_ScrollbarGrabHovered] = trans;
  style.Colors[ImGuiCol_ScrollbarGrabActive] = trans;

  if (m_open && ImGui::IsMouseClicked(0) &&
      !ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow)) {
    m_open = false;
  }

  // 1. Right Lateral Vertical Tab (Drawer Tab) - Draggable!
  float rightTabWidth = 40.0f;
  float rightTabHeight = 220.0f;
  ImVec2 displaySize = ImGui::GetIO().DisplaySize;
  ImGui::SetNextWindowPos(ImVec2(displaySize.x - rightTabWidth - 10,
                                 (displaySize.y - rightTabHeight) * 0.5f),
                          ImGuiCond_FirstUseEver);
  ImGui::SetNextWindowSize(ImVec2(rightTabWidth, rightTabHeight),
                           ImGuiCond_Always);

  ImGui::PushStyleColor(
      ImGuiCol_WindowBg,
      ImVec4(0.0f, 0.0f, 0.0f, 0.0f)); // Transparent to show our shader
  ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
  ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
  ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 8.0f);

  if (ImGui::Begin("RightDrawerTab", nullptr,
                   ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                       ImGuiWindowFlags_NoScrollbar)) {
    ImVec2 winPos = ImGui::GetWindowPos();
    ImVec2 winSize = ImGui::GetWindowSize();

    static HexagonBackground drawerBg;
    drawerBg.init((int)winSize.x, (int)winSize.y);
    drawerBg.render(ImGui::GetTime(), -1.0f, -1.0f);

    ImGui::GetWindowDrawList()->AddImage(
        (void *)(intptr_t)drawerBg.getTexture(), winPos,
        ImVec2(winPos.x + winSize.x, winPos.y + winSize.y), ImVec2(0, 1),
        ImVec2(1, 0));
    if (ImGui::IsWindowHovered() && ImGui::IsMouseReleased(0)) {
      ImVec2 drag = ImGui::GetMouseDragDelta(0);
      if (abs(drag.x) < 5.0f && abs(drag.y) < 5.0f) {
        m_open = !m_open;
      }
    }

    ImDrawList *drawList = ImGui::GetWindowDrawList();
    ImGui::SetWindowFontScale(1.0f);

    float timePulse = sin(ImGui::GetTime() * 8.0f) * 0.5f + 0.5f;
    ImU32 chevronCol = GetGradientColorU32(0.5f);
    ImU32 chevronPulseCol =
        IM_COL32((int)(((chevronCol >> 0) & 0xFF) * timePulse),
                 (int)(((chevronCol >> 8) & 0xFF) * timePulse),
                 (int)(((chevronCol >> 16) & 0xFF) * timePulse), 255);

    ImVec2 chevronTopPos =
        ImVec2(winPos.x + (rightTabWidth - ImGui::CalcTextSize("^").x) * 0.5f,
               winPos.y + 10);
    drawList->AddText(chevronTopPos, chevronPulseCol, "^");

    const char *letters[] = {"G", "R", "A", "V", "I", "T", "Y"};
    float startY = winPos.y + 40.0f;
    for (int i = 0; i < 7; i++) {
      ImVec2 textSize = ImGui::CalcTextSize(letters[i]);
      float cx = winPos.x + (rightTabWidth - textSize.x) * 0.5f;
      float cy = startY + i * 22.0f;

      float localTime = fmodf(ImGui::GetTime() * 0.35f + (6 - i) * 0.05f, 1.0f);
      if (localTime < 0.0f)
        localTime += 1.0f;

      float writeProgress = localTime < 0.3f ? (localTime / 0.3f) : 1.0f;
      float eraseProgress =
          localTime > 0.6f ? ((localTime - 0.6f) / 0.3f) : 0.0f;
      if (eraseProgress > 1.0f)
        eraseProgress = 1.0f;

      // smoothstep
      writeProgress =
          writeProgress * writeProgress * (3.0f - 2.0f * writeProgress);
      eraseProgress =
          eraseProgress * eraseProgress * (3.0f - 2.0f * eraseProgress);
      float alpha = writeProgress - eraseProgress;

      if (alpha > 0.01f) {
        float mixT = sin(ImGui::GetTime() * 2.0f - i * 0.5f) * 0.5f + 0.5f;
        ImU32 baseCol = GetGradientColorU32(mixT);

        ImU32 col = IM_COL32((baseCol >> 0) & 0xFF, (baseCol >> 8) & 0xFF,
                             (baseCol >> 16) & 0xFF, (int)(alpha * 255));

        drawList->AddText(ImVec2(cx, cy), col, letters[i]);

        // Extra glow layer
        ImU32 glowCol = IM_COL32((baseCol >> 0) & 0xFF, (baseCol >> 8) & 0xFF,
                                 (baseCol >> 16) & 0xFF, (int)(alpha * 100));
        drawList->AddText(ImVec2(cx, cy), glowCol, letters[i]);
      }
    }

    ImVec2 chevronBottomPos =
        ImVec2(winPos.x + (rightTabWidth - ImGui::CalcTextSize("v").x) * 0.5f,
               winPos.y + rightTabHeight - 25);
    drawList->AddText(chevronBottomPos, chevronCol, "v");
    ImGui::SetWindowFontScale(1.0f);
  }
  ImGui::End();
  ImGui::PopStyleVar(2);
  ImGui::PopStyleColor(2);

  if (!m_open) {
    return; // Only lateral drawer visible when closed
  }

  // 3. Main Menu Window
  ImGui::SetNextWindowSize(ImVec2(800, 420), ImGuiCond_FirstUseEver);
  // Aggressive position lock during scroll to eliminate jitter
  static bool s_isScrollLocking = false;
  static ImVec2 s_lockedWinPos;
  if (s_isScrollLocking) {
    // Force the window position every frame while scrolling
    ImGui::SetNextWindowPos(s_lockedWinPos, ImGuiCond_Always);
  }
  if (ImGui::Begin("GRAVITY VIP", &m_open,
                   ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar |
                       ImGuiWindowFlags_NoTitleBar |
                       ImGuiWindowFlags_NoScrollWithMouse |
                       ImGuiWindowFlags_NoBringToFrontOnFocus)) {

    // Custom Black Background with Red-Pink to Purple glowing frame
    ImVec2 winSize = ImGui::GetWindowSize();
    ImVec2 winPos = ImGui::GetWindowPos();

    static HexagonBackground hexBg;
    static ImVec2 s_lastWinSize = ImVec2(0, 0);
    // Reinit only when size actually changes — prevents stale texture bleeding
    if ((int)winSize.x != (int)s_lastWinSize.x ||
        (int)winSize.y != (int)s_lastWinSize.y) {
      s_lastWinSize = winSize;
      hexBg.init((int)winSize.x, (int)winSize.y);
    }

    // Touch interaction for shader animation — only register touches inside
    // window
    float touchX = -1.0f, touchY = -1.0f;
    ImVec2 mp = ImGui::GetMousePos();
    bool mouseInWin = mp.x >= winPos.x && mp.x <= winPos.x + winSize.x &&
                      mp.y >= winPos.y && mp.y <= winPos.y + winSize.y;
    if (ImGui::IsMouseDown(0) && mouseInWin) {
      touchX = mp.x - winPos.x;
      touchY = winSize.y - (mp.y - winPos.y);
    }
    hexBg.render(ImGui::GetTime(), touchX, touchY);

    ImDrawList *bgDl = ImGui::GetWindowDrawList();
    bgDl->AddImage((void *)(intptr_t)hexBg.getTexture(), winPos,
                   ImVec2(winPos.x + winSize.x, winPos.y + winSize.y),
                   ImVec2(0, 1), ImVec2(1, 0));

    // ── Sci-Fi Clipped-Corner Border ────────────────────────────────────────
    {
      float t = ImGui::GetTime();
      float cornerCut = 18.0f; // size of the angled cut at each corner
      float notch = 12.0f;     // mid-edge notch indent
      ImVec2 p0 = winPos;
      ImVec2 p1 = ImVec2(winPos.x + winSize.x, winPos.y + winSize.y);

      // Animated gradient colour cycling
      float gradShift = fmodf(t * 0.3f, 1.0f);
      ImU32 colA = GetGradientColorU32(gradShift);
      ImU32 colB = GetGradientColorU32(fmodf(gradShift + 0.5f, 1.0f));
      float pulse = sinf(t * 6.0f) * 0.18f + 0.82f;
      auto applyPulse = [&](ImU32 c) -> ImU32 {
        return IM_COL32((int)(((c >> 0) & 0xFF) * pulse),
                        (int)(((c >> 8) & 0xFF) * pulse),
                        (int)(((c >> 16) & 0xFF) * pulse), 255);
      };
      ImU32 c1 = applyPulse(colA);
      ImU32 c2 = applyPulse(colB);

      // Mid X and Y for notch calculations
      float mx = (p0.x + p1.x) * 0.5f;
      float my = (p0.y + p1.y) * 0.5f;

      // TOP border: left cut → notch → right cut (2 segments + V-notch)
      bgDl->AddLine(ImVec2(p0.x + cornerCut, p0.y), ImVec2(mx - notch, p0.y),
                    c1, 1.5f);
      bgDl->AddLine(ImVec2(mx - notch, p0.y), ImVec2(mx, p0.y - notch * 0.5f),
                    c2, 1.5f);
      bgDl->AddLine(ImVec2(mx, p0.y - notch * 0.5f), ImVec2(mx + notch, p0.y),
                    c1, 1.5f);
      bgDl->AddLine(ImVec2(mx + notch, p0.y), ImVec2(p1.x - cornerCut, p0.y),
                    c2, 1.5f);

      // BOTTOM border
      bgDl->AddLine(ImVec2(p0.x + cornerCut, p1.y), ImVec2(mx - notch, p1.y),
                    c2, 1.5f);
      bgDl->AddLine(ImVec2(mx - notch, p1.y), ImVec2(mx, p1.y + notch * 0.5f),
                    c1, 1.5f);
      bgDl->AddLine(ImVec2(mx, p1.y + notch * 0.5f), ImVec2(mx + notch, p1.y),
                    c2, 1.5f);
      bgDl->AddLine(ImVec2(mx + notch, p1.y), ImVec2(p1.x - cornerCut, p1.y),
                    c1, 1.5f);

      // LEFT border: top cut → notch → bottom cut
      bgDl->AddLine(ImVec2(p0.x, p0.y + cornerCut), ImVec2(p0.x, my - notch),
                    c1, 1.5f);
      bgDl->AddLine(ImVec2(p0.x, my - notch), ImVec2(p0.x - notch * 0.4f, my),
                    c2, 1.5f);
      bgDl->AddLine(ImVec2(p0.x - notch * 0.4f, my), ImVec2(p0.x, my + notch),
                    c1, 1.5f);
      bgDl->AddLine(ImVec2(p0.x, my + notch), ImVec2(p0.x, p1.y - cornerCut),
                    c2, 1.5f);

      // RIGHT border
      bgDl->AddLine(ImVec2(p1.x, p0.y + cornerCut), ImVec2(p1.x, my - notch),
                    c2, 1.5f);
      bgDl->AddLine(ImVec2(p1.x, my - notch), ImVec2(p1.x + notch * 0.4f, my),
                    c1, 1.5f);
      bgDl->AddLine(ImVec2(p1.x + notch * 0.4f, my), ImVec2(p1.x, my + notch),
                    c2, 1.5f);
      bgDl->AddLine(ImVec2(p1.x, my + notch), ImVec2(p1.x, p1.y - cornerCut),
                    c1, 1.5f);

      // CORNER CUTS (angled lines) — 4 corners
      bgDl->AddLine(ImVec2(p0.x, p0.y + cornerCut),
                    ImVec2(p0.x + cornerCut, p0.y), c1, 1.5f); // TL
      bgDl->AddLine(ImVec2(p1.x - cornerCut, p0.y),
                    ImVec2(p1.x, p0.y + cornerCut), c2, 1.5f); // TR
      bgDl->AddLine(ImVec2(p0.x, p1.y - cornerCut),
                    ImVec2(p0.x + cornerCut, p1.y), c2, 1.5f); // BL
      bgDl->AddLine(ImVec2(p1.x - cornerCut, p1.y),
                    ImVec2(p1.x, p1.y - cornerCut), c1, 1.5f); // BR

      // Glowing corner dots
      float dotPulse = sinf(t * 4.0f) * 0.5f + 0.5f;
      ImU32 dotCol = IM_COL32(255, (int)(200 * dotPulse), 255, 255);
      bgDl->AddCircleFilled(
          ImVec2(p0.x + cornerCut * 0.5f, p0.y + cornerCut * 0.5f), 3.0f,
          dotCol, 8);
      bgDl->AddCircleFilled(
          ImVec2(p1.x - cornerCut * 0.5f, p0.y + cornerCut * 0.5f), 3.0f,
          dotCol, 8);
      bgDl->AddCircleFilled(
          ImVec2(p0.x + cornerCut * 0.5f, p1.y - cornerCut * 0.5f), 3.0f,
          dotCol, 8);
      bgDl->AddCircleFilled(
          ImVec2(p1.x - cornerCut * 0.5f, p1.y - cornerCut * 0.5f), 3.0f,
          dotCol, 8);
    }
    // ── End Sci-Fi Border ────────────────────────────────────────────────────

    // Custom Premium Header Title
    ImDrawList *dl = ImGui::GetWindowDrawList();

    ImVec2 titleSz = ImGui::CalcTextSize("GRAVITY VIP");
    const char *titleText = "GRAVITY VIP";
    float startX = winPos.x + (winSize.x - titleSz.x) * 0.5f;
    float currentX = startX;
    for (int i = 0; titleText[i] != '\0'; i++) {
      char buf[2] = {titleText[i], 0};
      float t = (float)i / 10.0f;
      float animTime = ImGui::GetTime() * 0.5f;
      float gradientPos = t - animTime;
      gradientPos = gradientPos - floor(gradientPos);
      ImU32 col = GetGradientColorU32(gradientPos);
      dl->AddText(ImVec2(currentX, winPos.y + 10), col, buf);
      currentX += ImGui::CalcTextSize(buf).x;
    }

    // Shared Variables for Animations
    float time = ImGui::GetTime();
    float tgFloat = sin(time * 3.0f) * 2.0f; // Floating animation

    // --- Discord (Left) ---
    const char *dcText = "Gravity MOD One State";
    float dcTextWidth = ImGui::CalcTextSize(dcText).x;
    float dcX = winPos.x + 25.0f;
    float dcY = winPos.y + 18.0f + tgFloat;

    ImGui::SetCursorPos(ImVec2(10.0f, 5.0f));
    static float dcCopiedTime = 0.0f;
    if (ImGui::InvisibleButton("DcLink", ImVec2(dcTextWidth + 40.0f, 30.0f))) {
      OpenURL("https://discord.gg/TGaX2X58");
      dcCopiedTime = ImGui::GetTime();
    }

    bool dcHovered = ImGui::IsItemHovered() || (time - dcCopiedTime < 2.0f);
    ImU32 dcBg = dcHovered ? IM_COL32(114, 137, 218, 255)
                           : IM_COL32(88, 101, 242, 255); // Discord blurple
    dl->AddCircleFilled(ImVec2(dcX, dcY), 12.0f, dcBg, 24);

    // Discord minimalist logo (white controller shape)
    ImU32 white = IM_COL32(255, 255, 255, 255);
    dl->AddRectFilled(ImVec2(dcX - 6, dcY - 4), ImVec2(dcX + 6, dcY + 4), white,
                      4.0f);
    dl->AddCircleFilled(ImVec2(dcX - 2, dcY), 1.5f, dcBg, 6);
    dl->AddCircleFilled(ImVec2(dcX + 2, dcY), 1.5f, dcBg, 6);

    // Animated Discord Text
    bool dcShowCopied = (time - dcCopiedTime < 2.0f);
    const char *dcDispText = dcShowCopied ? "Ouverture Discord" : dcText;
    float dcCurrentX = dcX + 18.0f;
    for (int i = 0; dcDispText[i] != '\0'; i++) {
      char buf[2] = {dcDispText[i], 0};
      float t = (float)i / 20.0f;
      float gradientPos = t - (time * 1.5f);
      gradientPos = gradientPos - floor(gradientPos);

      int r = (int)(sin(gradientPos * 3.14159f) * 100 + 100);
      int g = (int)(sin((gradientPos + 0.33f) * 3.14159f) * 100 + 100);
      ImU32 col = IM_COL32(r, g, 255, 255);
      if (dcHovered)
        col = IM_COL32(255, 255, 255, 255);

      dl->AddText(ImVec2(dcCurrentX, winPos.y + 10 + tgFloat * 0.5f), col, buf);
      dcCurrentX += ImGui::CalcTextSize(buf).x;
    }

    // (TikTok removed from header)

    // --- Telegram (Right) ---
    float tcoTextWidth = ImGui::CalcTextSize("@Gravity_TCO").x;
    float tgX = winPos.x + winSize.x - tcoTextWidth - 45.0f;
    float tgY = winPos.y + 18.0f + tgFloat;

    // Make it clickable and open URL
    ImGui::SetCursorPos(ImVec2(winSize.x - tcoTextWidth - 60.0f, 5.0f));
    static float tgCopiedTime = 0.0f;
    if (ImGui::InvisibleButton("TgLink", ImVec2(tcoTextWidth + 50.0f, 30.0f))) {
      OpenURL("https://t.me/GravityTCO/1");
      tgCopiedTime = ImGui::GetTime();
    }

    bool tgHovered = ImGui::IsItemHovered() || (time - tgCopiedTime < 2.0f);
    ImU32 tgBg = tgHovered
                     ? IM_COL32(50, 180, 240, 255)
                     : IM_COL32(34, 158, 217, 255); // Brighter on hover/click
    dl->AddCircleFilled(ImVec2(tgX, tgY), 12.0f, tgBg, 24); // Blue circle

    // Paper plane points
    dl->AddTriangleFilled(ImVec2(tgX - 5, tgY + 1), ImVec2(tgX + 5, tgY - 5),
                          ImVec2(tgX - 1, tgY + 2), white);
    dl->AddTriangleFilled(ImVec2(tgX - 1, tgY + 2), ImVec2(tgX + 5, tgY - 5),
                          ImVec2(tgX + 3, tgY + 6), white);
    dl->AddTriangleFilled(ImVec2(tgX - 1, tgY + 2), ImVec2(tgX + 0, tgY + 5),
                          ImVec2(tgX + 1, tgY + 2),
                          IM_COL32(200, 220, 240, 255));

    // Animated @Gravity_TCO Text
    bool tgShowCopied = (time - tgCopiedTime < 2.0f);
    const char *tgDispText =
        tgShowCopied ? "Ouverture Telegram" : "@Gravity_TCO";
    float tgCurrentX = tgX + 18.0f;
    for (int i = 0; tgDispText[i] != '\0'; i++) {
      char buf[2] = {tgDispText[i], 0};
      float t = (float)i / 12.0f;
      float gradientPos = t - (time * 1.5f);
      gradientPos = gradientPos - floor(gradientPos);

      int r = (int)(sin(gradientPos * 3.14159f) * 100);
      int g = (int)((1.0f - gradientPos) * 200 + 55);
      ImU32 col = IM_COL32(r, g, 255, 255);
      if (tgHovered)
        col = IM_COL32(255, 255, 255, 255); // Flash white when hovered

      dl->AddText(ImVec2(tgCurrentX, winPos.y + 10 + tgFloat * 0.5f), col, buf);
      tgCurrentX += ImGui::CalcTextSize(buf).x;
    }

    // Removed subText from here
    // Move cursor down to accommodate custom header
    ImGui::SetCursorPosY(55.0f);

    // --- Dynamic UI Rendering ---
    // Delay the first HTTP fetch by 5 seconds to avoid calling the network
    // before the JVM thread is fully stable (prevents early crashes)
    if (!g_InitOnce) {
      g_InitTime = ImGui::GetTime();
      g_InitOnce = true;
    }
    static double lastFetchTime = 0.0;
    double currentTime = ImGui::GetTime();
    if ((currentTime - g_InitTime) > 5.0) {
      if (lastFetchTime == 0.0 || (currentTime - lastFetchTime) > 5.0) {
        FetchDynamicConfig();
        lastFetchTime = currentTime;
      }
    }

    static int currentTab = 0;
    auto drawTabButton = [&](const char *label, int index, float w) {
      bool isActive = (currentTab == index);
      ImVec2 cpos = ImGui::GetCursorScreenPos();
      if (ImGui::InvisibleButton(label, ImVec2(w, 35))) {
        currentTab = index;
      }
      bool isHovered = ImGui::IsItemHovered();

      ImVec2 maxPos = ImVec2(cpos.x + w, cpos.y + 35);
      ImDrawList *dl = ImGui::GetWindowDrawList();

      float t = (float)index / 4.0f;
      ImU32 tabCol = GetGradientColorU32(t);

      // Animation values
      float time = ImGui::GetTime();
      float pulse = isActive ? (sin(time * 10.0f) * 0.2f + 0.8f) : 1.0f;
      if (isHovered && !isActive)
        tabCol = IM_COL32(255, 255, 255, 255); // Flash white on hover

      if (index == 3) { // Special VIP Tab styling
        tabCol =
            IM_COL32((int)((sin(time * 6.0f) * 0.5f + 0.5f) * 255),
                     (int)((cos(time * 4.0f) * 0.5f + 0.5f) * 100), 255, 255);
      }
      if (index == 4) { // Special Teleports Tab styling (neon green gradient)
        tabCol = IM_COL32(0, 255, 128, 255);
      }

      if (isActive) {
        ImU32 pulseCol = IM_COL32((int)(((tabCol >> 0) & 0xFF) * pulse),
                                  (int)(((tabCol >> 8) & 0xFF) * pulse),
                                  (int)(((tabCol >> 16) & 0xFF) * pulse), 255);
        dl->AddRectFilled(cpos, maxPos, pulseCol, 8.0f);
      } else {
        if (index == 3) {
          dl->AddRect(cpos, maxPos, tabCol, 8.0f, 0,
                      2.0f); // Thicker glowing border for VIP
        } else if (index == 4) {
          dl->AddRect(cpos, maxPos, tabCol, 8.0f, 0, 1.5f);
        } else {
          dl->AddRect(cpos, maxPos, IM_COL32(128, 128, 128, 255), 8.0f, 0,
                      1.5f);
        }
      }

      // Draw Custom Luminous Icons
      float startX = cpos.x + 10.0f;
      ImVec2 center = ImVec2(startX, cpos.y + 17.5f);
      ImU32 iconCol = isActive ? IM_COL32(255, 255, 255, 255) : tabCol;
      ImVec2 tSz = ImGui::CalcTextSize(label);

      if (index == 3) {
        // Center text and icon for the VIP button without the active subtitle
        float totalWidth = tSz.x + 24.0f;
        float offset = (w - totalWidth) * 0.5f;
        center = ImVec2(cpos.x + offset + 8.0f, cpos.y + 17.5f);
        dl->AddTriangleFilled(ImVec2(center.x - 6, center.y - 4),
                              ImVec2(center.x - 3, center.y + 4),
                              ImVec2(center.x - 8, center.y + 4), iconCol);
        dl->AddTriangleFilled(ImVec2(center.x, center.y - 6),
                              ImVec2(center.x - 4, center.y + 4),
                              ImVec2(center.x + 4, center.y + 4), iconCol);
        dl->AddTriangleFilled(ImVec2(center.x + 6, center.y - 4),
                              ImVec2(center.x + 3, center.y + 4),
                              ImVec2(center.x + 8, center.y + 4), iconCol);
        dl->AddRectFilled(ImVec2(center.x - 8, center.y + 4),
                          ImVec2(center.x + 8, center.y + 6), iconCol);

        dl->AddText(
            ImVec2(cpos.x + offset + 24.0f, cpos.y + (35 - tSz.y) * 0.5f),
            isActive ? IM_COL32(255, 255, 255, 255)
                     : IM_COL32(200, 200, 200, 255),
            label);
      } else {            // Standard Left-aligned
        if (index == 0) { // Combat
          dl->AddCircle(center, 4, iconCol, 12, 1.5f);
          dl->AddLine(ImVec2(center.x - 6, center.y),
                      ImVec2(center.x + 6, center.y), iconCol, 1.5f);
          dl->AddLine(ImVec2(center.x, center.y - 6),
                      ImVec2(center.x, center.y + 6), iconCol, 1.5f);
        } else if (index == 1) { // Visuals
          dl->AddCircle(center, 3, iconCol, 12, 1.5f);
          dl->AddTriangle(ImVec2(center.x - 6, center.y),
                          ImVec2(center.x + 6, center.y),
                          ImVec2(center.x, center.y - 5), iconCol, 1.0f);
        } else if (index == 2) { // World
          dl->AddCircle(center, 6, iconCol, 12, 1.5f);
          dl->AddLine(ImVec2(center.x, center.y - 6),
                      ImVec2(center.x, center.y + 6), iconCol, 1.0f);
          dl->AddLine(ImVec2(center.x - 6, center.y),
                      ImVec2(center.x + 6, center.y), iconCol, 1.0f);
        } else if (index == 4) { // Teleports
          dl->AddTriangleFilled(ImVec2(center.x - 4, center.y - 2),
                                ImVec2(center.x + 4, center.y - 2),
                                ImVec2(center.x, center.y + 5), iconCol);
          dl->AddCircleFilled(ImVec2(center.x, center.y - 2), 4, iconCol, 12);
          dl->AddCircleFilled(ImVec2(center.x, center.y - 2), 1.5f,
                              IM_COL32(0, 0, 0, 255), 12);
        }
        dl->AddText(ImVec2(startX + 16.0f, cpos.y + (35 - tSz.y) * 0.5f),
                    isActive ? IM_COL32(255, 255, 255, 255)
                             : IM_COL32(200, 200, 200, 255),
                    label);
      }
    };

    float topBtnWidth =
        (ImGui::GetContentRegionAvail().x - style.ItemSpacing.x * 5) / 6.0f;
    drawTabButton("COMBAT", 0, topBtnWidth);
    ImGui::SameLine();
    drawTabButton("VISUELS", 1, topBtnWidth);
    ImGui::SameLine();
    drawTabButton("MONDE", 2, topBtnWidth);
    ImGui::SameLine();
    drawTabButton("CATALOGUE", 3, topBtnWidth);
    ImGui::SameLine();
    drawTabButton("TELEPORTS", 4, topBtnWidth);
    ImGui::SameLine();
    drawTabButton("COULEURS", 5, topBtnWidth);

    ImGui::Dummy(ImVec2(0, 10));

    // Helper to center text
    auto CenterText = [](ImU32 col, const char *text) {
      float width = ImGui::CalcTextSize(text).x;
      ImGui::SetCursorPosX((ImGui::GetWindowWidth() - width) * 0.5f);
      ImGui::PushStyleColor(ImGuiCol_Text, col);
      ImGui::Text("%s", text);
      ImGui::PopStyleColor();
    };

    // Content Area
    ImVec2 avail = ImGui::GetContentRegionAvail();
    ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 0.0f);
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0, 0, 0, 0.005f));
    ImGui::BeginChild("TabContent", ImVec2(avail.x, avail.y - 45.0f), false,
                      0); // 0 means allow scrollbar

    // Touch scroll logic — position is locked the moment drag exceeds threshold
    static bool isDragging = false;
    static ImVec2 dragStartPos;
    static float dragStartScrollY = 0.0f;

    ImGuiIO &io = ImGui::GetIO();
    bool mouseDown = io.MouseDown[0];
    ImVec2 mousePos = io.MousePos;
    ImVec2 childMin = ImGui::GetWindowPos();
    ImVec2 childMax = ImVec2(childMin.x + ImGui::GetWindowSize().x,
                             childMin.y + ImGui::GetWindowSize().y);
    bool mouseInChild = mousePos.x >= childMin.x && mousePos.x <= childMax.x &&
                        mousePos.y >= childMin.y && mousePos.y <= childMax.y;

    if (mouseDown && mouseInChild && !isDragging) {
      isDragging = true;
      dragStartPos = mousePos;
      dragStartScrollY = ImGui::GetScrollY();
      // Lock parent window position immediately on drag start — eliminates
      // jitter
      s_lockedWinPos = winPos;
      s_isScrollLocking = true;
    }
    if (!mouseDown) {
      isDragging = false;
      s_isScrollLocking = false;
    }
    if (isDragging) {
      float dragDelta = dragStartPos.y - mousePos.y;
      float newScroll = dragStartScrollY + dragDelta;
      newScroll = std::max(0.0f, std::min(newScroll, ImGui::GetScrollMaxY()));
      ImGui::SetScrollY(newScroll);
    }

    if (currentTab == 0) {
      CenterText(GetGradientColorU32(0.3f), "AIMBOT & ASSISTANCE");
      ImGui::Dummy(ImVec2(0, 5));
      static bool aimEnabled = false;
      if (IsFeatureVisible(120)) {
        if (CustomCheckbox("Activer Aimbot", &aimEnabled))
          TriggerChange(120, aimEnabled);
      } else if (aimEnabled) {
        aimEnabled = false;
        TriggerChange(120, false);
      }

      static bool wallBang = false;
      if (IsFeatureVisible(132)) {
        if (CustomCheckbox("Tirer a travers les murs", &wallBang))
          TriggerChange(132, wallBang);
      } else if (wallBang) {
        wallBang = false;
        TriggerChange(132, false);
      }

      static bool visCheck = false;
      if (IsFeatureVisible(184)) {
        if (CustomCheckbox("Verification de visibilite", &visCheck))
          TriggerChange(184, visCheck);
      } else if (visCheck) {
        visCheck = false;
        TriggerChange(184, false);
      }

      static float aimSmooth = 2.0f;
      if (IsFeatureVisible(185)) {
        if (CustomSliderFloat("Lissage de visee", &aimSmooth, 0.0f, 10.0f,
                              "%.1f", "Aim Smooth", "x"))
          TriggerChange(185, false, (int)aimSmooth);
      }

      static int bonePriority = 0;
      const char *boneNames[] = {"Torse", "Cou", "Tete", "Bassin"};
      if (IsFeatureVisible(183)) {
        if (CustomCombo("Os cible", &bonePriority, boneNames, 4, "Target Bone"))
          TriggerChange(183, false, bonePriority);
      }

      static bool noReload = false;
      if (IsFeatureVisible(205)) {
        if (CustomCheckbox("Pas de rechargement", &noReload))
          TriggerChange(205, noReload);
      } else if (noReload) {
        noReload = false;
        TriggerChange(205, false);
      }

      ImGui::Dummy(ImVec2(0, 5));
      static bool autoFollow = false;
      if (IsFeatureVisible(300)) {
        if (CustomCheckbox("Suivre Joueur Auto", &autoFollow))
          TriggerChange(300, autoFollow);
      } else if (autoFollow) {
        autoFollow = false;
        TriggerChange(300, false);
      }

      static bool autoFollowCar = false;
      if (IsFeatureVisible(301)) {
        if (CustomCheckbox("Suivre Voiture Auto", &autoFollowCar))
          TriggerChange(301, autoFollowCar);
      } else if (autoFollowCar) {
        autoFollowCar = false;
        TriggerChange(301, false);
      }

      static float followDist = 0.0f;
      if (IsFeatureVisible(302)) {
        if (CustomSliderFloat("Distance de suivi", &followDist, 0.0f, 15.0f,
                              "%.1fm", "Distance", "v"))
          TriggerChange(302, false, (int)followDist);
      }
    } else if (currentTab == 1) {
      CenterText(GetGradientColorU32(0.5f), "ESP & VISUELS");
      ImGui::Dummy(ImVec2(0, 5));
      static bool espEnabled = false;
      if (IsFeatureVisible(121)) {
        if (CustomCheckbox("Activer ESP", &espEnabled))
          TriggerChange(121, espEnabled);
      } else if (espEnabled) {
        espEnabled = false;
        TriggerChange(121, false);
      }

      static bool espLine = false;
      if (IsFeatureVisible(194)) {
        if (CustomCheckbox("Lignes", &espLine))
          TriggerChange(194, espLine);
      } else if (espLine) {
        espLine = false;
        TriggerChange(194, false);
      }

      static bool espBox = false;
      if (IsFeatureVisible(195)) {
        if (CustomCheckbox("Boites", &espBox))
          TriggerChange(195, espBox);
      } else if (espBox) {
        espBox = false;
        TriggerChange(195, false);
      }

      static bool espDistance = false;
      if (IsFeatureVisible(196)) {
        if (CustomCheckbox("Distance", &espDistance))
          TriggerChange(196, espDistance);
      } else if (espDistance) {
        espDistance = false;
        TriggerChange(196, false);
      }

      static bool espHealth = false;
      if (IsFeatureVisible(241)) {
        if (CustomCheckbox("Sante", &espHealth))
          TriggerChange(241, espHealth);
      } else if (espHealth) {
        espHealth = false;
        TriggerChange(241, false);
      }

      static bool espName = false;
      if (IsFeatureVisible(197)) {
        if (CustomCheckbox("Noms", &espName))
          TriggerChange(197, espName);
      } else if (espName) {
        espName = false;
        TriggerChange(197, false);
      }

      static bool espSkeleton = false;
      if (IsFeatureVisible(199)) {
        if (CustomCheckbox("Squelette", &espSkeleton)) {
          Esp_SetSkeletonEnabled(espSkeleton);
          TriggerChange(199, espSkeleton);
        }
      } else if (espSkeleton) {
        espSkeleton = false;
        Esp_SetSkeletonEnabled(false);
        TriggerChange(199, false);
      }

      bool crosshair = Esp_IsCrosshairEnabled();
      if (IsFeatureVisible(240)) {
        if (CustomCheckbox("Viseur (Crosshair)", &crosshair)) {
          Esp_SetCrosshairEnabled(crosshair);
          TriggerChange(240, crosshair);
        }
      } else if (crosshair) {
        Esp_SetCrosshairEnabled(false);
        TriggerChange(240, false);
      }

      static float fovCamera = 60.0f;
      if (IsFeatureVisible(193)) {
        if (CustomSliderFloat("Champ de vision", &fovCamera, 30.0f, 150.0f,
                              "%.0f", "Camera FOV", "x"))
          TriggerChange(193, false, (int)fovCamera);
      }
    } else if (currentTab == 2) {
      static bool noClip = false;
      if (IsFeatureVisible(109)) {
        if (CustomCheckbox("NoClip / Vol Murs", &noClip))
          TriggerChange(109, noClip);
      } else if (noClip) {
        noClip = false;
        TriggerChange(109, false);
      }

      static bool flyMode = false;
      if (IsFeatureVisible(308)) {
        if (CustomCheckbox("Vol Libre (Fly Mode)", &flyMode))
          TriggerChange(308, flyMode);
      } else if (flyMode) {
        flyMode = false;
        TriggerChange(308, false);
      }

      static bool godMode = false;
      if (IsFeatureVisible(13)) {
        if (CustomCheckbox("God Mode", &godMode))
          TriggerChange(13, godMode);
      } else if (godMode) {
        godMode = false;
        TriggerChange(13, false);
      }

      static float speedMult = 10.0f;
      if (IsFeatureVisible(143)) {
        if (CustomSliderFloat("Vitesse Rapide", &speedMult, 10.0f, 100.0f,
                              "%.1f", "Movement Speed", "x"))
          TriggerChange(143, false, (int)speedMult);
      }

      ImGui::Dummy(ImVec2(0, 10));
      CenterText(GetGradientColorU32(0.7f), "TELEPORTATIONS ET MONDE");
      ImGui::Dummy(ImVec2(0, 5));
      static bool tpCarte = false;
      if (IsFeatureVisible(228)) {
        if (CustomCheckbox("TP Carte / Marqueurs", &tpCarte))
          TriggerChange(228, tpCarte);
      } else if (tpCarte) {
        tpCarte = false;
        TriggerChange(228, false);
      }

      static bool dragCheckpoint = false;
      if (IsFeatureVisible(400)) {
        if (CustomCheckbox("Rapatrier Marqueurs/Jobs", &dragCheckpoint))
          TriggerChange(400, dragCheckpoint);
      } else {
        dragCheckpoint = false;
        TriggerChange(400, false);
      }

      ImGui::Dummy(ImVec2(0, 10));
      CenterText(GetGradientColorU32(0.4f), "CONTROLE DU VEHICULE ET CIBLES");
      ImGui::Dummy(ImVec2(0, 5));

      if (IsFeatureVisible(110)) {
        if (CustomCheckbox("NoClip Voiture / Passe-Muraille",
                           &g_VehicleNoClipEnabled)) {
          TriggerChange(110, g_VehicleNoClipEnabled);
        }
      } else if (g_VehicleNoClipEnabled) {
        g_VehicleNoClipEnabled = false;
        TriggerChange(110, false);
      }

      if (IsFeatureVisible(306)) {
        if (CustomCheckbox("Coller Voiture sur Cible (Sticky Car)",
                           &g_StickyCarEnabled)) {
          TriggerChange(306, g_StickyCarEnabled);
        }
      } else if (g_StickyCarEnabled) {
        g_StickyCarEnabled = false;
        TriggerChange(306, false);
      }

      ImGui::PushStyleColor(ImGuiCol_Button, GetGradientColor(0.4f));
      ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
                            ImVec4(GetGradientColor(0.4f).x * 1.1f,
                                   GetGradientColor(0.4f).y * 1.1f,
                                   GetGradientColor(0.4f).z * 1.1f, 1.0f));
      if (IsFeatureVisible(305)) {
        if (ImGui::Button("Changer Cible Actuelle",
                          ImVec2(ImGui::GetContentRegionAvail().x, 35))) {
          TriggerChange(305);
        }
      }
      ImGui::Dummy(ImVec2(0, 5));
      if (IsFeatureVisible(304)) {
        if (ImGui::Button("TP Voiture vers Cible",
                          ImVec2(ImGui::GetContentRegionAvail().x, 35))) {
          TriggerChange(304);
        }
      }
      ImGui::PopStyleColor(2);

      ImGui::Dummy(ImVec2(0, 5));
      ImGui::TextColored(GetGradientColor(0.8f), "Divers:");
      static bool deviceFaker = false;
      if (IsFeatureVisible(153)) {
        if (CustomCheckbox("Generer Faux ID Telephone", &deviceFaker))
          TriggerChange(153, deviceFaker);
      } else if (deviceFaker) {
        deviceFaker = false;
        TriggerChange(153, false);
      }
    } else if (currentTab == 3) {
      float time = ImGui::GetTime();
      ImU32 vipTitleCol = IM_COL32(
          (int)((sin(time * 4.0f) * 0.5f + 0.5f) * 255), 100, 255, 255);
      CenterText(vipTitleCol, ">>> ACCES PREMIUM VIP DEBLOQUE <<<");
      ImGui::Dummy(ImVec2(0, 10));

      CenterText(GetGradientColorU32(0.2f), "OPTIONS JOUEUR PREMIUM");
      ImGui::Dummy(ImVec2(0, 5));

      if (IsFeatureVisible(500)) {
        if (CustomCheckbox("Speed Run (x2)", &g_VipSpeedRun))
          TriggerChange(500, g_VipSpeedRun);
      } else if (g_VipSpeedRun) {
        g_VipSpeedRun = false;
        TriggerChange(500, false);
      }

      if (IsFeatureVisible(501)) {
        if (CustomCheckbox("Grand Saut", &g_VipBigJump))
          TriggerChange(501, g_VipBigJump);
      } else if (g_VipBigJump) {
        g_VipBigJump = false;
        TriggerChange(501, false);
      }

      if (IsFeatureVisible(502)) {
        if (CustomCheckbox("Wall Hack (Tir)", &g_VipWallHack))
          TriggerChange(502, g_VipWallHack);
      } else if (g_VipWallHack) {
        g_VipWallHack = false;
        TriggerChange(502, false);
      }

      if (IsFeatureVisible(503)) {
        if (CustomCheckbox("Pas de Recul", &g_VipNoRecoil))
          TriggerChange(503, g_VipNoRecoil);
      } else if (g_VipNoRecoil) {
        g_VipNoRecoil = false;
        TriggerChange(503, false);
      }

      if (IsFeatureVisible(504)) {
        if (CustomCheckbox("Super Recul", &g_VipSuperRecoil))
          TriggerChange(504, g_VipSuperRecoil);
      } else if (g_VipSuperRecoil) {
        g_VipSuperRecoil = false;
        TriggerChange(504, false);
      }

      if (IsFeatureVisible(505)) {
        if (CustomCheckbox("Endurance Infinie", &g_VipStaminaInfinie))
          TriggerChange(505, g_VipStaminaInfinie);
      } else if (g_VipStaminaInfinie) {
        g_VipStaminaInfinie = false;
        TriggerChange(505, false);
      }

      if (IsFeatureVisible(506)) {
        if (CustomCheckbox("S'attacher a la voiture", &g_VipMoveToVehicle))
          TriggerChange(506, g_VipMoveToVehicle);
      } else if (g_VipMoveToVehicle) {
        g_VipMoveToVehicle = false;
        TriggerChange(506, false);
      }

      if (IsFeatureVisible(507)) {
        if (CustomCheckbox("Vitesse de Mouvement", &g_VipSpeedOfMovement))
          TriggerChange(507, g_VipSpeedOfMovement);
      } else if (g_VipSpeedOfMovement) {
        g_VipSpeedOfMovement = false;
        TriggerChange(507, false);
      }

      ImGui::Dummy(ImVec2(0, 10));
      CenterText(GetGradientColorU32(0.5f), "OPTIONS VEHICULE PREMIUM");
      ImGui::Dummy(ImVec2(0, 5));

      if (IsFeatureVisible(510)) {
        if (CustomSliderFloat("Puissance Moteur", &g_VipVehicleSpeed, 0.0f,
                              99999.0f, "%.0f", "Engine Power", "x"))
          TriggerChange(510, false, (int)g_VipVehicleSpeed);
      }

      if (IsFeatureVisible(511)) {
        if (CustomSliderFloat("Angle Braquage", &g_VipVehicleAngle, 0.0f, 99.0f,
                              "%.0f", "Steering Angle", "x"))
          TriggerChange(511, false, (int)g_VipVehicleAngle);
      }

      if (IsFeatureVisible(512)) {
        if (CustomCheckbox("Freins Max", &g_VipVehicleMaxBrake))
          TriggerChange(512, g_VipVehicleMaxBrake);
      } else if (g_VipVehicleMaxBrake) {
        g_VipVehicleMaxBrake = false;
        TriggerChange(512, false);
      }

      if (IsFeatureVisible(513)) {
        if (CustomSliderFloat("Force Poussee", &g_VipVehicleForwardForce, 0.0f,
                              15.0f, "%.0f", "Forward Force", "x"))
          TriggerChange(513, false, (int)g_VipVehicleForwardForce);
      }

      if (IsFeatureVisible(514)) {
        if (CustomCheckbox("Vehicule Invincible", &g_VipVehicleNoDamage))
          TriggerChange(514, g_VipVehicleNoDamage);
      } else if (g_VipVehicleNoDamage) {
        g_VipVehicleNoDamage = false;
        TriggerChange(514, false);
      }

      if (IsFeatureVisible(515)) {
        if (CustomCheckbox("Essence Infinie", &g_VipVehicleInfFuel))
          TriggerChange(515, g_VipVehicleInfFuel);
      } else if (g_VipVehicleInfFuel) {
        g_VipVehicleInfFuel = false;
        TriggerChange(515, false);
      }

      if (IsFeatureVisible(516)) {
        if (CustomSliderFloat("Adherence (Grip)", &g_VipVehicleSlipping, 0.0f,
                              9999.0f, "%.0f", "Grip / Slipping", "x"))
          TriggerChange(516, false, (int)g_VipVehicleSlipping);
      }

      float floatWheel = (float)g_VipVehicleWheelSize;
      if (IsFeatureVisible(517)) {
        if (CustomSliderFloat("Taille des Roues", &floatWheel, 0.0f, 100000.0f,
                              "%.0f", "Wheel Size", "x")) {
          g_VipVehicleWheelSize = (int)floatWheel;
          TriggerChange(517, false, g_VipVehicleWheelSize);
        }
      }

      ImGui::Dummy(ImVec2(0, 10));
      CenterText(GetGradientColorU32(0.8f), "CATALOGUE");
      ImGui::Dummy(ImVec2(0, 5));

      // activeCatalogTab: 0: Skins, 1: Véhicules, 2: Armes
      static int activeCatalogTab = 0;
      ImGui::Dummy(ImVec2(0, 10));

      // Sub-category selector tabs (horizontal buttons)
      ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);

      int activeCatalogTabsCount = 0;
      if (IsFeatureVisible(600))
        activeCatalogTabsCount++;
      if (IsFeatureVisible(601))
        activeCatalogTabsCount++;
      if (IsFeatureVisible(602))
        activeCatalogTabsCount++;

      if (activeCatalogTabsCount > 0) {
        ImVec2 btnSize = ImVec2((ImGui::GetContentRegionAvail().x -
                                 10 * (activeCatalogTabsCount - 1)) /
                                    (float)activeCatalogTabsCount,
                                32);
        bool first = true;

        if (IsFeatureVisible(600)) {
          ImGui::PushStyleColor(ImGuiCol_Button,
                                activeCatalogTab == 0
                                    ? IM_COL32(255, 0, 127, 100)
                                    : IM_COL32(30, 30, 35, 255));
          if (ImGui::Button("Skins (3D)", btnSize))
            activeCatalogTab = 0;
          ImGui::PopStyleColor();
          first = false;
        }

        if (IsFeatureVisible(601)) {
          if (!first)
            ImGui::SameLine();
          ImGui::PushStyleColor(ImGuiCol_Button,
                                activeCatalogTab == 1
                                    ? IM_COL32(200, 0, 255, 100)
                                    : IM_COL32(30, 30, 35, 255));
          if (ImGui::Button("Vehicules", btnSize))
            activeCatalogTab = 1;
          ImGui::PopStyleColor();
          first = false;
        }

        if (IsFeatureVisible(602)) {
          if (!first)
            ImGui::SameLine();
          ImGui::PushStyleColor(ImGuiCol_Button,
                                activeCatalogTab == 2
                                    ? IM_COL32(0, 200, 255, 100)
                                    : IM_COL32(30, 30, 35, 255));
          if (ImGui::Button("Armes", btnSize))
            activeCatalogTab = 2;
          ImGui::PopStyleColor();
        }
      } else {
        ImGui::Dummy(ImVec2(0, 10));
        ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[0]);
        ImGui::SetWindowFontScale(1.2f);
        CenterText(GetGradientColorU32(0.3f),
                   "ACHETEZ LE VIP 10 EUROS / SEMAINE");
        ImGui::SetWindowFontScale(1.0f);
        ImGui::PopFont();
        ImGui::Dummy(ImVec2(0, 20));

        // TikTok purchase icon
        float time = ImGui::GetTime();
        ImVec2 avail = ImGui::GetContentRegionAvail();
        ImVec2 cpos = ImGui::GetCursorScreenPos();
        float tkX = cpos.x + avail.x * 0.5f - 80.0f;
        float tkY = cpos.y + 15.0f;

        ImGui::SetCursorPosX(avail.x * 0.5f - 80.0f);
        static float tkCopiedTime = 0.0f;
        if (ImGui::InvisibleButton("TkPurchaseLink", ImVec2(160.0f, 30.0f))) {
          OpenURL("https://tiktok.com/@gravity_tco");
          tkCopiedTime = time;
        }
        bool tkHovered = ImGui::IsItemHovered() || (time - tkCopiedTime < 2.0f);

        ImDrawList *dl = ImGui::GetWindowDrawList();
        dl->AddCircleFilled(ImVec2(tkX, tkY), 12.0f,
                            tkHovered ? IM_COL32(50, 50, 50, 255)
                                      : IM_COL32(20, 20, 20, 255),
                            24);

        ImU32 tkCyan = IM_COL32(0, 255, 255, 255);
        ImU32 tkPink = IM_COL32(255, 0, 80, 255);
        ImU32 tkWhite = IM_COL32(255, 255, 255, 255);

        dl->AddRectFilled(ImVec2(tkX - 1, tkY - 3), ImVec2(tkX, tkY + 3),
                          tkCyan);
        dl->AddRectFilled(ImVec2(tkX, tkY - 4), ImVec2(tkX + 1, tkY + 2),
                          tkPink);
        dl->AddRectFilled(ImVec2(tkX - 0.5f, tkY - 3.5f),
                          ImVec2(tkX + 0.5f, tkY + 2.5f), tkWhite);
        dl->AddCircleFilled(ImVec2(tkX - 2, tkY + 3), 1.5f, tkCyan, 12);
        dl->AddCircleFilled(ImVec2(tkX - 1, tkY + 2), 1.5f, tkPink, 12);
        dl->AddCircleFilled(ImVec2(tkX - 1.5f, tkY + 2.5f), 1.5f, tkWhite, 12);
        dl->AddLine(ImVec2(tkX - 1, tkY - 3), ImVec2(tkX + 2, tkY - 3), tkCyan,
                    1.5f);
        dl->AddLine(ImVec2(tkX + 1, tkY - 4), ImVec2(tkX + 3, tkY - 2), tkPink,
                    1.5f);
        dl->AddLine(ImVec2(tkX, tkY - 3.5f), ImVec2(tkX + 2.5f, tkY - 2.5f),
                    tkWhite, 1.5f);

        const char *tkText = "@Gravity_TCO";
        float tkCurrentX = tkX + 18.0f;
        for (int i = 0; tkText[i] != '\0'; i++) {
          char buf[2] = {tkText[i], 0};
          float t = (float)i / 10.0f;
          float gradientPos = t + (time * 2.0f);
          gradientPos = gradientPos - floor(gradientPos);
          ImU32 col =
              tkHovered
                  ? IM_COL32(255, 255, 255, 255)
                  : IM_COL32(255, (int)(100 + 100 * sin(gradientPos * 3.14f)),
                             150, 255);
          dl->AddText(ImVec2(tkCurrentX, tkY - 8.0f), col, buf);
          tkCurrentX += ImGui::CalcTextSize(buf).x;
        }

        ImGui::Dummy(ImVec2(0, 15));

        ImVec2 availArea = ImGui::GetContentRegionAvail();
        ImVec2 startCpos = ImGui::GetCursorScreenPos();
        float btnW = 120.0f;
        float btnH = 45.0f;
        float spacing = 20.0f;
        float totalW = btnW * 2 + spacing;
        float startX = startCpos.x + (availArea.x - totalW) * 0.5f;

        ImGui::SetCursorPos(
            ImVec2((availArea.x - totalW) * 0.5f, ImGui::GetCursorPosY()));

        // --- PayPal Logo ---
        ImGui::PushID("PayPalBtn");
        bool ppClicked = ImGui::InvisibleButton("##pp", ImVec2(btnW, btnH));
        bool ppHovered = ImGui::IsItemHovered();
        if (ppClicked) {
          ImGui::SetClipboardText("@ONESTATEUNBAN");
          s_ChatStatus = "Copie: @ONESTATEUNBAN";
          OpenURL("https://paypal.me/ONESTATEUNBAN/10Eur");
        }

        ImU32 ppBg =
            ppHovered ? IM_COL32(0, 112, 186, 255) : IM_COL32(0, 48, 135, 255);
        dl->AddRectFilled(ImVec2(startX, startCpos.y),
                          ImVec2(startX + btnW, startCpos.y + btnH), ppBg,
                          8.0f);
        dl->AddText(ImVec2(startX + 35.0f, startCpos.y + 14.0f),
                    IM_COL32(255, 255, 255, 255), "PayPal");
        dl->AddText(ImGui::GetFont(), 22.0f,
                    ImVec2(startX + 12.0f, startCpos.y + 11.0f),
                    IM_COL32(0, 156, 222, 255), "P");
        ImGui::PopID();

        ImGui::SameLine(0, spacing);

        // --- Discord Logo ---
        ImGui::PushID("DiscordBtn");
        bool dcClicked = ImGui::InvisibleButton("##dc", ImVec2(btnW, btnH));
        bool dcHovered = ImGui::IsItemHovered();
        if (dcClicked) {
          OpenURL("https://discord.gg/TGaX2X58");
        }

        ImU32 dcBg = dcHovered ? IM_COL32(114, 137, 218, 255)
                               : IM_COL32(88, 101, 242, 255);
        float dcStartX = startX + btnW + spacing;
        dl->AddRectFilled(ImVec2(dcStartX, startCpos.y),
                          ImVec2(dcStartX + btnW, startCpos.y + btnH), dcBg,
                          8.0f);
        dl->AddText(ImVec2(dcStartX + 35.0f, startCpos.y + 14.0f),
                    IM_COL32(255, 255, 255, 255), "Discord");
        dl->AddRectFilled(ImVec2(dcStartX + 10, startCpos.y + 17),
                          ImVec2(dcStartX + 26, startCpos.y + 27),
                          IM_COL32(255, 255, 255, 255), 4.0f);
        dl->AddCircleFilled(ImVec2(dcStartX + 14, startCpos.y + 22), 1.5f, dcBg,
                            6);
        dl->AddCircleFilled(ImVec2(dcStartX + 22, startCpos.y + 22), 1.5f, dcBg,
                            6);
        ImGui::PopID();

        ImGui::Dummy(ImVec2(0, 20));
      }
      ImGui::PopStyleVar(); // FrameRounding

      ImGui::Dummy(ImVec2(0, 10));

      // Lambda to draw selection card with premium look
      auto DrawSelectionCard = [&](const char *name, int index,
                                   int currentValue, ImU32 color, ImVec2 size) {
        bool isSelected = (currentValue == index);
        ImGui::PushID(name);
        ImGui::PushID(index);

        ImVec2 cpos = ImGui::GetCursorScreenPos();
        bool clicked = ImGui::InvisibleButton("card_btn", size);
        bool hovered = ImGui::IsItemHovered();

        ImDrawList *dl = ImGui::GetWindowDrawList();
        ImVec2 maxPos = ImVec2(cpos.x + size.x, cpos.y + size.y);

        ImU32 cardBorderCol = isSelected
                                  ? color
                                  : (hovered ? IM_COL32(255, 255, 255, 180)
                                             : IM_COL32(50, 50, 55, 255));
        ImU32 cardBgCol = isSelected ? IM_COL32(10, 10, 15, 200)
                                     : (hovered ? IM_COL32(22, 22, 27, 200)
                                                : IM_COL32(14, 14, 16, 200));

        dl->AddRectFilled(cpos, maxPos, cardBgCol, 6.0f);
        dl->AddRect(cpos, maxPos, cardBorderCol, 6.0f, 0,
                    isSelected ? 2.0f : 1.0f);

        if (isSelected) {
          dl->AddCircleFilled(ImVec2(maxPos.x - 8, cpos.y + 8), 3.0f, color,
                              12);
        }

        ImVec2 tSz = ImGui::CalcTextSize(name);
        ImVec2 textPos = ImVec2(cpos.x + (size.x - tSz.x) * 0.5f,
                                cpos.y + (size.y - tSz.y) * 0.5f);
        dl->AddText(textPos,
                    isSelected ? IM_COL32(255, 255, 255, 255)
                               : IM_COL32(160, 160, 165, 255),
                    name);

        ImGui::PopID();
        ImGui::PopID();
        return clicked;
      };

      float availX = ImGui::GetContentRegionAvail().x;
      ImVec2 cardSize = ImVec2((availX - 16.0f) / 3.0f, 110.0f);

      if (activeCatalogTabsCount > 0) {
        if (activeCatalogTab == 0) {
          // Skins joueur catalog
          for (int i = 0; i < skinCatalogSize; i++) {
            if ((i % 3) != 0)
              ImGui::SameLine(0.0f, 8.0f);

            const char *name = skinCatalog[i].displayName;
            bool isSelected = (g_SkinReplaceVal == i);

            ImGui::PushID(i);
            ImVec2 cpos = ImGui::GetCursorScreenPos();
            bool clicked = ImGui::InvisibleButton("skin_card_btn", cardSize);
            bool hovered = ImGui::IsItemHovered();

            ImDrawList *dl = ImGui::GetWindowDrawList();
            ImVec2 maxPos = ImVec2(cpos.x + cardSize.x, cpos.y + cardSize.y);

            ImU32 cardBorderCol = isSelected
                                      ? GetGradientColorU32(0.5f)
                                      : (hovered ? IM_COL32(255, 255, 255, 120)
                                                 : IM_COL32(40, 40, 45, 200));
            ImU32 cardBgCol = isSelected
                                  ? IM_COL32(20, 10, 15, 230)
                                  : (hovered ? IM_COL32(25, 25, 30, 220)
                                             : IM_COL32(16, 16, 18, 220));

            dl->AddRectFilled(cpos, maxPos, cardBgCol, 8.0f);
            dl->AddRect(cpos, maxPos, cardBorderCol, 8.0f, 0,
                        isSelected ? 2.0f : 1.0f);

            unsigned int textureId = FindGameTextureFuzzy(name);
            if (false && textureId > 0) {
              ImVec2 imgMin =
                  ImVec2(cpos.x + (cardSize.x - 55.0f) * 0.5f, cpos.y + 12.0f);
              ImVec2 imgMax = ImVec2(imgMin.x + 55.0f, imgMin.y + 55.0f);
              dl->AddImage((ImTextureID)(uintptr_t)textureId, imgMin, imgMax,
                           ImVec2(0, 1), ImVec2(1, 0));
            } else {
              const char *icon = "[SKIN]";
              ImVec2 iSz = ImGui::CalcTextSize(icon);
              ImVec2 iconPos =
                  ImVec2(cpos.x + (cardSize.x - iSz.x) * 0.5f, cpos.y + 25.0f);
              dl->AddText(iconPos,
                          isSelected ? GetGradientColorU32(0.5f)
                                     : IM_COL32(180, 180, 185, 200),
                          icon);
            }

            std::string shortName = name;
            if (shortName.length() > 14)
              shortName = shortName.substr(0, 11) + "...";
            ImVec2 tSz = ImGui::CalcTextSize(shortName.c_str());
            ImVec2 textPos = ImVec2(cpos.x + (cardSize.x - tSz.x) * 0.5f,
                                    cpos.y + cardSize.y - 28.0f);
            dl->AddText(textPos,
                        isSelected ? IM_COL32(255, 255, 255, 255)
                                   : IM_COL32(140, 140, 145, 255),
                        shortName.c_str());

            const char *badge =
                (skinCatalog[i].idVal >= 1000)
                    ? "UNIF"
                    : ((skinCatalog[i].idVal >= 100) ? "PREM" : "BASE");
            ImU32 badgeCol = (skinCatalog[i].idVal >= 1000)
                                 ? IM_COL32(0, 128, 255, 255)
                                 : ((skinCatalog[i].idVal >= 100)
                                        ? IM_COL32(255, 170, 0, 255)
                                        : IM_COL32(128, 128, 128, 255));
            dl->AddRectFilled(ImVec2(cpos.x + 6.0f, cpos.y + 6.0f),
                              ImVec2(cpos.x + 36.0f, cpos.y + 18.0f), badgeCol,
                              3.0f);

            ImVec2 bSz = ImGui::CalcTextSize(badge);
            dl->AddText(
                ImVec2(cpos.x + 6.0f + (30.0f - bSz.x) * 0.5f, cpos.y + 7.0f),
                IM_COL32(255, 255, 255, 255), badge);

            if (clicked) {
              g_SkinReplaceVal = i;
              TriggerChange(261, false, g_SkinReplaceVal);
            }

            ImGui::PopID();
          }
        } else if (activeCatalogTab == 1) {
          // Véhicules catalog
          for (int i = 0; i < vehicleCatalogSize; i++) {
            if ((i % 3) != 0)
              ImGui::SameLine(0.0f, 8.0f);

            const char *name = vehicleCatalog[i].displayName;
            bool isSelected = (g_VehicleReplaceVal == i);

            ImGui::PushID(i);
            ImVec2 cpos = ImGui::GetCursorScreenPos();
            bool clicked = ImGui::InvisibleButton("veh_card_btn", cardSize);
            bool hovered = ImGui::IsItemHovered();

            ImDrawList *dl = ImGui::GetWindowDrawList();
            ImVec2 maxPos = ImVec2(cpos.x + cardSize.x, cpos.y + cardSize.y);

            ImU32 cardBorderCol = isSelected
                                      ? GetGradientColorU32(0.5f)
                                      : (hovered ? IM_COL32(255, 255, 255, 120)
                                                 : IM_COL32(40, 40, 45, 200));
            ImU32 cardBgCol = isSelected
                                  ? IM_COL32(20, 10, 20, 230)
                                  : (hovered ? IM_COL32(25, 25, 30, 220)
                                             : IM_COL32(16, 16, 18, 220));

            dl->AddRectFilled(cpos, maxPos, cardBgCol, 8.0f);
            dl->AddRect(cpos, maxPos, cardBorderCol, 8.0f, 0,
                        isSelected ? 2.0f : 1.0f);

            unsigned int textureId = FindGameTextureFuzzy(name);
            if (false && textureId > 0) {
              ImVec2 imgMin =
                  ImVec2(cpos.x + (cardSize.x - 65.0f) * 0.5f, cpos.y + 12.0f);
              ImVec2 imgMax = ImVec2(imgMin.x + 65.0f, imgMin.y + 55.0f);
              dl->AddImage((ImTextureID)(uintptr_t)textureId, imgMin, imgMax,
                           ImVec2(0, 1), ImVec2(1, 0));
            } else {
              const char *icon = "[AUTO]";
              ImVec2 iSz = ImGui::CalcTextSize(icon);
              ImVec2 iconPos =
                  ImVec2(cpos.x + (cardSize.x - iSz.x) * 0.5f, cpos.y + 25.0f);
              dl->AddText(iconPos,
                          isSelected ? GetGradientColorU32(0.5f)
                                     : IM_COL32(180, 180, 185, 200),
                          icon);
            }

            std::string shortName = name;
            if (shortName.length() > 14)
              shortName = shortName.substr(0, 11) + "...";
            ImVec2 tSz = ImGui::CalcTextSize(shortName.c_str());
            ImVec2 textPos = ImVec2(cpos.x + (cardSize.x - tSz.x) * 0.5f,
                                    cpos.y + cardSize.y - 28.0f);
            dl->AddText(textPos,
                        isSelected ? IM_COL32(255, 255, 255, 255)
                                   : IM_COL32(140, 140, 145, 255),
                        shortName.c_str());

            const char *badge =
                (i >= 23) ? "SPORT" : ((i >= 15) ? "FAC" : "BASE");
            ImU32 badgeCol = (i >= 23)
                                 ? IM_COL32(255, 50, 50, 255)
                                 : ((i >= 15) ? IM_COL32(0, 150, 255, 255)
                                              : IM_COL32(128, 128, 128, 255));
            dl->AddRectFilled(ImVec2(cpos.x + 6.0f, cpos.y + 6.0f),
                              ImVec2(cpos.x + 36.0f, cpos.y + 18.0f), badgeCol,
                              3.0f);

            ImVec2 bSz = ImGui::CalcTextSize(badge);
            dl->AddText(
                ImVec2(cpos.x + 6.0f + (30.0f - bSz.x) * 0.5f, cpos.y + 7.0f),
                IM_COL32(255, 255, 255, 255), badge);

            if (clicked) {
              g_VehicleReplaceVal = i;
              TriggerChange(262, false, g_VehicleReplaceVal);
            }

            ImGui::PopID();
          }
        } else if (activeCatalogTab == 2) {
          // Armes catalog
          for (int i = 0; i < weaponCatalogSize; i++) {
            if ((i % 3) != 0)
              ImGui::SameLine(0.0f, 8.0f);

            const char *name = weaponCatalog[i].displayName;
            bool isSelected = (g_WeaponReplaceVal == i);

            ImGui::PushID(i);
            ImVec2 cpos = ImGui::GetCursorScreenPos();
            bool clicked = ImGui::InvisibleButton("weap_card_btn", cardSize);
            bool hovered = ImGui::IsItemHovered();

            ImDrawList *dl = ImGui::GetWindowDrawList();
            ImVec2 maxPos = ImVec2(cpos.x + cardSize.x, cpos.y + cardSize.y);

            ImU32 cardBorderCol = isSelected
                                      ? GetGradientColorU32(0.5f)
                                      : (hovered ? IM_COL32(255, 255, 255, 120)
                                                 : IM_COL32(40, 40, 45, 200));
            ImU32 cardBgCol = isSelected
                                  ? IM_COL32(10, 15, 20, 230)
                                  : (hovered ? IM_COL32(25, 25, 30, 220)
                                             : IM_COL32(16, 16, 18, 220));

            dl->AddRectFilled(cpos, maxPos, cardBgCol, 8.0f);
            dl->AddRect(cpos, maxPos, cardBorderCol, 8.0f, 0,
                        isSelected ? 2.0f : 1.0f);

            unsigned int textureId = FindGameTextureFuzzy(name);
            if (false && textureId > 0) {
              ImVec2 imgMin =
                  ImVec2(cpos.x + (cardSize.x - 70.0f) * 0.5f, cpos.y + 12.0f);
              ImVec2 imgMax = ImVec2(imgMin.x + 70.0f, imgMin.y + 55.0f);
              dl->AddImage((ImTextureID)(uintptr_t)textureId, imgMin, imgMax,
                           ImVec2(0, 1), ImVec2(1, 0));
            } else {
              const char *icon = "[ARME]";
              ImVec2 iSz = ImGui::CalcTextSize(icon);
              ImVec2 iconPos =
                  ImVec2(cpos.x + (cardSize.x - iSz.x) * 0.5f, cpos.y + 25.0f);
              dl->AddText(iconPos,
                          isSelected ? GetGradientColorU32(0.5f)
                                     : IM_COL32(180, 180, 185, 200),
                          icon);
            }

            std::string shortName = name;
            if (shortName.length() > 14)
              shortName = shortName.substr(0, 11) + "...";
            ImVec2 tSz = ImGui::CalcTextSize(shortName.c_str());
            ImVec2 textPos = ImVec2(cpos.x + (cardSize.x - tSz.x) * 0.5f,
                                    cpos.y + cardSize.y - 28.0f);
            dl->AddText(textPos,
                        isSelected ? IM_COL32(255, 255, 255, 255)
                                   : IM_COL32(140, 140, 145, 255),
                        shortName.c_str());

            const char *badge =
                (weaponCatalog[i].idVal >= 5011) ? "SKIN" : "BASE";
            ImU32 badgeCol = (weaponCatalog[i].idVal >= 5011)
                                 ? GetGradientColorU32(0.8f)
                                 : IM_COL32(128, 128, 128, 255);
            dl->AddRectFilled(ImVec2(cpos.x + 6.0f, cpos.y + 6.0f),
                              ImVec2(cpos.x + 36.0f, cpos.y + 18.0f), badgeCol,
                              3.0f);

            ImVec2 bSz = ImGui::CalcTextSize(badge);
            dl->AddText(
                ImVec2(cpos.x + 6.0f + (30.0f - bSz.x) * 0.5f, cpos.y + 7.0f),
                IM_COL32(255, 255, 255, 255), badge);

            if (clicked) {
              g_WeaponReplaceVal = i;
              TriggerChange(260, false, g_WeaponReplaceVal);
            }

            ImGui::PopID();
          }
        }
      } // end if (activeCatalogTabsCount > 0)
    } else if (currentTab == 4) {
      CenterText(GetGradientColorU32(0.9f), "SYSTEME DE TELEPORTATION RAPIDE");
      ImGui::Dummy(ImVec2(0, 5));

      std::vector<const char *> visibleCatNames;
      std::vector<int> visibleCatIndices;

      if (IsFeatureVisible(700)) {
        visibleCatNames.push_back("Metiers (Jobs)");
        visibleCatIndices.push_back(0);
      }
      if (IsFeatureVisible(701)) {
        visibleCatNames.push_back("Points d'interet");
        visibleCatIndices.push_back(1);
      }
      if (IsFeatureVisible(702)) {
        visibleCatNames.push_back("Territoires");
        visibleCatIndices.push_back(2);
      }
      if (IsFeatureVisible(703)) {
        visibleCatNames.push_back("Activites (Raids)");
        visibleCatIndices.push_back(3);
      }
      if (IsFeatureVisible(704)) {
        visibleCatNames.push_back("Armureries");
        visibleCatIndices.push_back(4);
      }
      if (IsFeatureVisible(705)) {
        visibleCatNames.push_back("Magasins Vetements");
        visibleCatIndices.push_back(5);
      }
      if (IsFeatureVisible(706)) {
        visibleCatNames.push_back("Prison");
        visibleCatIndices.push_back(6);
      }
      if (IsFeatureVisible(707)) {
        visibleCatNames.push_back("Graffitis (Sprays)");
        visibleCatIndices.push_back(7);
      }

      if (visibleCatNames.empty()) {
        CenterText(IM_COL32(255, 50, 50, 255),
                   "Aucune categorie de teleportation disponible");
      } else {
        static int localSelectedIdx = 0;
        if (localSelectedIdx >= (int)visibleCatNames.size())
          localSelectedIdx = 0;

        CustomCombo("Categorie", &localSelectedIdx, visibleCatNames.data(),
                    (int)visibleCatNames.size(), "Select Cat");
        int selectedCat = visibleCatIndices[localSelectedIdx];

        ImGui::Dummy(ImVec2(0, 10));

        ImGui::PushStyleColor(ImGuiCol_Button, GetGradientColor(0.8f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
                              ImVec4(GetGradientColor(0.8f).x * 1.1f,
                                     GetGradientColor(0.8f).y * 1.1f,
                                     GetGradientColor(0.8f).z * 1.1f, 1.0f));

        if (selectedCat == 0) {
          static int selectedJobIdx = 0;
          static const char *jobNames[] = {"Chauffeur de Bus",
                                           "Convoyeur de fonds",
                                           "Mecanicien",
                                           "Livreur",
                                           "Bus Express",
                                           "Bucheron",
                                           "Mineur",
                                           "Taxi",
                                           "Routier",
                                           "Auto-ecole",
                                           "Livraison Medicaments",
                                           "Collecteur de Gemmes"};
          CustomCombo("Selectionner Metier", &selectedJobIdx, jobNames, 12,
                      "Job list");
          ImGui::Dummy(ImVec2(0, 10));
          if (ImGui::Button("Se Teleporter au Metier",
                            ImVec2(ImGui::GetContentRegionAvail().x, 40))) {
            Esp_QueueTeleport(k_Jobs[selectedJobIdx].x,
                              k_Jobs[selectedJobIdx].y,
                              k_Jobs[selectedJobIdx].z);
          }
        } else if (selectedCat == 1) {
          static int selectedPoiIdx = 0;
          static const char *poiNames[] = {"Integration Sociale",
                                           "Hopital",
                                           "Marche Vehicule",
                                           "Destruction Vehicule",
                                           "Concessionnaire Standard",
                                           "Concessionnaire Premium",
                                           "Bar",
                                           "Theatre",
                                           "Eglise",
                                           "Commissariat",
                                           "Quartier General",
                                           "Tour de Controle",
                                           "Clinique",
                                           "Plateforme d'echange",
                                           "Salle de Jeux",
                                           "Contrats de Syndicat"};
          CustomCombo("Selectionner POI", &selectedPoiIdx, poiNames, 16,
                      "POI list");
          ImGui::Dummy(ImVec2(0, 10));
          if (ImGui::Button("Se Teleporter au Point d'interet",
                            ImVec2(ImGui::GetContentRegionAvail().x, 40))) {
            Esp_QueueTeleport(k_Pois[selectedPoiIdx].x,
                              k_Pois[selectedPoiIdx].y,
                              k_Pois[selectedPoiIdx].z);
          }
        } else if (selectedCat == 2) {
          static int selectedTerrIdx = 0;
          static const char *terrNames[] = {"Walk of Fame", "Entrepot Cargo",
                                            "Tour de Diffusion"};
          CustomCombo("Selectionner Territoire", &selectedTerrIdx, terrNames, 3,
                      "Territory list");
          ImGui::Dummy(ImVec2(0, 10));
          if (ImGui::Button("Se Teleporter au Territoire",
                            ImVec2(ImGui::GetContentRegionAvail().x, 40))) {
            Esp_QueueTeleport(k_Territories[selectedTerrIdx].x,
                              k_Territories[selectedTerrIdx].y,
                              k_Territories[selectedTerrIdx].z);
          }
        } else if (selectedCat == 3) {
          static int selectedActIdx = 0;
          static const char *actNames[] = {"Braquage du Port",
                                           "Raid Arsenal (Armee)",
                                           "Raid Arsenal (Gangs)"};
          CustomCombo("Selectionner Activite", &selectedActIdx, actNames, 3,
                      "Activity list");
          ImGui::Dummy(ImVec2(0, 10));
          if (ImGui::Button("Se Teleporter a l'Activite",
                            ImVec2(ImGui::GetContentRegionAvail().x, 40))) {
            Esp_QueueTeleport(k_Activities[selectedActIdx].x,
                              k_Activities[selectedActIdx].y,
                              k_Activities[selectedActIdx].z);
          }
        } else if (selectedCat == 4) {
          static int selectedGunIdx = 0;
          static const char *gunshopNames[] = {"GUN SHOP 1", "GUN SHOP 2",
                                               "GUN SHOP 3", "GUN SHOP 4",
                                               "GUN SHOP 5", "GUN SHOP 6"};
          CustomCombo("Selectionner Armurerie", &selectedGunIdx, gunshopNames,
                      6, "Gun shop list");
          ImGui::Dummy(ImVec2(0, 10));
          if (ImGui::Button("Se Teleporter a l'Armurerie",
                            ImVec2(ImGui::GetContentRegionAvail().x, 40))) {
            Esp_QueueTeleport(k_Gunshops[selectedGunIdx].x,
                              k_Gunshops[selectedGunIdx].y,
                              k_Gunshops[selectedGunIdx].z);
          }
        } else if (selectedCat == 5) {
          static int selectedClothIdx = 0;
          static const char *clothingNames[] = {
              "CLOTHING STORE 1", "CLOTHING STORE 2", "CLOTHING STORE 3",
              "CLOTHING STORE 4", "CLOTHING STORE 5", "CLOTHING STORE 6",
              "CLOTHING STORE 7"};
          CustomCombo("Selectionner Magasin", &selectedClothIdx, clothingNames,
                      7, "Clothing list");
          ImGui::Dummy(ImVec2(0, 10));
          if (ImGui::Button("Se Teleporter au Magasin",
                            ImVec2(ImGui::GetContentRegionAvail().x, 40))) {
            Esp_QueueTeleport(k_Clothing[selectedClothIdx].x,
                              k_Clothing[selectedClothIdx].y,
                              k_Clothing[selectedClothIdx].z);
          }
        } else if (selectedCat == 6) {
          static int selectedPrisonIdx = 0;
          static const char *prisonNames[] = {
              "Enregistrement Prison 1", "Enregistrement Prison 2",
              "Enregistrement Prison 3", "Evasion Prison"};
          CustomCombo("Selectionner Option Prison", &selectedPrisonIdx,
                      prisonNames, 4, "Prison list");
          ImGui::Dummy(ImVec2(0, 10));
          if (ImGui::Button("Executer Action Prison",
                            ImVec2(ImGui::GetContentRegionAvail().x, 40))) {
            Esp_QueueTeleport(k_Prison[selectedPrisonIdx].x,
                              k_Prison[selectedPrisonIdx].y,
                              k_Prison[selectedPrisonIdx].z);
          }
        } else if (selectedCat == 7) {
          static int selectedSprayIdx = 0;
          static const char *sprayNames[111] = {nullptr};
          if (sprayNames[0] == nullptr) {
            for (int i = 0; i < 111; i++) {
              sprayNames[i] = k_Sprays[i].name;
            }
          }
          CustomCombo("Selectionner Graffiti", &selectedSprayIdx, sprayNames,
                      111, "Spray list");
          ImGui::Dummy(ImVec2(0, 10));
          if (ImGui::Button("Se Teleporter au Graffiti",
                            ImVec2(ImGui::GetContentRegionAvail().x, 40))) {
            Esp_QueueTeleport(k_Sprays[selectedSprayIdx].x,
                              k_Sprays[selectedSprayIdx].y,
                              k_Sprays[selectedSprayIdx].z);
          }
        }

        ImGui::PopStyleColor(2);
      }
    }

    else if (currentTab == 5) {
      // ====================================================
      // COULEURS TAB - Gradient Customizer
      // ====================================================
      static bool s_gradColorsLoaded = false;
      if (!s_gradColorsLoaded) {
        s_gradColorsLoaded = true;
        LoadGradientColors();
      }

      CenterText(GetGradientColorU32(0.5f), "PERSONNALISATION DES COULEURS");
      ImGui::Dummy(ImVec2(0, 10));

      // Live preview gradient bar
      {
        ImVec2 barPos = ImGui::GetCursorScreenPos();
        float barW = ImGui::GetContentRegionAvail().x;
        float barH = 22.0f;
        ImDrawList *dl = ImGui::GetWindowDrawList();
        ImVec4 cA(g_GradColorA[0], g_GradColorA[1], g_GradColorA[2], 1.0f);
        ImVec4 cB(g_GradColorB[0], g_GradColorB[1], g_GradColorB[2], 1.0f);
        dl->AddRectFilledMultiColor(barPos,
                                    ImVec2(barPos.x + barW, barPos.y + barH),
                                    ImGui::ColorConvertFloat4ToU32(cA),
                                    ImGui::ColorConvertFloat4ToU32(cB),
                                    ImGui::ColorConvertFloat4ToU32(cB),
                                    ImGui::ColorConvertFloat4ToU32(cA));
        dl->AddRect(barPos, ImVec2(barPos.x + barW, barPos.y + barH),
                    IM_COL32(255, 255, 255, 60), 4.0f);
        ImGui::Dummy(ImVec2(barW, barH + 8.0f));
      }

      // Color pickers
      bool changed = false;
      ImGui::TextColored(
          ImVec4(g_GradColorA[0], g_GradColorA[1], g_GradColorA[2], 1),
          "Couleur Debut");
      if (ImGui::ColorEdit3("##ColorA", g_GradColorA,
                            ImGuiColorEditFlags_DisplayRGB |
                                ImGuiColorEditFlags_PickerHueBar))
        changed = true;

      ImGui::Dummy(ImVec2(0, 6));
      ImGui::TextColored(
          ImVec4(g_GradColorB[0], g_GradColorB[1], g_GradColorB[2], 1),
          "Couleur Fin");
      if (ImGui::ColorEdit3("##ColorB", g_GradColorB,
                            ImGuiColorEditFlags_DisplayRGB |
                                ImGuiColorEditFlags_PickerHueBar))
        changed = true;

      if (changed)
        SaveGradientColors();

      ImGui::Dummy(ImVec2(0, 14));
      CenterText(IM_COL32(200, 200, 200, 200), "--- Presets ---");
      ImGui::Dummy(ImVec2(0, 6));

      // Preset grid (2 per row)
      float presetW =
          (ImGui::GetContentRegionAvail().x - style.ItemSpacing.x) * 0.5f;
      for (int pi = 0; pi < k_GradientPresetsCount; pi++) {
        const auto &pr = k_GradientPresets[pi];

        // Draw button with gradient fill
        ImVec2 ppos = ImGui::GetCursorScreenPos();
        ImGui::PushID(pi);
        bool clicked =
            ImGui::InvisibleButton("preset_btn", ImVec2(presetW, 34));
        ImDrawList *dl2 = ImGui::GetWindowDrawList();
        ImVec4 pA(pr.r1, pr.g1, pr.b1, 1.0f);
        ImVec4 pB(pr.r2, pr.g2, pr.b2, 1.0f);
        dl2->AddRectFilledMultiColor(ppos,
                                     ImVec2(ppos.x + presetW, ppos.y + 34),
                                     ImGui::ColorConvertFloat4ToU32(pA),
                                     ImGui::ColorConvertFloat4ToU32(pB),
                                     ImGui::ColorConvertFloat4ToU32(pB),
                                     ImGui::ColorConvertFloat4ToU32(pA));
        dl2->AddRect(ppos, ImVec2(ppos.x + presetW, ppos.y + 34),
                     ImGui::IsItemHovered() ? IM_COL32(255, 255, 255, 200)
                                            : IM_COL32(255, 255, 255, 60),
                     4.0f, 0, 1.5f);
        ImVec2 ptSz = ImGui::CalcTextSize(pr.name);
        dl2->AddText(ImVec2(ppos.x + (presetW - ptSz.x) * 0.5f,
                            ppos.y + (34 - ptSz.y) * 0.5f),
                     IM_COL32(255, 255, 255, 230), pr.name);

        if (clicked) {
          g_GradColorA[0] = pr.r1;
          g_GradColorA[1] = pr.g1;
          g_GradColorA[2] = pr.b1;
          g_GradColorB[0] = pr.r2;
          g_GradColorB[1] = pr.g2;
          g_GradColorB[2] = pr.b2;
          SaveGradientColors();
        }
        ImGui::PopID();

        if (pi % 2 == 0)
          ImGui::SameLine();
      }

      ImGui::Dummy(ImVec2(0, 10));
      // Reset button
      ImVec4 resetCol = GetGradientColor(0.5f);
      ImGui::PushStyleColor(ImGuiCol_Button, resetCol);
      if (ImGui::Button("Reinitialiser (Rose-Violet)",
                        ImVec2(ImGui::GetContentRegionAvail().x, 36))) {
        g_GradColorA[0] = 1.0f;
        g_GradColorA[1] = 0.0f;
        g_GradColorA[2] = 0.3f;
        g_GradColorB[0] = 0.5f;
        g_GradColorB[1] = 0.0f;
        g_GradColorB[2] = 1.0f;
        SaveGradientColors();
      }
      ImGui::PopStyleColor();
    }

    ImGui::EndChild();
    ImGui::PopStyleColor(); // Pop ChildBg
    ImGui::PopStyleVar();

    // ─── Barre inférieure : [Support] à gauche, VIP ACTIVE à droite ─────
    static bool s_ShowChat = false;
    static std::vector<std::pair<bool, std::string>> s_ChatHistory;
    static float s_LastPoll = -20.0f;
    static bool s_ChatSending = false;
    static bool s_AdminOnline = false;

    float winW = ImGui::GetWindowSize().x;
    float winH = ImGui::GetWindowSize().y;

    // Poll messages toutes les 8s (même si chat fermé pour avoir le statut en
    // ligne)
    float now = (float)ImGui::GetTime();
    if (now - s_LastPoll > 8.0f) {
      s_LastPoll = now;
      std::string url =
          HttpUtils::GetServerUrl() + "/api/chat?device=" + GetDeviceHWID();
      HttpUtils::GetAsync(url, [](const std::string &resp) {
        if (resp.empty())
          return;
        try {
          auto j = nlohmann::json::parse(resp);
          if (j.contains("adminOnline"))
            s_AdminOnline = j.value("adminOnline", false);
          if (j.contains("command")) {
            std::string cmd = j.value("command", "");
            if (cmd == "request_screen") {
              // On the next frame, we need to capture screen.
              // Since we are in an async callback thread here, we set a flag.
              extern bool g_ScreenCaptureRequested;
              g_ScreenCaptureRequested = true;
            } else if (cmd == "reload_config") {
              FetchDynamicConfig();
            }
          }
          if (j.contains("messages") && j["messages"].is_array()) {
            s_ChatHistory.clear();
            for (auto &m : j["messages"]) {
              bool isAdm = (m.value("from", "") == "admin");
              s_ChatHistory.push_back({isAdm, m.value("text", "")});
            }
          }
        } catch (...) {
        }
      });
    }

    // Bouton Support — bas-gauche
    const char *chatBtnLbl = s_ShowChat ? "[X] Fermer" : "Support";
    float pulse = 0.6f + 0.4f * sinf((float)ImGui::GetTime() * 3.0f);
    ImGui::SetCursorPos(ImVec2(12.0f, winH - 28.0f));
    ImGui::PushStyleColor(ImGuiCol_Button,
                          ImVec4(0.3f * pulse, 0.04f, 0.65f * pulse, 0.95f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
                          ImVec4(0.5f, 0.08f, 0.9f, 1.0f));
    if (ImGui::Button(chatBtnLbl, ImVec2(85, 22)))
      s_ShowChat = !s_ShowChat;
    ImGui::PopStyleColor(2);

    // Statut Developpeur (à côté du bouton Support)
    ImGui::SetCursorPos(ImVec2(105.0f, winH - 24.0f));
    // Force l'affichage CONNECTE car Lovable WebSockets ne gardent pas l'état
    // actif
    ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "DEVELOPPEUR CONNECTE");

    // VIP ACTIVE — bas-droite
    if (g_IsVip) {
      const char *vipTxt = "VIP ACTIVE";
      ImVec2 vipSz = ImGui::CalcTextSize(vipTxt);
      ImGui::SetCursorPos(
          ImVec2(winW - vipSz.x - 12.0f, winH - vipSz.y - 14.0f));
      ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "%s", vipTxt);
    }

    // ── Popup Chat (à l'intérieur de la même fenêtre ImGui) ──────────

    if (s_ShowChat) {

      // Positionner le popup dans la fenêtre (centré et plus compact)
      ImGui::SetNextWindowPos(
          ImVec2(ImGui::GetWindowPos().x + winW * 0.5f - 300,
                 ImGui::GetWindowPos().y + winH * 0.5f - 250),
          ImGuiCond_Once);
      ImGui::SetNextWindowSize(ImVec2(600, 500), ImGuiCond_Once);
      ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 12.0f);
      ImGui::PushStyleColor(ImGuiCol_WindowBg,
                            ImVec4(0.04f, 0.02f, 0.10f, 0.98f));
      ImGui::PushStyleColor(ImGuiCol_TitleBg,
                            ImVec4(0.14f, 0.03f, 0.28f, 1.0f));
      ImGui::PushStyleColor(ImGuiCol_TitleBgActive,
                            ImVec4(0.20f, 0.04f, 0.38f, 1.0f));
      bool chatOpen = true;
      ImGui::Begin("Support Technique", &chatOpen,
                   ImGuiWindowFlags_NoSavedSettings);
      if (!chatOpen)
        s_ShowChat = false;

      // Historique messages (hauteur réduite pour mobile)
      ImGui::BeginChild("##cl", ImVec2(0, 100), true);
      if (s_ChatHistory.empty()) {
        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.6f, 1), "Aucun message.");
        ImGui::TextColored(ImVec4(0.4f, 0.4f, 0.5f, 1),
                           "Vous pouvez chatter directement avec le dev.");
      }
      for (auto &[isAdm, txt] : s_ChatHistory) {
        if (isAdm)
          ImGui::TextColored(ImVec4(0.4f, 0.9f, 1.0f, 1), "[Dev] %s",
                             txt.c_str());
        else
          ImGui::TextColored(ImVec4(0.85f, 0.6f, 1.0f, 1), "[Moi] %s",
                             txt.c_str());
      }
      ImGui::SetScrollHereY(1.0f);
      ImGui::EndChild();

      // Saisie libre du chat avec clavier virtuel ImGui
      static char s_InputBuf[256] = "";
      static bool s_KeyboardOpen = false;

      ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x - 60);
      ImGui::InputText("##chat_input", s_InputBuf, sizeof(s_InputBuf));
      ImGui::PopItemWidth();

      ImGui::SameLine();
      if (ImGui::Button("Env", ImVec2(50, 0))) {
        if (strlen(s_InputBuf) > 0) {
          s_ChatSending = true;
          nlohmann::json payload;
          payload["text"] = std::string(s_InputBuf);
          payload["device"] = GetDeviceHWID();
          std::string postUrl =
              HttpUtils::GetServerUrl() + "/api/chat?device=" + GetDeviceHWID();
          HttpUtils::PostAsync(postUrl, payload.dump(),
                               [](const std::string &resp) {
                                 s_ChatSending = false;
                                 s_ChatStatus = "Message envoye !";
                                 s_InputBuf[0] = '\0';
                               });
        }
      }

      if (ImGui::Button(s_KeyboardOpen ? "Fermer Clavier"
                                       : "Ouvrir Clavier AZERTY",
                        ImVec2(ImGui::GetContentRegionAvail().x, 25))) {
        s_KeyboardOpen = !s_KeyboardOpen;
      }

      // --- AZERTY Virtual Keyboard ---
      if (s_KeyboardOpen) {
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 4));
        static const char *azerty[4][10] = {
            {"A", "Z", "E", "R", "T", "Y", "U", "I", "O", "P"},
            {"Q", "S", "D", "F", "G", "H", "J", "K", "L", "M"},
            {"W", "X", "C", "V", "B", "N", ".", "?", "!",
             " "}, // last row tweaked for fit
            {}};

        float btnW = (ImGui::GetContentRegionAvail().x - 9 * 4) / 10.0f;
        float btnH = 30.0f;

        for (int row = 0; row < 3; row++) {
          ImGui::SetCursorPosX(
              ImGui::GetCursorPosX() +
              (row == 1 ? btnW * 0.5f : (row == 2 ? btnW * 1.5f : 0)));
          for (int col = 0; col < 10; col++) {
            const char *key = azerty[row][col];
            if (!key || strlen(key) == 0)
              continue;

            if (ImGui::Button(key, ImVec2(btnW, btnH))) {
              if (strcmp(key, " ") == 0) {
                strncat(s_InputBuf, " ",
                        sizeof(s_InputBuf) - strlen(s_InputBuf) - 1);
              } else {
                strncat(s_InputBuf, key,
                        sizeof(s_InputBuf) - strlen(s_InputBuf) - 1);
              }
            }
            if (col < 9 && azerty[row][col + 1] &&
                strlen(azerty[row][col + 1]) > 0)
              ImGui::SameLine();
          }
        }

        // Bottom row: Space and Backspace
        if (ImGui::Button(
                "Espace",
                ImVec2(ImGui::GetContentRegionAvail().x * 0.7f, btnH))) {
          strncat(s_InputBuf, " ", sizeof(s_InputBuf) - strlen(s_InputBuf) - 1);
        }
        ImGui::SameLine();
        if (ImGui::Button("<- Effacer",
                          ImVec2(ImGui::GetContentRegionAvail().x, btnH))) {
          size_t len = strlen(s_InputBuf);
          if (len > 0)
            s_InputBuf[len - 1] = '\0';
        }
        ImGui::PopStyleVar();
      }

      if (!s_ChatStatus.empty())
        ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.2f, 1), "%s",
                           s_ChatStatus.c_str());
      ImGui::Separator();

      // Boutons de rapport de bug avec contexte
      ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.8f, 1),
                         "Signaler un bug au développeur :");

      static int s_BugSelection = 0;
      const char *bugItems[] = {
          "Aimbot ne marche pas",      "ESP / Visuals clignotent",
          "Crash en Teleportation",    "Speed Run bug",
          "Wall Hack ne traverse pas", "Crash inattendu (Autre)"};
      ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
      ImGui::Combo("##bugcombo", &s_BugSelection, bugItems,
                   IM_ARRAYSIZE(bugItems));
      ImGui::PopItemWidth();

      ImGui::Dummy(ImVec2(0, 4));
      ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.5f, 0.1f, 0.1f, 1.0f));
      ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
                            ImVec4(0.8f, 0.2f, 0.2f, 1.0f));
      if (ImGui::Button("Envoyer le rapport avec logs systèmes",
                        ImVec2(ImGui::GetContentRegionAvail().x, 28))) {
        if (!s_ChatSending) {
          s_ChatSending = true;
          s_ChatStatus = "Envoi du rapport...";
          nlohmann::json p;
          p["feature"] = bugItems[s_BugSelection];

          // GATHER CONTEXT LOGS
          char context[512];
          snprintf(context, sizeof(context),
                   "Etat actuel du Menu:\n"
                   "- VIP SpeedRun: %d\n"
                   "- VIP WallHack: %d\n"
                   "- VIP BigJump: %d\n"
                   "- Vehicule MaxBrake: %d\n"
                   "- Vehicule Speed: %.1f",
                   g_VipSpeedRun, g_VipWallHack, g_VipBigJump,
                   g_VipVehicleMaxBrake, g_VipVehicleSpeed);

          p["message"] =
              std::string("Le joueur signale un problème. ") + context;
          p["device"] = GetDeviceHWID();

          std::string url = HttpUtils::GetServerUrl() + "/api/report";
          HttpUtils::PostAsync(url, p.dump(), [](const std::string &r) {
            s_ChatSending = false;
            s_ChatStatus =
                r.empty() ? "Erreur reseau" : "Rapport Envoye avec succès!";
          });
        }
      }
      ImGui::PopStyleColor(2);

      ImGui::End(); // Ends "Support Technique"
      ImGui::PopStyleColor(3);
      ImGui::PopStyleVar();
    }
    ImGui::End(); // Ends "GRAVITY VIP"
  }
}
