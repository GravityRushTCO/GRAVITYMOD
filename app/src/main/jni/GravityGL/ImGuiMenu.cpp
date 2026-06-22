#include "ImGuiMenu.h"
#include "../BuildVersion.h"
#include "../HttpUtils.h"
#include "../Hwid.h"
#include "../Includes/Logger.h"
#include "../Includes/obfuscate.h"
#include "../LicenseSystem.h"
#include "../imgui/imgui.h"
#include "../json.hpp"
#include "Catalog.h"
#include "Esp.h"
#include "HexagonBackground.h"
#include "ModelRenderer.h"
#include "PlanetRenderer.h"
#include <atomic>
#include <set>
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
#include <string.h>
#include <vector>


int g_MenuLanguage = 0; // 0=FR, 1=EN, 2=AR, 3=ES
const char *TR(const char *fr) {
  if (strcmp(fr, "Activer Aimbot") == 0) {
    if (g_MenuLanguage == 1)
      return "Enable Aimbot";
    if (g_MenuLanguage == 2)
      return "تفعيل التصويب";
    if (g_MenuLanguage == 3)
      return "Activar Aimbot";
  }
  if (strcmp(fr, "Tirer a travers les murs") == 0) {
    if (g_MenuLanguage == 1)
      return "Wallbang";
    if (g_MenuLanguage == 2)
      return "التصويب عبر الجدران";
    if (g_MenuLanguage == 3)
      return "Disparar a través de las paredes";
  }
  if (strcmp(fr, "Verification de visibilite") == 0) {
    if (g_MenuLanguage == 1)
      return "Visibility Check";
    if (g_MenuLanguage == 2)
      return "التحقق من الرؤية";
    if (g_MenuLanguage == 3)
      return "Verificar visibilidad";
  }
  if (strcmp(fr, "Lissage de visee") == 0) {
    if (g_MenuLanguage == 1)
      return "Aim Smoothing";
    if (g_MenuLanguage == 2)
      return "تنعيم التصويب";
    if (g_MenuLanguage == 3)
      return "Suavizado de apuntado";
  }
  if (strcmp(fr, "Os cible") == 0) {
    if (g_MenuLanguage == 1)
      return "Target Bone";
    if (g_MenuLanguage == 2)
      return "عظم الهدف";
    if (g_MenuLanguage == 3)
      return "Hueso objetivo";
  }
  if (strcmp(fr, "Suivre Joueur / Voiture Auto") == 0) {
    if (g_MenuLanguage == 1)
      return "Auto-Follow Target";
    if (g_MenuLanguage == 2)
      return "متابعة تلقائية للهدف";
    if (g_MenuLanguage == 3)
      return "Seguir objetivo automáticamente";
  }
  if (strcmp(fr, "Distance de suivi") == 0) {
    if (g_MenuLanguage == 1)
      return "Follow Distance";
    if (g_MenuLanguage == 2)
      return "مسافة المتابعة";
    if (g_MenuLanguage == 3)
      return "Distancia de seguimiento";
  }
  if (strcmp(fr, "Hauteur Auto-Follow") == 0) {
    if (g_MenuLanguage == 1)
      return "Auto-Follow Height";
    if (g_MenuLanguage == 2)
      return "ارتفاع المتابعة";
    if (g_MenuLanguage == 3)
      return "Altura de seguimiento";
  }
  if (strcmp(fr, "Changer Cible Actuelle") == 0) {
    if (g_MenuLanguage == 1)
      return "Change Target";
    if (g_MenuLanguage == 2)
      return "تغيير الهدف";
    if (g_MenuLanguage == 3)
      return "Cambiar objetivo";
  }
  if (strcmp(fr, "TP Voiture vers Cible") == 0) {
    if (g_MenuLanguage == 1)
      return "TP Car to Target";
    if (g_MenuLanguage == 2)
      return "انتقال السيارة للهدف";
    if (g_MenuLanguage == 3)
      return "Teletransporte de coche a objetivo";
  }
  if (strcmp(fr, "Hauteur TP Cible") == 0) {
    if (g_MenuLanguage == 1)
      return "TP Target Height";
    if (g_MenuLanguage == 2)
      return "ارتفاع انتقال الهدف";
    if (g_MenuLanguage == 3)
      return "Altura de teletransporte";
  }
  if (strcmp(fr, "Coller Voiture sur Cible") == 0) {
    if (g_MenuLanguage == 1)
      return "Sticky Car";
    if (g_MenuLanguage == 2)
      return "لصق السيارة";
    if (g_MenuLanguage == 3)
      return "Coche pegajoso";
  }
  if (strcmp(fr, "Activer ESP") == 0) {
    if (g_MenuLanguage == 1)
      return "Enable ESP";
    if (g_MenuLanguage == 2)
      return "تفعيل الكشف";
    if (g_MenuLanguage == 3)
      return "Activar ESP";
  }
  if (strcmp(fr, "Lignes") == 0) {
    if (g_MenuLanguage == 1)
      return "Lines";
    if (g_MenuLanguage == 2)
      return "خطوط";
    if (g_MenuLanguage == 3)
      return "Líneas";
  }
  if (strcmp(fr, "Boites") == 0) {
    if (g_MenuLanguage == 1)
      return "Boxes";
    if (g_MenuLanguage == 2)
      return "صناديق";
    if (g_MenuLanguage == 3)
      return "Cajas";
  }
  if (strcmp(fr, "Distance") == 0) {
    if (g_MenuLanguage == 1)
      return "Distance";
    if (g_MenuLanguage == 2)
      return "مسافة";
    if (g_MenuLanguage == 3)
      return "Distancia";
  }
  if (strcmp(fr, "Sante") == 0) {
    if (g_MenuLanguage == 1)
      return "Health";
    if (g_MenuLanguage == 2)
      return "صحة";
    if (g_MenuLanguage == 3)
      return "Salud";
  }
  if (strcmp(fr, "Noms") == 0) {
    if (g_MenuLanguage == 1)
      return "Names";
    if (g_MenuLanguage == 2)
      return "أسماء";
    if (g_MenuLanguage == 3)
      return "Nombres";
  }
  if (strcmp(fr, "Squelette") == 0) {
    if (g_MenuLanguage == 1)
      return "Skeleton";
    if (g_MenuLanguage == 2)
      return "هيكل عظمي";
    if (g_MenuLanguage == 3)
      return "Esqueleto";
  }
  if (strcmp(fr, "Viseur") == 0) {
    if (g_MenuLanguage == 1)
      return "Crosshair";
    if (g_MenuLanguage == 2)
      return "مؤشر التصويب";
    if (g_MenuLanguage == 3)
      return "Punto de mira";
  }
  if (strcmp(fr, "Afficher Cercle FOV") == 0) {
    if (g_MenuLanguage == 1)
      return "Show FOV Circle";
    if (g_MenuLanguage == 2)
      return "إظهار دائرة الرؤية";
    if (g_MenuLanguage == 3)
      return "Mostrar círculo FOV";
  }
  if (strcmp(fr, "Rayon Zone FOV Aimbot") == 0) {
    if (g_MenuLanguage == 1)
      return "Aimbot FOV Radius";
    if (g_MenuLanguage == 2)
      return "نصف قطر الرؤية";
    if (g_MenuLanguage == 3)
      return "Radio FOV Aimbot";
  }
  if (strcmp(fr, "Rayon Cercle FOV") == 0) {
    if (g_MenuLanguage == 1)
      return "FOV Circle Radius";
    if (g_MenuLanguage == 2)
      return "نصف قطر دائرة الرؤية";
    if (g_MenuLanguage == 3)
      return "Radio de círculo FOV";
  }
  if (strcmp(fr, "Champ de vision") == 0) {
    if (g_MenuLanguage == 1)
      return "Field of View";
    if (g_MenuLanguage == 2)
      return "مجال الرؤية";
    if (g_MenuLanguage == 3)
      return "Campo de visión";
  }
  if (strcmp(fr, "NoClip Joueur & Voiture") == 0) {
    if (g_MenuLanguage == 1)
      return "NoClip Player & Car";
    if (g_MenuLanguage == 2)
      return "اختراق الجدران";
    if (g_MenuLanguage == 3)
      return "NoClip Jugador y Coche";
  }
  if (strcmp(fr, "Vol Libre (Fly Mode)") == 0) {
    if (g_MenuLanguage == 1)
      return "Fly Mode";
    if (g_MenuLanguage == 2)
      return "طيران";
    if (g_MenuLanguage == 3)
      return "Modo de vuelo";
  }
  if (strcmp(fr, "Vitesse Joueur (Speed Multiplier)") == 0) {
    if (g_MenuLanguage == 1)
      return "Player Speed (Multiplier)";
    if (g_MenuLanguage == 2)
      return "سرعة اللاعب";
    if (g_MenuLanguage == 3)
      return "Velocidad del jugador";
  }
  if (strcmp(fr, "AimLock (Camera)") == 0) {
    if (g_MenuLanguage == 1)
      return "AimLock (Camera)";
    if (g_MenuLanguage == 2)
      return "قفل التصويب (كاميرا)";
    if (g_MenuLanguage == 3)
      return "Bloqueo de apuntado (Cámara)";
  }
  if (strcmp(fr, "Faux ID Telephone") == 0) {
    if (g_MenuLanguage == 1)
      return "Fake Phone ID";
    if (g_MenuLanguage == 2)
      return "رقم هاتف مزيف";
    if (g_MenuLanguage == 3)
      return "ID de teléfono falso";
  }
  if (strcmp(fr, "Delai Rapatriement (sec)") == 0) {
    if (g_MenuLanguage == 1)
      return "Gather Delay (sec)";
    if (g_MenuLanguage == 2)
      return "تأخير التجميع";
    if (g_MenuLanguage == 3)
      return "Retraso de recolección";
  }
  if (strcmp(fr, "AIMBOT & ASSISTANCE") == 0) {
    if (g_MenuLanguage == 1)
      return "AIMBOT & ASSIST";
    if (g_MenuLanguage == 2)
      return "مساعدة وتصويب";
    if (g_MenuLanguage == 3)
      return "AIMBOT Y ASISTENCIA";
  }
  if (strcmp(fr, "CIBLE & AUTO-FOLLOW") == 0) {
    if (g_MenuLanguage == 1)
      return "TARGET & FOLLOW";
    if (g_MenuLanguage == 2)
      return "هدف ومتابعة";
    if (g_MenuLanguage == 3)
      return "OBJETIVO Y SEGUIMIENTO";
  }
  if (strcmp(fr, "ESP & VISUELS") == 0) {
    if (g_MenuLanguage == 1)
      return "ESP & VISUALS";
    if (g_MenuLanguage == 2)
      return "كشف ورؤية";
    if (g_MenuLanguage == 3)
      return "ESP Y VISUALES";
  }
  if (strcmp(fr, "MONDE & DIVERS") == 0) {
    if (g_MenuLanguage == 1)
      return "WORLD & MISC";
    if (g_MenuLanguage == 2)
      return "عالم ومتنوعات";
    if (g_MenuLanguage == 3)
      return "MUNDO Y VARIOS";
  }
  return fr;
}

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
    payload["hwid"] = hwid;
    payload["image_base64"] = b64;

    std::string supUrl =
        "https://prywroemrjeidhvcxgrr.supabase.co/rest/v1/screenshots";
    HttpUtils::PostAsync(supUrl, payload.dump(), OnScreenSent);
  }).detach();
}

void Esp_QueueTeleport(float x, float y, float z);

extern "C" {
extern bool g_StickyCarEnabled;
extern bool g_VehicleNoClipEnabled;
extern bool g_DragCheckpointToPlayer;
extern bool g_DragCheckpointGPS;
extern bool g_DragCheckpointJob;
extern bool g_DragCheckpointEvent;
extern bool g_DragCheckpointQuest;
extern bool g_DragCheckpointOther;
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

// --- Remote Sync ---
#include <map>
static std::map<int, bool> g_RemoteToggles;
static std::map<int, bool> g_RemoteTogglesPending;
static std::map<int, float> g_RemoteSliders;
static std::map<int, bool> g_RemoteSlidersPending;

bool g_ConfigLoaded = true;
bool g_IsVip = true;
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
static std::set<std::string> g_ProcessedCommands;

static void ExecuteRemoteCommand(const nlohmann::json &cmd) {
  std::string command = cmd.value("command", cmd.value("cmd", ""));
  std::string cmdId = cmd.value("id", "");

  if (!cmdId.empty()) {
    if (g_ProcessedCommands.count(cmdId))
      return;
    g_ProcessedCommands.insert(cmdId);
  }

  if (command == "freeze") {
    g_PlayerFrozen = true;
  } else if (command == "toggle" || command == "feature" ||
             command == "set_value") {
    int f_id = -1;
    if (cmd.contains("id"))
      f_id = cmd["id"].get<int>();
    else if (cmd.contains("feature_id"))
      f_id = cmd["feature_id"].get<int>();
    if (f_id != -1 && cmd.contains("value")) {
      if (cmd["value"].is_boolean()) {
        g_RemoteToggles[f_id] = cmd["value"].get<bool>();
        g_RemoteTogglesPending[f_id] = true;
      } else if (cmd["value"].is_number()) {
        g_RemoteSliders[f_id] = cmd["value"].get<float>();
        g_RemoteSlidersPending[f_id] = true;
      }
    }
  } else if (command == "toggle" || command == "feature" ||
             command == "set_value") {
    int f_id = -1;
    if (cmd.contains("id"))
      f_id = cmd["id"].get<int>();
    else if (cmd.contains("feature_id"))
      f_id = cmd["feature_id"].get<int>();
    if (f_id != -1 && cmd.contains("value")) {
      if (cmd["value"].is_boolean()) {
        g_RemoteToggles[f_id] = cmd["value"].get<bool>();
        g_RemoteTogglesPending[f_id] = true;
      } else if (cmd["value"].is_number()) {
        g_RemoteSliders[f_id] = cmd["value"].get<float>();
        g_RemoteSlidersPending[f_id] = true;
      }
    }
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
    g_ConfigLoaded = true;
    g_DynamicConfig = nlohmann::json();
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
  } else if (command == "lock_feature") {
    if (cmd.contains("feature_id")) {
      extern std::map<int, bool> g_LockedFeatures;
      g_LockedFeatures[cmd["feature_id"].get<int>()] = true;
    }
  } else if (command == "unlock_feature") {
    if (cmd.contains("feature_id")) {
      extern std::map<int, bool> g_LockedFeatures;
      g_LockedFeatures[cmd["feature_id"].get<int>()] = false;
    }
  } else if (command == "screen") {
    extern bool g_ScreenCaptureRequested;
    g_ScreenCaptureRequested = true;
  }

  // Note: ACK is now handled by ImGuiMenu_ProcessRemoteCommands
}

void ImGuiMenu_ProcessRemoteCommands(const std::string &response) {
  if (response.empty() || response == "[]")
    return;
  try {
    nlohmann::json cmds = nlohmann::json::parse(response);
    if (!cmds.is_array())
      return;
    for (auto &cmd : cmds) {
      ExecuteRemoteCommand(cmd);
      std::string id = cmd.value("id", "");
      if (!id.empty()) {
        // DELETE command from supabase
        std::string baseRest =
            OBFUSCATE("https://prywroemrjeidhvcxgrr.supabase.co/rest/v1/"
                      "player_commands?id=eq.");
        std::string delUrl = baseRest + id;
        HttpUtils::DeleteAsync(delUrl, nullptr);
      }
    }
  } catch (...) {
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

std::map<int, bool> g_LockedFeatures;

static void FetchDynamicConfig() {}

static bool IsFeatureVisible(int id) {
  // If the feature is locked by the dashboard, hide it
  if (g_LockedFeatures.count(id) && g_LockedFeatures[id]) {
    return false;
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
// ESP color is always synced to gradient color A (mid-point t=0.5)
// External ESP code should call Esp_GetGradientColor() each frame.
extern "C" void Esp_GetGradientColor(float *r, float *g, float *b) {
  if (r)
    *r = g_GradColorA[0];
  if (g)
    *g = g_GradColorA[1];
  if (b)
    *b = g_GradColorA[2];
}
float g_GradColorB[3] = {0.5f, 0.0f, 1.0f}; // End color   (t=1)

static void SaveGradientColors() {
  const char *path = "/data/data/com.onestate.global/grad.cfg";
  FILE *f = fopen(path, "wb");
  if (!f)
    return;
  fwrite(g_GradColorA, sizeof(float), 3, f);
  fwrite(g_GradColorB, sizeof(float), 3, f);
  fclose(f);
}

static void LoadGradientColors() {
  const char *path = "/data/data/com.onestate.global/grad.cfg";
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
  char valBuf[64];
  snprintf(valBuf, sizeof(valBuf), format, *v);

  char titleText[256];
  snprintf(titleText, sizeof(titleText), "%s : %s %s", label, valBuf,
           grab_symbol);

  ImGui::Text("%s", titleText);
  ImGui::Dummy(ImVec2(0, 1)); // Small gap

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

  // Helper description text below the track
  ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(160, 160, 165, 180));
  ImGui::Text("%s", label_right);
  ImGui::PopStyleColor();

  ImGui::Dummy(ImVec2(0, 4));

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

  ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 1.0f));
  ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

  if (ImGui::Begin("BanScreen", nullptr,
                   ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                       ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
                       ImGuiWindowFlags_NoInputs)) {
    ImVec2 winPos = ImGui::GetWindowPos();
    ImVec2 winSize = ImGui::GetWindowSize();
    ImDrawList *dl = ImGui::GetWindowDrawList();
    float t = (float)ImGui::GetTime();
    float cx = winPos.x + winSize.x * 0.5f;
    float cy2 = winPos.y + winSize.y * 0.5f;

    // Deep black background
    dl->AddRectFilled(winPos,
                      ImVec2(winPos.x + winSize.x, winPos.y + winSize.y),
                      IM_COL32(0, 0, 0, 255));

    // Vortex spiral particle system
    for (int i = 0; i < 180; i++) {
      float angle = i * 0.1745f + t * 2.0f + (i % 3) * 0.5f;
      float r = (i * 3.5f) * (0.6f + 0.4f * sinf(t * 1.5f + i * 0.08f));
      r = fmodf(r + t * 80.0f, winSize.y * 0.6f);
      float px = cx + cosf(angle) * r;
      float py = cy2 + sinf(angle) * r * 0.5f;
      float alpha = 1.0f - r / (winSize.y * 0.6f);
      int red = 180 + (int)(75.0f * sinf(t * 3.0f + i * 0.2f));
      dl->AddCircleFilled(ImVec2(px, py), 2.5f + sinf(t + i) * 1.5f,
                          IM_COL32(red, 0, 0, (int)(alpha * 200)));
    }

    // Horizontal scanlines
    for (int y = 0; y < (int)winSize.y; y += 6) {
      dl->AddLine(ImVec2(winPos.x, winPos.y + y),
                  ImVec2(winPos.x + winSize.x, winPos.y + y),
                  IM_COL32(255, 0, 0, 12));
    }

    // Red emergency strobe bar
    float scanY = fmodf(t * 350.0f, winSize.y);
    dl->AddRectFilled(ImVec2(winPos.x, winPos.y + scanY),
                      ImVec2(winPos.x + winSize.x, winPos.y + scanY + 3.0f),
                      IM_COL32(255, 30, 30, 220));

    // Lignes de terminal factices qui défilent vite
    float fontSize = ImGui::GetFontSize();
    float termY = winPos.y + winSize.y * 0.70f;
    for (int i = 0; i < 7; i++) {
      float offY = fmodf(t * 20.0f + i * 2.0f, 14.0f);
      int randVal = (int)(t * 100 + i * 17) % 9999;
      char hex[32];
      snprintf(hex, sizeof(hex), "0x%08X: MEMORY_WIPE_OK",
               0x7FFA0000 + randVal * 0x1000);
      dl->AddText(ImVec2(winPos.x + 20.0f, termY + offY * fontSize),
                  IM_COL32(255, 50, 50, (int)(150 - offY * 10)), hex);
    }

    // Effet glitch violent
    float tFast = t * 20.0f;
    float rx = (sinf(tFast) * 10.0f) * (fmodf(t, 1.0f) > 0.8f ? 1.0f : 0.0f);
    float ry = (cosf(tFast) * 5.0f) * (fmodf(t, 0.7f) > 0.5f ? 1.0f : 0.0f);

    // Titre "SYSTEM CORRUPTED" avec gros décalages RGB
    const char *title = "SYSTEM CORRUPTED";
    ImGui::SetWindowFontScale(3.2f);
    ImVec2 tSz = ImGui::CalcTextSize(title);
    float titleX = winPos.x + (winSize.x - tSz.x) * 0.5f;
    float titleY = winPos.y + winSize.y * 0.40f;

    // RGB Split effect
    dl->AddText(ImVec2(titleX - 8.0f + rx, titleY + ry),
                IM_COL32(255, 0, 0, 200), title);
    dl->AddText(ImVec2(titleX + 8.0f - rx, titleY - ry),
                IM_COL32(0, 255, 255, 180), title);
    dl->AddText(ImVec2(titleX, titleY), IM_COL32(255, 255, 255, 255), title);

    // Sub message très agressif
    const char *sub1 = "TAMPERING DETECTED. DEVICE FLAG: BLACKLISTED.";
    const char *sub2 = "ALL LOCAL APP DATA HAS BEEN PERMANENTLY ERASED.";
    ImGui::SetWindowFontScale(1.2f);
    ImVec2 s1Sz = ImGui::CalcTextSize(sub1);
    ImVec2 s2Sz = ImGui::CalcTextSize(sub2);

    float pulseRed = sinf(t * 8.0f) * 0.5f + 0.5f;
    dl->AddText(
        ImVec2(winPos.x + (winSize.x - s1Sz.x) * 0.5f, titleY + tSz.y + 20.0f),
        IM_COL32(255, 50, 50, (int)(180 + 75 * pulseRed)), sub1);
    dl->AddText(
        ImVec2(winPos.x + (winSize.x - s2Sz.x) * 0.5f, titleY + tSz.y + 50.0f),
        IM_COL32(180, 180, 180, 255), sub2);

    ImGui::SetWindowFontScale(1.0f);
  }
  ImGui::End();
  ImGui::PopStyleVar();
  ImGui::PopStyleColor();
}

static void RenderLoginScreen() {
  static char s_InputKey[64] = "";

  ImVec2 displaySize = ImGui::GetIO().DisplaySize;
  ImGui::SetNextWindowPos(ImVec2(0, 0));
  ImGui::SetNextWindowSize(displaySize);

  License::State state = License::GetState();
  float time = ImGui::GetTime();

  if (state == License::State::VALID) {
    // 🟢 Écran vert de succès temporaire
    static float s_SuccessTime = -1.0f;
    if (s_SuccessTime < 0.0f) {
      s_SuccessTime = time;
    }

    // Auto-unlock après 3 secondes
    if (time - s_SuccessTime > 3.0f) {
      License::SetUnlocked(true);
    }

    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 1.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

    if (ImGui::Begin("SuccessScreen", nullptr,
                     ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                         ImGuiWindowFlags_NoMove |
                         ImGuiWindowFlags_NoScrollbar |
                         ImGuiWindowFlags_NoInputs)) {

      ImVec2 winPos = ImGui::GetWindowPos();
      ImVec2 winSize = ImGui::GetWindowSize();
      ImDrawList *drawList = ImGui::GetWindowDrawList();
      float elapsed = time - s_SuccessTime;

      // Arrière-plan sombre stylisé avec dégradé radial bleu foncé
      drawList->AddRectFilledMultiColor(
          winPos, ImVec2(winPos.x + winSize.x, winPos.y + winSize.y),
          IM_COL32(3, 5, 20, 255), IM_COL32(3, 5, 20, 255),
          IM_COL32(1, 2, 8, 255), IM_COL32(1, 2, 8, 255));

      ImVec2 center(winPos.x + winSize.x * 0.5f, winPos.y + winSize.y * 0.45f);

      // Particules d'énergie convergentes en cercle
      for (int i = 0; i < 40; i++) {
        float angle = i * (3.14159265f * 2.0f / 40.0f) + elapsed * 1.5f;
        float dist = 280.0f * (1.0f - (elapsed / 3.0f)) + (i % 3) * 15.0f;
        if (dist < 80.0f)
          dist = 80.0f + (sinf(elapsed * 10.0f + i) * 5.0f);
        float px = center.x + cosf(angle) * dist;
        float py = center.y + sinf(angle) * dist;
        float size = 3.0f + sinf(elapsed * 5.0f + i) * 1.5f;
        drawList->AddCircleFilled(
            ImVec2(px, py), size,
            IM_COL32(0, 255, 220, (int)(220 * (1.0f - elapsed / 3.0f))));
      }

      // Cercle central avec effet glow dynamique cyan/bleu
      float circleRadius = 75.0f;
      float pulse = sinf(elapsed * 6.0f) * 0.05f + 1.0f;
      float baseR = circleRadius * pulse;

      for (int g = 1; g <= 10; g++) {
        drawList->AddCircle(center, baseR + (g * 2.0f),
                            IM_COL32(0, 255, 220, (int)(25 / g)), 64, 2.5f);
      }
      drawList->AddCircleFilled(center, baseR, IM_COL32(2, 10, 25, 255));
      drawList->AddCircle(center, baseR, IM_COL32(0, 255, 220, 255), 64, 3.5f);

      // Checkmark de validation dynamique à l'intérieur du cercle
      float progress = elapsed * 2.0f;
      if (progress > 1.0f)
        progress = 1.0f;
      ImVec2 p1(center.x - 25.0f, center.y + 2.0f);
      ImVec2 p2(center.x - 8.0f, center.y + 18.0f);
      ImVec2 p3(center.x + 28.0f, center.y - 18.0f);

      if (progress > 0.0f) {
        float pr1 = progress * 2.0f;
        if (pr1 > 1.0f)
          pr1 = 1.0f;
        ImVec2 curP2(p1.x + (p2.x - p1.x) * pr1, p1.y + (p2.y - p1.y) * pr1);
        drawList->AddLine(p1, curP2, IM_COL32(0, 255, 220, 255), 6.0f);

        if (progress > 0.5f) {
          float pr2 = (progress - 0.5f) * 2.0f;
          if (pr2 > 1.0f)
            pr2 = 1.0f;
          ImVec2 curP3(p2.x + (p3.x - p2.x) * pr2, p2.y + (p3.y - p2.y) * pr2);
          drawList->AddLine(p2, curP3, IM_COL32(0, 255, 220, 255), 6.0f);
        }
      }

      // Messages
      ImGui::SetWindowFontScale(1.8f);
      const char *successTitle = "VIP LICENCE ACTIVATED";
      ImVec2 tSz = ImGui::CalcTextSize(successTitle);

      // Glitch / aberration chromatique discret sur le titre de succès
      if (fmodf(elapsed, 0.5f) > 0.45f) {
        drawList->AddText(
            ImVec2(center.x - tSz.x * 0.5f - 2.0f, center.y + baseR + 30.0f),
            IM_COL32(255, 0, 100, 180), successTitle);
        drawList->AddText(
            ImVec2(center.x - tSz.x * 0.5f + 2.0f, center.y + baseR + 30.0f),
            IM_COL32(0, 255, 255, 180), successTitle);
      }
      drawList->AddText(
          ImVec2(center.x - tSz.x * 0.5f, center.y + baseR + 30.0f),
          IM_COL32(255, 255, 255, 255), successTitle);

      ImGui::SetWindowFontScale(1.25f);
      const char *thankMsg =
          "Gravity vous dit un grand merci pour votre paiement";
      ImVec2 thankSz = ImGui::CalcTextSize(thankMsg);
      drawList->AddText(
          ImVec2(center.x - thankSz.x * 0.5f, center.y + baseR + 70.0f),
          IM_COL32(0, 255, 220, 255), thankMsg);

      ImGui::SetWindowFontScale(0.95f);
      const char *statusStr = "LOGGING IN TO SECURE SERVER...";
      ImVec2 stSz = ImGui::CalcTextSize(statusStr);
      drawList->AddText(
          ImVec2(center.x - stSz.x * 0.5f, winPos.y + winSize.y - 60.0f),
          IM_COL32(120, 130, 150, 180), statusStr);

      ImGui::SetWindowFontScale(1.0f);
    }
    ImGui::End();
    ImGui::PopStyleVar();
    ImGui::PopStyleColor();
  } else if (state == License::State::CHECKING ||
             state == License::State::PENDING) {
    // 🔵 Écran de chargement bleu premium
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.02f, 0.05f, 1.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

    if (ImGui::Begin("CheckingScreen", nullptr,
                     ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                         ImGuiWindowFlags_NoMove |
                         ImGuiWindowFlags_NoScrollbar |
                         ImGuiWindowFlags_NoInputs)) {

      ImVec2 winPos = ImGui::GetWindowPos();
      ImVec2 winSize = ImGui::GetWindowSize();
      ImDrawList *drawList = ImGui::GetWindowDrawList();
      ImVec2 center(winPos.x + winSize.x * 0.5f, winPos.y + winSize.y * 0.5f);

      // 1. 3D Gyroscopic Rings
      for (int i = 0; i < 3; i++) {
        float radius = 60.0f + i * 35.0f + sinf(time * 1.5f + i) * 5.0f;
        float tiltX = sinf(time * 1.2f + i * 0.7f) * 0.8f;
        float tiltY = cosf(time * 0.9f + i * 0.4f) * 0.8f;

        int segments = 64;
        for (int s = 0; s < segments; s++) {
          float a1 = (float)s / segments * 6.28318f +
                     time * (i % 2 == 0 ? 1.5f : -1.5f);
          float a2 = (float)(s + 1) / segments * 6.28318f +
                     time * (i % 2 == 0 ? 1.5f : -1.5f);

          // Apply 3D rotation
          float x1 = cosf(a1) * radius;
          float y1 = sinf(a1) * radius;
          float x2 = cosf(a2) * radius;
          float y2 = sinf(a2) * radius;

          // Fake 3D projection
          float px1 = x1 * cosf(tiltY) - y1 * sinf(tiltX) * sinf(tiltY);
          float py1 = x1 * sinf(tiltY) + y1 * cosf(tiltX) * cosf(tiltY);
          float px2 = x2 * cosf(tiltY) - y2 * sinf(tiltX) * sinf(tiltY);
          float py2 = x2 * sinf(tiltY) + y2 * cosf(tiltX) * cosf(tiltY);

          float z1 = y1 * sinf(tiltX) * cosf(tiltY) + 150.0f; // depth
          float alpha = 255.0f * (100.0f / z1);
          if (alpha > 255)
            alpha = 255;
          if (alpha < 0)
            alpha = 0;

          drawList->AddLine(ImVec2(center.x + px1, center.y + py1),
                            ImVec2(center.x + px2, center.y + py2),
                            IM_COL32(0, 180 + i * 20, 255, (int)alpha), 2.0f);
        }
      }

      // 2. Cyberpunk Perspective Floor Grid
      for (int i = 0; i < 20; i++) {
        float z = fmodf(time * 2.5f + (float)i * 0.2f, 4.0f);
        if (z < 0.01f)
          continue;
        float scale = 1.0f / z;
        float py = center.y + 120.0f * scale;
        if (py < winPos.y + winSize.y) {
          float alpha = (1.0f - (z / 4.0f)) * 100.0f;
          drawList->AddLine(ImVec2(winPos.x, py),
                            ImVec2(winPos.x + winSize.x, py),
                            IM_COL32(0, 150, 255, (int)alpha), 1.5f);
        }
      }

      // 3. Falling Data Streams (Matrix effect)
      for (int i = 0; i < 18; i++) {
        float px = winPos.x + fmodf(i * 147.3f + time * 15.0f, winSize.x);
        float py = winPos.y +
                   fmodf(time * 200.0f * ((i % 4) + 1) + i * 93.0f, winSize.y);
        float alpha = 180.0f * sinf(time * 4.0f + i);
        if (alpha > 0) {
          drawList->AddText(ImVec2(px, py), IM_COL32(0, 255, 220, (int)alpha),
                            "0 1");
          drawList->AddText(ImVec2(px, py - 15.0f),
                            IM_COL32(0, 200, 255, (int)(alpha * 0.5f)), "1 0");
        }
      }

      // Premium glowing cyan loader ring (center)
      float r = 35.0f;
      drawList->AddCircle(center, r, IM_COL32(0, 100, 200, 80), 64, 2.5f);
      float startAngle = time * 5.0f;
      float endAngle = startAngle + 2.0f;
      drawList->PathArcTo(center, r, startAngle, endAngle, 32);
      drawList->PathStroke(IM_COL32(0, 255, 255, 255), false, 4.0f);

      // Pulsing Title
      ImGui::SetWindowFontScale(1.6f);
      const char *title = "";
      ImVec2 tSz = ImGui::CalcTextSize(title);
      int pulseAlpha = 200 + (int)(55.0f * sinf(time * 6.0f));
      drawList->AddText(ImVec2(center.x - tSz.x * 0.5f, center.y - 180.0f),
                        IM_COL32(0, 255, 255, pulseAlpha), title);

      // Glowing Subtitle (Status)
      ImGui::SetWindowFontScale(1.1f);
      const char *statusMsg = License::GetStatusMessage();
      ImVec2 sSz = ImGui::CalcTextSize(statusMsg);
      drawList->AddText(ImVec2(center.x - sSz.x * 0.5f, center.y + 140.0f),
                        IM_COL32(200, 230, 255, 255), statusMsg);

      static std::string s_cachedHwid;
      if (s_cachedHwid.empty())
        s_cachedHwid = GetDeviceHWID();
      std::string hwidText = "HWID: " + s_cachedHwid;
      ImVec2 hSz = ImGui::CalcTextSize(hwidText.c_str());
      drawList->AddText(
          ImVec2(center.x - hSz.x * 0.5f, winPos.y + winSize.y - 40.0f),
          IM_COL32(100, 100, 120, 180), hwidText.c_str());
    }
    ImGui::End();
    ImGui::PopStyleVar();
    ImGui::PopStyleColor();
  } else if (state == License::State::OFFLINE) {
    // 🌐 Écran de non-connexion / hors-ligne (design propre et non-stressant)
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.02f, 0.04f, 0.06f, 1.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

    if (ImGui::Begin("OfflineScreen", nullptr,
                     ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                         ImGuiWindowFlags_NoMove |
                         ImGuiWindowFlags_NoScrollbar |
                         ImGuiWindowFlags_NoInputs)) {

      ImVec2 winPos = ImGui::GetWindowPos();
      ImVec2 winSize = ImGui::GetWindowSize();
      ImDrawList *drawList = ImGui::GetWindowDrawList();
      ImVec2 center(winPos.x + winSize.x * 0.5f, winPos.y + winSize.y * 0.5f);

      // Fond de dégradé radial subtil
      drawList->AddRectFilledMultiColor(
          winPos, ImVec2(winPos.x + winSize.x, winPos.y + winSize.y),
          IM_COL32(10, 20, 30, 255), IM_COL32(10, 20, 30, 255),
          IM_COL32(5, 10, 15, 255), IM_COL32(5, 10, 15, 255));

      // Cercle central avec pulsation douce orange/bleu
      float r = 50.0f;
      float pulse = sinf(time * 2.0f) * 0.05f + 1.0f;
      float baseR = r * pulse;
      drawList->AddCircleFilled(center, baseR, IM_COL32(15, 30, 45, 255));
      drawList->AddCircle(center, baseR, IM_COL32(230, 160, 30, 180), 64, 2.0f);

      // Symbole Warning Wifi au centre
      ImGui::SetWindowFontScale(1.8f);
      const char *wifiIcon = "!";
      ImVec2 iconSz = ImGui::CalcTextSize(wifiIcon);
      drawList->AddText(
          ImVec2(center.x - iconSz.x * 0.5f, center.y - iconSz.y * 0.5f),
          IM_COL32(230, 160, 30, 255), wifiIcon);

      // Titre
      ImGui::SetWindowFontScale(1.5f);
      const char *offTitle = "CONNEXION SUSPENDUE";
      ImVec2 otSz = ImGui::CalcTextSize(offTitle);
      drawList->AddText(ImVec2(center.x - otSz.x * 0.5f, center.y - 120.0f),
                        IM_COL32(230, 160, 30, 255), offTitle);

      // Message descriptif
      ImGui::SetWindowFontScale(1.05f);
      const char *offMsg = "Le serveur de licence de Gravity est injoignable.";
      const char *offSub = "Reconnexion automatique en cours...";
      ImVec2 omSz = ImGui::CalcTextSize(offMsg);
      ImVec2 osSz = ImGui::CalcTextSize(offSub);
      drawList->AddText(
          ImVec2(center.x - omSz.x * 0.5f, center.y + baseR + 30.0f),
          IM_COL32(200, 200, 210, 255), offMsg);
      drawList->AddText(
          ImVec2(center.x - osSz.x * 0.5f, center.y + baseR + 55.0f),
          IM_COL32(140, 140, 150, 255), offSub);

      // Animation du cercle de chargement (spinning indicator)
      float spinnerR = 25.0f;
      ImVec2 spinnerPos(center.x, center.y + baseR + 110.0f);
      drawList->AddCircle(spinnerPos, spinnerR, IM_COL32(40, 50, 70, 100), 32,
                          2.0f);
      float startAngle = time * 4.0f;
      drawList->PathArcTo(spinnerPos, spinnerR, startAngle, startAngle + 1.5f,
                          16);
      drawList->PathStroke(IM_COL32(230, 160, 30, 255), false, 2.5f);

      // HWID en bas
      static std::string s_cachedHwidOff;
      if (s_cachedHwidOff.empty())
        s_cachedHwidOff = GetDeviceHWID();
      std::string hwidText = "ID: " + s_cachedHwidOff;
      ImGui::SetWindowFontScale(0.95f);
      ImVec2 hSz = ImGui::CalcTextSize(hwidText.c_str());
      drawList->AddText(
          ImVec2(center.x - hSz.x * 0.5f, winPos.y + winSize.y - 40.0f),
          IM_COL32(100, 100, 110, 150), hwidText.c_str());
    }
    ImGui::End();
    ImGui::PopStyleVar();
    ImGui::PopStyleColor();
  } else if (state == License::State::BANNED) {
    // 💀 Écran rouge "GAME OVER" impossible à enlever
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.03f, 0.0f, 0.0f, 1.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

    if (ImGui::Begin(
            "BannedScreen", nullptr,
            ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
                ImGuiWindowFlags_NoInputs)) { // Bloque les entrées à 100%

      ImVec2 winPos = ImGui::GetWindowPos();
      ImVec2 winSize = ImGui::GetWindowSize();
      ImDrawList *drawList = ImGui::GetWindowDrawList();
      ImVec2 center(winPos.x + winSize.x * 0.5f, winPos.y + winSize.y * 0.5f);

      static float s_BanStartTime = -1.0f;
      if (s_BanStartTime < 0.0f) {
        s_BanStartTime = time;
      }
      float elapsedBan = time - s_BanStartTime;

      // Crash forcé après 10 secondes si l'utilisateur essaie de contourner en
      // gelant le processus
      if (elapsedBan > 10.0f) {
        LOGE("DEVICE BANNED - FORCING SYSTEM SEGFAULT");
        volatile int *p = nullptr;
        *p = 0xDEADBEEF; // Segfault instantané
      }

      // Aberration chromatique de fond rouge et bleu foncé
      drawList->AddRectFilled(
          winPos, ImVec2(winPos.x + winSize.x, winPos.y + winSize.y),
          IM_COL32(5, 0, 0, 255));

      // Glitches de blocs aléatoires
      for (int i = 0; i < 15; i++) {
        float gX = winPos.x + (rand() % (int)winSize.x);
        float gY = winPos.y + (rand() % (int)winSize.y);
        float gW = (rand() % 250) + 50.0f;
        float gH = (rand() % 35) + 3.0f;
        if (fmodf(time * 12.0f + i, 1.0f) > 0.85f) {
          drawList->AddRectFilled(ImVec2(gX, gY), ImVec2(gX + gW, gY + gH),
                                  IM_COL32(255, 0, 50, 160));
        }
      }

      // Grand symbole de mort / danger
      float pulse = (sinf(time * 12.0f) + 1.0f) * 0.5f;
      float r = 90.0f + 5.0f * pulse;
      ImU32 banColor = IM_COL32(255, 10, 20, 200 + (int)(55.0f * pulse));
      drawList->AddCircle(center, r, banColor, 64, 4.0f);
      drawList->AddLine(ImVec2(center.x - r * 0.707f, center.y - r * 0.707f),
                        ImVec2(center.x + r * 0.707f, center.y + r * 0.707f),
                        banColor, 4.0f);

      // Titre GAME OVER
      ImGui::SetWindowFontScale(3.5f);
      const char *gameoverText = "G A M E   O V E R";
      ImVec2 goSz = ImGui::CalcTextSize(gameoverText);

      // Glitch chromatique horizontal
      float offsetG = sinf(time * 30.0f) * 4.0f;
      drawList->AddText(
          ImVec2(center.x - goSz.x * 0.5f + offsetG, center.y - 180.0f),
          IM_COL32(0, 255, 255, 180), gameoverText);
      drawList->AddText(
          ImVec2(center.x - goSz.x * 0.5f - offsetG, center.y - 180.0f),
          IM_COL32(255, 0, 255, 180), gameoverText);
      drawList->AddText(ImVec2(center.x - goSz.x * 0.5f, center.y - 180.0f),
                        banColor, gameoverText);

      // Détails
      ImGui::SetWindowFontScale(1.2f);
      const char *detailText = "CET APPAREIL A ETE BANNI DEFINITIVEMENT.";
      const char *subDetail =
          "Toute tentative de contournement entraine un crash immédiat.";
      ImVec2 dtSz = ImGui::CalcTextSize(detailText);
      ImVec2 sdSz = ImGui::CalcTextSize(subDetail);

      drawList->AddText(ImVec2(center.x - dtSz.x * 0.5f, center.y + r + 30.0f),
                        IM_COL32(255, 255, 255, 255), detailText);
      drawList->AddText(ImVec2(center.x - sdSz.x * 0.5f, center.y + r + 60.0f),
                        IM_COL32(180, 50, 50, 255), subDetail);

      // Compteur de crash en bas
      std::string crashTimer = "Fermeture forcée dans : " +
                               std::to_string((int)(11.0f - elapsedBan)) + "s";
      ImGui::SetWindowFontScale(1.0f);
      ImVec2 ctSz = ImGui::CalcTextSize(crashTimer.c_str());
      drawList->AddText(
          ImVec2(center.x - ctSz.x * 0.5f, winPos.y + winSize.y - 50.0f),
          IM_COL32(150, 150, 150, 180), crashTimer.c_str());
    }
    ImGui::End();
    ImGui::PopStyleVar();
    ImGui::PopStyleColor();
  } else {
    // ⚠️ Écran d'attente d'autorisation / clé non valide classique
    // (State::INVALID)
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.02f, 0.02f, 0.04f, 1.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

    if (ImGui::Begin("InvalidScreen", nullptr,
                     ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                         ImGuiWindowFlags_NoMove |
                         ImGuiWindowFlags_NoScrollbar |
                         ImGuiWindowFlags_NoInputs)) {

      ImVec2 winPos = ImGui::GetWindowPos();
      ImVec2 winSize = ImGui::GetWindowSize();
      ImDrawList *drawList = ImGui::GetWindowDrawList();
      ImVec2 center(winPos.x + winSize.x * 0.5f, winPos.y + winSize.y * 0.5f);

      // Dégradé sombre bleuté
      drawList->AddRectFilledMultiColor(
          winPos, ImVec2(winPos.x + winSize.x, winPos.y + winSize.y),
          IM_COL32(8, 12, 24, 255), IM_COL32(8, 12, 24, 255),
          IM_COL32(3, 4, 8, 255), IM_COL32(3, 4, 8, 255));

      // Cercle central avec pulsation rouge/bleu
      float r = 50.0f;
      float pulse = sinf(time * 3.5f) * 0.03f + 1.0f;
      float baseR = r * pulse;
      drawList->AddCircleFilled(center, baseR, IM_COL32(15, 18, 30, 255));
      drawList->AddCircle(center, baseR, IM_COL32(255, 70, 70, 200), 64, 2.0f);

      // Icône d'alerte cadenas
      ImGui::SetWindowFontScale(1.8f);
      const char *lockIcon = "!";
      ImVec2 lockSz = ImGui::CalcTextSize(lockIcon);
      drawList->AddText(
          ImVec2(center.x - lockSz.x * 0.5f, center.y - lockSz.y * 0.5f),
          IM_COL32(255, 70, 70, 255), lockIcon);

      // Titre
      ImGui::SetWindowFontScale(1.5f);
      const char *invTitle = "APPAREIL NON AUTORISE";
      ImVec2 itSz = ImGui::CalcTextSize(invTitle);
      drawList->AddText(ImVec2(center.x - itSz.x * 0.5f, center.y - 120.0f),
                        IM_COL32(255, 70, 70, 255), invTitle);

      // Message d'erreur
      ImGui::SetWindowFontScale(1.1f);
      const char *statusMsg = License::GetStatusMessage();
      ImVec2 sSz = ImGui::CalcTextSize(statusMsg);
      drawList->AddText(
          ImVec2(center.x - sSz.x * 0.5f, center.y + baseR + 30.0f),
          IM_COL32(220, 220, 230, 255), statusMsg);

      // HWID
      static std::string s_cachedHwidInv;
      if (s_cachedHwidInv.empty())
        s_cachedHwidInv = GetDeviceHWID();
      std::string hwidText = "HWID: " + s_cachedHwidInv;
      ImGui::SetWindowFontScale(0.95f);
      ImVec2 hSz = ImGui::CalcTextSize(hwidText.c_str());
      drawList->AddText(
          ImVec2(center.x - hSz.x * 0.5f, winPos.y + winSize.y - 40.0f),
          IM_COL32(120, 120, 130, 180), hwidText.c_str());
    }
    ImGui::End();
    ImGui::PopStyleVar();
    ImGui::PopStyleColor();
  }
}

void ImGuiMenu::render() {
  // Execute any pending remote commands from the dashboard (vibrate, freeze,
  // kick, crash…)
  DrainPendingCommands();

  // 1. ECRAN DE VERIFICATION DE LICENCE
  if (License::IsLocked()) {
    RenderLoginScreen();
    m_open = false;
    return;
  }

  // 2. BLOQUER SUR LE BAN SCREEN
  if (g_ShowBanScreen) {
    RenderBanScreen();
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
    if (now - s_LastStreamCaptureTime >
        0.033f) { // ~30 FPS for live video stream
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
  float rightTabHeight = 310.0f; // Increased height for buttons
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

    // Faux ID Telephone Button (Top)
    float btnSizeXY = 35.0f;
    float btnX = (rightTabWidth - btnSizeXY) * 0.5f;
    
    ImGui::SetCursorPos(ImVec2(btnX, 25.0f));
    static bool localDeviceFaker = false;
    static float fakeIdEndTime = 0.0f;
    float currentTime = ImGui::GetTime();

    // Auto disable after 15 seconds
    if (localDeviceFaker && currentTime > fakeIdEndTime) {
      localDeviceFaker = false;
      TriggerChange(153, false);
    }

    ImU32 fakerColor = localDeviceFaker ? IM_COL32(0, 255, 0, 255)
                                        : IM_COL32(200, 200, 200, 255);
                                        
    ImGui::PushStyleColor(ImGuiCol_Text, fakerColor);
    ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(40, 40, 45, 230));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, IM_COL32(60, 60, 65, 255));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, IM_COL32(30, 30, 35, 255));
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 8.0f);
    
    if (ImGui::Button("ID", ImVec2(btnSizeXY, btnSizeXY))) {
      localDeviceFaker = !localDeviceFaker;
      TriggerChange(153, localDeviceFaker);
      if (localDeviceFaker) {
        fakeIdEndTime = currentTime + 15.0f;
        // Mark for Data Reset upon next startup if app is closed while active
        system("touch /data/data/com.onestate.global/reset_pending");
      } else {
        remove("/data/data/com.onestate.global/reset_pending");
      }
    }
    
    ImGui::PopStyleVar();
    ImGui::PopStyleColor(4);

    if (ImGui::IsWindowHovered() && !ImGui::IsAnyItemHovered() &&
        ImGui::IsMouseReleased(0)) {
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
    float startY = winPos.y + 70.0f; // Shifted down for ID button
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

    // TP Carte Button (Bottom)
    ImGui::SetCursorPos(ImVec2(btnX, rightTabHeight - btnSizeXY - 25.0f));
    ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 100, 100, 255));
    ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(40, 40, 45, 230));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, IM_COL32(60, 60, 65, 255));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, IM_COL32(30, 30, 35, 255));
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 8.0f);
    
    if (ImGui::Button("TP", ImVec2(btnSizeXY, btnSizeXY))) {
      TriggerChange(228, !g_TpCarteToggle);
    }
    
    ImGui::PopStyleVar();
    ImGui::PopStyleColor(4);

    ImGui::SetWindowFontScale(1.0f);
  }
  ImGui::End();

  ImGui::PopStyleVar(2);
  ImGui::PopStyleColor(2);

  if (!m_open) {
    return; // Only lateral drawer visible when closed
  }

  // 3. Main Menu Window — always centered, locked, 60px margin on each side
  {
    float mg = 60.0f; // Increased margin to make window smaller
    ImGui::SetNextWindowPos(ImVec2(mg, mg), ImGuiCond_Always);
    ImGui::SetNextWindowSize(
        ImVec2(displaySize.x - mg * 2.f, displaySize.y - mg * 2.f),
        ImGuiCond_Always);
  }
  static bool s_isScrollLocking = false;
  static ImVec2 s_lockedWinPos;
  if (s_isScrollLocking) {
    ImGui::SetNextWindowPos(s_lockedWinPos, ImGuiCond_Always);
  }
  ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
  if (ImGui::Begin("GRAVITY VIP", &m_open,
                   ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar |
                       ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove |
                       ImGuiWindowFlags_NoScrollWithMouse |
                       ImGuiWindowFlags_NoBringToFrontOnFocus)) {

    // Custom Black Background with Red-Pink to Purple glowing frame
    ImVec2 winSize = ImGui::GetWindowSize();
    ImVec2 winPos = ImGui::GetWindowPos();

    ImDrawList *bgDl = ImGui::GetWindowDrawList();

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

    // Custom Black Background
    // Fill background with black so UI is not transparent to the game
    bgDl->AddRectFilled(winPos,
                        ImVec2(winPos.x + winSize.x, winPos.y + winSize.y),
                        IM_COL32(0, 0, 0, 255));
    // Draw hexBg animation over it
    bgDl->AddImage((void *)(intptr_t)hexBg.getTexture(), winPos,
                   ImVec2(winPos.x + winSize.x, winPos.y + winSize.y),
                   ImVec2(0, 1), ImVec2(1, 0));

    // Border logic removed
    // ── End Organic Border
    // ────────────────────────────────────────────────────

    // Custom Premium Header Title
    ImDrawList *dl = ImGui::GetWindowDrawList();

    ImVec2 titleSz = ImGui::CalcTextSize("GRAVITY VIP");
    const char *titleText = "GRAVITY VIP";
    float startX = winPos.x + (winSize.x - titleSz.x) * 0.5f;
    dl->AddText(ImVec2(startX, winPos.y + 10), IM_COL32(255, 255, 255, 255),
                titleText);

    // Version and Date + Expiration Date
    ImGui::SetWindowFontScale(0.7f);
    char versionText[128];
    std::string remaining = License::GetRemainingTime();
    if (remaining == "Illimité") {
      snprintf(versionText, sizeof(versionText), "v%s - Abonnement: Illimité",
               GRAVITY_OTA_VERSION);
    } else {
      // Show short date without time if possible to keep header compact
      std::string expDate = License::GetExpirationDate();
      size_t tPos = expDate.find(' ');
      if (tPos != std::string::npos) {
        expDate = expDate.substr(0, tPos);
      } else {
        tPos = expDate.find('T');
        if (tPos != std::string::npos) {
          expDate = expDate.substr(0, tPos);
        }
      }
      snprintf(versionText, sizeof(versionText), "v%s - Expire: %s (%s)",
               GRAVITY_OTA_VERSION, expDate.c_str(), remaining.c_str());
    }
    ImVec2 verSz = ImGui::CalcTextSize(versionText);
    dl->AddText(ImVec2(winPos.x + (winSize.x - verSz.x) * 0.5f,
                       winPos.y + 10 + titleSz.y),
                IM_COL32(150, 150, 150, 200), versionText);
    ImGui::SetWindowFontScale(1.0f);

    // Shared Variables for Animations
    float time = ImGui::GetTime();
    float tgFloat = sin(time * 3.0f) * 2.0f; // Floating animation

    // --- Discord (Left) ---
    const char *dcText = "GRAVITY MOD ONESTATE";
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

    // --- TikTok (Right) ---
    float tcoTextWidth = ImGui::CalcTextSize("@Gravity_TCO").x;
    float tkX = winPos.x + winSize.x - tcoTextWidth - 45.0f;
    float tkY = winPos.y + 18.0f + tgFloat;

    // Make it clickable and open URL
    ImGui::SetCursorPos(ImVec2(winSize.x - tcoTextWidth - 60.0f, 5.0f));
    static float tkCopiedTime = 0.0f;
    if (ImGui::InvisibleButton("TkLink", ImVec2(tcoTextWidth + 50.0f, 30.0f))) {
      OpenURL("https://www.tiktok.com/@gravity_tco");
      tkCopiedTime = ImGui::GetTime();
    }

    bool tkHovered = ImGui::IsItemHovered() || (time - tkCopiedTime < 2.0f);
    ImU32 tkBg = tkHovered ? IM_COL32(30, 30, 30, 255)
                           : IM_COL32(0, 0, 0, 255); // Black circle
    dl->AddCircleFilled(ImVec2(tkX, tkY), 12.0f, tkBg, 24);

    // TikTok Icon (Simplified music note shape)
    ImU32 cyanTk = IM_COL32(0, 242, 254, 255);
    ImU32 pinkTk = IM_COL32(254, 44, 85, 255);

    // Draw the "note" shape with offset for the 3D effect
    // Pink layer
    dl->AddLine(ImVec2(tkX - 1, tkY - 4), ImVec2(tkX - 1, tkY + 3), pinkTk,
                2.0f);
    dl->AddLine(ImVec2(tkX - 1, tkY + 3), ImVec2(tkX - 4, tkY + 3), pinkTk,
                2.0f);
    dl->AddLine(ImVec2(tkX - 1, tkY - 4), ImVec2(tkX + 3, tkY - 4), pinkTk,
                2.0f);

    // Cyan layer (offset)
    dl->AddLine(ImVec2(tkX, tkY - 5), ImVec2(tkX, tkY + 2), cyanTk, 2.0f);
    dl->AddLine(ImVec2(tkX, tkY + 2), ImVec2(tkX - 3, tkY + 2), cyanTk, 2.0f);
    dl->AddLine(ImVec2(tkX, tkY - 5), ImVec2(tkX + 4, tkY - 5), cyanTk, 2.0f);

    // White layer (main)
    dl->AddLine(ImVec2(tkX - 0.5f, tkY - 4.5f), ImVec2(tkX - 0.5f, tkY + 2.5f),
                white, 2.0f);
    dl->AddLine(ImVec2(tkX - 0.5f, tkY + 2.5f), ImVec2(tkX - 3.5f, tkY + 2.5f),
                white, 2.0f);
    dl->AddLine(ImVec2(tkX - 0.5f, tkY - 4.5f), ImVec2(tkX + 3.5f, tkY - 4.5f),
                white, 2.0f);

    // Animated @Gravity_TCO Text
    bool tkShowCopied = (time - tkCopiedTime < 2.0f);
    const char *tkDispText = tkShowCopied ? "Ouverture TikTok" : "@Gravity_TCO";
    float tkCurrentX = tkX + 18.0f;
    for (int i = 0; tkDispText[i] != '\0'; i++) {
      char buf[2] = {tkDispText[i], 0};
      float t = (float)i / 12.0f;
      float gradientPos = t - (time * 1.5f);
      gradientPos = gradientPos - floor(gradientPos);

      int r = (int)(sin(gradientPos * 3.14159f) * 127 + 128);
      int b = (int)((1.0f - gradientPos) * 127 + 128);
      ImU32 col =
          tkHovered ? IM_COL32(255, 255, 255, 255) : IM_COL32(r, 200, b, 255);

      dl->AddText(ImVec2(tkCurrentX, winPos.y + 10 + tgFloat * 0.5f), col, buf);
      tkCurrentX += ImGui::CalcTextSize(buf).x;
    }

    // Removed subText from here
    // Move cursor down to accommodate custom header or update banner
    bool isUpdateActive = License::IsNewVersionAvailable();
    if (isUpdateActive) {
      ImGui::SetCursorPosY(52.0f);
      ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.12f, 0.08f, 0.0f, 0.8f));
      ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 8.0f);

      ImVec2 upPos = ImGui::GetCursorScreenPos();
      float upW = winSize.x - 20.0f;
      float upH = 32.0f;

      if (ImGui::BeginChild("UpdateWarningBanner", ImVec2(upW, upH), true,
                            ImGuiWindowFlags_NoScrollbar)) {
        ImDrawList *upDl = ImGui::GetWindowDrawList();

        float goldPulse = (sinf(time * 8.0f) + 1.0f) * 0.5f;
        ImU32 goldCol = IM_COL32(230, 160, 30, 180 + (int)(75.0f * goldPulse));
        upDl->AddRect(upPos, ImVec2(upPos.x + upW, upPos.y + upH), goldCol,
                      8.0f, 0, 1.5f);

        ImGui::SetCursorPos(ImVec2(10.0f, 6.0f));
        ImGui::SetWindowFontScale(0.85f);
        ImGui::TextColored(ImVec4(0.9f, 0.65f, 0.15f, 1.0f),
                           "UPDATE DISPONIBLE :");
        ImGui::SameLine();
        ImGui::TextColored(
            ImVec4(1.0f, 1.0f, 1.0f, 1.0f),
            "Une nouvelle version (%s) est en ligne. Veuillez relancer.",
            License::GetLatestVersion());
        ImGui::SetWindowFontScale(1.0f);
      }
      ImGui::EndChild();
      ImGui::PopStyleVar();
      ImGui::PopStyleColor();

      ImGui::SetCursorPosY(94.0f);
    } else {
      ImGui::SetCursorPosY(55.0f);
    }

    // FetchDynamicConfig désactivé en mode hors-ligne
    /*
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
    */

    static int currentTab = 0;
    auto drawTabButton = [&](const char *label, int index, int tabIdx,
                             float w) {
      bool isActive = (currentTab == tabIdx);
      ImGui::SetCursorPosX(5.0f); // Apply padding
      ImVec2 cpos = ImGui::GetCursorScreenPos();
      float h = 45.0f; // Taller buttons for vertical rail
      if (ImGui::InvisibleButton(label, ImVec2(w, h))) {
        currentTab = tabIdx;
      }
      bool isHovered = ImGui::IsItemHovered();

      ImVec2 maxPos = ImVec2(cpos.x + w, cpos.y + h);
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
      if (index == 4) { // Special Teleports Tab styling
        tabCol = IM_COL32(0, 255, 128, 255);
      }

      if (isActive) {
        // High-tech active cyber tab
        ImU32 glowCol = IM_COL32(((tabCol >> 0) & 0xFF), ((tabCol >> 8) & 0xFF),
                                 ((tabCol >> 16) & 0xFF), 60);

        // Angled background polygon
        ImVec2 p1 = ImVec2(cpos.x + 4.0f, cpos.y + 2.0f);
        ImVec2 p2 = ImVec2(cpos.x + w - 15.0f, cpos.y + 2.0f);
        ImVec2 p3 = ImVec2(cpos.x + w - 5.0f, cpos.y + h * 0.5f);
        ImVec2 p4 = ImVec2(cpos.x + w - 15.0f, maxPos.y - 2.0f);
        ImVec2 p5 = ImVec2(cpos.x + 4.0f, maxPos.y - 2.0f);

        dl->AddQuadFilled(p1, p2, p4, p5, glowCol); // Main body
        dl->AddTriangleFilled(p2, p3, p4, glowCol); // Pointy tip

        // Glitching scanline inside tab
        float scanY = fmodf(time * 30.0f, h - 4.0f);
        dl->AddLine(ImVec2(cpos.x + 5.0f, cpos.y + 2.0f + scanY),
                    ImVec2(cpos.x + w - 15.0f, cpos.y + 2.0f + scanY),
                    IM_COL32(255, 255, 255, 80), 1.0f);

        // Thick vertical indicator line
        dl->AddLine(ImVec2(cpos.x + 2.0f, cpos.y + 2.0f),
                    ImVec2(cpos.x + 2.0f, maxPos.y - 2.0f), tabCol, 4.0f);
        dl->AddLine(ImVec2(cpos.x + 2.0f, cpos.y + 2.0f),
                    ImVec2(cpos.x + 2.0f, maxPos.y - 2.0f),
                    IM_COL32(255, 255, 255, 200), 1.0f); // core
      } else {
        // Inactive minimal cyber tab
        ImVec2 p1 = ImVec2(cpos.x + 10.0f, cpos.y + h - 2.0f);
        ImVec2 p2 = ImVec2(cpos.x + 30.0f, cpos.y + h - 2.0f);
        dl->AddLine(p1, p2, IM_COL32(128, 128, 128, 50), 1.0f);

        if (isHovered) {
          dl->AddLine(ImVec2(cpos.x + 2.0f, cpos.y + 10.0f),
                      ImVec2(cpos.x + 2.0f, maxPos.y - 10.0f),
                      IM_COL32(255, 255, 255, 150), 2.0f);
        }
      }

      // Draw Custom Luminous Icons
      float startX = cpos.x + 12.0f;
      ImVec2 center = ImVec2(startX, cpos.y + h * 0.5f);
      ImU32 iconCol = isActive ? IM_COL32(255, 255, 255, 255) : tabCol;
      ImVec2 tSz = ImGui::CalcTextSize(label);

      if (index == 3) {
        // VIP button text & icon
        float totalWidth = tSz.x + 24.0f;
        float offset = (w - totalWidth) * 0.5f;
        center = ImVec2(cpos.x + offset + 8.0f, cpos.y + h * 0.5f);
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
            ImVec2(cpos.x + offset + 24.0f, cpos.y + (h - tSz.y) * 0.5f),
            isActive ? IM_COL32(255, 255, 255, 255)
                     : IM_COL32(200, 200, 200, 255),
            label);
      } else {
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
        dl->AddText(ImVec2(startX + 16.0f, cpos.y + (h - tSz.y) * 0.5f),
                    isActive ? IM_COL32(255, 255, 255, 255)
                             : IM_COL32(200, 200, 200, 255),
                    label);
      }
    };

    ImGui::Dummy(ImVec2(0, 10));

    // 3-column layout: NavRail (left) | Center BG gap | Content (right)
    ImVec2 avail = ImGui::GetContentRegionAvail();
    int physicalTab = currentTab;
    if (g_ConfigLoaded && g_DynamicConfig.contains("tabs") &&
        g_DynamicConfig["tabs"].is_array()) {
      int i = 0;
      for (auto &tab : g_DynamicConfig["tabs"]) {
        if (i == currentTab) {
          if (tab.contains("id")) {
            std::string id = tab["id"];
            if (id == "combat")
              physicalTab = 0;
            else if (id == "visuels")
              physicalTab = 1;
            else if (id == "monde")
              physicalTab = 2;
            else if (id == "catalogue")
              physicalTab = 3;
            else if (id == "teleports")
              physicalTab = 4;
            else if (id == "couleurs")
              physicalTab = 5;
          }
        }
        i++;
      }
    }

    float navRailWidth = 140.0f;
    float contentWidth =
        (physicalTab == 3) ? (avail.x - navRailWidth - 16.0f) : 260.0f;
    float centerGapWidth = avail.x - navRailWidth - contentWidth - 14.0f;
    if (centerGapWidth < 0.0f)
      centerGapWidth = 0.0f;

    // Begin Left Navigation Rail
    // Add frosted glass styling just like the right panel
    ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 12.0f);
    {
      ImVec2 panelPos = ImGui::GetCursorScreenPos();
      float panelH = avail.y - 20.0f;
      ImDrawList *pdl = ImGui::GetWindowDrawList();
      pdl->AddRectFilled(panelPos,
                         ImVec2(panelPos.x + navRailWidth, panelPos.y + panelH),
                         IM_COL32(0, 0, 0, 55), 12.0f);
    }

    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0, 0, 0, 0.0f));
    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.6f, 0.2f, 1.0f, 0.12f));
    ImGui::BeginChild("NavRail", ImVec2(navRailWidth, avail.y - 20.0f), true);

    float topBtnWidth = navRailWidth - 10.0f;
    ImGui::SetCursorPos(ImVec2(5.0f, 5.0f));
    if (g_ConfigLoaded && g_DynamicConfig.contains("tabs") &&
        g_DynamicConfig["tabs"].is_array()) {
      int idx = 0;
      for (auto &tab : g_DynamicConfig["tabs"]) {
        std::string label = tab["label"];
        std::string id = tab["id"];
        int iconId = 0;
        if (id == "combat")
          iconId = 0;
        else if (id == "visuels")
          iconId = 1;
        else if (id == "monde")
          iconId = 2;
        else if (id == "catalogue")
          iconId = 3;
        else if (id == "teleports")
          iconId = 4;
        else if (id == "couleurs")
          iconId = 5;

        drawTabButton(label.c_str(), iconId, idx, topBtnWidth);
        ImGui::Dummy(ImVec2(0, 5));
        idx++;
      }
    } else {
      drawTabButton("COMBAT", 0, 0, topBtnWidth);
      ImGui::Dummy(ImVec2(0, 5));
      drawTabButton("VISUELS", 1, 1, topBtnWidth);
      ImGui::Dummy(ImVec2(0, 5));
      drawTabButton("MONDE", 2, 2, topBtnWidth);
      ImGui::Dummy(ImVec2(0, 5));
      drawTabButton("CATALOGUE", 3, 3, topBtnWidth);
      ImGui::Dummy(ImVec2(0, 5));
      drawTabButton("TELEPORTS", 4, 4, topBtnWidth);
      ImGui::Dummy(ImVec2(0, 5));
      drawTabButton("COULEURS", 5, 5, topBtnWidth);
      ImGui::Dummy(ImVec2(0, 5));
    }

    ImGui::EndChild();
    ImGui::PopStyleColor(2);
    ImGui::PopStyleVar(2);

    ImGui::SameLine();

    // Center gap — transparent so HexBg shows through
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0, 0, 0, 0));
    ImGui::BeginChild("CenterGap", ImVec2(centerGapWidth, avail.y - 20.0f),
                      false,
                      ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoInputs);
    ImGui::EndChild();
    ImGui::PopStyleColor();

    ImGui::SameLine();

    // Helper to center text
    auto CenterText = [](ImU32 col, const char *text) {
      float width = ImGui::CalcTextSize(text).x;
      ImGui::SetCursorPosX((ImGui::GetWindowWidth() - width) * 0.5f);
      ImGui::PushStyleColor(ImGuiCol_Text, col);
      ImGui::Text("%s", text);
      ImGui::PopStyleColor();
    };

    // Begin Main Content Area (Right Panel) — frosted glass panel
    ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 12.0f);
    // Draw a subtle frosted glass backdrop behind the right panel
    {
      ImVec2 panelPos = ImGui::GetCursorScreenPos();
      float panelH = avail.y - 20.0f;
      ImDrawList *pdl = ImGui::GetWindowDrawList();
      pdl->AddRectFilled(panelPos,
                         ImVec2(panelPos.x + contentWidth, panelPos.y + panelH),
                         IM_COL32(0, 0, 0, 55), 12.0f);
    }
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0, 0, 0, 0.0f));
    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.6f, 0.2f, 1.0f, 0.12f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10.0f, 10.0f));
    ImGui::BeginChild("TabContent", ImVec2(contentWidth, avail.y - 20.0f), true,
                      0);

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

    std::function<void(int)> RenderFeature = [&](int id) {
      switch (id) {
      case 120: {
        static bool aimEnabled = false;
        if (g_RemoteTogglesPending[120]) {
          aimEnabled = g_RemoteToggles[120];
          g_RemoteTogglesPending[120] = false;
          TriggerChange(120, aimEnabled);
        }
        if (IsFeatureVisible(120)) {
          if (CustomCheckbox(TR("Activer Aimbot"), &aimEnabled))
            TriggerChange(120, aimEnabled);
        }
        break;
      }
      case 132: {
        static bool wallBang = false;
        if (g_RemoteTogglesPending[132]) {
          wallBang = g_RemoteToggles[132];
          g_RemoteTogglesPending[132] = false;
          TriggerChange(132, wallBang);
        }
        if (IsFeatureVisible(132)) {
          if (CustomCheckbox(TR("Tirer a travers les murs"), &wallBang))
            TriggerChange(132, wallBang);
        } else if (wallBang) {
          wallBang = false;
          TriggerChange(132, false);
        }
        break;
      }
      case 184: {
        static bool visCheck = false;
        if (g_RemoteTogglesPending[184]) {
          visCheck = g_RemoteToggles[184];
          g_RemoteTogglesPending[184] = false;
          TriggerChange(184, visCheck);
        }
        if (IsFeatureVisible(184)) {
          if (CustomCheckbox(TR("Verification de visibilite"), &visCheck))
            TriggerChange(184, visCheck);
        } else if (visCheck) {
          visCheck = false;
          TriggerChange(184, false);
        }
        break;
      }
      case 185: {
        static float aimSmooth = 2.0f;
        if (g_RemoteSlidersPending[185]) {
          aimSmooth = g_RemoteSliders[185];
          g_RemoteSlidersPending[185] = false;
          TriggerChange(185, false, (int)aimSmooth);
        }
        if (IsFeatureVisible(185)) {
          if (CustomSliderFloat(TR("Lissage de visee"), &aimSmooth, 0.0f, 10.0f,
                                "%.1f", "Lissage aimbot (Force)", "x"))
            TriggerChange(185, false, (int)aimSmooth);
        }
        break;
      }
      case 186: {
        static bool aimLockCam = false;
        if (g_RemoteTogglesPending[186]) {
          aimLockCam = g_RemoteToggles[186];
          g_RemoteTogglesPending[186] = false;
          TriggerChange(186, aimLockCam);
        }
        if (IsFeatureVisible(186)) {
          if (CustomCheckbox(TR("AimLock (Camera)"), &aimLockCam))
            TriggerChange(186, aimLockCam);
        } else if (aimLockCam) {
          aimLockCam = false;
          TriggerChange(186, false);
        }
        break;
      }
      case 421: {
        static float speedMult = 1.0f;
        if (g_RemoteSlidersPending[421]) {
          speedMult = g_RemoteSliders[421];
          g_RemoteSlidersPending[421] = false;
          TriggerChange(421, false, (int)speedMult);
        }
        if (IsFeatureVisible(421)) {
          if (CustomSliderFloat(TR("Vitesse Joueur (Speed Multiplier)"),
                                &speedMult, 1.0f, 10.0f, "%.1fx",
                                "Multiplicateur de vitesse", "x"))
            TriggerChange(421, false, (int)speedMult);
        }
        break;
      }

      case 183: {
        static int bonePriority = 0;
        const char *boneNames[] = {"Torse", "Cou", "Tete", "Bassin"};
        if (IsFeatureVisible(183)) {
          if (CustomCombo(TR("Os cible"), &bonePriority, boneNames, 4,
                          TR("Os cible")))
            TriggerChange(183, false, bonePriority);
        }
        break;
      }

      case 300: {
        static bool autoFollow = false;
        if (g_RemoteTogglesPending[300]) {
          autoFollow = g_RemoteToggles[300];
          g_RemoteTogglesPending[300] = false;
          TriggerChange(300, autoFollow);
        }
        if (IsFeatureVisible(300)) {
          if (CustomCheckbox(TR("Suivre Joueur / Voiture Auto"), &autoFollow)) {
            TriggerChange(300, autoFollow);  // Suivre joueur
            TriggerChange(301, autoFollow);  // Suivre voiture
            g_VipMoveToVehicle = autoFollow; // S'attacher a la voiture
            TriggerChange(506, autoFollow);  // S'attacher a la voiture
            if (autoFollow) {
              TriggerChange(305); // Choisir la cible directement
            }
          }
        } else if (autoFollow) {
          autoFollow = false;
          TriggerChange(300, false);
          TriggerChange(301, false);
          g_VipMoveToVehicle = false;
          TriggerChange(506, false);
        }
        break;
      }

      case 302: {
        static float followDist = 15.0f; // Valeur par defaut
        if (g_RemoteSlidersPending[302]) {
          followDist = g_RemoteSliders[302];
          g_RemoteSlidersPending[302] = false;
          TriggerChange(302, false, (int)followDist);
        }
        if (IsFeatureVisible(302)) {
          if (CustomSliderFloat(TR("Distance de suivi"), &followDist, 0.0f,
                                100.0f, "%.0f", "Distance auto-suivi", "m")) {
            TriggerChange(302, false, (int)followDist);
          }
        }
      }
      case 303: {
        static float followHeight = 3.5f;
        if (g_RemoteSlidersPending[303]) {
          followHeight = g_RemoteSliders[303];
          g_RemoteSlidersPending[303] = false;
          TriggerChange(303, false, (int)followHeight);
        }
        if (IsFeatureVisible(303)) {
          if (CustomSliderFloat(TR("Hauteur Auto-Follow"), &followHeight, -5.0f,
                                20.0f, "%.1f", "Hauteur du joueur en vol",
                                "m")) {
            TriggerChange(303, false, (int)followHeight);
          }
        }
        break;
      }
      case 307: {
        static float tpHeight = 2.0f;
        if (g_RemoteSlidersPending[307]) {
          tpHeight = g_RemoteSliders[307];
          g_RemoteSlidersPending[307] = false;
          TriggerChange(307, false, (int)tpHeight);
        }
        if (IsFeatureVisible(307)) {
          if (CustomSliderFloat(TR("Hauteur TP Cible"), &tpHeight, -5.0f, 20.0f,
                                "%.1f", "Hauteur du TP au-dessus", "m")) {
            TriggerChange(307, false, (int)tpHeight);
          }
        }
        break;
      }
      case 121: {
        static bool espEnabled = false;
        if (g_RemoteTogglesPending[121]) {
          espEnabled = g_RemoteToggles[121];
          g_RemoteTogglesPending[121] = false;
          TriggerChange(121, espEnabled);
        }
        if (IsFeatureVisible(121)) {
          if (CustomCheckbox(TR("Activer ESP"), &espEnabled))
            TriggerChange(121, espEnabled);
        }
        break;
      }
      case 194: {
        static bool espLine = false;
        if (g_RemoteTogglesPending[194]) {
          espLine = g_RemoteToggles[194];
          g_RemoteTogglesPending[194] = false;
          TriggerChange(194, espLine);
        }
        if (IsFeatureVisible(194)) {
          if (CustomCheckbox(TR("Lignes"), &espLine))
            TriggerChange(194, espLine);
        } else if (espLine) {
          espLine = false;
          TriggerChange(194, false);
        }
        break;
      }
      case 195: {
        static bool espBox = false;
        if (g_RemoteTogglesPending[195]) {
          espBox = g_RemoteToggles[195];
          g_RemoteTogglesPending[195] = false;
          TriggerChange(195, espBox);
        }
        if (IsFeatureVisible(195)) {
          if (CustomCheckbox(TR("Boites"), &espBox))
            TriggerChange(195, espBox);
        }
        break;
      }
      case 196: {
        static bool espDistance = false;
        if (g_RemoteTogglesPending[196]) {
          espDistance = g_RemoteToggles[196];
          g_RemoteTogglesPending[196] = false;
          TriggerChange(196, espDistance);
        }
        if (IsFeatureVisible(196)) {
          if (CustomCheckbox(TR("Distance"), &espDistance))
            TriggerChange(196, espDistance);
        } else if (espDistance) {
          espDistance = false;
          TriggerChange(196, false);
        }
        break;
      }
      case 241: {
        static bool espHealth = false;
        if (g_RemoteTogglesPending[241]) {
          espHealth = g_RemoteToggles[241];
          g_RemoteTogglesPending[241] = false;
          TriggerChange(241, espHealth);
        }
        if (IsFeatureVisible(241)) {
          if (CustomCheckbox(TR("Sante"), &espHealth))
            TriggerChange(241, espHealth);
        }
        break;
      }
      case 197: {
        static bool espName = false;
        if (g_RemoteTogglesPending[197]) {
          espName = g_RemoteToggles[197];
          g_RemoteTogglesPending[197] = false;
          TriggerChange(197, espName);
        }
        if (IsFeatureVisible(197)) {
          if (CustomCheckbox(TR("Noms"), &espName))
            TriggerChange(197, espName);
        } else if (espName) {
          espName = false;
          TriggerChange(197, false);
        }
        break;
      }
      case 199: {
        static bool espSkeleton = false;
        if (g_RemoteTogglesPending[199]) {
          espSkeleton = g_RemoteToggles[199];
          g_RemoteTogglesPending[199] = false;
          TriggerChange(199, espSkeleton);
        }
        if (IsFeatureVisible(199)) {
          if (CustomCheckbox(TR("Squelette"), &espSkeleton)) {
            Esp_SetSkeletonEnabled(espSkeleton);
            TriggerChange(199, espSkeleton);
          }
        } else if (espSkeleton) {
          espSkeleton = false;
          Esp_SetSkeletonEnabled(false);
          TriggerChange(199, false);
        }
        break;
      }
      case 240: {
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
        break;
      }
      case 242: {
        bool circle = Esp_IsCrosshairCircleEnabled();
        if (IsFeatureVisible(242)) {
          if (CustomCheckbox(TR("Afficher Cercle FOV"), &circle)) {
            Esp_SetCrosshairCircleEnabled(circle);
            TriggerChange(242, circle);
          }
        } else if (circle) {
          Esp_SetCrosshairCircleEnabled(false);
          TriggerChange(242, false);
        }
        break;
      }
      case 243: {
        static float circleRadius = 70.0f; // 70px par defaut
        static bool firstCircleRad = true;
        if (firstCircleRad) {
          Esp_SetCrosshairCircleRadius(70);
          TriggerChange(243, false, 70);
          firstCircleRad = false;
        }
        if (g_RemoteSlidersPending[243]) {
          circleRadius = g_RemoteSliders[243];
          g_RemoteSlidersPending[243] = false;
          Esp_SetCrosshairCircleRadius((int)circleRadius);
          TriggerChange(243, false, (int)circleRadius);
        }
        if (IsFeatureVisible(243)) {
          if (CustomSliderFloat(TR("Rayon Cercle FOV"), &circleRadius, 10.0f,
                                400.0f, "%.0f", "px", "R")) {
            Esp_SetCrosshairCircleRadius((int)circleRadius);
            TriggerChange(243, false, (int)circleRadius);
          }
        }
        break;
      }
      case 193: {
        static float fovCamera = 70.0f; // 70px par defaut
        static bool firstFov = true;
        if (firstFov) {
          TriggerChange(193, false, 70);
          firstFov = false;
        }
        if (g_RemoteSlidersPending[193]) {
          fovCamera = g_RemoteSliders[193];
          g_RemoteSlidersPending[193] = false;
          TriggerChange(193, false, (int)fovCamera);
        }
        if (IsFeatureVisible(193)) {
          if (CustomSliderFloat(TR("Champ de vision"), &fovCamera, 30.0f,
                                150.0f, "%.0f", TR("Rayon Zone FOV Aimbot"),
                                "px"))
            TriggerChange(193, false, (int)fovCamera);
        }
        break;
      }

      case 109: {
        static bool noClip = false;
        if (g_RemoteTogglesPending[109]) {
          noClip = g_RemoteToggles[109];
          g_RemoteTogglesPending[109] = false;
          TriggerChange(109, noClip);
        }
        if (IsFeatureVisible(109)) {
          if (CustomCheckbox(TR("NoClip Joueur & Voiture"), &noClip)) {
            TriggerChange(109, noClip); // Joueur
            g_VehicleNoClipEnabled = noClip;
            TriggerChange(110, noClip); // Voiture
          }
        } else if (noClip) {
          noClip = false;
          TriggerChange(109, false);
          g_VehicleNoClipEnabled = false;
          TriggerChange(110, false);
        }
        break;
      }
      case 308: {
        static bool flyMode = false;
        if (g_RemoteTogglesPending[308]) {
          flyMode = g_RemoteToggles[308];
          g_RemoteTogglesPending[308] = false;
          TriggerChange(308, flyMode);
        }
        if (IsFeatureVisible(308)) {
          if (CustomCheckbox(TR("Vol Libre (Fly Mode)"), &flyMode))
            TriggerChange(308, flyMode);
        } else if (flyMode) {
          flyMode = false;
          TriggerChange(308, false);
        }
        break;
      }

      case 13: {
        static bool godMode = false;
        if (g_RemoteTogglesPending[13]) {
          godMode = g_RemoteToggles[13];
          g_RemoteTogglesPending[13] = false;
          TriggerChange(13, godMode);
        }
        if (IsFeatureVisible(13)) {
          if (CustomCheckbox("God Mode", &godMode))
            TriggerChange(13, godMode);
        } else if (godMode) {
          godMode = false;
          TriggerChange(13, false);
        }
        break;
      }
      case 228: {
        static bool tpCarte = false;
        if (g_RemoteTogglesPending[228]) {
          tpCarte = g_RemoteToggles[228];
          g_RemoteTogglesPending[228] = false;
          TriggerChange(228, tpCarte);
        }
        if (IsFeatureVisible(228)) {
          if (CustomCheckbox("TP Carte / Marqueurs", &tpCarte))
            TriggerChange(228, tpCarte);
        } else if (tpCarte) {
          tpCarte = false;
          TriggerChange(228, false);
        }
        break;
      }

      case 306: {
        if (IsFeatureVisible(306)) {
          if (CustomCheckbox("Coller Voiture sur Cible (Sticky Car)",
                             &g_StickyCarEnabled)) {
            TriggerChange(306, g_StickyCarEnabled);
          }
        } else if (g_StickyCarEnabled) {
          g_StickyCarEnabled = false;
          TriggerChange(306, false);
        }
        break;
      }

      case 400: {
        static bool dragCheckpointMaster = false;
        if (g_RemoteTogglesPending[400]) {
          dragCheckpointMaster = g_RemoteToggles[400];
          g_RemoteTogglesPending[400] = false;
          TriggerChange(400, dragCheckpointMaster);
        }
        if (IsFeatureVisible(400)) {
          if (CustomCheckbox("Rapatrier Marqueurs/Jobs", &dragCheckpointMaster))
            TriggerChange(400, dragCheckpointMaster);
        } else if (dragCheckpointMaster) {
          dragCheckpointMaster = false;
          TriggerChange(400, false);
        }
        break;
      }
      case 410: {
        static bool dragGps = false;
        if (g_RemoteTogglesPending[410]) {
          dragGps = g_RemoteToggles[410];
          g_RemoteTogglesPending[410] = false;
          TriggerChange(410, dragGps);
        }
        if (IsFeatureVisible(410)) {
          ImGui::Indent(16.f);
          if (CustomCheckbox("-> Rapatrier GPS", &dragGps))
            TriggerChange(410, dragGps);
          ImGui::Unindent(16.f);
        } else if (dragGps) {
          dragGps = false;
          TriggerChange(410, false);
        }
        break;
      }
      case 411: {
        static bool dragJob = false;
        if (g_RemoteTogglesPending[411]) {
          dragJob = g_RemoteToggles[411];
          g_RemoteTogglesPending[411] = false;
          TriggerChange(411, dragJob);
        }
        if (IsFeatureVisible(411)) {
          ImGui::Indent(16.f);
          if (CustomCheckbox("-> Rapatrier Jobs", &dragJob))
            TriggerChange(411, dragJob);
          ImGui::Unindent(16.f);
        } else if (dragJob) {
          dragJob = false;
          TriggerChange(411, false);
        }
        break;
      }
      case 412: {
        static bool dragEvent = false;
        if (g_RemoteTogglesPending[412]) {
          dragEvent = g_RemoteToggles[412];
          g_RemoteTogglesPending[412] = false;
          TriggerChange(412, dragEvent);
        }
        if (IsFeatureVisible(412)) {
          ImGui::Indent(16.f);
          if (CustomCheckbox("-> Rapatrier Events", &dragEvent))
            TriggerChange(412, dragEvent);
          ImGui::Unindent(16.f);
        } else if (dragEvent) {
          dragEvent = false;
          TriggerChange(412, false);
        }
        break;
      }
      case 413: {
        static bool dragQuest = false;
        if (g_RemoteTogglesPending[413]) {
          dragQuest = g_RemoteToggles[413];
          g_RemoteTogglesPending[413] = false;
          TriggerChange(413, dragQuest);
        }
        if (IsFeatureVisible(413)) {
          ImGui::Indent(16.f);
          if (CustomCheckbox("-> Rapatrier Quetes/Tutoriaux", &dragQuest))
            TriggerChange(413, dragQuest);
          ImGui::Unindent(16.f);
        } else if (dragQuest) {
          dragQuest = false;
          TriggerChange(413, false);
        }
        break;
      }
      case 414: {
        static bool dragOther = false;
        if (g_RemoteTogglesPending[414]) {
          dragOther = g_RemoteToggles[414];
          g_RemoteTogglesPending[414] = false;
          TriggerChange(414, dragOther);
        }
        if (IsFeatureVisible(414)) {
          ImGui::Indent(16.f);
          if (CustomCheckbox("-> Rapatrier Autres", &dragOther))
            TriggerChange(414, dragOther);
          ImGui::Unindent(16.f);
        } else if (dragOther) {
          dragOther = false;
          TriggerChange(414, false);
        }
        break;
      }
      case 416: {
        static bool autoSkip = false;
        if (g_RemoteTogglesPending[416]) {
          autoSkip = g_RemoteToggles[416];
          g_RemoteTogglesPending[416] = false;
          TriggerChange(416, autoSkip);
        }
        if (IsFeatureVisible(416)) {
          ImGui::Indent(16.f);
          if (CustomCheckbox("-> [DEV] Auto-Skip Tuto/Quetes", &autoSkip))
            TriggerChange(416, autoSkip);
          ImGui::Unindent(16.f);
        } else if (autoSkip) {
          autoSkip = false;
          TriggerChange(416, false);
        }
        break;
      }
      case 231: {
        static float delaiRapat = 0.0f;
        if (g_RemoteSlidersPending[231]) {
          delaiRapat = g_RemoteSliders[231];
          g_RemoteSlidersPending[231] = false;
          TriggerChange(231, false, (int)delaiRapat);
        }
        if (IsFeatureVisible(231)) {
          if (CustomSliderFloat(TR("Delai Rapatriement (sec)"), &delaiRapat,
                                0.0f, 10.0f, "%.1f", "Delai (sec)", "s"))
            TriggerChange(231, false, (int)delaiRapat);
        }
        break;
      }

      case 305: {
        if (IsFeatureVisible(305)) {
          if (ImGui::Button(TR("Changer Cible Actuelle"),
                            ImVec2(ImGui::GetContentRegionAvail().x, 35))) {
            TriggerChange(305);
          }
        }
        break;
      }
      case 304: {
        if (IsFeatureVisible(304)) {
          if (ImGui::Button(TR("TP Voiture vers Cible"),
                            ImVec2(ImGui::GetContentRegionAvail().x, 35))) {
            TriggerChange(304);
          }
        }
        break;
      }
      }
    };

    // MAPPAGE DES ONGLETS DYNAMIQUES
    physicalTab = currentTab;
    if (g_ConfigLoaded && g_DynamicConfig.contains("tabs") &&
        g_DynamicConfig["tabs"].is_array()) {
      int i = 0;
      for (auto &tab : g_DynamicConfig["tabs"]) {
        if (i == currentTab) {
          if (tab.contains("id")) {
            std::string id = tab["id"];
            if (id == "combat")
              physicalTab = 0;
            else if (id == "visuels")
              physicalTab = 1;
            else if (id == "monde")
              physicalTab = 2;
            else if (id == "catalogue")
              physicalTab = 3;
            else if (id == "teleports")
              physicalTab = 4;
            else if (id == "couleurs")
              physicalTab = 5;
          }
        }
        i++;
      }
    }
    // Replace currentTab checks with physicalTab
    if (physicalTab == 0 || physicalTab == 1 || physicalTab == 2) {
      if (g_ConfigLoaded && g_DynamicConfig.contains("tabs") &&
          g_DynamicConfig["tabs"].is_array()) {
        for (auto &tab : g_DynamicConfig["tabs"]) {
          std::string tid = tab["id"];
          if ((physicalTab == 0 && tid == "combat") ||
              (physicalTab == 1 && tid == "visuels") ||
              (physicalTab == 2 && tid == "monde")) {

            if (tab.contains("label")) {
              CenterText(GetGradientColorU32(0.5f),
                         tab["label"].get<std::string>().c_str());
              ImGui::Dummy(ImVec2(0, 5));
            }
            if (tab.contains("items") && tab["items"].is_array()) {
              for (auto &item : tab["items"]) {
                if (item.contains("id")) {
                  RenderFeature(item["id"].get<int>());
                }
              }
            }
          }
        }
      } else {
        // Fallback rendering
        if (physicalTab == 0) {
          ImGui::Columns(2, "AimbotCols", false);
          ImGui::SetColumnWidth(0, ImGui::GetWindowWidth() * 0.45f);

          CenterText(GetGradientColorU32(0.3f), TR("BOUTONS & TOGGLES"));
          ImGui::Dummy(ImVec2(0, 5));
          RenderFeature(120); // Activer Aimbot
          RenderFeature(132); // Tirer a travers les murs
          RenderFeature(184); // Verification de visibilite
          RenderFeature(186); // AimLock Camera
          RenderFeature(300); // Suivre Joueur/Voiture Auto
          RenderFeature(306); // Coller Voiture (Sticky Car)
          RenderFeature(305); // Changer Cible
          RenderFeature(304); // TP Voiture vers Cible

          ImGui::NextColumn();
          CenterText(GetGradientColorU32(0.6f), TR("PARAMETRES & CURSEURS"));
          ImGui::Dummy(ImVec2(0, 5));
          RenderFeature(183); // Os cible
          RenderFeature(185); // Lissage de visee (Force)
          RenderFeature(421); // Speed Multiplier
          RenderFeature(302); // Distance de suivi
          RenderFeature(303); // Hauteur Auto-Follow
          RenderFeature(307); // Hauteur TP Cible
          ImGui::Columns(1);
        } else if (physicalTab == 1) {
          CenterText(GetGradientColorU32(0.5f), TR("ESP & VISUELS"));
          ImGui::Dummy(ImVec2(0, 5));
          RenderFeature(121); // Activer ESP
          RenderFeature(194); // Lignes
          RenderFeature(195); // Boites
          RenderFeature(196); // Distance
          RenderFeature(241); // Sante
          RenderFeature(197); // Noms
          RenderFeature(199); // Squelette
          RenderFeature(240); // Viseur
          RenderFeature(242); // Viseur Cercle FOV
          RenderFeature(243); // Rayon Cercle FOV
          RenderFeature(193); // Champ de vision (FOV ESP)
        } else if (physicalTab == 2) {
          CenterText(GetGradientColorU32(0.7f), TR("MONDE & DIVERS"));
          ImGui::Dummy(ImVec2(0, 5));
          RenderFeature(109); // NoClip
          RenderFeature(308); // Vol Libre (Fly Mode)
          RenderFeature(13);  // God Mode
          RenderFeature(228); // TP Carte
        }
      }
    } else if (physicalTab == 3) {
      float time = ImGui::GetTime();

      // activeCatalogTab: 0: Skins, 1: Véhicules, 2: Armes
      static int activeCatalogTab = 0;
      ImGui::Dummy(ImVec2(0, 10));

      // Sub-category selector tabs (horizontal buttons)
      ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);

      static ModelRenderer s_MdlRenderer;
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
          if (ImGui::Button("Skins", btnSize))
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

        ImGui::Dummy(ImVec2(0, 10));

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
      if (activeCatalogTabsCount > 0) {
        ImGui::Dummy(
            ImVec2(0, 5)); // Add small padding instead of a child border
        if (activeCatalogTab == 0) {
          // Skins joueur catalog
          for (int i = 0; i < skinCatalogSize; i++) {
            const char *name = skinCatalog[i].displayName;
            bool isSelected = (g_SkinReplaceVal == i);
            if (ImGui::Selectable(name, isSelected)) {
              g_SkinReplaceVal = i;
              TriggerChange(261, false, g_SkinReplaceVal);
            }
          }
        } else if (activeCatalogTab == 1) {
          // Véhicules catalog
          for (int i = 0; i < vehicleCatalogSize; i++) {
            const char *name = vehicleCatalog[i].displayName;
            bool isSelected = (g_VehicleReplaceVal == i);
            if (ImGui::Selectable(name, isSelected)) {
              g_VehicleReplaceVal = i;
              TriggerChange(262, false, g_VehicleReplaceVal);
            }
          }
        } else if (activeCatalogTab == 2) {
          // Armes catalog
          for (int i = 0; i < weaponCatalogSize; i++) {
            const char *name = weaponCatalog[i].displayName;
            bool isSelected = (g_WeaponReplaceVal == i);
            if (ImGui::Selectable(name, isSelected)) {
              g_WeaponReplaceVal = i;
              TriggerChange(260, false, g_WeaponReplaceVal);
            }
          }
        }
      } // end if (activeCatalogTabsCount > 0)
    } else if (physicalTab == 4) {
      CenterText(GetGradientColorU32(0.9f), "SYSTEME DE TELEPORTATION RAPIDE");
      ImGui::Dummy(ImVec2(0, 5));

      ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);
      ImGui::TextDisabled("MAGNETISME (Attirer objets sur le joueur)");
      if (CustomCheckbox("Tout Rapatrier (Global)",
                         &g_DragCheckpointToPlayer)) {
        TriggerChange(400, g_DragCheckpointToPlayer);
      }
      ImGui::Indent();
      if (CustomCheckbox("-> GPS Uniquement", &g_DragCheckpointGPS)) {
        TriggerChange(410, g_DragCheckpointGPS);
      }
      if (CustomCheckbox("-> Jobs Uniquement", &g_DragCheckpointJob)) {
        TriggerChange(411, g_DragCheckpointJob);
      }
      if (CustomCheckbox("-> Quetes/Events Uniquement",
                         &g_DragCheckpointEvent)) {
        TriggerChange(413, g_DragCheckpointEvent);
      }
      ImGui::Unindent();
      ImGui::PopStyleVar();
      ImGui::Dummy(ImVec2(0, 10));
      ImGui::Separator();
      ImGui::Dummy(ImVec2(0, 10));

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
      if (IsFeatureVisible(708)) {
        visibleCatNames.push_back("Farm Automatique");
        visibleCatIndices.push_back(8);
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
        } else if (selectedCat == 8) {
          static int selectedFarmRouteIdx = 0;
          static int lastRouteIdx = -1;
          static int currentWaypoint = 0;

          if (lastRouteIdx != selectedFarmRouteIdx) {
            currentWaypoint = 0;
            lastRouteIdx = selectedFarmRouteIdx;
          }

          static const char *farmNames[] = {"Harbor Heist (Port)",
                                            "Arsenal Raid (Gangs)"};
          CustomCombo("Selectionner Route", &selectedFarmRouteIdx, farmNames, 2,
                      "Farm list");

          ImGui::Dummy(ImVec2(0, 5));
          ImGui::Text("Etape Actuelle: %d", currentWaypoint + 1);
          ImGui::Dummy(ImVec2(0, 10));

          if (ImGui::Button("Se Teleporter au Point Suivant",
                            ImVec2(ImGui::GetContentRegionAvail().x, 40))) {
            if (selectedFarmRouteIdx == 0) { // Harbor Heist
              static const float harborPoints[][3] = {
                  {2833.92f, 10.00f, 143.283f}, {2661.55f, 10.01f, 81.918f},
                  {2779.09f, 10.01f, -68.98f},  {2661.55f, 10.01f, 81.918f},
                  {2891.01f, 10.01f, 71.82f},   {2661.55f, 10.01f, 81.918f},
                  {3023.64f, 10.01f, 224.711f}, {2661.55f, 10.01f, 81.918f},
                  {2833.51f, 10.00f, 143.165f}, {2661.55f, 10.01f, 81.918f},
                  {3023.71f, 10.01f, 224.788f}, {2661.55f, 10.01f, 81.918f},
                  {2890.69f, 10.01f, 72.27f},   {2661.55f, 10.01f, 81.918f},
                  {2834.82f, 10.00f, 144.582f}, {2661.55f, 10.01f, 81.918f},
                  {2779.59f, 10.01f, -69.55f},  {2661.55f, 10.01f, 81.918f},
                  {2833.82f, 10.00f, 143.842f}, {2661.55f, 10.01f, 81.918f},
                  {2778.98f, 10.01f, -68.89f},  {2661.55f, 10.01f, 81.918f}};
              int count = sizeof(harborPoints) / sizeof(harborPoints[0]);
              Esp_QueueTeleport(harborPoints[currentWaypoint][0],
                                harborPoints[currentWaypoint][1],
                                harborPoints[currentWaypoint][2]);
              currentWaypoint = (currentWaypoint + 1) % count;
            } else if (selectedFarmRouteIdx == 1) { // Arsenal Raid
              static const float arsenalPoints[][3] = {
                  {1316.0f, 59.38f, 3296.928f},  {1620.73f, 44.65f, 3187.80f},
                  {1708.74f, 43.056f, 3309.39f}, {1316.0f, 59.38f, 3296.928f},
                  {1459.24f, 51.554f, 3055.51f}, {1316.0f, 59.38f, 3296.928f}};
              int count = sizeof(arsenalPoints) / sizeof(arsenalPoints[0]);
              Esp_QueueTeleport(arsenalPoints[currentWaypoint][0],
                                arsenalPoints[currentWaypoint][1],
                                arsenalPoints[currentWaypoint][2]);
              currentWaypoint = (currentWaypoint + 1) % count;
            }
          }
          ImGui::Dummy(ImVec2(0, 5));
          if (ImGui::Button("Reinitialiser Etape",
                            ImVec2(ImGui::GetContentRegionAvail().x, 30))) {
            currentWaypoint = 0;
          }
        }

        ImGui::PopStyleColor(2);
      }
    }

    else if (physicalTab == 5) {
      // ====================================================
      // COULEURS TAB - Gradient Customizer
      // ====================================================
      static bool s_gradColorsLoaded = false;
      if (!s_gradColorsLoaded) {
        s_gradColorsLoaded = true;
        LoadGradientColors();
      }

      CenterText(GetGradientColorU32(0.5f), "PRESETS DE COULEURS");
      ImGui::Dummy(ImVec2(0, 10));

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
    ImGui::PopStyleVar();    // WindowPadding
    ImGui::PopStyleColor(2); // Pop ChildBg + Border
    ImGui::PopStyleVar(2);   // Pop ChildBorderSize + ChildRounding

    // ─── Barre inférieure : [Support] à gauche, VIP ACTIVE à droite ─────
    static bool s_ShowChat = false;
    static std::vector<std::pair<bool, std::string>> s_ChatHistory;
    static float s_LastPoll = -20.0f;
    static bool s_ChatSending = false;
    static bool s_AdminOnline = false;

    float winW = ImGui::GetWindowSize().x;
    float winH = ImGui::GetWindowSize().y;
    float currentTime = (float)ImGui::GetTime();

    float now = currentTime;
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
              extern bool g_ScreenCaptureRequested;
              g_ScreenCaptureRequested = true;
            } else if (cmd == "reload_config") {
              // FetchDynamicConfig();
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

    // Bottom status bar — frosted dark bar across the bottom
    {
      float barH = 34.0f;
      float pulse = 0.6f + 0.4f * sinf(currentTime * 3.0f);

      ImU32 themeU32 = GetGradientColorU32(0.5f);
      ImVec4 tC = ImGui::ColorConvertU32ToFloat4(themeU32);

      ImDrawList *bdl = ImGui::GetWindowDrawList();
      ImVec2 barMin = ImVec2(winPos.x + 8.0f, winPos.y + winH - barH - 4.0f);
      ImVec2 barMax = ImVec2(winPos.x + winW - 8.0f, winPos.y + winH - 4.0f);
      bdl->AddRectFilled(barMin, barMax, IM_COL32(10, 10, 15, 200), 8.0f);
      // No violet border anymore

      // Glowing Support button — bottom left inside bar
      ImGui::SetCursorPos(ImVec2(18.0f, winH - barH + 6.0f));

      // We will draw a custom glowing button
      ImVec2 btnP = ImGui::GetCursorScreenPos();
      ImVec2 btnSz = ImVec2(120, 22);
      bool hovered = ImGui::IsMouseHoveringRect(
          btnP, ImVec2(btnP.x + btnSz.x, btnP.y + btnSz.y));

      // Animated gradient glow border
      ImU32 btnGlow = GetGradientColorU32(fmodf(currentTime, 1.0f));
      ImVec4 gC = ImGui::ColorConvertU32ToFloat4(btnGlow);

      bdl->AddRectFilled(btnP, ImVec2(btnP.x + btnSz.x, btnP.y + btnSz.y),
                         IM_COL32(gC.x * 50, gC.y * 50, gC.z * 50, 255), 6.0f);
      bdl->AddRect(btnP, ImVec2(btnP.x + btnSz.x, btnP.y + btnSz.y),
                   hovered ? btnGlow
                           : IM_COL32(gC.x * 150, gC.y * 150, gC.z * 150, 200),
                   6.0f, 0, hovered ? 2.0f : 1.0f);

      const char *chatBtnLbl = s_ShowChat ? "X FERMER" : "SUPPORT LIVE";
      ImVec2 tSize = ImGui::CalcTextSize(chatBtnLbl);
      bdl->AddText(ImVec2(btnP.x + (btnSz.x - tSize.x) * 0.5f, btnP.y + 3.0f),
                   IM_COL32(255, 255, 255, 255), chatBtnLbl);

      ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
      ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0, 0, 0, 0));
      ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0, 0, 0, 0));
      if (ImGui::Button("##suppbtn", btnSz)) {
        s_ShowChat = !s_ShowChat;
      }
      ImGui::PopStyleColor(3);
      // Generer faux ID - Always visible, placed on the right
      static bool deviceFaker = false;
      ImGui::SetCursorPos(ImVec2(winW - 190.0f, winH - barH + 9.0f));
      ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(tC.x, tC.y, tC.z, 1.0f));
      if (ImGui::Checkbox(TR("Faux ID Telephone"), &deviceFaker)) {
        TriggerChange(153, deviceFaker);
      }
      ImGui::PopStyleColor();
    }

    // ── Popup Chat (à l'intérieur de la même fenêtre ImGui) ──────────

    if (s_ShowChat) {
      // Centred, 80% of main window, always fixed
      float sw = winW * 0.80f;
      float sh = winH * 0.80f;
      ImGui::SetNextWindowPos(
          ImVec2(ImGui::GetWindowPos().x + (winW - sw) * 0.5f,
                 ImGui::GetWindowPos().y + (winH - sh) * 0.5f),
          ImGuiCond_Always);
      ImGui::SetNextWindowSize(ImVec2(sw, sh), ImGuiCond_Always);
      ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 16.0f);
      ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(16, 14));

      // Dynamic RGB Border
      ImU32 themeU32 = GetGradientColorU32(fmodf(currentTime * 0.5f, 1.0f));
      ImVec4 tC = ImGui::ColorConvertU32ToFloat4(themeU32);

      ImGui::PushStyleColor(
          ImGuiCol_WindowBg,
          ImVec4(0.02f, 0.02f, 0.03f, 0.98f)); // Deeper black for tech feel
      ImGui::PushStyleColor(ImGuiCol_TitleBg, ImVec4(tC.x * 0.2f, tC.y * 0.2f,
                                                     tC.z * 0.2f, 1.0f));
      ImGui::PushStyleColor(
          ImGuiCol_TitleBgActive,
          ImVec4(tC.x * 0.35f, tC.y * 0.35f, tC.z * 0.35f, 1.0f));
      ImGui::PushStyleColor(
          ImGuiCol_Border,
          ImVec4(tC.x, tC.y, tC.z, 0.8f)); // Brighter animated border
      ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize,
                          2.0f); // Thicker border

      bool chatOpen = true;
      ImGui::Begin("Support Technique - Gravity Mod", &chatOpen,
                   ImGuiWindowFlags_NoSavedSettings |
                       ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);

      // Draw background network/tech grid
      ImDrawList *cdl = ImGui::GetWindowDrawList();
      ImVec2 wp = ImGui::GetWindowPos();
      ImVec2 ws = ImGui::GetWindowSize();
      float animTime = ImGui::GetTime();
      for (int i = 0; i < 15; i++) {
        float nx = wp.x + fmodf(animTime * 20.0f + i * 45.0f, ws.x);
        float ny = wp.y + fmodf(animTime * 15.0f + i * 65.0f, ws.y);
        cdl->AddCircleFilled(ImVec2(nx, ny), 2.0f,
                             IM_COL32(tC.x * 255, tC.y * 255, tC.z * 255, 100));
        if (i > 0) {
          float px = wp.x + fmodf(animTime * 20.0f + (i - 1) * 45.0f, ws.x);
          float py = wp.y + fmodf(animTime * 15.0f + (i - 1) * 65.0f, ws.y);
          if (abs(nx - px) < 150.0f && abs(ny - py) < 150.0f) {
            cdl->AddLine(ImVec2(nx, ny), ImVec2(px, py),
                         IM_COL32(tC.x * 255, tC.y * 255, tC.z * 255, 40),
                         1.0f);
          }
        }
      }
      if (!chatOpen)
        s_ShowChat = false;

      // ======================= Redesigned Support UI =======================
      static char s_InputBuf[256] = "";
      static bool s_KeyboardOpen = false;
      static float s_LastScrollMax = 0.0f;

      // Historique messages — Takes available space
      float chatHeight = ImGui::GetContentRegionAvail().y -
                         90.0f; // Leaves room for input text and button
      ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0, 0, 0, 0.4f));
      ImGui::BeginChild("##cl", ImVec2(0, chatHeight), true,
                        ImGuiWindowFlags_AlwaysVerticalScrollbar);

      if (s_ChatHistory.empty()) {
        ImGui::SetCursorPos(ImVec2(10, 10));
        ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.7f, 1), "Aucun message...");
        ImGui::TextColored(ImVec4(0.4f, 0.4f, 0.5f, 1),
                           "Commencez à discuter avec le développeur !");
      } else {
        for (auto &[isAdm, txt] : s_ChatHistory) {
          ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 8.0f);
          ImVec2 textSize =
              ImGui::CalcTextSize(txt.c_str(), NULL, true,
                                  ImGui::GetContentRegionAvail().x - 60.0f);
          if (isAdm) {
            // Admin message (Left)
            ImGui::PushStyleColor(
                ImGuiCol_ChildBg,
                ImVec4(tC.x * 0.3f, tC.y * 0.3f, tC.z * 0.3f, 0.8f));
            ImGui::BeginChild(txt.c_str(),
                              ImVec2(textSize.x + 20, textSize.y + 16), false,
                              ImGuiWindowFlags_NoScrollbar);
            ImGui::TextWrapped("%s", txt.c_str());
            ImGui::EndChild();
            ImGui::PopStyleColor();
          } else {
            // User message (Right)
            ImGui::SetCursorPosX(ImGui::GetWindowContentRegionMax().x -
                                 textSize.x - 24.0f);
            ImGui::PushStyleColor(
                ImGuiCol_ChildBg,
                ImVec4(tC.x * 0.15f, tC.y * 0.15f, tC.z * 0.15f, 0.8f));
            ImGui::BeginChild(txt.c_str(),
                              ImVec2(textSize.x + 20, textSize.y + 16), false,
                              ImGuiWindowFlags_NoScrollbar);
            ImGui::TextWrapped("%s", txt.c_str());
            ImGui::EndChild();
            ImGui::PopStyleColor();
          }
          ImGui::PopStyleVar();
          ImGui::Dummy(ImVec2(0, 4));
        }
      }

      // Auto-scroll on new message
      if (ImGui::GetScrollMaxY() > s_LastScrollMax) {
        ImGui::SetScrollHereY(1.0f);
        s_LastScrollMax = ImGui::GetScrollMaxY();
      }
      ImGui::EndChild();
      ImGui::PopStyleColor();

      // Chat input area
      ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x - 120);
      ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10, 8));
      ImGui::InputTextWithHint("##chat_input", "Tapez votre message ici...",
                               s_InputBuf, sizeof(s_InputBuf));
      ImGui::PopStyleVar();
      ImGui::PopItemWidth();

      ImGui::SameLine();
      ImGui::PushStyleColor(
          ImGuiCol_Button, ImVec4(tC.x * 0.5f, tC.y * 0.5f, tC.z * 0.5f, 1.0f));
      ImGui::PushStyleColor(
          ImGuiCol_ButtonHovered,
          ImVec4(tC.x * 0.7f, tC.y * 0.7f, tC.z * 0.7f, 1.0f));
      if (ImGui::Button("ENVOYER", ImVec2(110, 32))) {
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
                                 s_ChatStatus = "Message envoyé !";
                                 s_InputBuf[0] = '\0';
                               });
        }
      }
      ImGui::PopStyleColor(2);

      // Keyboard Toggle Button
      ImGui::Dummy(ImVec2(0, 2));
      ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.15f, 0.15f, 0.25f, 1.0f));
      if (ImGui::Button(s_KeyboardOpen ? "v Masquer le Clavier"
                                       : "^ Ouvrir Clavier Visuel",
                        ImVec2(ImGui::GetContentRegionAvail().x, 26))) {
        s_KeyboardOpen = !s_KeyboardOpen;
      }
      ImGui::PopStyleColor();

      if (!s_ChatStatus.empty()) {
        ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.2f, 1), "%s",
                           s_ChatStatus.c_str());
      }

      ImGui::End(); // Ends "Support Technique"
      ImGui::PopStyleColor(4);
      ImGui::PopStyleVar(3);
      // --- BIG AZERTY Virtual Keyboard (full layout with numbers) ---
      if (s_KeyboardOpen) {
        ImVec2 winPos = ImGui::GetWindowPos();
        float winW = ImGui::GetWindowWidth();
        float winH = ImGui::GetWindowHeight();

        // Render keyboard as a completely separate overlay window
        // Fix keyboard position to avoid going off-screen
        float kbH = 220.0f;
        float kbY = ImGui::GetIO().DisplaySize.y - kbH -
                    20.0f; // 20px padding from the absolute bottom
        if (kbY < winPos.y + winH * 0.5f)
          kbY = winPos.y + winH * 0.5f; // ensure it doesn't overlap too high

        ImGui::SetNextWindowPos(
            ImVec2(ImGui::GetIO().DisplaySize.x * 0.5f - (winW * 0.9f) * 0.5f,
                   kbY),
            ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(winW * 0.9f, 220.0f), ImGuiCond_Always);

        ImU32 themeU32 = GetGradientColorU32(0.5f);
        ImVec4 tC = ImGui::ColorConvertU32ToFloat4(themeU32);

        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 12.0f);
        ImGui::PushStyleColor(
            ImGuiCol_WindowBg,
            ImVec4(tC.x * 0.08f, tC.y * 0.08f, tC.z * 0.08f, 0.98f));
        ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(tC.x * 0.6f, tC.y * 0.6f,
                                                      tC.z * 0.6f, 1.0f));

        ImGui::Begin("Clavier Virtuel", nullptr,
                     ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                         ImGuiWindowFlags_NoMove |
                         ImGuiWindowFlags_NoSavedSettings);

        ImGui::Dummy(ImVec2(0, 4));
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(5, 5));
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 6.0f);

        float totalW = ImGui::GetContentRegionAvail().x;
        float btnH = 42.0f;

        // Row 0: Numbers 1-0 (10 keys)
        static const char *numRow[10] = {"1", "2", "3", "4", "5",
                                         "6", "7", "8", "9", "0"};
        float numBtnW = (totalW - 9 * 5) / 10.0f;
        for (int c = 0; c < 10; c++) {
          ImGui::PushStyleColor(
              ImGuiCol_Button,
              ImVec4(tC.x * 0.2f, tC.y * 0.2f, tC.z * 0.2f, 1.0f));
          if (ImGui::Button(numRow[c], ImVec2(numBtnW, btnH - 8)))
            strncat(s_InputBuf, numRow[c],
                    sizeof(s_InputBuf) - strlen(s_InputBuf) - 1);
          ImGui::PopStyleColor();
          if (c < 9)
            ImGui::SameLine();
        }

        // Row 1: A Z E R T Y U I O P
        static const char *row1[10] = {"A", "Z", "E", "R", "T",
                                       "Y", "U", "I", "O", "P"};
        float btnW = (totalW - 9 * 5) / 10.0f;
        for (int c = 0; c < 10; c++) {
          if (ImGui::Button(row1[c], ImVec2(btnW, btnH)))
            strncat(s_InputBuf, row1[c],
                    sizeof(s_InputBuf) - strlen(s_InputBuf) - 1);
          if (c < 9)
            ImGui::SameLine();
        }

        // Row 2: Q S D F G H J K L M (offset 0.5 key)
        static const char *row2[10] = {"Q", "S", "D", "F", "G",
                                       "H", "J", "K", "L", "M"};
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + btnW * 0.5f);
        for (int c = 0; c < 10; c++) {
          if (ImGui::Button(row2[c], ImVec2(btnW, btnH)))
            strncat(s_InputBuf, row2[c],
                    sizeof(s_InputBuf) - strlen(s_InputBuf) - 1);
          if (c < 9)
            ImGui::SameLine();
        }

        // Row 3: W X C V B N . ? ! (9 keys, offset 1.5 key)
        static const char *row3[9] = {"W", "X", "C", "V", "B",
                                      "N", ".", "?", "!"};
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + btnW * 1.5f);
        for (int c = 0; c < 9; c++) {
          if (ImGui::Button(row3[c], ImVec2(btnW, btnH)))
            strncat(s_InputBuf, row3[c],
                    sizeof(s_InputBuf) - strlen(s_InputBuf) - 1);
          if (c < 8)
            ImGui::SameLine();
        }

        // Bottom row: ESPACE + EFFACER
        float spaceW = totalW * 0.68f;
        float bkspW = totalW - spaceW - 5.0f;
        ImGui::PushStyleColor(ImGuiCol_Button,
                              ImVec4(0.18f, 0.18f, 0.28f, 1.0f));
        if (ImGui::Button("  ESPACE  ", ImVec2(spaceW, btnH)))
          strncat(s_InputBuf, " ", sizeof(s_InputBuf) - strlen(s_InputBuf) - 1);
        ImGui::PopStyleColor();
        ImGui::SameLine();
        ImGui::PushStyleColor(ImGuiCol_Button,
                              ImVec4(0.45f, 0.08f, 0.08f, 1.0f));
        if (ImGui::Button("<= EFFACER", ImVec2(bkspW, btnH))) {
          size_t len = strlen(s_InputBuf);
          if (len > 0)
            s_InputBuf[len - 1] = '\0';
        }
        ImGui::PopStyleColor();
        ImGui::PopStyleVar(2);
        ImGui::End();
        ImGui::PopStyleColor(2);
        ImGui::PopStyleVar();
      }
    }
    ImGui::End();
  }
  ImGui::PopStyleVar(); // Pop WindowBorderSize pushed before GRAVITY VIP Begin
}