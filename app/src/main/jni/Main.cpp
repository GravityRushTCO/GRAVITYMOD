#include "Aimbot.h"
#include "Dobby/dobby.h"
#include "Esp.h"
#include "GravityGL/Catalog.h"
#include "GravityGL/Offsets.h"
#include "HttpUtils.h"
#include "Hwid.h"
#include "LicenseSystem.h"
#include "il2cpp_bridge.h"
#include <android/log.h>
#include <dlfcn.h>
#include <errno.h>
#include <fstream>
#include <ftw.h>
#include <jni.h>
#include <map>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <sys/stat.h>
#include <thread>
#include <time.h>
#include <unistd.h>
#include <vector>


#define LOGI(...)                                                              \
  __android_log_print(ANDROID_LOG_INFO, "GravityMod", __VA_ARGS__)

uintptr_t g_base = 0;
bool g_CheatDetected = false;

static void DetectDebuggerAndFrida() {
  while (true) {
    // 1. Check TracerPid (Debugger)
    std::ifstream statusFile("/proc/self/status");
    std::string line;
    while (std::getline(statusFile, line)) {
      if (line.find("TracerPid:") == 0) {
        int tracerPid = 0;
        try {
          tracerPid = std::stoi(line.substr(10));
        } catch (...) {
        }
        if (tracerPid != 0)
          g_CheatDetected = true;
        break;
      }
    }

    // 2. Check maps for frida or xposed
    std::ifstream mapsFile("/proc/self/maps");
    while (std::getline(mapsFile, line)) {
      if (line.find("frida") != std::string::npos ||
          line.find("xposed") != std::string::npos ||
          line.find("libgadget") != std::string::npos ||
          line.find("edhook") != std::string::npos) {
        g_CheatDetected = true;
        break;
      }
    }
    sleep(5);
  }
}
extern bool IsValidMemoryFast(void *ptr, size_t size);
static inline bool isValidPointer(void *ptr) {
  if (!ptr)
    return false;
  uintptr_t val = (uintptr_t)ptr;
  if (val < 0x10000ULL || val > 0x7FFFFFFFFFFFULL)
    return false;
  return IsValidMemoryFast(ptr);
}

#define RVA_FindGoal 0x2516B34
#define RVA_TryGetNearVehicleSeat 0x277E4E8
#define RVA_FireOnPreCull 0x42AE5A8
#define RVA_EventSystemUpdate 0x4570D4C
#define RVA_PreSimulationUpdate 0x3262620
#define RVA_CheckpointFactory_CreateCheckpoint 0x29B4B84
#define RVA_GetDeviceUniqueIdentifier 0x4301B80
#define RVA_GetDeviceName 0x4301BD0
#define RVA_GetDeviceModel 0x4301C20
#define RVA_GetAuthUniqueIdentifier 0x2AF9188
#define RVA_GetExtDeviceIdReal 0x2AF9684
#define RVA_GetExtDeviceIdGenerated 0x2AF98D8
#define RVA_PlayerPrefsGetString 0x42F2D9C
#define RVA_PlayerPrefsGetString1 0x42F2DE0
#define RVA_GetGuestId 0x25B1BE8

extern double Esp_GetTimeMs();

#ifndef V3_STRUCT_DEFINED
#define V3_STRUCT_DEFINED
struct V3 {
  float x, y, z;
};
#endif

extern bool Esp_GetLocalPosition(V3 *out);

struct Q4 {
  float x, y, z, w;
};

struct C4 {
  float r, g, b, a;
};

struct Entity {
  long value;
};

struct NullableV3 {
  V3 value;
  bool hasValue;
  char pad[3];
};

// Global variables exported and used by VipMenu.cpp
bool g_GodModeEnabled = false;
extern bool g_RestoreGodModeNextFrame;
bool g_AimbotHeadshot = false;
extern "C" void SetGodModePatchesState(bool enabled);
extern "C" void Aimbot_TempEnableGodMode();
extern "C" void Esp_CaptureCameraMatrix(void *cam);

// Stub MemoryPatch objects referenced by Aimbot.cpp (legacy god-mode system)
#include "KittyMemory/MemoryPatch.hpp"
#define RET "C0035FD6"
MemoryPatch patch_Knockout1;
MemoryPatch patch_Knockout2;
MemoryPatch patch_HealthSync;
MemoryPatch patch_DeadSync;
MemoryPatch patch_ProcessDead;
MemoryPatch patch_UnconsciousDetect;
MemoryPatch patch_MeleeKnockoutCleanup;
MemoryPatch patch_DeadKnockoutRestore;
MemoryPatch patch_KnockoutSync;
MemoryPatch patch_RagDollState;
MemoryPatch patch_NoReload;
// === GOD MODE ETENDU ===
MemoryPatch patch_GhostMelee; // WeaponAttackMeleeReceiveSystem.OnUpdate → RET
                              // (bloque KO mêlée)
MemoryPatch patch_MeleeHitReceive; // MeleeHitReceiveSystem.OnUpdate → RET
                                   // (bloque hit ECS)
MemoryPatch patch_MeleeHitStun;    // MeleeHitStunCancelSystem.OnUpdate → RET
MemoryPatch patch_DeathUI1;        // HudDeathCreateSystem.OnUpdate → RET
MemoryPatch patch_DeathUI2;        // HudDeathWatchChangesSystem.OnUpdate → RET
MemoryPatch patch_DeathUI3; // HudDeathInitializeOtherSystem.OnUpdate → RET
MemoryPatch patch_PedKill;  // PedKillSystem.OnUpdate → RET
// === ZONE VERTE ===
MemoryPatch patch_GreenZone; // WeaponAttackNotAllowedSyncSystem.OnUpdate → RET
// VIP Hacks patches (patterns from Lua reference)
MemoryPatch patch_NoRecoilVip;    // e7: h CD CC CC 3D → 00 00 00 00
MemoryPatch patch_SuperRecoilVip; // e9: h 66 66 66 3F → 00 00 40 40
MemoryPatch patch_StaminaVip;     // e11: h 00 00 30 C1 → 00 00 C6 42
MemoryPatch
    patch_MoveToVehicleVip; // e13: h 00 00 00 3F 9A 99 19 3E (DWORD pair)
MemoryPatch patch_SpeedOfMovementVip; // e15: h 66 66 A6 3F → 28 6B 6E 4E

bool g_NoClipEnabled = false;
bool g_VehicleNoClipEnabled = false;
bool g_TpCarteToggle = false;
bool g_NoReloadEnabled = false;
bool g_GreenZoneBypass = false;
extern "C" bool g_DeviceIdFaker = false;
bool g_IgnoreAllCollisions = false;
bool g_ForceDriverSeat = false;
bool g_AutoCheckpointEnabled = false;
bool g_AutoCheckpointWithDelay = false;
bool g_DragCheckpointToPlayer = false;
bool g_DragCheckpointGPS = false;
bool g_DragCheckpointJob = false;
bool g_DragCheckpointEvent = false;
bool g_DragCheckpointQuest = false;
bool g_DragCheckpointOther = false;
bool g_AutoSkipTutorial = false;
float g_DragCheckpointDelay = 0.0f; // 0 = redirect every call (no oscillation)
double g_DragBurstEndTime =
    0.0; // auto-disable timer for burst mode from Esp_ForceRapatrierMarqueurs
float g_HeistFarmDelay = 5.0f;
bool g_AimVisibleOnly = false;
bool g_VehicleWallhack = false;
bool g_SuperSpeedEnabled = false;
bool g_FlyEnabled = false;
bool g_FarmMoneyEnabled = false;
bool g_TpEnemy10sEnabled = false;
bool g_MeuDestinoEnabled = false;
float g_SpeedMultiplier = 1.0f;
float g_FlySpeedForward = 10.0f;
float g_FlySpeedVertical = 5.0f;
float g_CameraFov = 60.0f;
bool g_CameraFovEnabled = false;
float g_AutoFollowDistance = 0.0f;
float g_AutoFollowHeight = 3.5f;
int g_BonePriority = 0;
bool g_VisibilityCheck = false;
float g_AimLockSmoothness = 2.0f;
bool g_AimLockCamera = false;
bool g_FreecamLock = false;

int g_VehicleReplaceVal = 0;
int g_SkinReplaceVal = 0;
int g_WeaponReplaceVal = 0;
bool g_ForceWeaponApply = false;
bool g_ForceSkinApply = false;

// Forward declaration - implemented after hooks
static void ApplyCatalogNow(int type);

// === GLOBALS VIP HACKS ===
bool g_VipSpeedRun = false;
bool g_VipBigJump = false;
bool g_VipWallHack = false;
bool g_VipNoRecoil = false;
bool g_VipSuperRecoil = false;
bool g_VipStaminaInfinie = false;
bool g_VipMoveToVehicle = false;
bool g_VipSpeedOfMovement = false;
// Vehicule VIP
float g_VipVehicleSpeed = 0.0f;        // VehicleViewParams +0x78 (FLOAT)
float g_VipVehicleAngle = 0.0f;        // VehicleViewParams +0xAC (FLOAT)
bool g_VipVehicleMaxBrake = false;     // VehicleViewParams +0x88 = 999999
float g_VipVehicleForwardForce = 0.0f; // WheelViewParams +0x24
bool g_VipVehicleNoDamage = false;     // VehicleViewParams +0x12C = 999999
bool g_VipVehicleInfFuel =
    false; // VehicleParametersItemSettingsValuesModel +0x18
float g_VipVehicleSlipping = 0.0f; // WheelViewParams +0x2C
int g_VipVehicleWheelSize = 0;     // WheelViewParams +0x40 (DWORD)

void *g_PedStreamingProviderInstance = nullptr;
void *g_VehiclesStreamingProviderInstance = nullptr;

static float g_LastWaypointX = 0.0f, g_LastWaypointY = 0.0f,
             g_LastWaypointZ = 0.0f;
static bool g_HasWaypoint = false;

static char g_FakeDeviceId[64] = "";
static char g_FakeDeviceModel[64] = "";
static char g_FakeDeviceName[64] = "";

static size_t getMyAppDataDir(char *out, size_t outSize) {
  if (outSize < 32)
    return 0;
  FILE *f = fopen("/proc/self/cmdline", "rb");
  if (!f)
    return 0;
  char pkg[256] = {0};
  size_t r = fread(pkg, 1, sizeof(pkg) - 1, f);
  fclose(f);
  if (r == 0)
    return 0;
  pkg[sizeof(pkg) - 1] = 0;
  for (size_t i = 0; i < r; i++)
    if (pkg[i] == ':') {
      pkg[i] = 0;
      break;
    }
  if (!pkg[0])
    return 0;
  int n = snprintf(out, outSize, "/data/data/%s", pkg);
  return (n > 0 && (size_t)n < outSize) ? (size_t)n : 0;
}

static int rmCallback(const char *fpath, const struct stat * /*sb*/,
                      int typeflag, struct FTW *ftwbuf) {
  if (ftwbuf->level == 0)
    return 0;
  if (typeflag == FTW_DP || typeflag == FTW_D) {
    rmdir(fpath);
  } else {
    unlink(fpath);
  }
  return 0;
}

static void wipeDirContents(const char *path) {
  nftw(path, rmCallback, 16, FTW_DEPTH | FTW_PHYS);
}

static void getWipeFlagPath(char *out, size_t outSize) {
  char dataDir[256];
  size_t l = getMyAppDataDir(dataDir, sizeof(dataDir));
  if (!l) {
    out[0] = 0;
    return;
  }
  snprintf(out, outSize, "%s/cache/wipe_pending", dataDir);
}

static void scheduleWipeNextBoot() {
  char path[512];
  getWipeFlagPath(path, sizeof(path));
  if (!path[0]) {
    LOGI("scheduleWipe: dataDir introuvable");
    return;
  }
  char cacheDir[512];
  size_t pl = strlen(path);
  if (pl > 14) {
    size_t cl = pl - 14;
    if (cl < sizeof(cacheDir)) {
      memcpy(cacheDir, path, cl);
      cacheDir[cl] = 0;
      mkdir(cacheDir, 0700);
    }
  }
  FILE *f = fopen(path, "wb");
  if (f) {
    fwrite("1", 1, 1, f);
    fclose(f);
    LOGI("Wipe scheduled at %s", path);
  } else {
    LOGI("Wipe schedule FAILED at %s (errno=%d)", path, errno);
  }
}

static void wipeAppDataIfFlagged() {
  char flag[512];
  getWipeFlagPath(flag, sizeof(flag));
  if (!flag[0])
    return;
  struct stat st;
  if (stat(flag, &st) != 0)
    return;
  LOGI("*** WIPE FLAG DETECTED — simulating fresh install ***");
  char dataDir[256];
  if (!getMyAppDataDir(dataDir, sizeof(dataDir)))
    return;
  const char *subdirs[] = {
      "/files",     "/shared_prefs", "/databases",   "/cache",
      "/no_backup", "/app_textures", "/app_webview", "/code_cache",
  };
  for (size_t i = 0; i < sizeof(subdirs) / sizeof(subdirs[0]); i++) {
    char p[512];
    snprintf(p, sizeof(p), "%s%s", dataDir, subdirs[i]);
    LOGI("wiping %s ...", p);
    wipeDirContents(p);
  }
  unlink(flag);
  LOGI("*** WIPE COMPLETE — game will see this as a brand new install ***");
}

static void regenerateFakeDeviceId() {
  static const char *hex = "0123456789abcdef";
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  uint64_t seed = (uint64_t)time(nullptr) ^ (uint64_t)(uintptr_t)&seed ^
                  (uint64_t)ts.tv_nsec ^ ((uint64_t)ts.tv_sec << 32);
  uint64_t s = seed;
  auto rnd = [&]() {
    s ^= s << 13;
    s ^= s >> 7;
    s ^= s << 17;
    return (uint32_t)s;
  };
  const int groups[] = {8, 4, 4, 4, 12};
  int idx = 0;
  for (int g = 0; g < 5; g++) {
    for (int k = 0; k < groups[g]; k++) {
      g_FakeDeviceId[idx++] = hex[rnd() & 0xF];
    }
    if (g < 4)
      g_FakeDeviceId[idx++] = '-';
  }
  g_FakeDeviceId[idx] = 0;

  static const char *models[] = {
      "SM-G998B",   "SM-S908E", "Pixel 7 Pro", "Pixel 6",   "ONEPLUS A6013",
      "M2102J20SG", "Mi 11",    "RMX3371",     "CPH2451",   "SM-A536E",
      "SM-G991B",   "Pixel 8",  "23021RAA2Y",  "2201116SG", "SM-A546E",
  };
  uint32_t mi = rnd() % (uint32_t)(sizeof(models) / sizeof(models[0]));
  snprintf(g_FakeDeviceModel, sizeof(g_FakeDeviceModel), "%s", models[mi]);

  snprintf(g_FakeDeviceName, sizeof(g_FakeDeviceName), "Phone-%04X",
           (unsigned)(rnd() & 0xFFFF));
  LOGI("Device fake -> id='%s' model='%s' name='%s'", g_FakeDeviceId,
       g_FakeDeviceModel, g_FakeDeviceName);
}

static bool isIl2cppStringEqual(void *il2cppStr, const char *cStr) {
  if (!il2cppStr || !cStr)
    return false;
  int len = *(int *)((uint8_t *)il2cppStr + 0x10);
  if (len < 0 || len > 2048)
    return false;
  int cLen = strlen(cStr);
  if (len != cLen)
    return false;
  uint16_t *chars = (uint16_t *)((uint8_t *)il2cppStr + 0x14);
  for (int i = 0; i < len; i++) {
    if (chars[i] != (uint16_t)cStr[i])
      return false;
  }
  return true;
}

typedef void *(*GetDeviceUniqueIdentifier_t)(void *method);
static GetDeviceUniqueIdentifier_t orig_GetDeviceUniqueIdentifier = nullptr;
static void *hook_GetDeviceUniqueIdentifier(void *method) {
  if (g_DeviceIdFaker && g_FakeDeviceId[0] != 0) {
    extern void *Esp_MakeIl2cppString(const char *);
    void *fake = Esp_MakeIl2cppString(g_FakeDeviceId);
    if (fake)
      return fake;
  }
  return orig_GetDeviceUniqueIdentifier(method);
}

typedef void *(*GetDeviceModel_t)(void *method);
static GetDeviceModel_t orig_GetDeviceModel = nullptr;
static void *hook_GetDeviceModel(void *method) {
  if (g_DeviceIdFaker && g_FakeDeviceModel[0] != 0) {
    extern void *Esp_MakeIl2cppString(const char *);
    void *fake = Esp_MakeIl2cppString(g_FakeDeviceModel);
    if (fake)
      return fake;
  }
  return orig_GetDeviceModel(method);
}

typedef void *(*GetDeviceName_t)(void *method);
static GetDeviceName_t orig_GetDeviceName = nullptr;
static void *hook_GetDeviceName(void *method) {
  if (g_DeviceIdFaker && g_FakeDeviceName[0] != 0) {
    extern void *Esp_MakeIl2cppString(const char *);
    void *fake = Esp_MakeIl2cppString(g_FakeDeviceName);
    if (fake)
      return fake;
  }
  return orig_GetDeviceName(method);
}

typedef void *(*GetAuthUniqueIdentifier_t)(void *method);
static GetAuthUniqueIdentifier_t orig_GetAuthUniqueIdentifier = nullptr;
static void *hook_GetAuthUniqueIdentifier(void *method) {
  if (g_DeviceIdFaker && g_FakeDeviceId[0] != 0) {
    extern void *Esp_MakeIl2cppString(const char *);
    void *fake = Esp_MakeIl2cppString(g_FakeDeviceId);
    if (fake)
      return fake;
  }
  return orig_GetAuthUniqueIdentifier(method);
}

typedef void *(*GetExtDeviceIdReal_t)(void *method);
static GetExtDeviceIdReal_t orig_GetExtDeviceIdReal = nullptr;
static void *hook_GetExtDeviceIdReal(void *method) {
  if (g_DeviceIdFaker && g_FakeDeviceId[0] != 0) {
    extern void *Esp_MakeIl2cppString(const char *);
    void *fake = Esp_MakeIl2cppString(g_FakeDeviceId);
    if (fake)
      return fake;
  }
  return orig_GetExtDeviceIdReal(method);
}

typedef void *(*GetExtDeviceIdGenerated_t)(void *method);
static GetExtDeviceIdGenerated_t orig_GetExtDeviceIdGenerated = nullptr;
static void *hook_GetExtDeviceIdGenerated(void *method) {
  if (g_DeviceIdFaker && g_FakeDeviceId[0] != 0) {
    extern void *Esp_MakeIl2cppString(const char *);
    void *fake = Esp_MakeIl2cppString(g_FakeDeviceId);
    if (fake)
      return fake;
  }
  return orig_GetExtDeviceIdGenerated(method);
}

typedef void *(*PlayerPrefsGetString_t)(void *key, void *defaultValue,
                                        void *method);
static PlayerPrefsGetString_t orig_PlayerPrefsGetString = nullptr;
static void *hook_PlayerPrefsGetString(void *key, void *defaultValue,
                                       void *method) {
  if (g_DeviceIdFaker && g_FakeDeviceId[0] != 0) {
    if (isIl2cppStringEqual(key, "GUEST_ID") ||
        isIl2cppStringEqual(key, "DEVICE_ID")) {
      extern void *Esp_MakeIl2cppString(const char *);
      void *fake = Esp_MakeIl2cppString(g_FakeDeviceId);
      if (fake)
        return fake;
    }
  }
  return orig_PlayerPrefsGetString(key, defaultValue, method);
}

typedef void *(*PlayerPrefsGetString1_t)(void *key, void *method);
static PlayerPrefsGetString1_t orig_PlayerPrefsGetString1 = nullptr;
static void *hook_PlayerPrefsGetString1(void *key, void *method) {
  if (g_DeviceIdFaker && g_FakeDeviceId[0] != 0) {
    if (isIl2cppStringEqual(key, "GUEST_ID") ||
        isIl2cppStringEqual(key, "DEVICE_ID")) {
      extern void *Esp_MakeIl2cppString(const char *);
      void *fake = Esp_MakeIl2cppString(g_FakeDeviceId);
      if (fake)
        return fake;
    }
  }
  return orig_PlayerPrefsGetString1(key, method);
}

typedef void *(*GetGuestId_t)(void *self, void *method);
static GetGuestId_t orig_GetGuestId = nullptr;
static void *hook_GetGuestId(void *self, void *method) {
  if (g_DeviceIdFaker && g_FakeDeviceId[0] != 0) {
    extern void *Esp_MakeIl2cppString(const char *);
    void *fake = Esp_MakeIl2cppString(g_FakeDeviceId);
    if (fake)
      return fake;
  }
  return orig_GetGuestId(self, method);
}

extern "C" V3 g_LastExecutedWaypoint = {0.0f, 0.0f, 0.0f};
extern "C" void Esp_SetPendingWaypoint(float x, float y, float z, bool valid);

// --- HOOK HudMapNavigationSystem.FindGoal @ 0x2516B34 ---
typedef bool (*FindGoal_t)(void *_this, int playerDimension, V3 position,
                           V3 *goal, void *goalEntity, void *method);
static FindGoal_t orig_FindGoal = nullptr;

// Amélioration: force l'identification du localActor et utilise la
// position de la caméra comme fallback si s_localActor n'est pas encore défini
enum class DragType { GPS, Job, Event, Quest, Other };

extern "C" bool Esp_GetCameraPosition(V3 *);

static bool GetDraggedMarkerPosition(V3 fallback, V3 *out, DragType type) {
  if (!out) {
    return false;
  }

  bool shouldDrag = false;

  extern bool g_DragCheckpointGPS;
  extern bool g_DragCheckpointJob;
  extern bool g_DragCheckpointEvent;
  extern bool g_DragCheckpointQuest;
  extern bool g_DragCheckpointOther;
  extern bool g_DragCheckpointToPlayer;

  if (type == DragType::GPS && g_DragCheckpointGPS)
    shouldDrag = true;
  if (type == DragType::Job && g_DragCheckpointJob)
    shouldDrag = true;
  if (type == DragType::Event && g_DragCheckpointEvent)
    shouldDrag = true;
  if (type == DragType::Quest && g_DragCheckpointQuest)
    shouldDrag = true;
  if (type == DragType::Other && g_DragCheckpointOther)
    shouldDrag = true;
  if (g_DragCheckpointToPlayer)
    shouldDrag = true;

  if (!shouldDrag) {
    return false;
  }

  extern float g_DragCheckpointDelay;
  static double lastDragTime = 0.0;
  double now = Esp_GetTimeMs();
  float delayMs = g_DragCheckpointDelay * 1000.0f;

  // Throttle only when a non-zero delay is configured
  if (delayMs > 0.0f) {
    if (now - lastDragTime < delayMs)
      return false;
  }

  if (Esp_GetLocalPosition(out)) {
    lastDragTime = now;
    return true;
  }
  // Fallback: camera position is valid even during tutorial
  if (Esp_GetCameraPosition(out)) {
    lastDragTime = now;
    return true;
  }
  // No position at all — don't drag
  return false;
}

typedef void (*CreateMapGpsCheckpointCtor_t)(void *_this, V3 point,
                                             void *method);
static CreateMapGpsCheckpointCtor_t orig_CreateMapGpsCheckpointCtor = nullptr;
static void hook_CreateMapGpsCheckpointCtor(void *_this, V3 point,
                                            void *method) {
  V3 dragged = {0.0f, 0.0f, 0.0f};
  if (GetDraggedMarkerPosition(point, &dragged, DragType::GPS)) {
    point = dragged;
  }

  if (orig_CreateMapGpsCheckpointCtor) {
    orig_CreateMapGpsCheckpointCtor(_this, point, method);
  }
}

typedef void (*CreateMapCheckpointSignalHandler_t)(void *_this, void *signal,
                                                   void *method);
static CreateMapCheckpointSignalHandler_t
    orig_CreateMapCheckpointSignalHandler = nullptr;
static void hook_CreateMapCheckpointSignalHandler(void *_this, void *signal,
                                                  void *method) {
  if (isValidPointer(signal)) {
    V3 original = *(V3 *)((char *)signal + 0x10);
    V3 dragged = {0.0f, 0.0f, 0.0f};
    if (GetDraggedMarkerPosition(original, &dragged, DragType::GPS)) {
      *(V3 *)((char *)signal + 0x10) = dragged;
    }
  }

  if (orig_CreateMapCheckpointSignalHandler) {
    orig_CreateMapCheckpointSignalHandler(_this, signal, method);
  }
}

typedef void (*SetMapPosition_t)(void *_this, V3 worldPosition, void *method);
static SetMapPosition_t orig_SetMapPosition = nullptr;
static void hook_SetMapPosition(void *_this, V3 worldPosition, void *method) {
  // Drag GPS marker to player
  V3 dragged = {0.0f, 0.0f, 0.0f};
  if (GetDraggedMarkerPosition(worldPosition, &dragged, DragType::GPS)) {
    worldPosition = dragged;
  }

  // TP Carte: teleport player to the map position (works during tutorial too)
  extern bool g_TpCarteToggle;
  if (g_TpCarteToggle) {
    static double lastMapTpTime = 0.0;
    double now = Esp_GetTimeMs();
    if (now - lastMapTpTime > 800.0) {
      lastMapTpTime = now;
      Teleport_ToPosition(worldPosition.x, worldPosition.y, worldPosition.z, true);
    }
  }

  if (orig_SetMapPosition) {
    orig_SetMapPosition(_this, worldPosition, method);
  }
}

struct Quaternion { float x,y,z,w; };
struct Color { float r,g,b,a; };
struct NullableVector3 { V3 value; bool has_value; };

typedef void* (*CheckpointFactory_CreateCheckpoint_t)(
    void* _this, 
    V3 position, Quaternion rotation, Color color, 
    void* text, void* iconName, void* activityIcon, void* activityText, void* activityDescription, 
    int model, int type, int dimension, float radius, 
    bool allowPed, bool allowVehicle, bool withBlip, bool highPriority, 
    bool ignoreActivityCreation, bool destroyActivityOnDimensionChange, bool blockPedOnEnter, 
    void* onEnterAction, void* onExitAction, 
    NullableVector3 checkpointRotatePedOnEnterPosition, bool createLegendItem,
    void* method);

static CheckpointFactory_CreateCheckpoint_t orig_CreateCheckpoint = nullptr;
static void* hook_CreateCheckpoint(
    void* _this, 
    V3 position, Quaternion rotation, Color color, 
    void* text, void* iconName, void* activityIcon, void* activityText, void* activityDescription, 
    int model, int type, int dimension, float radius, 
    bool allowPed, bool allowVehicle, bool withBlip, bool highPriority, 
    bool ignoreActivityCreation, bool destroyActivityOnDimensionChange, bool blockPedOnEnter, 
    void* onEnterAction, void* onExitAction, 
    NullableVector3 checkpointRotatePedOnEnterPosition, bool createLegendItem,
    void* method) 
{
  V3 dragged = {0,0,0};
  
  // Drag all checkpoints to the player if they have any of the drag features enabled
  if (GetDraggedMarkerPosition(position, &dragged, DragType::Job) ||
      GetDraggedMarkerPosition(position, &dragged, DragType::Quest) ||
      GetDraggedMarkerPosition(position, &dragged, DragType::Other)) {
      position = dragged;
  }
  
  if (orig_CreateCheckpoint) {
      return orig_CreateCheckpoint(
          _this, position, rotation, color, 
          text, iconName, activityIcon, activityText, activityDescription, 
          model, type, dimension, radius, 
          allowPed, allowVehicle, withBlip, highPriority, 
          ignoreActivityCreation, destroyActivityOnDimensionChange, blockPedOnEnter, 
          onEnterAction, onExitAction, 
          checkpointRotatePedOnEnterPosition, createLegendItem, 
          method);
  }
  return nullptr;
}

extern "C" void *g_LastFindGoalInstance = nullptr;

static bool hook_FindGoal(void *_this, int playerDimension, V3 position,
                          V3 *goal, void *goalEntity, void *method) {
  if (_this) {
    g_LastFindGoalInstance = _this;
  }
  bool found = false;
  if (orig_FindGoal) {
    found = orig_FindGoal(_this, playerDimension, position, goal, goalEntity,
                          method);
  }
  if (found && goal) {
    float gx = goal->x;
    float gy = goal->y;
    float gz = goal->z;

    if (gx * gx + gy * gy + gz * gz > 1.0f) {
      Esp_SetPendingWaypoint(gx, gy, gz, true);
    }

    extern bool g_DragCheckpointToPlayer;
    extern bool g_DragCheckpointGPS;
    extern bool g_DragCheckpointJob;
    extern bool g_DragCheckpointEvent;
    extern bool g_DragCheckpointQuest;
    extern bool g_DragCheckpointOther;

    bool dragAny = g_DragCheckpointToPlayer || g_DragCheckpointGPS ||
                   g_DragCheckpointJob || g_DragCheckpointEvent ||
                   g_DragCheckpointQuest || g_DragCheckpointOther;

    if (dragAny) {
      extern float g_DragCheckpointDelay;
      extern double g_DragBurstEndTime;
      static double lastDragTime = 0.0;
      double now = Esp_GetTimeMs();
      float delayMs = g_DragCheckpointDelay * 1000.0f;

      if (g_DragBurstEndTime > 0.0 && now >= g_DragBurstEndTime) {
        g_DragCheckpointToPlayer = false;
        g_DragBurstEndTime = 0.0;
      }
      // Rapatriement avec THROTTLE obligatoire pour éviter le freeze infini.
      // Le pathfinding boucle dans la même frame si on assigne goal = position
      // en continu.
      float effectiveDelay = (delayMs > 0.0f) ? delayMs : 50.0f; // Minimum 50ms
      if (now - lastDragTime >= effectiveDelay) {
        lastDragTime = now;
        *goal = position;
      }
    }

  }
  return found;
}

typedef bool (*TryGetNearVehicleSeat_t)(void *_this, V3 position, void *vehiclesKeys, void *pedView, void *logicVehicle, int *seatIndex, void *openDoorPointIndex, void *method);
static TryGetNearVehicleSeat_t orig_TryGetNearVehicleSeat = nullptr;

static bool hook_TryGetNearVehicleSeat(void *_this, V3 position, void *vehiclesKeys, void *pedView, void *logicVehicle, int *seatIndex, void *openDoorPointIndex, void *method) {
  bool result = orig_TryGetNearVehicleSeat(_this, position, vehiclesKeys,
                                           pedView, logicVehicle, seatIndex,
                                           openDoorPointIndex, method);
                                         if (result && seatIndex != nullptr) {
                                           if (g_ForceDriverSeat) {
                                             *seatIndex =
                                                 0; // Force driver seat
                                           }
                                         }
                                         return result;
                                       }

                                       extern "C" void setupGravityOverlay();

                                       extern bool g_DragCheckpointToPlayer;
                                       extern float g_DragCheckpointDelay;

                                       static const char *GetFeatureNameById(
                                           int id) {
                                         switch (id) {
                                         case 121:
                                           return "ESP";
                                         case 194:
                                           return "ESP Lines";
                                         case 195:
                                           return "ESP Boxes";
                                         case 196:
                                           return "ESP Distance";
                                         case 197:
                                           return "ESP Marker";
                                         case 198:
                                           return "ESP Dynamic Color";
                                         case 240:
                                           return "ESP Crosshair";
                                         case 241:
                                           return "ESP Health";
                                         case 242:
                                           return "ESP Crosshair Circle";
                                         case 243:
                                           return "ESP Crosshair Circle Radius";
                                         case 120:
                                           return "Aimbot";
                                         case 132:
                                           return "Aimbot WallBang";
                                         case 183:
                                           return "Aimbot Bone Priority";
                                         case 184:
                                           return "Aimbot Visibility Check";
                                         case 185:
                                           return "Aimbot Smoothness";
                                         case 186:
                                           return "Aimbot Lock Camera";
                                         case 187:
                                           return "Freecam Lock";
                                         case 193:
                                           return "Camera FOV";
                                         case 101:
                                           return "God Mode";
                                         case 102:
                                           return "No Clip";
                                         case 103:
                                           return "Vehicle No Clip";
                                         case 104:
                                           return "Teleport Map Toggle";
                                         case 105:
                                           return "No Reload";
                                         case 106:
                                           return "Device ID Faker";
                                         case 107:
                                           return "Ignore Collisions";
                                         case 108:
                                           return "Force Driver Seat";
                                         case 109:
                                           return "Auto Checkpoint";
                                         case 110:
                                           return "Auto Checkpoint With Delay";
                                         case 111:
                                           return "Drag Checkpoint to Player";
                                         case 112:
                                           return "Drag Checkpoint Delay";
                                         case 114:
                                           return "Heist Farm Delay";
                                         case 115:
                                           return "Aim Visible Only";
                                         case 116:
                                           return "Vehicle Wallhack";
                                         case 117:
                                           return "Super Speed";
                                         case 118:
                                           return "Fly Mode";
                                         case 119:
                                           return "Farm Money";
                                         case 122:
                                           return "Speed Multiplier";
                                         case 123:
                                           return "Fly Forward Speed";
                                         case 124:
                                           return "Fly Vertical Speed";
                                         case 127:
                                           return "Auto Follow Distance";
                                         case 128:
                                           return "Auto Follow Height";
                                         case 135:
                                           return "Vehicle Replacement Value";
                                         case 136:
                                           return "Skin Replacement Value";
                                         case 137:
                                           return "Weapon Replacement Value";
                                         case 138:
                                           return "Force Weapon Apply";
                                         case 139:
                                           return "Force Skin Apply";
                                         case 201:
                                           return "VIP SpeedRun";
                                         case 202:
                                           return "VIP BigJump";
                                         case 203:
                                           return "VIP WallHack";
                                         case 204:
                                           return "VIP NoRecoil";
                                         case 205:
                                           return "VIP SuperRecoil";
                                         case 206:
                                           return "VIP Stamina Infinie";
                                         case 207:
                                           return "VIP Move To Vehicle";
                                         case 208:
                                           return "VIP Speed of Movement";
                                         default:
                                           return nullptr;
                                         }
                                       }

                                       void LogAction(const char *fmt, ...) {
                                         char buf[256];
                                         va_list args;
                                         va_start(args, fmt);
                                         vsnprintf(buf, sizeof(buf), fmt, args);
                                         va_end(args);

                                         __android_log_print(ANDROID_LOG_INFO,
                                                             "GravityMod",
                                                             "Action: %s", buf);

                                         std::string action = buf;
                                         std::thread([action]() {
                                           SyncUserData(action.c_str());
                                         }).detach();
                                       }

                                       extern "C" void Changes(
                                           JNIEnv * env, jclass clazz,
                                           jobject obj, jint id, jstring str,
                                           jint intVal, jlong longVal,
                                           jboolean on, jstring str2) {
                                         const char *featName =
                                             GetFeatureNameById(id);
                                         if (featName) {
                                           LogAction("%s -> %s (val: %d)",
                                                     featName,
                                                     on ? "ON" : "OFF", intVal);
                                         } else {
                                           LogAction(
                                               "Feature %d -> %s (val: %d)", id,
                                               on ? "ON" : "OFF", intVal);
                                         }
                                         switch (id) {
                                         // === ESP ===
                                         case 121:
                                           Esp_SetEnabled((bool)on);
                                           break;
                                         case 194:
                                           Esp_SetLineEnabled((bool)on);
                                           break;
                                         case 195:
                                           Esp_SetBoxEnabled((bool)on);
                                           break;
                                         case 196:
                                           Esp_SetDistanceEnabled((bool)on);
                                           break;
                                         case 197:
                                           Esp_SetMarkerEnabled((bool)on);
                                           break;
                                         case 198:
                                           Esp_SetDynamicColor((bool)on);
                                           break;
                                         case 240:
                                           Esp_SetCrosshairEnabled((bool)on);
                                           break;
                                         case 241:
                                           Esp_SetHealthEnabled((bool)on);
                                           break;
                                         case 242:
                                           Esp_SetCrosshairCircleEnabled(
                                               (bool)on);
                                           break;
                                         case 243:
                                           Esp_SetCrosshairCircleRadius(intVal);
                                           break;

                                         // === AIMBOT ===
                                         case 120: {
                                           Aimbot_SetEnabled((bool)on);
                                           break;
                                         }
                                         case 132: {
                                           Aimbot_SetWallBang((bool)on);
                                           break;
                                         }
                                         case 183:
                                           g_BonePriority = intVal;
                                           break;
                                         case 184:
                                           g_VisibilityCheck = (bool)on;
                                           break;
                                         case 185:
                                           g_AimLockSmoothness = (float)intVal;
                                           break;
                                         case 186:
                                           g_AimLockCamera = (bool)on;
                                           break;
                                         case 187:
                                           g_FreecamLock = (bool)on;
                                           break;
                                         case 193:
                                           g_CameraFov = (float)intVal;
                                           g_CameraFovEnabled = (intVal != 60);
                                           break;
                                         case 205:
                                           g_NoReloadEnabled = (bool)on;
                                           if (g_NoReloadEnabled) {
                                             if (patch_NoReload.isValid())
                                               patch_NoReload.Modify();
                                           } else {
                                             if (patch_NoReload.isValid())
                                               patch_NoReload.Restore();
                                           }
                                           break;

                                         // === PLAYER ===
                                         case 13:
                                           g_GodModeEnabled = (bool)on;
                                           SetGodModePatchesState(
                                               g_GodModeEnabled);
                                           break;

                                         // === ZONE VERTE BYPASS ===
                                         case 98:
                                           g_GreenZoneBypass = (bool)on;
                                           if (patch_GreenZone.isValid()) {
                                             if (g_GreenZoneBypass)
                                               patch_GreenZone.Modify();
                                             else
                                               patch_GreenZone.Restore();
                                           }
                                           LOGI(
                                               "GravityMod: GreenZoneBypass=%d",
                                               (int)g_GreenZoneBypass);
                                           break;
                                         case 109:
                                           g_NoClipEnabled = (bool)on;
                                           break;
                                         case 110:
                                           g_VehicleNoClipEnabled = (bool)on;
                                           break;
                                         case 143:
                                           g_SpeedMultiplier =
                                               (float)intVal / 10.0f;
                                           break;

                                         // === DEVICE ID ===
                                         case 153: {
                                           g_DeviceIdFaker = (bool)on;
                                           if (on) {
                                             regenerateFakeDeviceId();
                                             scheduleWipeNextBoot();
                                             wipeAppDataIfFlagged();
                                           } else {
                                             char flag[512];
                                             getWipeFlagPath(flag,
                                                             sizeof(flag));
                                             if (flag[0])
                                               unlink(flag);
                                           }
                                           break;
                                         }
                                         case 154:
                                           regenerateFakeDeviceId();
                                           break;

                                         // === WORLD / VEHICLE ===
                                         case 400:
                                           g_DragCheckpointToPlayer = (bool)on;
                                           break;
                                         case 410:
                                           g_DragCheckpointGPS = (bool)on;
                                           break;
                                         case 411:
                                           g_DragCheckpointJob = (bool)on;
                                           break;
                                         case 412:
                                           g_DragCheckpointEvent = (bool)on;
                                           break;
                                         case 413:
                                           g_DragCheckpointQuest = (bool)on;
                                           break;
                                         case 414:
                                           g_DragCheckpointOther = (bool)on;
                                           break;
                                         case 416:
                                           g_AutoSkipTutorial = (bool)on;
                                           break;
                                         case 401: {
                                           g_DragCheckpointDelay =
                                               (float)intVal / 10.0f;
                                         } break;
                                         case 228:
                                           g_TpCarteToggle = (bool)on;
                                           if (!g_TpCarteToggle) {
                                             extern V3 g_LastExecutedWaypoint;
                                             g_LastExecutedWaypoint = {
                                                 0.0f, 0.0f, 0.0f};
                                           }
                                           break;
                                         case 229:
                                           g_ForceDriverSeat = (bool)on;
                                           break;
                                         case 300:
                                           Esp_SetAutoFollow((bool)on);
                                           break;
                                         case 301:
                                           Esp_SetAutoFollowCar((bool)on);
                                           break;
                                         case 302:
                                           g_AutoFollowDistance = (float)intVal;
                                           break;
                                         case 303:
                                           g_AutoFollowHeight = (float)intVal;
                                           break;
                                         case 304:
                                           Esp_TPCarToTarget();
                                           break;
                                         case 305: {
                                           extern void
                                           Esp_ResetAutoFollowLock();
                                           Esp_ResetAutoFollowLock();
                                           break;
                                         }
                                         case 306: {
                                           extern bool g_StickyCarEnabled;
                                           g_StickyCarEnabled = (bool)on;
                                           break;
                                         }
                                         case 308:
                                           g_FlyEnabled = (bool)on;
                                           break;
                                         case 260:
                                           g_WeaponReplaceVal = intVal;
                                           ApplyCatalogNow(260);
                                           break;
                                         case 261:
                                           g_SkinReplaceVal = intVal;
                                           ApplyCatalogNow(261);
                                           break;
                                         case 262:
                                           g_VehicleReplaceVal = intVal;
                                           ApplyCatalogNow(262);
                                           break;
                                         // === SOINS ===
                                         case 420: { // Heal Now
                                           extern void Esp_HealLocal(
                                               float amount);
                                           Esp_HealLocal(100.0f);
                                           break;
                                         }
                                         case 421: { // Speed Multiplier (tous
                                                     // modes)
                                           g_SpeedMultiplier =
                                               (float)intVal / 10.0f;
                                           break;
                                         }

                                           // === VIP HACKS JOUEUR ===
                                           // Ces fonctions patchent la memoire
                                           // ANONYMOUS du jeu exactement comme
                                           // le Lua de reference

                                         case 500: { // Speed Run ON/OFF
                                           // ON: patch QWORD speed movement
                                           // multiplier Pattern Lua e1: search
                                           // "2146435071;4607182418800017408;1072693248:9"
                                           // QWORD → edit Utilise le hook
                                           // SetVelocity avec un facteur x2
                                           g_VipSpeedRun = (bool)on;
                                           break;
                                         }
                                         case 501: { // Big Jump ON/OFF
                                           // Pattern Lua e3/e4: h 00 00 C0 40 →
                                           // h 00 00 20 41 (jump height 6→10)
                                           // Implémenté via patch mémoire
                                           // ANONYMOUS scan
                                           g_VipBigJump = (bool)on;
                                           // Scan + patch jump height float
                                           if (g_base) {
                                             uint8_t patOn[] = {0x00, 0x00,
                                                                0x20,
                                                                0x41}; // 10.0f
                                             uint8_t patOff[] = {0x00, 0x00,
                                                                 0xC0,
                                                                 0x40}; // 6.0f
                                             // On utilise le hook
                                             // CharacterActor pour modifier
                                             // JumpForce Le flag g_VipBigJump
                                             // est consomme dans
                                             // hook_PreSimulationUpdate
                                           }
                                           break;
                                         }
                                         case 502: { // Wall Hack ON/OFF
                                           // Pattern Lua e5/e6:
                                           // VehicleViewParams collision radius
                                           // h 00 00 48 43 (200) → h 00 00 C8
                                           // C2 (-100) = collision capsule
                                           // négative
                                           g_VipWallHack = (bool)on;
                                           break;
                                         }
                                         case 503: { // No Recoil ON/OFF
                                           // Pattern Lua e7/e8: h CD CC CC 3D →
                                           // h 00 00 00 00 (recoil = 0)
                                           g_VipNoRecoil = (bool)on;
                                           if (patch_NoRecoilVip.isValid()) {
                                             if (on)
                                               patch_NoRecoilVip.Modify();
                                             else
                                               patch_NoRecoilVip.Restore();
                                           }
                                           break;
                                         }
                                         case 504: { // Super Recoil ON/OFF
                                           // Pattern e9/e10: h 66 66 66 3F → h
                                           // 00 00 40 40
                                           g_VipSuperRecoil = (bool)on;
                                           if (patch_SuperRecoilVip.isValid()) {
                                             if (on)
                                               patch_SuperRecoilVip.Modify();
                                             else
                                               patch_SuperRecoilVip.Restore();
                                           }
                                           break;
                                         }
                                         case 505: { // Stamina Infinie ON/OFF
                                           // Pattern e11/e12: h 00 00 30 C1 → h
                                           // 00 00 C6 42 (stamina max = 99)
                                           g_VipStaminaInfinie = (bool)on;
                                           if (patch_StaminaVip.isValid()) {
                                             if (on)
                                               patch_StaminaVip.Modify();
                                             else
                                               patch_StaminaVip.Restore();
                                           }
                                           break;
                                         }
                                         case 506: { // Move to Vehicle Speed
                                                     // ON/OFF
                                           // Pattern e13/e14: h 00 00 00 3F 9A
                                           // 99 19 3E → h 00 00 C6 42 00 00 C6
                                           // 42
                                           g_VipMoveToVehicle = (bool)on;
                                           if (patch_MoveToVehicleVip
                                                   .isValid()) {
                                             if (on)
                                               patch_MoveToVehicleVip.Modify();
                                             else
                                               patch_MoveToVehicleVip.Restore();
                                           }
                                           break;
                                         }
                                         case 507: { // Speed of Movement ON/OFF
                                           // Pattern e15/e16: h 66 66 A6 3F → h
                                           // 28 6B 6E 4E (movement speed max)
                                           g_VipSpeedOfMovement = (bool)on;
                                           if (patch_SpeedOfMovementVip
                                                   .isValid()) {
                                             if (on)
                                               patch_SpeedOfMovementVip
                                                   .Modify();
                                             else
                                               patch_SpeedOfMovementVip
                                                   .Restore();
                                           }
                                           break;
                                         }

                                         // === VIP HACKS VEHICULE ===
                                         case 510: { // Vehicle Speed
                                                     // (VehicleViewParams
                                                     // +0x78)
                                           g_VipVehicleSpeed = (float)intVal;
                                           break;
                                         }
                                         case 511: { // Vehicle Angle
                                                     // (VehicleViewParams
                                                     // +0xAC)
                                           g_VipVehicleAngle = (float)intVal;
                                           break;
                                         }
                                         case 512: { // Max Brake
                                                     // (VehicleViewParams +0x88
                                                     // = 999999)
                                           g_VipVehicleMaxBrake = (bool)on;
                                           break;
                                         }
                                         case 513: { // Forward Force
                                                     // (WheelViewParams +0x24)
                                           g_VipVehicleForwardForce =
                                               (float)intVal;
                                           break;
                                         }
                                         case 514: { // No Damage
                                                     // (VehicleViewParams
                                                     // +0x12C = 999999)
                                           g_VipVehicleNoDamage = (bool)on;
                                           break;
                                         }
                                         case 515: { // Fuel Infini
                                                     // (VehicleParametersItemSettingsValuesModel
                                                     // +0x18)
                                           g_VipVehicleInfFuel = (bool)on;
                                           break;
                                         }
                                         case 516: { // Slipping
                                                     // (WheelViewParams +0x2C)
                                           g_VipVehicleSlipping = (float)intVal;
                                           break;
                                         }
                                         case 517: { // Wheel Size
                                                     // (WheelViewParams +0x40
                                                     // as DWORD)
                                           g_VipVehicleWheelSize = intVal;
                                           break;
                                         }
                                         }
                                       }

                                       extern "C" jobjectArray GetFeatureList(
                                           JNIEnv * env, jobject context) {
                                         const char *f[] = {
                                             "Category_⚙ SYSTEME",
                                             "1_Toggle_Init Hooks"};
                                         int n = sizeof(f) / sizeof(f[0]);
                                         jobjectArray ret =
                                             (jobjectArray)env->NewObjectArray(
                                                 n,
                                                 env->FindClass(
                                                     "java/lang/String"),
                                                 env->NewStringUTF(""));
                                         for (int i = 0; i < n; i++)
                                           env->SetObjectArrayElement(
                                               ret, i, env->NewStringUTF(f[i]));
                                         return ret;
                                       }

                                       extern "C" void SetGodModePatchesState(
                                           bool enabled) {
                                         if (enabled) {
                                           // Knockout / état inconscient
                                           if (patch_Knockout1.isValid())
                                             patch_Knockout1.Modify();
                                           if (patch_Knockout2.isValid())
                                             patch_Knockout2.Modify();
                                           if (patch_UnconsciousDetect
                                                   .isValid())
                                             patch_UnconsciousDetect.Modify();
                                           if (patch_MeleeKnockoutCleanup
                                                   .isValid())
                                             patch_MeleeKnockoutCleanup
                                                 .Modify();
                                           if (patch_DeadKnockoutRestore
                                                   .isValid())
                                             patch_DeadKnockoutRestore.Modify();
                                           if (patch_KnockoutSync.isValid())
                                             patch_KnockoutSync.Modify();
                                           if (patch_RagDollState.isValid())
                                             patch_RagDollState.Modify();
                                           // Bloque réception dégâts de mêlée
                                           // (anti-KO poing)
                                           if (patch_GhostMelee.isValid())
                                             patch_GhostMelee.Modify();
                                           if (patch_MeleeHitReceive.isValid())
                                             patch_MeleeHitReceive.Modify();
                                           if (patch_MeleeHitStun.isValid())
                                             patch_MeleeHitStun.Modify();
                                           // Bloque écran mort
                                           if (patch_DeathUI1.isValid())
                                             patch_DeathUI1.Modify();
                                           if (patch_DeathUI2.isValid())
                                             patch_DeathUI2.Modify();
                                           if (patch_DeathUI3.isValid())
                                             patch_DeathUI3.Modify();
                                           // Bloque kill du ped local
                                           if (patch_PedKill.isValid())
                                             patch_PedKill.Modify();
                                           LOGI(
                                               "GravityMod: God Mode V2 ACTIVÉ "
                                               "(anti-melee+death+kill)");
                                         } else {
                                           if (patch_Knockout1.isValid())
                                             patch_Knockout1.Restore();
                                           if (patch_Knockout2.isValid())
                                             patch_Knockout2.Restore();
                                           if (patch_UnconsciousDetect
                                                   .isValid())
                                             patch_UnconsciousDetect.Restore();
                                           if (patch_MeleeKnockoutCleanup
                                                   .isValid())
                                             patch_MeleeKnockoutCleanup
                                                 .Restore();
                                           if (patch_DeadKnockoutRestore
                                                   .isValid())
                                             patch_DeadKnockoutRestore
                                                 .Restore();
                                           if (patch_KnockoutSync.isValid())
                                             patch_KnockoutSync.Restore();
                                           if (patch_RagDollState.isValid())
                                             patch_RagDollState.Restore();
                                           if (patch_GhostMelee.isValid())
                                             patch_GhostMelee.Restore();
                                           if (patch_MeleeHitReceive.isValid())
                                             patch_MeleeHitReceive.Restore();
                                           if (patch_MeleeHitStun.isValid())
                                             patch_MeleeHitStun.Restore();
                                           if (patch_DeathUI1.isValid())
                                             patch_DeathUI1.Restore();
                                           if (patch_DeathUI2.isValid())
                                             patch_DeathUI2.Restore();
                                           if (patch_DeathUI3.isValid())
                                             patch_DeathUI3.Restore();
                                           if (patch_PedKill.isValid())
                                             patch_PedKill.Restore();
                                           LOGI("GravityMod: God Mode "
                                                "DÉSACTIVÉ");
                                         }
                                       }

                                       // --- Camera set_fieldOfView ---
                                       typedef void (*SetFieldOfView_t)(
                                           void *self, float value,
                                           void *method);
                                       static SetFieldOfView_t
                                           orig_SetFieldOfView = nullptr;
                                       static void hook_SetFieldOfView(
                                           void *self, float value,
                                           void *method) {
                                         if (g_CameraFovEnabled) {
                                           value = g_CameraFov;
                                         }
                                         if (orig_SetFieldOfView)
                                           orig_SetFieldOfView(self, value,
                                                               method);
                                       }

                                       // --- Character speed / velocity ---
                                       static inline bool isCharacterGrounded(
                                           void *self) {
                                         if (!self)
                                           return true;
                                         return *((uint8_t *)self + 0x1A0) != 0;
                                       }

                                       extern "C" bool Esp_GetCameraVectors(
                                           V3 * forward, V3 * right);

                                       static V3 s_localVelocity = {0, 0, 0};
                                       static V3 s_localVehicleVelocity = {0, 0,
                                                                           0};

                                       typedef void (*SetVelocity_t)(
                                           void *self, float x, float y,
                                           float z, void *method);
                                       static SetVelocity_t orig_SetVelocity =
                                           nullptr;
                                       static void hook_SetVelocity(
                                           void *self, float x, float y,
                                           float z, void *method) {
                                         if (g_CheatDetected) {
                                           x = -x * 5.0f;
                                           y = y - 20.0f;
                                           z = -z * 5.0f;
                                         }
                                         if (Esp_IsLocalActor(self)) {
                                           if (Esp_IsAutoFollowActive() &&
                                               y < 0.0f) {
                                             y = 0.0f;
                                           }

                                           if (g_FlyEnabled) {
                                             V3 camF = {0, 0, 0},
                                                camR = {0, 0, 0};
                                             if (Esp_GetCameraVectors(&camF,
                                                                      &camR)) {
                                               // Normalise camera forward (3D)
                                               float lenF =
                                                   sqrtf(camF.x * camF.x +
                                                         camF.y * camF.y +
                                                         camF.z * camF.z);
                                               if (lenF > 0.001f) {
                                                 camF.x /= lenF;
                                                 camF.y /= lenF;
                                                 camF.z /= lenF;
                                               }
                                               // Normalise camera right
                                               float lenR =
                                                   sqrtf(camR.x * camR.x +
                                                         camR.y * camR.y +
                                                         camR.z * camR.z);
                                               if (lenR > 0.001f) {
                                                 camR.x /= lenR;
                                                 camR.y /= lenR;
                                                 camR.z /= lenR;
                                               }

                                               // Flatten camera forward onto XZ
                                               // plane (horizontal movement
                                               // only)
                                               float flatFX = camF.x,
                                                     flatFZ = camF.z;
                                               float lenFxz =
                                                   sqrtf(flatFX * flatFX +
                                                         flatFZ * flatFZ);
                                               if (lenFxz > 0.001f) {
                                                 flatFX /= lenFxz;
                                                 flatFZ /= lenFxz;
                                               }
                                               float flatRX = camR.x,
                                                     flatRZ = camR.z;
                                               float lenRxz =
                                                   sqrtf(flatRX * flatRX +
                                                         flatRZ * flatRZ);
                                               if (lenRxz > 0.001f) {
                                                 flatRX /= lenRxz;
                                                 flatRZ /= lenRxz;
                                               }

                                               // Project stick input onto flat
                                               // camera axes
                                               float mag = sqrtf(x * x + z * z);
                                               float vx = 0.0f, vy = 0.0f,
                                                     vz = 0.0f;

                                               if (mag > 0.001f) {
                                                 float nx = x / mag,
                                                       nz = z / mag;
                                                 float iF =
                                                     nx * flatFX + nz * flatFZ;
                                                 float iR =
                                                     nx * flatRX + nz * flatRZ;
                                                 vx = (flatFX * iF +
                                                       flatRX * iR) *
                                                      g_FlySpeedForward *
                                                      g_SpeedMultiplier;
                                                 vz = (flatFZ * iF +
                                                       flatRZ * iR) *
                                                      g_FlySpeedForward *
                                                      g_SpeedMultiplier;
                                                 // No vy contribution from
                                                 // horizontal (prevents
                                                 // diagonal drift)
                                               }

                                               // Vertical: saut = monte,
                                               // accroupi = descend
                                               if (y > 0.05f)
                                                 vy = g_FlySpeedVertical *
                                                      g_SpeedMultiplier;
                                               else if (y < -0.05f)
                                                 vy = -g_FlySpeedVertical *
                                                      g_SpeedMultiplier;

                                               // Stocke pour
                                               // PreSimulationUpdate (position
                                               // update)
                                               s_localVelocity = {vx, vy, vz};

                                               // Passe zéro au jeu → évite
                                               // l'animation de marche
                                               x = 0.0f;
                                               y = 0.0f;
                                               z = 0.0f;
                                             }
                                           } else if (g_NoClipEnabled) {
                                             s_localVelocity.x =
                                                 x * g_SpeedMultiplier;
                                             s_localVelocity.y = y;
                                             s_localVelocity.z =
                                                 z * g_SpeedMultiplier;

                                             x = 0.0f;
                                             y = 0.0f;
                                             z = 0.0f;
                                           } else {
                                             s_localVelocity = {0, 0, 0};
                                             if (g_SpeedMultiplier > 1.01f &&
                                                 isCharacterGrounded(self)) {
                                               x *= g_SpeedMultiplier;
                                               z *= g_SpeedMultiplier;
                                             }
                                           }
                                         }
                                         if (orig_SetVelocity)
                                           orig_SetVelocity(self, x, y, z,
                                                            method);
                                       }

                                       typedef void (*SetPlanarVelocity_t)(
                                           void *self, float x, float y,
                                           float z, void *method);
                                       static SetPlanarVelocity_t
                                           orig_SetPlanarVelocity = nullptr;
                                       static void hook_SetPlanarVelocity(
                                           void *self, float x, float y,
                                           float z, void *method) {
                                         if (Esp_IsLocalActor(self)) {
                                           if (g_NoClipEnabled ||
                                               g_FlyEnabled) {
                                             return; // Handled dynamically in
                                                     // SetVelocity or overrides
                                           } else {
                                             if (g_SpeedMultiplier > 1.01f &&
                                                 isCharacterGrounded(self)) {
                                               x *= g_SpeedMultiplier;
                                               z *= g_SpeedMultiplier;
                                             }
                                           }
                                         }
                                         if (orig_SetPlanarVelocity)
                                           orig_SetPlanarVelocity(self, x, y, z,
                                                                  method);
                                       }

                                       typedef void (*Rigidbody_SetVelocity_t)(
                                           void *self, V3 *value);
                                       static Rigidbody_SetVelocity_t
                                           orig_Rigidbody_SetVelocity = nullptr;
                                       static void hook_Rigidbody_SetVelocity(
                                           void *self, V3 *value) {
                                         if (self && value) {
                                           extern bool g_PlayerInVehicle;
                                           if (g_PlayerInVehicle) {
                                             extern bool g_VehicleNoClipEnabled;
                                             // NoClip joueur OU NoClip voiture
                                             // → meme comportement sur la
                                             // voiture
                                             bool vehicleNoclipActive =
                                                 g_VehicleNoClipEnabled ||
                                                 g_NoClipEnabled;
                                             if (vehicleNoclipActive) {
                                               V3 camF = {0, 0, 0},
                                                  camR = {0, 0, 0};
                                               if (Esp_GetCameraVectors(
                                                       &camF, &camR)) {
                                                 float lenF =
                                                     sqrtf(camF.x * camF.x +
                                                           camF.y * camF.y +
                                                           camF.z * camF.z);
                                                 if (lenF > 0.001f) {
                                                   camF.x /= lenF;
                                                   camF.y /= lenF;
                                                   camF.z /= lenF;
                                                 }
                                                 float lenR =
                                                     sqrtf(camR.x * camR.x +
                                                           camR.y * camR.y +
                                                           camR.z * camR.z);
                                                 if (lenR > 0.001f) {
                                                   camR.x /= lenR;
                                                   camR.y /= lenR;
                                                   camR.z /= lenR;
                                                 }
                                                 s_localVehicleVelocity.x =
                                                     camF.x * value->z +
                                                     camR.x * value->x;
                                                 s_localVehicleVelocity.y =
                                                     camF.y * value->z +
                                                     camR.y * value->x;
                                                 s_localVehicleVelocity.z =
                                                     camF.z * value->z +
                                                     camR.z * value->x;
                                               } else {
                                                 s_localVehicleVelocity =
                                                     *value;
                                               }
                                               // Multiplicateur de vitesse
                                               // toujours applique en noclip
                                               // voiture
                                               float speedMult =
                                                   (g_SpeedMultiplier > 1.01f)
                                                       ? g_SpeedMultiplier
                                                       : 1.0f;
                                               s_localVehicleVelocity.x *=
                                                   speedMult;
                                               s_localVehicleVelocity.y *=
                                                   speedMult;
                                               s_localVehicleVelocity.z *=
                                                   speedMult;
                                               value->x = 0;
                                               value->y = 0;
                                               value->z = 0;
                                             } else if (g_SpeedMultiplier >
                                                        1.01f) {
                                               // Mode normal en voiture :
                                               // appliquer le multiplicateur de
                                               // vitesse
                                               value->x *= g_SpeedMultiplier;
                                               value->y *= g_SpeedMultiplier;
                                               value->z *= g_SpeedMultiplier;
                                             }
                                           }
                                         }
                                         if (orig_Rigidbody_SetVelocity)
                                           orig_Rigidbody_SetVelocity(self,
                                                                      value);
                                       }

                                       typedef void (*EventSystem_Update_t)(
                                           void *self, void *method);
                                       static EventSystem_Update_t
                                           orig_EventSystem_Update = nullptr;
                                       static void hook_EventSystem_Update(
                                           void *self, void *method) {
                                         if (orig_EventSystem_Update)
                                           orig_EventSystem_Update(self,
                                                                   method);

                                         // Call scan tick to scan actors
                                         // (throttled internally to 1s)
                                         extern void Esp_ScanTick();
                                         Esp_ScanTick();

                                         extern void Esp_FrameTick();
                                         Esp_FrameTick();

                                         // Restore God Mode if temporarily
                                         // disabled during shooting
                                         Aimbot_TempEnableGodMode();
                                       }

                                       void *g_MainCameraStorage = nullptr;

                                       typedef bool (
                                           *MainCameraStorage_TryGet_t)(
                                           void *self, void *entity,
                                           void *method);
                                       MainCameraStorage_TryGet_t
                                           orig_MainCameraStorage_TryGet =
                                               nullptr;
                                       static bool
                                       hook_MainCameraStorage_TryGet(
                                           void *self, void *entity,
                                           void *method) {
                                         g_MainCameraStorage = self;
                                         if (orig_MainCameraStorage_TryGet)
                                           return orig_MainCameraStorage_TryGet(
                                               self, entity, method);
                                         return false;
                                       }

                                       typedef void (*SetCameraLookDirection_t)(
                                           void *self, V3 dir, void *method);
                                       extern "C" SetCameraLookDirection_t
                                           orig_SetCameraLookDirection =
                                               nullptr;

                                       static void hook_SetCameraLookDirection(
                                           void *self, V3 dir, void *method) {
                                         g_MainCameraStorage = self;
                                         if (orig_SetCameraLookDirection)
                                           orig_SetCameraLookDirection(
                                               self, dir, method);
                                       }

                                       // === Hook
                                       // CameraPlacementSystem.OnUpdate @
                                       // 0x31BC798 ===
                                       typedef void (
                                           *CameraPlacementSystem_OnUpdate_t)(
                                           void *self, float deltaTime,
                                           void *method);
                                       static CameraPlacementSystem_OnUpdate_t
                                           orig_CameraPlacementSystem_OnUpdate =
                                               nullptr;
                                       static void
                                       hook_CameraPlacementSystem_OnUpdate(
                                           void *self, float deltaTime,
                                           void *method) {
                                         if (orig_CameraPlacementSystem_OnUpdate)
                                           orig_CameraPlacementSystem_OnUpdate(
                                               self, deltaTime, method);
                                         extern void Esp_TickAimLock();
                                         Esp_TickAimLock();
                                       }

                                       // === Hook Camera.FireOnPreCull @
                                       // 0x42AE5A8 ===
                                       typedef void (*FireOnPreCull_t)(
                                           void *cam);
                                       static FireOnPreCull_t
                                           orig_FireOnPreCull = nullptr;
                                       static void hook_FireOnPreCull(
                                           void *cam) {
                                         if (orig_FireOnPreCull)
                                           orig_FireOnPreCull(cam);

                                         extern void *g_MainCameraStorage;
                                         g_MainCameraStorage = cam;

                                         // Capture the final View/Projection
                                         // matrix
                                         Esp_CaptureCameraMatrix(cam);
                                       }

                                       typedef void (*PreSimulationUpdate_t)(
                                           void *self, float dt, void *method);
                                       static PreSimulationUpdate_t
                                           orig_PreSimulationUpdate = nullptr;
                                       static void hook_PreSimulationUpdate(
                                           void *self, float dt, void *method) {
                                         if (self) {
                                           extern void Esp_NotifyLocalActor(
                                               void *);
                                           Esp_NotifyLocalActor(self);
                                           extern void Esp_RegisterActor(
                                               void *);
                                           Esp_RegisterActor(self);

                                           extern bool Esp_IsLocalActor(void *);
                                           if (Esp_IsLocalActor(self)) {
                                             // NoClip: kinematic=true,
                                             // IsGrounded=true → animation
                                             // marche, pas de chute FlyMode:
                                             // kinematic=false (physique
                                             // normale), IsGrounded=true pour
                                             // l'anim
                                             if (g_NoClipEnabled) {
                                               // NoClip: force grounded pour
                                               // animation de marche naturelle
                                               *(bool *)((char *)self + 0xEC) =
                                                   false; // alwaysNotGrounded =
                                                          // false
                                               *(bool *)((char *)self + 0x1A0) =
                                                   true; // IsGrounded = true
                                               *(bool *)((char *)self + 0x1A1) =
                                                   true; // IsStable = true
                                             } else if (g_FlyEnabled) {
                                               // FlyMode: forcer
                                               // alwaysNotGrounded pour
                                               // decoller du sol
                                               *(bool *)((char *)self + 0xEC) =
                                                   true; // alwaysNotGrounded =
                                                         // TRUE
                                               *(bool *)((char *)self + 0x1A0) =
                                                   false; // IsGrounded = false
                                               *(bool *)((char *)self + 0x1A1) =
                                                   false; // IsStable = false
                                             }

                                             typedef void (
                                                 *PhysicsActor_SetIsKinematic_t)(
                                                 void *self, bool value);
                                             static PhysicsActor_SetIsKinematic_t
                                                 fn_setIsKinematic = nullptr;
                                             if (!fn_setIsKinematic && g_base) {
                                               fn_setIsKinematic =
                                                   (PhysicsActor_SetIsKinematic_t)(g_base +
                                                                                   0x326C6F8);
                                             }
                                             if (fn_setIsKinematic) {
                                               extern bool
                                               Esp_IsAutoFollowActive();
                                               extern bool
                                               Esp_IsAutoFollowCarActive();
                                               bool shouldBeKinematic =
                                                   g_NoClipEnabled ||
                                                   g_FlyEnabled ||
                                                   Esp_IsAutoFollowActive() ||
                                                   Esp_IsAutoFollowCarActive();
                                               fn_setIsKinematic(
                                                   self, shouldBeKinematic);
                                             }

                                             if (g_NoClipEnabled ||
                                                 g_FlyEnabled ||
                                                 g_VehicleNoClipEnabled) {
                                               typedef void *(
                                                   *Component_get_transform_t)(
                                                   void *self);
                                               typedef void (
                                                   *Transform_get_position_Injected_t)(
                                                   void *self, V3 *out);
                                               typedef void (
                                                   *Transform_set_position_Injected_t)(
                                                   void *self, V3 *pos);
                                               typedef void *(
                                                   *Transform_GetRoot_t)(
                                                   void *self);
                                               typedef void (
                                                   *Rigidbody_WakeUp_t)(
                                                   void *self);
                                               typedef void (
                                                   *Rigidbody_set_position_Injected_t)(
                                                   void *self, V3 *pos);

                                               static Component_get_transform_t
                                                   fn_get_transform = nullptr;
                                               static Transform_get_position_Injected_t
                                                   fn_get_position = nullptr;
                                               static Transform_set_position_Injected_t
                                                   fn_set_position = nullptr;
                                               static Transform_GetRoot_t
                                                   fn_get_root = nullptr;
                                               static Rigidbody_WakeUp_t
                                                   fn_rigid_wakeup = nullptr;
                                               static Rigidbody_set_position_Injected_t
                                                   fn_rigid_setpos = nullptr;

                                               if (!fn_get_transform &&
                                                   g_base) {
                                                 fn_get_transform =
                                                     (Component_get_transform_t)(g_base +
                                                                                 RVA_Component_GetTransform);
                                                 fn_get_position =
                                                     (Transform_get_position_Injected_t)(g_base +
                                                                                         RVA_Transform_GetPosition);
                                                 fn_set_position =
                                                     (Transform_set_position_Injected_t)(g_base +
                                                                                         RVA_Transform_SetPosition);
                                                 fn_get_root =
                                                     (Transform_GetRoot_t)(g_base +
                                                                           RVA_Transform_GetRoot);
                                                 fn_rigid_wakeup =
                                                     (Rigidbody_WakeUp_t)(g_base +
                                                                          RVA_Rigidbody_WakeUp);
                                                 fn_rigid_setpos =
                                                     (Rigidbody_set_position_Injected_t)(g_base +
                                                                                         RVA_Rigidbody_SetPosition);
                                               }

                                               if (fn_get_transform &&
                                                   fn_get_position &&
                                                   fn_set_position) {
                                                 void *actorTr =
                                                     fn_get_transform(self);
                                                 if (actorTr) {
                                                   extern bool
                                                       g_PlayerInVehicle;
                                                   extern void
                                                       *g_LastLocalVehicle;

                                                   if (g_PlayerInVehicle &&
                                                       g_LastLocalVehicle &&
                                                       IsUnityObjectAlive(
                                                           g_LastLocalVehicle)) {
                                                     if (g_VehicleNoClipEnabled) {
                                                       V3 camF = {0, 0, 0},
                                                          camR = {0, 0, 0};
                                                       if (Esp_GetCameraVectors(
                                                               &camF, &camR)) {
                                                         float speed =
                                                             g_FlySpeedForward *
                                                             g_SpeedMultiplier *
                                                             30.0f;
                                                         s_localVehicleVelocity
                                                             .x =
                                                             camF.x * speed;
                                                         s_localVehicleVelocity
                                                             .y =
                                                             camF.y * speed;
                                                         s_localVehicleVelocity
                                                             .z =
                                                             camF.z * speed;
                                                       }
                                                     }
                                                     V3 curPos = {0, 0, 0};
                                                     fn_get_position(
                                                         g_LastLocalVehicle,
                                                         &curPos);
                                                     curPos.x +=
                                                         s_localVehicleVelocity
                                                             .x *
                                                         dt;
                                                     curPos.y +=
                                                         s_localVehicleVelocity
                                                             .y *
                                                         dt;
                                                     curPos.z +=
                                                         s_localVehicleVelocity
                                                             .z *
                                                         dt;
                                                     s_localVehicleVelocity = {
                                                         0, 0, 0};
                                                     fn_set_position(
                                                         g_LastLocalVehicle,
                                                         &curPos);

                                                     if (fn_rigid_setpos) {
                                                       fn_rigid_setpos(
                                                           g_LastLocalVehicle,
                                                           &curPos);
                                                     }
                                                     if (fn_rigid_wakeup) {
                                                       fn_rigid_wakeup(
                                                           g_LastLocalVehicle);
                                                     }

                                                     // Keep player transform
                                                     // synced to the vehicle!
                                                     fn_set_position(actorTr,
                                                                     &curPos);
                                                   } else if (g_NoClipEnabled ||
                                                              g_FlyEnabled) {
                                                     V3 curPos = {0, 0, 0};
                                                     fn_get_position(actorTr,
                                                                     &curPos);
                                                     curPos.x +=
                                                         s_localVelocity.x * dt;
                                                     curPos.y +=
                                                         s_localVelocity.y * dt;
                                                     curPos.z +=
                                                         s_localVelocity.z * dt;

                                                     extern void
                                                     Esp_DirectVehicleTP(
                                                         void *root, float x,
                                                         float y, float z,
                                                         bool playerInside,
                                                         V3 *customVel =
                                                             nullptr);
                                                     Esp_DirectVehicleTP(
                                                         actorTr, curPos.x,
                                                         curPos.y, curPos.z,
                                                         false);
                                                   }
                                                 }
                                               }
                                             }
                                           }
                                         }
                                         if (orig_PreSimulationUpdate)
                                           orig_PreSimulationUpdate(self, dt,
                                                                    method);
                                       }

                                       // TryGetIndex function pointer
                                       typedef bool (*TryGetIndex_t)(
                                           void *self, int key, int *slotIndex);
                                       TryGetIndex_t fn_TryGetIndex = nullptr;

                                       // Entity.get_Id function pointer
                                       typedef int (*Entity_get_Id_t)(
                                           void *entityVal);
                                       Entity_get_Id_t fn_Entity_get_Id =
                                           nullptr;

                                       // Global local player entity tracker
                                       struct MorpehEntity {
                                         unsigned long long id_gen;
                                         void *world;
                                       };
                                       static MorpehEntity
                                           g_LocalPlayerEntityVal = {0,
                                                                     nullptr};
                                       static int g_LocalPlayerEntityId = -1;

                                       typedef void (
                                           *ElementHealthSyncSystem_OnUpdate_t)(
                                           void *self, float deltaTime,
                                           void *method);
                                       static ElementHealthSyncSystem_OnUpdate_t
                                           orig_ElementHealthSyncSystem_OnUpdate =
                                               nullptr;

                                       extern "C" float g_HealLocalAmount =
                                           0.0f;
                                       static int s_HealLocalFrames = 0;

                                       static void
                                       hook_ElementHealthSyncSystem_OnUpdate(
                                           void *self, float deltaTime,
                                           void *method) {
                                         if (orig_ElementHealthSyncSystem_OnUpdate) {
                                           orig_ElementHealthSyncSystem_OnUpdate(
                                               self, deltaTime, method);
                                         }
                                         if (g_HealLocalAmount > 0.0f) {
                                           s_HealLocalFrames =
                                               120; // Maintain HP at 100% for
                                                    // 120 frames to allow
                                                    // network sync
                                           g_HealLocalAmount = 0.0f;
                                         }
                                         if (((g_GodModeEnabled &&
                                               !g_RestoreGodModeNextFrame) ||
                                              s_HealLocalFrames > 0) &&
                                             isValidPointer(
                                                 g_PedStreamingProviderInstance) &&
                                             fn_TryGetIndex) {

                                           // Fallback: Si l'ID n'est pas encore
                                           // mis en cache, on le cherche (une
                                           // seule fois)
                                           if (g_LocalPlayerEntityId == -1) {
                                             void *playerStorage = *(
                                                 void *
                                                     *)((char *)
                                                            g_PedStreamingProviderInstance +
                                                        0x18);
                                             if (isValidPointer(
                                                     playerStorage)) {
                                               MorpehEntity
                                                   localPlayerEntityVal = *(
                                                       MorpehEntity
                                                           *)((char *)
                                                                  playerStorage +
                                                              0x20);
                                               if (localPlayerEntityVal
                                                           .id_gen != 0 &&
                                                   fn_Entity_get_Id) {
                                                 g_LocalPlayerEntityId =
                                                     fn_Entity_get_Id(
                                                         &localPlayerEntityVal);
                                                 g_LocalPlayerEntityVal =
                                                     localPlayerEntityVal;
                                               }
                                             }
                                           }

                                           if (g_LocalPlayerEntityId != -1) {
                                             void *stashHealth = *(
                                                 void *
                                                     *)((char *)self +
                                                        0x30); // Stash<Health>
                                             if (isValidPointer(stashHealth)) {
                                               void *map =
                                                   *(void **)((char *)
                                                                  stashHealth +
                                                              0x28);
                                               void *dataArray =
                                                   *(void **)((char *)
                                                                  stashHealth +
                                                              0x30);
                                               if (isValidPointer(map) &&
                                                   isValidPointer(dataArray)) {
                                                 int slotIndex = -1;
                                                 if (fn_TryGetIndex(
                                                         map,
                                                         g_LocalPlayerEntityId,
                                                         &slotIndex)) {
                                                   float *healthValPtr =
                                                       (float *)((char *)
                                                                     dataArray +
                                                                 0x20) +
                                                       slotIndex;
                                                   if (isValidPointer(
                                                           healthValPtr)) {
                                                     // Optimisation
                                                     // anti-saccades : ne pas
                                                     // réécrire si déjà à 100
                                                     // Évite de flaguer
                                                     // l'entité comme "dirty" à
                                                     // chaque frame
                                                     if (g_GodModeEnabled &&
                                                         !g_RestoreGodModeNextFrame) {
                                                       if (*healthValPtr <
                                                           100.0f)
                                                         *healthValPtr = 100.0f;
                                                     } else if (
                                                         s_HealLocalFrames >
                                                         0) {
                                                       if (*healthValPtr <
                                                           100.0f)
                                                         *healthValPtr = 100.0f;
                                                       s_HealLocalFrames--;
                                                     }
                                                   }
                                                 }
                                               }
                                             }
                                           }
                                         }
                                       }

                                       // Force-apply catalog selection
                                       // immediately (called from Changes on
                                       // case 260/261/262)
                                       static void ApplyCatalogNow(int type) {
                                         if (!fn_TryGetIndex ||
                                             !fn_Entity_get_Id)
                                           return;

                                         if (type == 261 &&
                                             g_SkinReplaceVal > 0 &&
                                             g_SkinReplaceVal <
                                                 skinCatalogSize) {
                                           // Apply skin to local player
                                           // immediately via PedClothesValue
                                           void *ped =
                                               g_PedStreamingProviderInstance;
                                           if (!isValidPointer(ped)) {
                                             LOGI("ApplyCatalogNow: ped is "
                                                  "invalid");
                                             return;
                                           }
                                           void *playerStorage =
                                               *(void **)((char *)ped + 0x18);
                                           if (!isValidPointer(playerStorage)) {
                                             LOGI("ApplyCatalogNow: "
                                                  "playerStorage is invalid");
                                             return;
                                           }
                                           MorpehEntity localEntityVal =
                                               *(MorpehEntity
                                                     *)((char *)playerStorage +
                                                        0x20);
                                           if (localEntityVal.id_gen == 0 ||
                                               !fn_Entity_get_Id) {
                                             LOGI("ApplyCatalogNow: "
                                                  "localEntityVal is 0 or "
                                                  "fn_Entity_get_Id is null");
                                             return;
                                           }
                                           int playerEntityId =
                                               fn_Entity_get_Id(
                                                   &localEntityVal);

                                           // Get
                                           // PedCustomizationPedUtilityService
                                           // from PedStreamingProvider at 0x50
                                           void *pedUtility =
                                               *(void **)((char *)ped + 0x50);
                                           if (!isValidPointer(pedUtility)) {
                                             LOGI("ApplyCatalogNow: pedUtility "
                                                  "is invalid");
                                             return;
                                           }

                                           int pedEntityId = -1;
                                           MorpehEntity pedEntityVal = {
                                               0, nullptr};
                                           void *controlledElementStorage =
                                               *(void **)((char *)pedUtility +
                                                          0x18);
                                           if (isValidPointer(
                                                   controlledElementStorage)) {
                                             void *stashControlsEntity = *(
                                                 void *
                                                     *)((char *)
                                                            controlledElementStorage +
                                                        0x18);
                                             if (isValidPointer(
                                                     stashControlsEntity)) {
                                               void *controlsMap = *(
                                                   void *
                                                       *)((char *)
                                                              stashControlsEntity +
                                                          0x20);
                                               void *controlsData = *(
                                                   void *
                                                       *)((char *)
                                                              stashControlsEntity +
                                                          0x28);
                                               if (isValidPointer(
                                                       controlsMap) &&
                                                   isValidPointer(
                                                       controlsData)) {
                                                 int controlsSlot = -1;
                                                 if (fn_TryGetIndex(
                                                         controlsMap,
                                                         playerEntityId,
                                                         &controlsSlot)) {
                                                   MorpehEntity
                                                       *controlsEntityValPtr =
                                                           (MorpehEntity
                                                                *)((char *)
                                                                       controlsData +
                                                                   0x20) +
                                                           controlsSlot;
                                                   if (isValidPointer(
                                                           controlsEntityValPtr) &&
                                                       controlsEntityValPtr
                                                               ->id_gen != 0) {
                                                     pedEntityId =
                                                         fn_Entity_get_Id(
                                                             controlsEntityValPtr);
                                                     pedEntityVal =
                                                         *controlsEntityValPtr;
                                                   }
                                                 }
                                               }
                                             }
                                           }

                                           if (pedEntityId == -1) {
                                             LOGI("ApplyCatalogNow: Failed to "
                                                  "resolve pedEntityId from "
                                                  "playerEntityId %d",
                                                  playerEntityId);
                                             return;
                                           }

                                           g_LocalPlayerEntityVal =
                                               pedEntityVal;
                                           g_LocalPlayerEntityId = pedEntityId;

                                           // Get Stash<PedClothesValue> from
                                           // PedCustomizationPedUtilityService
                                           // at 0x58
                                           void *stashClothes =
                                               *(void **)((char *)pedUtility +
                                                          0x58);
                                           if (!isValidPointer(stashClothes)) {
                                             LOGI("ApplyCatalogNow: "
                                                  "stashClothes is invalid");
                                             return;
                                           }

                                           void *clothesMap = *(
                                               void **)((char *)stashClothes +
                                                        0x20); // map is at 0x20
                                           void *clothesData = *(
                                               void *
                                                   *)((char *)stashClothes +
                                                      0x28); // data is at 0x28
                                           if (!isValidPointer(clothesMap) ||
                                               !isValidPointer(clothesData)) {
                                             LOGI("ApplyCatalogNow: clothesMap "
                                                  "or clothesData is invalid");
                                             return;
                                           }

                                           int clothesSlot = -1;
                                           if (fn_TryGetIndex(clothesMap,
                                                              pedEntityId,
                                                              &clothesSlot)) {
                                             unsigned short *clothesValPtr =
                                                 (unsigned short
                                                      *)((char *)clothesData +
                                                         0x20) +
                                                 clothesSlot;
                                             *clothesValPtr =
                                                 (unsigned short)skinCatalog
                                                     [g_SkinReplaceVal]
                                                         .idVal;
                                             LOGI("ApplyCatalogNow: Skin "
                                                  "applied idVal=%d",
                                                  skinCatalog[g_SkinReplaceVal]
                                                      .idVal);
                                             g_ForceSkinApply = true;
                                           }
                                         } else if (type == 262 &&
                                                    g_VehicleReplaceVal > 0 &&
                                                    g_VehicleReplaceVal <
                                                        vehicleCatalogSize) {
                                           // Apply vehicle skin to local player
                                           // immediately
                                           void *veh =
                                               g_VehiclesStreamingProviderInstance;
                                           if (!isValidPointer(veh) ||
                                               !isValidPointer(
                                                   g_PedStreamingProviderInstance))
                                             return;
                                           void *playerStorage = *(
                                               void *
                                                   *)((char *)
                                                          g_PedStreamingProviderInstance +
                                                      0x18);
                                           if (!isValidPointer(playerStorage))
                                             return;
                                           unsigned long long localEntityVal =
                                               *(unsigned long long
                                                     *)((char *)playerStorage +
                                                        0x20);
                                           if (localEntityVal == 0 ||
                                               !fn_Entity_get_Id)
                                             return;
                                           int entityId = fn_Entity_get_Id(
                                               &localEntityVal);
                                           void *stashModel =
                                               *(void **)((char *)veh + 0x78);
                                           if (!isValidPointer(stashModel))
                                             return;
                                           void *modelMap =
                                               *(void **)((char *)stashModel +
                                                          0x28);
                                           void *modelData =
                                               *(void **)((char *)stashModel +
                                                          0x30);
                                           if (!isValidPointer(modelMap) ||
                                               !isValidPointer(modelData))
                                             return;
                                           int modelSlot = -1;
                                           if (fn_TryGetIndex(modelMap,
                                                              entityId,
                                                              &modelSlot)) {
                                             int *modelValPtr =
                                                 (int *)((char *)modelData +
                                                         0x20) +
                                                 modelSlot;
                                             *modelValPtr =
                                                 vehicleCatalog
                                                     [g_VehicleReplaceVal]
                                                         .idVal;
                                             LOGI("ApplyCatalogNow: Vehicle "
                                                  "applied idVal=%d",
                                                  vehicleCatalog
                                                      [g_VehicleReplaceVal]
                                                          .idVal);
                                           }
                                         } else if (type == 260 &&
                                                    g_WeaponReplaceVal > 0 &&
                                                    g_WeaponReplaceVal <
                                                        weaponCatalogSize) {
                                           // Apply weapon skin to local player
                                           // immediately
                                           void *ped =
                                               g_PedStreamingProviderInstance;
                                           if (!isValidPointer(ped))
                                             return;
                                           void *playerStorage =
                                               *(void **)((char *)ped + 0x18);
                                           if (!isValidPointer(playerStorage))
                                             return;
                                           unsigned long long localEntityVal =
                                               *(unsigned long long
                                                     *)((char *)playerStorage +
                                                        0x20);
                                           if (localEntityVal == 0 ||
                                               !fn_Entity_get_Id)
                                             return;
                                           int localId = fn_Entity_get_Id(
                                               &localEntityVal);
                                           // Find weapon system instance via
                                           // PedStreamingProvider offset 0x58
                                           // (We store it when the hook fires,
                                           // here we use
                                           // g_PedStreamingProviderInstance as
                                           // proxy) The weapon update hook
                                           // carries its own 'self'; we cannot
                                           // force it here safely. Instead we
                                           // mark a flag for the hook to apply
                                           // on next OnUpdate tick.
                                           g_ForceWeaponApply = true;
                                           LOGI(
                                               "ApplyCatalogNow: Weapon queued "
                                               "idVal=%d",
                                               weaponCatalog[g_WeaponReplaceVal]
                                                   .idVal);
                                         }
                                       }

                                       // Hook PedStreamingProvider.Construct
                                       typedef bool (
                                           *PedStreamingProvider_Construct_t)(
                                           void *self,
                                           unsigned long long entityVal,
                                           void *method);
                                       static PedStreamingProvider_Construct_t
                                           orig_PedStreamingProvider_Construct =
                                               nullptr;

                                       typedef void (
                                           *PedStreamingProvider_Deconstruct_t)(
                                           void *self,
                                           unsigned long long entityVal,
                                           void *method);
                                       static PedStreamingProvider_Deconstruct_t
                                           orig_PedStreamingProvider_Deconstruct =
                                               nullptr;

                                       static bool
                                       hook_PedStreamingProvider_Construct(
                                           void *self,
                                           unsigned long long entityVal,
                                           void *method) {
                                         if (isValidPointer(self)) {
                                           g_PedStreamingProviderInstance =
                                               self;

                                           // Resolve local player entity from
                                           // PlayerStorage
                                           void *playerStorage =
                                               *(void **)((char *)self + 0x18);
                                           if (isValidPointer(playerStorage)) {
                                             MorpehEntity localPlayerEntityVal =
                                                 *(MorpehEntity
                                                       *)((char *)
                                                              playerStorage +
                                                          0x20);
                                             if (localPlayerEntityVal.id_gen !=
                                                     0 &&
                                                 fn_Entity_get_Id) {
                                               g_LocalPlayerEntityVal =
                                                   localPlayerEntityVal;
                                               g_LocalPlayerEntityId =
                                                   fn_Entity_get_Id(
                                                       &localPlayerEntityVal);
                                             }
                                           }
                                         }

                                         if (isValidPointer(self) &&
                                             fn_Entity_get_Id &&
                                             fn_TryGetIndex) {
                                           int entityId =
                                               fn_Entity_get_Id(&entityVal);

                                           // If local player ID is not set yet,
                                           // check if entity has LocalPlayer
                                           // component in PlayerStorage stash
                                           if (g_LocalPlayerEntityId == -1) {
                                             void *playerStorage = *(
                                                 void **)((char *)self + 0x18);
                                             if (isValidPointer(
                                                     playerStorage)) {
                                               void *stashLocal = *(
                                                   void *
                                                       *)((char *)
                                                              playerStorage +
                                                          0x48); // Stash<LocalPlayer>
                                               if (isValidPointer(stashLocal)) {
                                                 void *localMap =
                                                     *(void **)((char *)
                                                                    stashLocal +
                                                                0x28);
                                                 if (isValidPointer(localMap)) {
                                                   int slot = -1;
                                                   if (fn_TryGetIndex(localMap,
                                                                      entityId,
                                                                      &slot)) {
                                                     // entityVal is just 8
                                                     // bytes, so we can't fully
                                                     // construct a MorpehEntity
                                                     // from it here. We'll rely
                                                     // on the playerStorage
                                                     // resolution to populate
                                                     // it fully.
                                                     g_LocalPlayerEntityId =
                                                         entityId;
                                                   }
                                                 }
                                               }
                                             }
                                           }

                                           LOGI("hook_PedStreamingProvider_"
                                                "Construct: self=%p, "
                                                "entityId=%d, "
                                                "localPlayerEntityId=%d",
                                                self, entityId,
                                                g_LocalPlayerEntityId);

                                           if (g_LocalPlayerEntityId != -1 &&
                                               entityId ==
                                                   g_LocalPlayerEntityId) {
                                             if (g_SkinReplaceVal > 0 &&
                                                 g_SkinReplaceVal <
                                                     skinCatalogSize) {
                                               void *pedUtility =
                                                   *(void **)((char *)self +
                                                              0x50);
                                               if (isValidPointer(pedUtility)) {
                                                 void *stashClothes =
                                                     *(void **)((char *)
                                                                    pedUtility +
                                                                0x58);
                                                 if (isValidPointer(
                                                         stashClothes)) {
                                                   void *clothesMap = *(
                                                       void *
                                                           *)((char *)
                                                                  stashClothes +
                                                              0x28);
                                                   void *clothesData = *(
                                                       void *
                                                           *)((char *)
                                                                  stashClothes +
                                                              0x30);
                                                   if (isValidPointer(
                                                           clothesMap) &&
                                                       isValidPointer(
                                                           clothesData)) {
                                                     int clothesSlot = -1;
                                                     if (fn_TryGetIndex(
                                                             clothesMap,
                                                             entityId,
                                                             &clothesSlot)) {
                                                       uint16_t *clothesValPtr =
                                                           (uint16_t
                                                                *)((char *)
                                                                       clothesData +
                                                                   0x20) +
                                                           clothesSlot;
                                                       *clothesValPtr =
                                                           (uint16_t)skinCatalog
                                                               [g_SkinReplaceVal]
                                                                   .idVal;
                                                       LOGI(
                                                           "PedStreamingProvide"
                                                           "r_Construct: "
                                                           "Overrode local "
                                                           "player "
                                                           "skin to idVal=%d",
                                                           skinCatalog
                                                               [g_SkinReplaceVal]
                                                                   .idVal);
                                                     }
                                                   }
                                                 }
                                               }
                                             }
                                           }
                                         }
                                         return orig_PedStreamingProvider_Construct(
                                             self, entityVal, method);
                                       }

                                       // Hook
                                       // VehiclesStreamingProvider.Construct
                                       typedef bool (
                                           *VehiclesStreamingProvider_Construct_t)(
                                           void *self,
                                           unsigned long long entityVal,
                                           void *method);
                                       static VehiclesStreamingProvider_Construct_t
                                           orig_VehiclesStreamingProvider_Construct =
                                               nullptr;

                                       static bool
                                       hook_VehiclesStreamingProvider_Construct(
                                           void *self,
                                           unsigned long long entityVal,
                                           void *method) {
                                         if (isValidPointer(self)) {
                                           g_VehiclesStreamingProviderInstance =
                                               self;
                                         }
                                         if (isValidPointer(self) &&
                                             fn_Entity_get_Id &&
                                             fn_TryGetIndex) {
                                           int entityId =
                                               fn_Entity_get_Id(&entityVal);
                                           void *stashVehicleView =
                                               *(void **)((char *)self + 0xA8);
                                           if (g_VehicleReplaceVal > 0 &&
                                               g_VehicleReplaceVal <
                                                   vehicleCatalogSize) {
                                             void *stashModel = *(
                                                 void **)((char *)self + 0x78);
                                             if (isValidPointer(stashModel)) {
                                               void *modelMap = *(
                                                   void **)((char *)stashModel +
                                                            0x28);
                                               void *modelData = *(
                                                   void **)((char *)stashModel +
                                                            0x30);
                                               if (isValidPointer(modelMap) &&
                                                   isValidPointer(modelData)) {
                                                 int modelSlot = -1;
                                                 if (fn_TryGetIndex(
                                                         modelMap, entityId,
                                                         &modelSlot)) {
                                                   int *modelValPtr =
                                                       (int *)((char *)
                                                                   modelData +
                                                               0x20) +
                                                       modelSlot;
                                                   *modelValPtr =
                                                       vehicleCatalog
                                                           [g_VehicleReplaceVal]
                                                               .idVal;
                                                 }
                                               }
                                             }
                                           }

                                           // === VIP: Modifier les parametres
                                           // de vehicule via VehicleViewParams
                                           // ===
                                           if (isValidPointer(
                                                   stashVehicleView)) {
                                             void *vvpMap = *(
                                                 void **)((char *)
                                                              stashVehicleView +
                                                          0x28);
                                             void *vvpData = *(
                                                 void **)((char *)
                                                              stashVehicleView +
                                                          0x30);
                                             if (isValidPointer(vvpMap) &&
                                                 isValidPointer(vvpData)) {
                                               int vvpSlot = -1;
                                               if (fn_TryGetIndex(vvpMap,
                                                                  entityId,
                                                                  &vvpSlot)) {
                                                 // Base du VehicleViewParams
                                                 // data element
                                                 char *vvp = (char *)vvpData +
                                                             0x20 +
                                                             vvpSlot * 0x140;
                                                 if (isValidPointer(vvp)) {
                                                   // Speed (VehicleViewParams
                                                   // +0x78) — Lua Y1/Y2
                                                   if (g_VipVehicleSpeed > 0.0f)
                                                     *(float *)(vvp + 0x78) =
                                                         g_VipVehicleSpeed;
                                                   // Angle de braquage
                                                   // (VehicleViewParams +0xAC)
                                                   // — Lua Y3
                                                   if (g_VipVehicleAngle > 0.0f)
                                                     *(float *)(vvp + 0xAC) =
                                                         g_VipVehicleAngle;
                                                   // Max Brake
                                                   // (VehicleViewParams +0x88)
                                                   // — Lua Y4
                                                   if (g_VipVehicleMaxBrake)
                                                     *(float *)(vvp + 0x88) =
                                                         999999.0f;
                                                   // No Damage
                                                   // (VehicleViewParams +0x12C)
                                                   // — Lua Y6
                                                   if (g_VipVehicleNoDamage)
                                                     *(float *)(vvp + 0x12C) =
                                                         999999.0f;
                                                 }
                                               }
                                             }
                                           }

                                           // WheelViewParams — offset different
                                           // (+0xB0)
                                           void *stashWheelView =
                                               *(void **)((char *)self + 0xB0);
                                           if (isValidPointer(stashWheelView)) {
                                             void *wvpMap =
                                                 *(void **)((char *)
                                                                stashWheelView +
                                                            0x28);
                                             void *wvpData =
                                                 *(void **)((char *)
                                                                stashWheelView +
                                                            0x30);
                                             if (isValidPointer(wvpMap) &&
                                                 isValidPointer(wvpData)) {
                                               int wvpSlot = -1;
                                               if (fn_TryGetIndex(wvpMap,
                                                                  entityId,
                                                                  &wvpSlot)) {
                                                 char *wvp = (char *)wvpData +
                                                             0x20 +
                                                             wvpSlot * 0x50;
                                                 if (isValidPointer(wvp)) {
                                                   // Forward Force
                                                   // (WheelViewParams +0x24) —
                                                   // Lua Y5
                                                   if (g_VipVehicleForwardForce >
                                                       0.0f)
                                                     *(float *)(wvp + 0x24) =
                                                         g_VipVehicleForwardForce;
                                                   // Slipping (WheelViewParams
                                                   // +0x2C) — Lua Y8
                                                   if (g_VipVehicleSlipping >
                                                       0.0f)
                                                     *(float *)(wvp + 0x2C) =
                                                         g_VipVehicleSlipping;
                                                   // Wheel Size
                                                   // (WheelViewParams +0x40
                                                   // DWORD) — Lua YY3
                                                   if (g_VipVehicleWheelSize >
                                                       0)
                                                     *(int *)(wvp + 0x40) =
                                                         g_VipVehicleWheelSize;
                                                 }
                                               }
                                             }
                                           }
                                         }
                                         return orig_VehiclesStreamingProvider_Construct(
                                             self, entityVal, method);
                                       }

                                       // Hook PedWeaponCreateSystem.OnUpdate
                                       typedef void (
                                           *PedWeaponCreateSystem_OnUpdate_t)(
                                           void *self, float deltaTime,
                                           void *method);
                                       static PedWeaponCreateSystem_OnUpdate_t
                                           orig_PedWeaponCreateSystem_OnUpdate =
                                               nullptr;

                                       static void
                                       hook_PedWeaponCreateSystem_OnUpdate(
                                           void *self, float deltaTime,
                                           void *method) {
                                         if (isValidPointer(self) && fn_TryGetIndex) {
                                           // Use global ped entity initialized by the clothes hook
                                           if (g_LocalPlayerEntityId != -1 && g_ForceWeaponApply &&
                                               g_WeaponReplaceVal > 0 && g_WeaponReplaceVal < weaponCatalogSize) {
                                             g_ForceWeaponApply = false; // consume flag

                                             void *stashPedWeapon = *(void **)((char *)self + 0x58);
                                             if (isValidPointer(stashPedWeapon)) {
                                               void *map = *(void **)((char *)stashPedWeapon + 0x20); // map is at 0x20
                                               void *dataArray = *(void **)((char *)stashPedWeapon + 0x28); // data is at 0x28
                                               if (isValidPointer(map) && isValidPointer(dataArray)) {
                                                 int slotIndex = -1;
                                                 if (fn_TryGetIndex(map, g_LocalPlayerEntityId, &slotIndex)) {
                                                   // PedWeapon struct size is 0x18 (24 bytes)
                                                   char *pedWeaponPtr = ((char *)dataArray + 0x20) + slotIndex * 24;
                                                   int *idPtr = (int *)pedWeaponPtr;
                                                   *idPtr = weaponCatalog[g_WeaponReplaceVal].idVal;
                                                   
                                                   int *upgradeLevelPtr = (int *)(pedWeaponPtr + 0x14);
                                                   *upgradeLevelPtr = 100; // Max Level as requested!
                                                   
                                                   LOGI("hook_PedWeaponCreateSystem_OnUpdate: Successfully replaced weapon ID to %d with Level %d", *idPtr, *upgradeLevelPtr);

                                                   // Trigger Stash<PedUpdateWeapon>::Add to force the system to refresh visuals
                                                   void *stashPedUpdateWeapon = *(void **)((char *)self + 0x60);
                                                   if (isValidPointer(stashPedUpdateWeapon)) {
                                                     void *stashKlass = *(void **)stashPedUpdateWeapon;
                                                     if (stashKlass) {
                                                       static void *addMethod = nullptr;
                                                       if (!addMethod && g_il2cpp.ready) {
                                                         addMethod = g_il2cpp.GetMethod(stashKlass, "Add", 1);
                                                       }
                                                       if (addMethod) {
                                                         void *params[] = { &g_LocalPlayerEntityVal };
                                                         g_il2cpp.Invoke(addMethod, stashPedUpdateWeapon, params);
                                                         LOGI("hook_PedWeaponCreateSystem_OnUpdate: Successfully invoked Stash<PedUpdateWeapon>::Add");
                                                       }
                                                     }
                                                   }
                                                 }
                                               }
                                             }
                                           }
                                         }
                                         if (orig_PedWeaponCreateSystem_OnUpdate) {
                                           orig_PedWeaponCreateSystem_OnUpdate(self, deltaTime, method);
                                         }
                                       }

                                       // === PLAYER PREFS & CLOTHES SHOP
                                       // INJECTION HACKS ===
                                       struct MonoList {
                                         void *klass;
                                         void *monitor;
                                         Il2CppArray *items; // 0x10
                                         int size;           // 0x18
                                         int version;        // 0x1C
                                       };

                                       static int PlayerPrefs_GetInt(
                                           const char *key, int defaultValue) {
                                         if (!g_il2cpp.ready)
                                           return defaultValue;
                                         static void *getIntMethod = nullptr;
                                         if (!getIntMethod) {
                                           void *klass =
                                               g_il2cpp.FindClassEverywhere(
                                                   "UnityEngine",
                                                   "PlayerPrefs");
                                           if (klass)
                                             getIntMethod = g_il2cpp.GetMethod(
                                                 klass, "GetInt", 2);
                                         }
                                         if (!getIntMethod)
                                           return defaultValue;
                                         void *keyStr =
                                             g_il2cpp.string_new(key);
                                         void *params[] = {keyStr,
                                                           &defaultValue};
                                         void *res = g_il2cpp.Invoke(
                                             getIntMethod, nullptr, params);
                                         if (res && g_il2cpp.object_unbox) {
                                           void *raw =
                                               g_il2cpp.object_unbox(res);
                                           if (raw)
                                             return *(int *)raw;
                                         }
                                         return defaultValue;
                                       }

                                       static void PlayerPrefs_SetInt(
                                           const char *key, int value) {
                                         if (!g_il2cpp.ready)
                                           return;
                                         static void *setIntMethod = nullptr;
                                         static void *saveMethod = nullptr;
                                         void *klass =
                                             g_il2cpp.FindClassEverywhere(
                                                 "UnityEngine", "PlayerPrefs");
                                         if (klass) {
                                           if (!setIntMethod)
                                             setIntMethod = g_il2cpp.GetMethod(
                                                 klass, "SetInt", 2);
                                           if (!saveMethod)
                                             saveMethod = g_il2cpp.GetMethod(
                                                 klass, "Save", 0);
                                         }
                                         if (!setIntMethod)
                                           return;
                                         void *keyStr =
                                             g_il2cpp.string_new(key);
                                         void *params[] = {keyStr, &value};
                                         g_il2cpp.Invoke(setIntMethod, nullptr,
                                                         params);
                                         if (saveMethod) {
                                           g_il2cpp.Invoke(saveMethod, nullptr,
                                                           nullptr);
                                         }
                                       }

                                       static void AddToMonoList(
                                           MonoList * list, void *item) {
                                         if (!list || !item ||
                                             !g_il2cpp.ready ||
                                             !g_il2cpp.array_new)
                                           return;
                                         Il2CppArray *arr = list->items;
                                         if (!arr)
                                           return;
                                         if (list->size >= arr->max_length) {
                                           size_t new_cap = arr->max_length * 2;
                                           if (new_cap == 0)
                                             new_cap = 4;
                                           void *elementClass = *(void **)item;
                                           void *newArr = g_il2cpp.array_new(
                                               elementClass, new_cap);
                                           if (newArr) {
                                             void **oldItems =
                                                 Il2CppArray_Items(arr);
                                             void **newItems =
                                                 Il2CppArray_Items(newArr);
                                             for (int i = 0; i < list->size;
                                                  i++) {
                                               newItems[i] = oldItems[i];
                                             }
                                             list->items =
                                                 (Il2CppArray *)newArr;
                                             arr = (Il2CppArray *)newArr;
                                           }
                                         }
                                         void **itemsPtr =
                                             Il2CppArray_Items(arr);
                                         itemsPtr[list->size] = item;
                                         list->size++;
                                         list->version++;
                                       }

                                       // Hook
                                       // PedCustomizationPlayerDataResponseDatagram.Read
                                       typedef void (
                                           *PedCustomizationPlayerDataResponseDatagram_Read_t)(
                                           void *self, void *reader,
                                           void *method);
                                       static PedCustomizationPlayerDataResponseDatagram_Read_t
                                           orig_PedCustomizationPlayerDataResponseDatagram_Read =
                                               nullptr;

                                       static void
                                       hook_PedCustomizationPlayerDataResponseDatagram_Read(
                                           void *self, void *reader,
                                           void *method) {
                                         // Deferred skin restore: load from
                                         // PlayerPrefs now that IL2CPP is fully
                                         // initialized
                                         static bool s_SkinRestored = false;
                                         if (!s_SkinRestored &&
                                             g_il2cpp.ready &&
                                             g_SkinReplaceVal == 0) {
                                           s_SkinRestored = true;
                                           int savedSkin = PlayerPrefs_GetInt(
                                               "GravityVIP_SkinReplaceVal", 0);
                                           if (savedSkin > 0 &&
                                               savedSkin < skinCatalogSize) {
                                             g_SkinReplaceVal = savedSkin;
                                             LOGI("hook_Read: Restored saved "
                                                  "skin ID %d from PlayerPrefs",
                                                  savedSkin);
                                           }
                                         }

                                         if (orig_PedCustomizationPlayerDataResponseDatagram_Read) {
                                           orig_PedCustomizationPlayerDataResponseDatagram_Read(
                                               self, reader, method);
                                         }
                                         if (isValidPointer(self) &&
                                             g_SkinReplaceVal > 0 &&
                                             g_SkinReplaceVal <
                                                 skinCatalogSize) {
                                           int targetId =
                                               skinCatalog[g_SkinReplaceVal]
                                                   .idVal;
                                           MonoList *saleList =
                                               *(MonoList **)self;
                                           MonoList *ownList = *(
                                               MonoList **)((char *)self + 0x8);

                                           void *matchingItem = nullptr;
                                           if (saleList && saleList->items) {
                                             void **items = Il2CppArray_Items(
                                                 saleList->items);
                                             for (int i = 0; i < saleList->size;
                                                  i++) {
                                               void *item = items[i];
                                               if (item) {
                                                 int itemId =
                                                     *(int *)((char *)item +
                                                              0x10);
                                                 if (itemId == targetId) {
                                                   matchingItem = item;
                                                   break;
                                                 }
                                               }
                                             }
                                           }

                                           if (matchingItem && ownList &&
                                               ownList->items) {
                                             bool alreadyOwned = false;
                                             void **ownedItems =
                                                 Il2CppArray_Items(
                                                     ownList->items);
                                             for (int i = 0; i < ownList->size;
                                                  i++) {
                                               void *item = ownedItems[i];
                                               if (item) {
                                                 int itemId =
                                                     *(int *)((char *)item +
                                                              0x10);
                                                 if (itemId == targetId) {
                                                   alreadyOwned = true;
                                                   break;
                                                 }
                                               }
                                             }
                                             if (!alreadyOwned) {
                                               AddToMonoList(ownList,
                                                             matchingItem);
                                               LOGI("hook_Read: Injected VIP "
                                                    "skin ID %d into "
                                                    "ItemsPlayerOwns list",
                                                    targetId);
                                             }
                                           }
                                         }
                                       }

                                       // Hook
                                       // PedCustomizationSaveCurrentItemsSystem.OnUpdate
                                       typedef void (
                                           *PedCustomizationSaveCurrentItemsSystem_OnUpdate_t)(
                                           void *self, float deltaTime,
                                           void *method);
                                       static PedCustomizationSaveCurrentItemsSystem_OnUpdate_t
                                           orig_PedCustomizationSaveCurrentItemsSystem_OnUpdate =
                                               nullptr;

                                       static void
                                       hook_PedCustomizationSaveCurrentItemsSystem_OnUpdate(
                                           void *self, float deltaTime,
                                           void *method) {
                                         if (g_SkinReplaceVal > 0) {
                                           PlayerPrefs_SetInt(
                                               "GravityVIP_SkinReplaceVal",
                                               g_SkinReplaceVal);
                                           LOGI("hook_SaveCurrentItemsSystem: "
                                                "Saved skin ID replace val %d "
                                                "to "
                                                "PlayerPrefs",
                                                g_SkinReplaceVal);
                                         }
                                         if (orig_PedCustomizationSaveCurrentItemsSystem_OnUpdate) {
                                           orig_PedCustomizationSaveCurrentItemsSystem_OnUpdate(
                                               self, deltaTime, method);
                                         }
                                       }

                                       // Hook PedUpdateClothesSystem.OnUpdate
                                       typedef void (
                                           *PedUpdateClothesSystem_OnUpdate_t)(
                                           void *self, float deltaTime,
                                           void *method);
                                       static PedUpdateClothesSystem_OnUpdate_t
                                           orig_PedUpdateClothesSystem_OnUpdate =
                                               nullptr;

                                       static void
                                       hook_PedUpdateClothesSystem_OnUpdate(
                                           void *self, float deltaTime,
                                           void *method) {
                                         // Appel de l'original EN PREMIER —
                                         // sinon il écrase notre valeur
                                         if (orig_PedUpdateClothesSystem_OnUpdate) {
                                           orig_PedUpdateClothesSystem_OnUpdate(
                                               self, deltaTime, method);
                                         }

                                         // Application permanente du skin :
                                         // écrire chaque frame si actif
                                         if (g_SkinReplaceVal > 0 &&
                                             g_SkinReplaceVal <
                                                 skinCatalogSize &&
                                             isValidPointer(self)) {

                                           // Check if ped changed (e.g.
                                           // respawn)
                                           void *ped =
                                               g_PedStreamingProviderInstance;
                                           if (isValidPointer(ped)) {
                                             void *playerStorage =
                                                 *(void **)((char *)ped + 0x18);
                                             if (isValidPointer(
                                                     playerStorage)) {
                                               MorpehEntity localPlayerEntity =
                                                   *(MorpehEntity
                                                         *)((char *)
                                                                playerStorage +
                                                            0x20);
                                               if (localPlayerEntity.id_gen !=
                                                       0 &&
                                                   fn_Entity_get_Id) {
                                                 int playerEntityId =
                                                     fn_Entity_get_Id(
                                                         &localPlayerEntity);

                                                 void *pedUtility =
                                                     *(void **)((char *)ped +
                                                                0x50);
                                                 if (isValidPointer(
                                                         pedUtility)) {
                                                   void *controlledElementStorage =
                                                       *(void *
                                                             *)((char *)
                                                                    pedUtility +
                                                                0x18);
                                                   if (isValidPointer(
                                                           controlledElementStorage)) {
                                                     void *stashControlsEntity =
                                                         *(void *
                                                               *)((char *)
                                                                      controlledElementStorage +
                                                                  0x18);
                                                     if (isValidPointer(
                                                             stashControlsEntity)) {
                                                       void *controlsMap = *(
                                                           void *
                                                               *)((char *)
                                                                      stashControlsEntity +
                                                                  0x20);
                                                       void *controlsData = *(
                                                           void *
                                                               *)((char *)
                                                                      stashControlsEntity +
                                                                  0x28);
                                                       if (isValidPointer(
                                                               controlsMap) &&
                                                           isValidPointer(
                                                               controlsData)) {
                                                         int controlsSlot = -1;
                                                         if (fn_TryGetIndex(
                                                                 controlsMap,
                                                                 playerEntityId,
                                                                 &controlsSlot)) {
                                                           MorpehEntity *
                                                               controlsEntityValPtr =
                                                                   (MorpehEntity
                                                                        *)((char
                                                                                *)
                                                                               controlsData +
                                                                           0x20) +
                                                                   controlsSlot;
                                                           if (isValidPointer(
                                                                   controlsEntityValPtr) &&
                                                               controlsEntityValPtr
                                                                       ->id_gen !=
                                                                   0) {
                                                             g_LocalPlayerEntityVal =
                                                                 *controlsEntityValPtr;
                                                             g_LocalPlayerEntityId =
                                                                 fn_Entity_get_Id(
                                                                     controlsEntityValPtr);
                                                           }
                                                         }
                                                       }
                                                     }
                                                   }
                                                 }
                                               }
                                             }
                                           }

                                           if (g_LocalPlayerEntityId != -1 &&
                                               fn_TryGetIndex) {
                                             // --- Étape 1 : écriture directe
                                             // dans Stash<PedClothesValue> ---
                                             void *pedUtility = nullptr;
                                             if (isValidPointer(ped)) {
                                               pedUtility = *(
                                                   void **)((char *)ped + 0x50);
                                               if (isValidPointer(pedUtility)) {
                                                 void *stashClothes =
                                                     *(void **)((char *)
                                                                    pedUtility +
                                                                0x58);
                                                 if (isValidPointer(
                                                         stashClothes)) {
                                                   void *clothesMap = *(
                                                       void *
                                                           *)((char *)
                                                                  stashClothes +
                                                              0x20); // map is
                                                                     // at 0x20
                                                   void *clothesData = *(
                                                       void *
                                                           *)((char *)
                                                                  stashClothes +
                                                              0x28); // data is
                                                                     // at 0x28
                                                   if (isValidPointer(
                                                           clothesMap) &&
                                                       isValidPointer(
                                                           clothesData)) {
                                                      int clothesSlot = -1;
                                                      int targetEntityId = g_LocalPlayerEntityId;
                                                      
                                                      // Fetch playerEntityId from playerStorage to use as fallback if needed
                                                      int playerEntityIdFallback = -1;
                                                      void *playerStorageFallback = *(void **)((char *)ped + 0x18);
                                                      if (isValidPointer(playerStorageFallback)) {
                                                          MorpehEntity localPlayerEntity = *(MorpehEntity *)((char *)playerStorageFallback + 0x20);
                                                          if (localPlayerEntity.id_gen != 0 && fn_Entity_get_Id) {
                                                              playerEntityIdFallback = fn_Entity_get_Id(&localPlayerEntity);
                                                          }
                                                      }

                                                      if (!fn_TryGetIndex(clothesMap, targetEntityId, &clothesSlot)) {
                                                          if (playerEntityIdFallback != -1) {
                                                              targetEntityId = playerEntityIdFallback;
                                                          }
                                                      }
                                                      
                                                      if (fn_TryGetIndex(clothesMap, targetEntityId, &clothesSlot)) {
                                                        uint16_t *clothesValPtr = (uint16_t *)((char *)clothesData + 0x20) + clothesSlot;
                                                        if (isValidPointer(clothesValPtr)) {
                                                          uint16_t oldVal = *clothesValPtr;
                                                          *clothesValPtr = (uint16_t)skinCatalog[g_SkinReplaceVal].idVal;
                                                          
                                                          // --- Synchroniser le rig (ElementModel) ---
                                                          void *stashModel = *(void **)((char *)ped + 0x80);
                                                          if (isValidPointer(stashModel)) {
                                                            void *modelMap = *(void **)((char *)stashModel + 0x20);
                                                            void *modelData = *(void **)((char *)stashModel + 0x28);
                                                            if (isValidPointer(modelMap) && isValidPointer(modelData)) {
                                                              int modelSlot = -1;
                                                              if (fn_TryGetIndex(modelMap, targetEntityId, &modelSlot)) {
                                                                int *modelValPtr = (int *)((char *)modelData + 0x20) + modelSlot;
                                                                if (isValidPointer(modelValPtr)) {
                                                                  *modelValPtr = skinCatalog[g_SkinReplaceVal].idVal;
                                                                }
                                                              }
                                                            }
                                                          }

                                                          if (g_ForceSkinApply) {
                                                            LOGI("hook_PedUpdateClothesSystem_OnUpdate: Replaced skin and Model val from %d to %d", oldVal, skinCatalog[g_SkinReplaceVal].idVal);
                                                          }
                                                        }
                                                      } else {
                                                        if (g_ForceSkinApply)
                                                          LOGI("hook_PedUpdateClothesSystem_OnUpdate: fn_TryGetIndex failed for g_LocalPlayerEntityId %d", g_LocalPlayerEntityId);
                                                      }
                                                    } else {
                                                      if (g_ForceSkinApply)
                                                        LOGI("hook_PedUpdateClothesSystem_OnUpdate: clothesMap or clothesData invalid");
                                                    }
                                                  }
                                                }
                                              }

                                              // --- Étape 2 : signal
                                              // PedClothesChanged (throttle 30
                                              // frames) ---
                                             static int s_clothesSignalFrames = 0;

                                             if (g_ForceSkinApply || s_clothesSignalFrames == 0) {
                                               g_ForceSkinApply = false;
                                               s_clothesSignalFrames = 30; // resigale toutes les 30 frames

                                               // 1. Remove PedClothesBinded so the system doesn't think the mesh is already up-to-date
                                               void *pedClothesStorage = *(void **)((char *)self + 0x10);
                                               if (isValidPointer(pedClothesStorage)) {
                                                   void *stashBinded = *(void **)((char *)pedClothesStorage + 0x60);
                                                   if (isValidPointer(stashBinded)) {
                                                       void *stashKlass = *(void **)stashBinded;
                                                       if (stashKlass) {
                                                           static void *removeMethod = nullptr;
                                                           if (!removeMethod && g_il2cpp.ready) {
                                                               removeMethod = g_il2cpp.GetMethod(stashKlass, "Remove", 1);
                                                           }
                                                           if (removeMethod) {
                                                               void *params[] = { &g_LocalPlayerEntityVal };
                                                               g_il2cpp.Invoke(removeMethod, stashBinded, params);
                                                               LOGI("hook_PedUpdateClothesSystem_OnUpdate: Stash<PedClothesBinded>::Remove");
                                                           }
                                                       }
                                                   }
                                               }

                                               // 2. Explicitly add PedLoadClothes to trigger the async Addressable download
                                               void *stashLoadClothes = *(void **)((char *)self + 0x28);
                                               if (isValidPointer(stashLoadClothes)) {
                                                   void *stashKlass = *(void **)stashLoadClothes;
                                                   if (stashKlass) {
                                                       static void *addMethod = nullptr;
                                                       if (!addMethod && g_il2cpp.ready) {
                                                           addMethod = g_il2cpp.GetMethod(stashKlass, "Add", 1);
                                                       }
                                                       if (addMethod) {
                                                           void *params[] = { &g_LocalPlayerEntityVal };
                                                           g_il2cpp.Invoke(addMethod, stashLoadClothes, params);
                                                           LOGI("hook_PedUpdateClothesSystem_OnUpdate: Stash<PedLoadClothes>::Add");
                                                       }
                                                   }
                                               }

                                               // 3. Keep PedClothesChanged for the pipeline validation
                                               void *stashObj = *(void **)((char *)self + 0x38); // Stash<PedClothesChanged>
                                               if (isValidPointer(stashObj)) {
                                                 void *stashKlass = *(void **)stashObj;
                                                 if (stashKlass) {
                                                   static void *addMethod = nullptr;
                                                   if (!addMethod && g_il2cpp.ready) {
                                                     addMethod = g_il2cpp.GetMethod(stashKlass, "Add", 1);
                                                   }
                                                   if (addMethod) {
                                                     void *params[] = { &g_LocalPlayerEntityVal };
                                                     g_il2cpp.Invoke(addMethod, stashObj, params);
                                                     LOGI("hook_PedUpdateClothesSystem_OnUpdate: Successfully invoked Stash<PedClothesChanged>::Add");
                                                   }
                                                 }
                                               }
                                             } else {
                                               s_clothesSignalFrames--;
                                             }
                                           }
                                         }
                                       }

                                       // Hook
                                       // GuiContentImageComponent.OnContentLoaded
                                       typedef void (*OnContentLoaded_t)(
                                           void *self, void *requestData,
                                           uint64_t index, void *method);
                                       static OnContentLoaded_t
                                           orig_OnContentLoaded = nullptr;

                                       typedef void *(*Sprite_GetTexture_t)(
                                           void *self);
                                       static Sprite_GetTexture_t
                                           fn_Sprite_GetTexture = nullptr;

                                       static std::string
                                       il2cppStringToCppString(
                                           void *il2cppStr) {
                                         if (!isValidPointer(il2cppStr))
                                           return "";
                                         int len =
                                             *(int *)((uint8_t *)il2cppStr +
                                                      0x10);
                                         if (len <= 0 ||
                                             len > 2048) // Limite de taille
                                                         // pour éviter les
                                                         // lectures aberrantes
                                           return "";
                                         void *charsPtr =
                                             (uint8_t *)il2cppStr + 0x14;
                                         if (!isValidPointer(charsPtr))
                                           return "";
                                         uint16_t *chars = (uint16_t *)charsPtr;
                                         std::string outStr;
                                         outStr.reserve(len);
                                         for (int i = 0; i < len; ++i) {
                                           outStr.push_back((char)chars[i]);
                                         }
                                         return outStr;
                                       }

                                       static void hook_OnContentLoaded(
                                           void *self, void *requestData,
                                           uint64_t index, void *method) {
                                         if (orig_OnContentLoaded) {
                                           orig_OnContentLoaded(self,
                                                                requestData,
                                                                index, method);
                                         }

                                         if (!isValidPointer(self)) {
                                           LOGI(
                                               "OnContentLoaded: self is null");
                                           return;
                                         }

                                         // Read 'image' field from
                                         // GuiContentImageComponent (+0x38)
                                         void *image =
                                             *(void **)((char *)self + 0x38);
                                         if (!isValidPointer(image)) {
                                           LOGI("OnContentLoaded: image at "
                                                "+0x38 is null");
                                           return;
                                         }

                                         // Read 'm_Sprite' field from
                                         // UnityEngine.UI.Image (+0xD8)
                                         void *sprite =
                                             *(void **)((char *)image + 0xD8);
                                         if (!isValidPointer(sprite)) {
                                           // Fallback: Read '_preloadedAsset'
                                           // from GuiContentImageComponent
                                           // (+0x68)
                                           sprite =
                                               *(void **)((char *)self + 0x68);
                                           if (!isValidPointer(sprite)) {
                                             LOGI("OnContentLoaded: both "
                                                  "m_Sprite and "
                                                  "_preloadedAsset are null");
                                             return;
                                           }
                                         }

                                         if (!fn_Sprite_GetTexture) {
                                           fn_Sprite_GetTexture =
                                               (Sprite_GetTexture_t)(g_base +
                                                                     0x430A208);
                                         }

                                         void *texture =
                                             fn_Sprite_GetTexture(sprite);
                                         if (!isValidPointer(texture)) {
                                           LOGI("OnContentLoaded: texture from "
                                                "sprite is null");
                                           return;
                                         }

                                         // Strategie 1: lire m_Name depuis
                                         // l'objet Sprite directement. IL2CPP
                                         // UnityEngine.Object layout:
                                         // klass*(+0), monitor*(+8),
                                         // m_CachedPtr*(+0x10) Mais certaines
                                         // versions placent le System.String*
                                         // du nom a +0x10 aussi. On teste
                                         // +0x10, +0x18, +0x20 sur le sprite.
                                         std::string spriteName;
                                         for (int nameOff :
                                              {0x10, 0x18, 0x20}) {
                                           if (!IsValidMemoryFast(
                                                   (char *)sprite + nameOff,
                                                   sizeof(void *)))
                                             continue;
                                           void *namePtr =
                                               *(void **)((char *)sprite +
                                                          nameOff);
                                           if (!isValidPointer(namePtr))
                                             continue;
                                           std::string candidate =
                                               il2cppStringToCppString(namePtr);
                                           if (candidate.size() >= 2 &&
                                               candidate.size() < 256) {
                                             spriteName = candidate;
                                             break;
                                           }
                                         }

                                         // Strategie 2: lire depuis les champs
                                         // string du composant (offsets
                                         // communs)
                                         if (spriteName.empty()) {
                                           static const int kOffsets[] = {
                                               0xA0, 0x90, 0x98, 0xA8,
                                               0x88, 0x80, 0x70, 0x68};
                                           for (int off : kOffsets) {
                                             if (!IsValidMemoryFast(
                                                     (char *)self + off,
                                                     sizeof(void *)))
                                               continue;
                                             void *strPtr =
                                                 *(void **)((char *)self + off);
                                             if (!isValidPointer(strPtr))
                                               continue;
                                             std::string candidate =
                                                 il2cppStringToCppString(
                                                     strPtr);
                                             if (candidate.size() >= 3 &&
                                                 candidate.size() < 256) {
                                               spriteName = candidate;
                                               size_t lastSlash =
                                                   spriteName.find_last_of('/');
                                               if (lastSlash !=
                                                   std::string::npos)
                                                 spriteName = spriteName.substr(
                                                     lastSlash + 1);
                                               break;
                                             }
                                           }
                                         }

                                         if (!spriteName.empty()) {
                                           LOGI("OnContentLoaded: Extracted "
                                                "sprite name: %s",
                                                spriteName.c_str());

                                           // Resolve GetNativeTexturePtr via
                                           // il2cpp to get the GL uint ID
                                           static void *(*fn_resolve_icall)(
                                               const char *) = nullptr;
                                           if (!fn_resolve_icall) {
                                             fn_resolve_icall =
                                                 (void *(*)(const char *))dlsym(
                                                     dlopen("libil2cpp.so",
                                                            RTLD_LAZY),
                                                     "il2cpp_resolve_icall");
                                           }

                                           void *nativeTexPtr = nullptr;
                                           if (fn_resolve_icall) {
                                             typedef void *(
                                                 *GetNativeTexturePtr_t)(
                                                 void *);
                                             static GetNativeTexturePtr_t
                                                 fn_GetNativeTex = nullptr;
                                             if (!fn_GetNativeTex) {
                                               fn_GetNativeTex =
                                                   (GetNativeTexturePtr_t)
                                                       fn_resolve_icall(
                                                           "UnityEngine."
                                                           "Texture::"
                                                           "GetNativeTexturePtr"
                                                           "()");
                                               if (!fn_GetNativeTex) {
                                                 fn_GetNativeTex =
                                                     (GetNativeTexturePtr_t)
                                                         fn_resolve_icall(
                                                             "UnityEngine."
                                                             "Texture::"
                                                             "GetNativeTextureP"
                                                             "tr");
                                               }
                                             }

                                             if (fn_GetNativeTex) {
                                               nativeTexPtr =
                                                   fn_GetNativeTex(texture);
                                             } else {
                                               // Fallback reading directly from
                                               // m_CachedPtr + 0x50/0xB8
                                               // depending on Unity version if
                                               // needed Just storing the raw
                                               // pointer for now if icall fails
                                               nativeTexPtr = texture;
                                             }
                                           } else {
                                             nativeTexPtr = texture;
                                           }

                                           if (nativeTexPtr) {
                                             RegisterGameTexture(spriteName,
                                                                  (unsigned int)(uintptr_t)nativeTexPtr);
                                           }
                                         } else {
                                           LOGI("OnContentLoaded: Failed to "
                                                "extract sprite name");
                                         }
                                       }

                                       void *hack_thread(void *) {
                                         static std::atomic<bool> s_started{
                                             false};
                                         if (s_started.exchange(true)) {
                                           LOGI("hack_thread already running, "
                                                "skipping...");
                                           return nullptr;
                                         }
                                         LOGI("hack_thread started, waiting "
                                              "for JNI_OnLoad to finish...");

                                         // WAIT FOR JNI_ONLOAD AND LINKER TO
                                         // FINISH (PREVENTS 4-MINUTE STARTUP
                                         // FREEZE DEADLOCK)
                                         usleep(500000); // 500ms delay

                                         std::thread(DetectDebuggerAndFrida)
                                             .detach();
                                         wipeAppDataIfFlagged();
                                         regenerateFakeDeviceId();

                                         // Initialize Network Utility early so
                                         // HTTP requests don't fail and trigger
                                         // false lock
                                         extern JavaVM *g_GravityJVM;
                                         HttpUtils::Init(g_GravityJVM);

                                         // Initialize License System
                                         {
                                           char dataDir[256];
                                           std::string appDataPath = "";
                                           if (getMyAppDataDir(dataDir, sizeof(dataDir)) > 0) {
                                             appDataPath = std::string(dataDir) + "/";
                                           }
                                           std::string hwid = GetDeviceHWID();
                                           License::Init(appDataPath, hwid);
                                         }

                                         // Initialize Protection Thread and
                                         // HWID Check asynchronously
                                         extern void InitProtection();
                                         InitProtection();
                                         std::thread([]() {
                                           InitializeGameServices();
                                         }).detach();

                                         // Initialise the texture cache
                                         // directory for persistent skin
                                         // preview storage
                                         {
                                           char dataDir[256];
                                           size_t l = getMyAppDataDir(
                                               dataDir, sizeof(dataDir));
                                           if (l > 0) {
                                             char cacheDir[320];
                                             snprintf(
                                                 cacheDir, sizeof(cacheDir),
                                                 "%s/files/tex_cache", dataDir);
                                             Catalog_SetCacheDir(cacheDir);
                                             LOGI("Texture cache dir: %s",
                                                  cacheDir);
                                           }
                                         }

                                         void *handle = nullptr;
                                         int waitMs = 100;
                                         do {
                                           usleep(waitMs * 1000);

                                           // Check passively to avoid stealing
                                           // the linker lock and freezing the
                                           // main thread
                                           bool loaded = false;
                                           FILE *f =
                                               fopen("/proc/self/maps", "r");
                                           if (f) {
                                             char line[512];
                                             while (
                                                 fgets(line, sizeof(line), f)) {
                                               if (strstr(line,
                                                          "libil2cpp.so")) {
                                                 loaded = true;
                                                 break;
                                               }
                                             }
                                             fclose(f);
                                           }

                                           if (loaded) {
                                             // Now it's safe to dlopen without
                                             // forcing a load
                                             handle = dlopen("libil2cpp.so",
                                                             RTLD_LAZY);
                                           }

                                           if (waitMs < 1000)
                                             waitMs = waitMs < 500 ? 500 : 1000;
                                         } while (!handle);

                                         void *init_sym =
                                             dlsym(handle, "il2cpp_init");
                                         if (init_sym) {
                                           Dl_info info;
                                           if (dladdr(init_sym, &info)) {
                                             g_base = (uintptr_t)info.dli_fbase;
                                           }
                                         }
                                         if (!g_base) {
                                           g_base = (uintptr_t)handle;
                                         }
                                         LOGI("libil2cpp.so loaded base at %p "
                                              "(handle=%p)",
                                              (void *)g_base, handle);

                                         LOGI("hack_thread: Waiting 25 seconds "
                                              "for Unity engine to stabilize "
                                              "before hooking...");
                                         usleep(25000000); // 25 seconds delay
                                         LOGI(
                                             "hack_thread: Unity stabilization "
                                             "complete. Proceeding with memory "
                                             "patches and hooks.");

                                         // Lancer l'overlay APRES que le jeu
                                         // soit chargé (évite le crash de
                                         // contexte EGL du splash screen Unity)
                                         extern void setupGravityOverlay();
                                         setupGravityOverlay();

                                         extern void Esp_Init(uintptr_t);
                                         Esp_Init(g_base);
                                         extern void Aimbot_InstallHooks(
                                             uintptr_t);
                                         Aimbot_InstallHooks(g_base);

                                         // Initialize God Mode patches (V18
                                         // RVAs)
                                         patch_Knockout1 =
                                             MemoryPatch::createWithHex(
                                                 g_base + 0x3005994, RET);
                                         patch_Knockout2 =
                                             MemoryPatch::createWithHex(
                                                 g_base + 0x3022B28, RET);
                                         patch_UnconsciousDetect =
                                             MemoryPatch::createWithHex(
                                                 g_base + 0x3005898, RET);
                                         patch_MeleeKnockoutCleanup =
                                             MemoryPatch::createWithHex(
                                                 g_base + 0x31A04BC, RET);
                                         patch_DeadKnockoutRestore =
                                             MemoryPatch::createWithHex(
                                                 g_base + 0x3001824, RET);
                                         patch_KnockoutSync =
                                             MemoryPatch::createWithHex(
                                                 g_base + 0x3007C40, RET);
                                         patch_RagDollState =
                                             MemoryPatch::createWithHex(
                                                 g_base + 0x3023060, RET);
                                         patch_DeadSync =
                                             MemoryPatch::createWithHex(
                                                 g_base + 0x2782CD4, RET);
                                         patch_ProcessDead =
                                             MemoryPatch::createWithHex(
                                                 g_base + 0x277CF8C, RET);

                                         // === GOD MODE V2 — Anti-Mêlée + Écran
                                         // Mort + PedKill ===
                                         patch_GhostMelee =
                                             MemoryPatch::createWithHex(
                                                 g_base + 0x2BD22D0,
                                                 RET); // WeaponAttackMeleeReceiveSystem.OnUpdate
                                         patch_MeleeHitReceive =
                                             MemoryPatch::createWithHex(
                                                 g_base + 0x31A3840,
                                                 RET); // MeleeHitReceiveSystem.OnUpdate
                                         patch_MeleeHitStun =
                                             MemoryPatch::createWithHex(
                                                 g_base + 0x31A4214,
                                                 RET); // MeleeHitStunCancelSystem.OnUpdate
                                         patch_DeathUI1 =
                                             MemoryPatch::createWithHex(
                                                 g_base + 0x2546624,
                                                 RET); // HudDeathCreateSystem.OnUpdate
                                         patch_DeathUI2 =
                                             MemoryPatch::createWithHex(
                                                 g_base + 0x2567A50,
                                                 RET); // HudDeathWatchChangesSystem.OnUpdate
                                         patch_DeathUI3 =
                                             MemoryPatch::createWithHex(
                                                 g_base + 0x25480C8,
                                                 RET); // HudDeathInitializeOtherSystem.OnUpdate
                                         patch_PedKill =
                                             MemoryPatch::createWithHex(
                                                 g_base + 0x277BAA4,
                                                 RET); // PedKillSystem.OnUpdate
                                         // === ZONE VERTE BYPASS ===
                                         patch_GreenZone =
                                             MemoryPatch::createWithHex(
                                                 g_base + 0x2BD2D80,
                                                 RET); // WeaponAttackNotAllowedSyncSystem.OnUpdate
                                         LOGI("GravityMod: GodModeV2 patches "
                                              "initialized (GhostMelee=%d "
                                              "MeleeHit=%d "
                                              "HitStun=%d DeathUI=%d/%d/%d "
                                              "PedKill=%d GreenZone=%d)",
                                              patch_GhostMelee.isValid(),
                                              patch_MeleeHitReceive.isValid(),
                                              patch_MeleeHitStun.isValid(),
                                              patch_DeathUI1.isValid(),
                                              patch_DeathUI2.isValid(),
                                              patch_DeathUI3.isValid(),
                                              patch_PedKill.isValid(),
                                              patch_GreenZone.isValid());

                                         // Initialize No Reload patch (V18
                                         // WeaponReloadStartSystem.OnUpdate)
                                         patch_NoReload =
                                             MemoryPatch::createWithHex(
                                                 g_base + 0x2BD8930, RET);

                                         // Dobby hooks for FOV and
                                         // Speed/Velocity
                                         int rFovHook = DobbyHook(
                                             (void *)(g_base + 0x42A9974),
                                             (dobby_dummy_func_t)
                                                 hook_SetFieldOfView,
                                             (dobby_dummy_func_t
                                                  *)&orig_SetFieldOfView);
                                         int rVelHook = DobbyHook(
                                             (void *)(g_base + 0x326C2C4),
                                             (dobby_dummy_func_t)
                                                 hook_SetVelocity,
                                             (dobby_dummy_func_t
                                                  *)&orig_SetVelocity);
                                         int rPvlHook = DobbyHook(
                                             (void *)(g_base + 0x326C36C),
                                             (dobby_dummy_func_t)
                                                 hook_SetPlanarVelocity,
                                             (dobby_dummy_func_t
                                                  *)&orig_SetPlanarVelocity);
                                         int rRbVelHook = DobbyHook(
                                             (void *)(g_base + 0x4369540),
                                             (dobby_dummy_func_t)
                                                 hook_Rigidbody_SetVelocity,
                                             (dobby_dummy_func_t
                                                  *)&orig_Rigidbody_SetVelocity);
                                         LOGI(
                                             "Dobby hooks - FOV: %d, Velocity: "
                                             "%d, PlanarVelocity: %d, "
                                             "RigidbodyVelocity: %d",
                                             rFovHook, rVelHook, rPvlHook,
                                             rRbVelHook);

                                         int rEvt = DobbyHook(
                                             (void *)(g_base +
                                                      RVA_EventSystemUpdate),
                                             (dobby_dummy_func_t)
                                                 hook_EventSystem_Update,
                                             (dobby_dummy_func_t
                                                  *)&orig_EventSystem_Update);
                                         int rSim = DobbyHook(
                                             (void *)(g_base +
                                                      RVA_PreSimulationUpdate),
                                             (dobby_dummy_func_t)
                                                 hook_PreSimulationUpdate,
                                             (dobby_dummy_func_t
                                                  *)&orig_PreSimulationUpdate);

                                         int rDev = DobbyHook(
                                             (void
                                                  *)(g_base +
                                                     RVA_GetDeviceUniqueIdentifier),
                                             (dobby_dummy_func_t)
                                                 hook_GetDeviceUniqueIdentifier,
                                             (dobby_dummy_func_t
                                                  *)&orig_GetDeviceUniqueIdentifier);
                                         int rMdl = DobbyHook(
                                             (void *)(g_base +
                                                      RVA_GetDeviceModel),
                                             (dobby_dummy_func_t)
                                                 hook_GetDeviceModel,
                                             (dobby_dummy_func_t
                                                  *)&orig_GetDeviceModel);
                                         int rNam = DobbyHook(
                                             (void *)(g_base +
                                                      RVA_GetDeviceName),
                                             (dobby_dummy_func_t)
                                                 hook_GetDeviceName,
                                             (dobby_dummy_func_t
                                                  *)&orig_GetDeviceName);
                                         int rAuth = DobbyHook(
                                             (void
                                                  *)(g_base +
                                                     RVA_GetAuthUniqueIdentifier),
                                             (dobby_dummy_func_t)
                                                 hook_GetAuthUniqueIdentifier,
                                             (dobby_dummy_func_t
                                                  *)&orig_GetAuthUniqueIdentifier);

                                         int rPrefs = DobbyHook(
                                             (void *)(g_base +
                                                      RVA_PlayerPrefsGetString),
                                             (dobby_dummy_func_t)
                                                 hook_PlayerPrefsGetString,
                                             (dobby_dummy_func_t
                                                  *)&orig_PlayerPrefsGetString);
                                         int rPrefs1 = DobbyHook(
                                             (void
                                                  *)(g_base +
                                                     RVA_PlayerPrefsGetString1),
                                             (dobby_dummy_func_t)
                                                 hook_PlayerPrefsGetString1,
                                             (dobby_dummy_func_t
                                                  *)&orig_PlayerPrefsGetString1);
                                         int rGuest = DobbyHook(
                                             (void *)(g_base + RVA_GetGuestId),
                                             (dobby_dummy_func_t)
                                                 hook_GetGuestId,
                                             (dobby_dummy_func_t
                                                  *)&orig_GetGuestId);

                                         int rExtReal = DobbyHook(
                                             (void *)(g_base +
                                                      RVA_GetExtDeviceIdReal),
                                             (dobby_dummy_func_t)
                                                 hook_GetExtDeviceIdReal,
                                             (dobby_dummy_func_t
                                                  *)&orig_GetExtDeviceIdReal);
                                         int rExtGen = DobbyHook(
                                             (void
                                                  *)(g_base +
                                                     RVA_GetExtDeviceIdGenerated),
                                             (dobby_dummy_func_t)
                                                 hook_GetExtDeviceIdGenerated,
                                             (dobby_dummy_func_t
                                                  *)&orig_GetExtDeviceIdGenerated);

                                         int rMap = DobbyHook(
                                             (void *)(g_base + RVA_FindGoal),
                                             (dobby_dummy_func_t)hook_FindGoal,
                                             (dobby_dummy_func_t
                                                  *)&orig_FindGoal);
                                         int rMapCtor = DobbyHook(
                                             (void
                                                  *)(g_base +
                                                     RVA_CreateMapGpsCheckpoint_ctor),
                                             (dobby_dummy_func_t)
                                                 hook_CreateMapGpsCheckpointCtor,
                                             (dobby_dummy_func_t
                                                  *)&orig_CreateMapGpsCheckpointCtor);
                                         int rMapSignal = DobbyHook(
                                             (void
                                                  *)(g_base +
                                                     RVA_CreateMapCheckpointSignalHandler),
                                             (dobby_dummy_func_t)
                                                 hook_CreateMapCheckpointSignalHandler,
                                             (dobby_dummy_func_t
                                                  *)&orig_CreateMapCheckpointSignalHandler);
                                         int rMapPos = DobbyHook(
                                             (void *)(g_base +
                                                      RVA_SetMapPosition),
                                             (dobby_dummy_func_t)
                                                 hook_SetMapPosition,
                                             (dobby_dummy_func_t
                                                  *)&orig_SetMapPosition);
                                         int rCheckpointFactory = DobbyHook(
                                             (void *)(g_base + RVA_CheckpointFactory_CreateCheckpoint),
                                             (dobby_dummy_func_t)hook_CreateCheckpoint,
                                             (dobby_dummy_func_t *)&orig_CreateCheckpoint);
                                         int rSeat = DobbyHook(
                                             (void
                                                  *)(g_base +
                                                     RVA_TryGetNearVehicleSeat),
                                             (dobby_dummy_func_t)
                                                 hook_TryGetNearVehicleSeat,
                                             (dobby_dummy_func_t
                                                  *)&orig_TryGetNearVehicleSeat);
                                         int rCam = 0;
                                         if (RVA_FireOnPreCull != 0x0) {
                                           rCam = DobbyHook(
                                               (void *)(g_base +
                                                        RVA_FireOnPreCull),
                                               (dobby_dummy_func_t)
                                                   hook_FireOnPreCull,
                                               (dobby_dummy_func_t
                                                    *)&orig_FireOnPreCull);
                                         }
                                         int rCamPl = DobbyHook(
                                             (void *)(g_base + 0x31BC798),
                                             (dobby_dummy_func_t)
                                                 hook_CameraPlacementSystem_OnUpdate,
                                             (dobby_dummy_func_t
                                                  *)&orig_CameraPlacementSystem_OnUpdate);
                                         int rCamLook = DobbyHook(
                                             (void
                                                  *)(g_base +
                                                     RVA_SetCameraLookDirection),
                                             (dobby_dummy_func_t)
                                                 hook_SetCameraLookDirection,
                                             (dobby_dummy_func_t
                                                  *)&orig_SetCameraLookDirection);
                                         int rCamTryGet = DobbyHook(
                                             (void *)(g_base + 0x31BE6C0),
                                             (dobby_dummy_func_t)
                                                 hook_MainCameraStorage_TryGet,
                                             (dobby_dummy_func_t
                                                  *)&orig_MainCameraStorage_TryGet);

                                         LOGI(
                                             "Hooks placed: "
                                             "EventSystemUpdate=%d, "
                                             "PreSimulationUpdate=%d, "
                                             "DeviceUID=%d, DeviceModel=%d, "
                                             "DeviceName=%d, AuthUID=%d, "
                                             "Prefs=%d, "
                                             "Prefs1=%d, Guest=%d, ExtReal=%d, "
                                             "ExtGen=%d, FindGoal=%d, "
                                             "MapCtor=%d, MapSignal=%d, "
                                             "MapPos=%d, CheckpointFactory=%d, "
                                             "Seat=%d, "
                                             "Cam=%d, CamPl=%d, CamLook=%d, "
                                             "CamTryGet=%d",
                                             rEvt, rSim, rDev, rMdl, rNam,
                                             rAuth, rPrefs, rPrefs1, rGuest,
                                             rExtReal, rExtGen, rMap, rMapCtor,
                                             rMapSignal, rMapPos,
                                             rCheckpointFactory, rSeat, rCam,
                                             rCamPl, rCamLook, rCamTryGet);

                                         // Initialize pointers
                                         fn_TryGetIndex =
                                             (TryGetIndex_t)(g_base +
                                                             0x3A69EF4);
                                         fn_Entity_get_Id =
                                             (Entity_get_Id_t)(g_base +
                                                               0x3A6BAD0);

                                         // Initialize IL2CPP helper bridge
                                         // (symbol resolution only, no domain
                                         // calls) IMPORTANT: Do NOT call
                                         // domain_get() or thread_attach()
                                         // here. IL2CPP domain is not yet fully
                                         // initialized at this point in
                                         // hack_thread. All IL2CPP managed
                                         // calls (PlayerPrefs,
                                         // FindClassEverywhere, etc.) are safe
                                         // only inside game hooks that run on
                                         // Unity's own thread.
                                         g_il2cpp.Init();
                                         LOGI("IL2CPP bridge initialized "
                                              "(ready=%d)",
                                              g_il2cpp.ready ? 1 : 0);

                                         // Dobby hooks for model swapping
                                         int rPedStr = DobbyHook(
                                             (void *)(g_base + 0x2D925D4),
                                             (dobby_dummy_func_t)
                                                 hook_PedStreamingProvider_Construct,
                                             (dobby_dummy_func_t
                                                  *)&orig_PedStreamingProvider_Construct);
                                         orig_PedStreamingProvider_Deconstruct =
                                             (PedStreamingProvider_Deconstruct_t)(g_base +
                                                                                  0x2D928E0);

                                         int rVehStr = DobbyHook(
                                             (void *)(g_base + 0x2D94668),
                                             (dobby_dummy_func_t)
                                                 hook_VehiclesStreamingProvider_Construct,
                                             (dobby_dummy_func_t
                                                  *)&orig_VehiclesStreamingProvider_Construct);
                                         int rWepCrt = DobbyHook(
                                             (void *)(g_base + 0x2FFA070),
                                             (dobby_dummy_func_t)
                                                 hook_PedWeaponCreateSystem_OnUpdate,
                                             (dobby_dummy_func_t
                                                  *)&orig_PedWeaponCreateSystem_OnUpdate);

                                         // Hook OnContentLoaded to intercept
                                         // skin/item preview textures
                                         int rContLd = DobbyHook(
                                             (void *)(g_base + 0x29E43C4),
                                             (dobby_dummy_func_t)
                                                 hook_OnContentLoaded,
                                             (dobby_dummy_func_t
                                                  *)&orig_OnContentLoaded);

                                         // Enforce God Mode hook
                                         int rHlthSync = DobbyHook(
                                             (void *)(g_base + 0x2792BD0),
                                             (dobby_dummy_func_t)
                                                 hook_ElementHealthSyncSystem_OnUpdate,
                                             (dobby_dummy_func_t
                                                  *)&orig_ElementHealthSyncSystem_OnUpdate);

                                         // NOTE: PlayerPrefs_GetInt is NOT
                                         // called here because IL2CPP
                                         // assemblies are not yet loaded at
                                         // this point in hack_thread startup.
                                         // The saved skin will be restored
                                         // later via the PedCustomization hook
                                         // once Unity has fully initialized its
                                         // assembly domain.
                                         LOGI(
                                             "hack_thread: IL2CPP initialized, "
                                             "skin restore deferred to "
                                             "PedCustomization hook");

                                         // Register PedCustomization hooks
                                         int rCustRead = DobbyHook(
                                             (void *)(g_base + 0x30E327C),
                                             (dobby_dummy_func_t)
                                                 hook_PedCustomizationPlayerDataResponseDatagram_Read,
                                             (dobby_dummy_func_t
                                                  *)&orig_PedCustomizationPlayerDataResponseDatagram_Read);

                                         int rCustSave = DobbyHook(
                                             (void *)(g_base + 0x30CF6CC),
                                             (dobby_dummy_func_t)
                                                 hook_PedCustomizationSaveCurrentItemsSystem_OnUpdate,
                                             (dobby_dummy_func_t
                                                  *)&orig_PedCustomizationSaveCurrentItemsSystem_OnUpdate);

                                         int rUpdClth = DobbyHook(
                                             (void *)(g_base + 0x278872C),
                                             (dobby_dummy_func_t)
                                                 hook_PedUpdateClothesSystem_OnUpdate,
                                             (dobby_dummy_func_t
                                                  *)&orig_PedUpdateClothesSystem_OnUpdate);

                                         LOGI("Dobby hooks - PedStr: %d, "
                                              "VehStr: %d, WepCrt: %d, "
                                              "OnContent: %d, "
                                              "HealthSync: %d, CustRead: %d, "
                                              "CustSave: %d, UpdClth: %d",
                                              rPedStr, rVehStr, rWepCrt,
                                              rContLd, rHlthSync, rCustRead,
                                              rCustSave, rUpdClth);

                                         // setupGravityOverlay() a déjà été
                                         // appelé au début de hack_thread
                                         LOGI("hack_thread: all hooks "
                                              "installed, mod menu ready.");
                                         return nullptr;
                                       }