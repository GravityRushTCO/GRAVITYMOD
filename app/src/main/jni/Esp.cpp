#include "Esp.h"
#include "Aimbot.h"
#include "GravityGL/Offsets.h"


#include "Dobby/dobby.h"
#include "GraffitiTags.h" // 144 coordonnées extraites du minimap
#include "GravityGL/Offsets.h"
#include "Includes/Logger.h"
#include "KittyMemory/KittyMemory.hpp"
#include "KittyMemory/KittyScanner.hpp"
#include "il2cpp_bridge.h"
#include <android/log.h>
#include <chrono>
#include <errno.h>
#include <fcntl.h>
#include <jni.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <thread>
#include <time.h>
#include <unistd.h>
#include <unordered_map>
#include <unordered_set>
#include <vector>


extern "C" void SetGodModePatchesState(bool enabled);

#define E_LOG_TAG "ESP"
#define E_LOGI(...)                                                            \
  __android_log_print(ANDROID_LOG_INFO, E_LOG_TAG, __VA_ARGS__)
#define E_LOGE(...)                                                            \
  __android_log_print(ANDROID_LOG_ERROR, E_LOG_TAG, __VA_ARGS__)

extern uintptr_t g_base;

extern void *g_PedStreamingProviderInstance;
extern void *g_VehiclesStreamingProviderInstance;

static inline bool isValidPointer(void *ptr) {
  if (!ptr)
    return false;
  uintptr_t addr = (uintptr_t)ptr;
  if (addr < 0x10000)
    return false;
  if (addr >= 0x7fffffffff)
    return false;
  return true;
}

extern bool (*fn_TryGetIndex)(void *self, int key, int *slotIndex);
extern int (*fn_Entity_get_Id)(unsigned long long *entityVal);

void *CustomGetRootTransform(void *transform);

static int s_teleportSpamFrames = 0;
static V3 s_teleportSpamPos = {0, 0, 0};
static V3 s_teleportSpamVel = {0, 0, 0};

static const float kAimMaxDist = 5000.0f;
// kAimHeightAboveFeet est maintenant DYNAMIQUE selon le mode :
//   - g_AimBotMode/g_SilentAimMode = 1 → Tête (1.7m above feet)
//   - g_AimBotMode/g_SilentAimMode = 2 → Torse (1.0m above feet)
//   - 0 → défaut Tête
extern int g_AimBotMode;
extern int g_SilentAimMode;
extern float g_AimFov;
extern bool g_AimVisibleOnly;
extern int g_BonePriority; // 0=Torso, 1=Neck, 2=Head, 3=Pelvis

static float getAimHeight() {
  switch (g_BonePriority) {
  case 1:
    return 1.3f; // Neck
  case 2:
    return 1.5f; // Head
  case 3:
    return 0.5f; // Pelvis
  case 0:
  default:
    return 1.0f; // Torso
  }
}

// Log fichier persistant lisible par l'utilisateur sans adb logcat. Utilisé
// pour le diagnostic du teleport (le user a signalé ne pas pouvoir voir les
// logs adb de son côté).
#include <stdarg.h>
#include <stdio.h>

static void TPLOG(const char *fmt, ...) {
  FILE *f = fopen("/sdcard/Download/teleport.log", "ab");
  if (!f)
    return;
  va_list ap;
  va_start(ap, fmt);
  vfprintf(f, fmt, ap);
  va_end(ap);
  fputc('\n', f);
  fclose(f);
}

// === Vec3 ===
#ifndef V3_STRUCT_DEFINED
#define V3_STRUCT_DEFINED
struct V3 {
  float x, y, z;
};
#endif
static inline V3 v3(float x, float y, float z) {
  V3 v = {x, y, z};
  return v;
}
static inline V3 v3sub(V3 a, V3 b) {
  return v3(a.x - b.x, a.y - b.y, a.z - b.z);
}
static inline float v3len(V3 a) {
  return sqrtf(a.x * a.x + a.y * a.y + a.z * a.z);
}

// === IL2CPP function pointers (resolved in Esp_Init) ===
// UnityEngine.Camera.get_main @ 0x41F4AC4
typedef void *(*Camera_get_main_t)(void);
static Camera_get_main_t fn_Camera_get_main = nullptr;

// UnityEngine.Component.get_gameObject @ 0x423D648 (component -> gameObject)
typedef void *(*Component_get_gameObject_t)(void *self);
static Component_get_gameObject_t fn_get_gameObject = nullptr;

// UnityEngine.Object.get_name @ 0x4245A70 (object -> System.String*)
typedef void *(*Object_get_name_t)(void *self);
static Object_get_name_t fn_get_name = nullptr;

// === Helper: convertit System.String* IL2CPP en C-string ASCII (caractères
// >127 -> '?'). === Layout System.String IL2CPP : klass(8), monitor(8),
// length(4) @ +0x10, chars[] @ +0x14 (UTF-16).
static void il2cppStringToChar(void *str, char *out, int outSize) {
  if (outSize <= 0)
    return;
  out[0] = 0;
  if (!str)
    return;
  int len = *(int *)((uint8_t *)str + 0x10);
  if (len <= 0 || len > 4096)
    return; // garde-fou
  uint16_t *chars = (uint16_t *)((uint8_t *)str + 0x14);
  int n = (len < outSize - 1) ? len : (outSize - 1);
  for (int i = 0; i < n; i++) {
    uint16_t c = chars[i];
    out[i] = (c >= 0x20 && c < 0x7F) ? (char)c : '?';
  }
  out[n] = 0;
}

// === Heuristique de classification du nom GameObject -> EspPedType ===
// On regarde les patterns courants des noms d'instances dans OneState RP.
// Si on ne reconnaît rien on retourne ESP_PED_PLAYER par défaut (= autre
// joueur).
static int classifyPedByName(const char *name) {
  if (!name || !*name)
    return ESP_PED_UNKNOWN;
  // Match insensible à la casse via comparaison rapide.
  auto containsCI = [](const char *s, const char *k) -> bool {
    for (; *s; s++) {
      const char *a = s;
      const char *b = k;
      while (*a && *b) {
        char ca = *a;
        char cb = *b;
        if (ca >= 'A' && ca <= 'Z')
          ca += 32;
        if (cb >= 'A' && cb <= 'Z')
          cb += 32;
        if (ca != cb)
          break;
        a++;
        b++;
      }
      if (!*b)
        return true;
    }
    return false;
  };
  // Police (priorité avant NPC car certains noms peuvent contenir les deux).
  if (containsCI(name, "police") || containsCI(name, "cop") ||
      containsCI(name, "swat") || containsCI(name, "lspd") ||
      containsCI(name, "fbi")) {
    return ESP_PED_POLICE;
  }
  //  tous les vrais joueurs ont leur GameObject nommé "PlayerClone"
  // (c'est le clone du prefab Player). C'est l'identifiant le plus fiable.
  if (containsCI(name, "playerclone") || containsCI(name, "player(clone)")) {
    return ESP_PED_PLAYER;
  }
  // PNJ / IA explicites.
  if (containsCI(name, "npc") || containsCI(name, "_ai_") ||
      containsCI(name, "bot") || containsCI(name, "pedestrian") ||
      containsCI(name, "civil") || containsCI(name, "vendor") ||
      containsCI(name, "shop") || containsCI(name, "ambient")) {
    return ESP_PED_NPC;
  }
  // Défaut : PLAYER. Sur  la majorité des peds proches du joueur
  // qu'il a envie d'esp sont des vrais joueurs. On préfère les marquer
  // PLAYER (rouge) par défaut quitte à avoir quelques faux positifs PNJ
  // plutôt que de tout cacher en gris.
  return ESP_PED_PLAYER;
}

// === Camera matrices (more reliable than WorldToScreenPoint_Injected) ===
// UnityEngine.Camera.get_worldToCameraMatrix_Injected  @ 0x41F3930  (void* cam,
// Matrix4x4* out) UnityEngine.Camera.get_projectionMatrix_Injected     @
// 0x41F3A68  (void* cam, Matrix4x4* out) UnityEngine.Camera.get_pixelWidth @
// 0x41F344C  (int  cam) UnityEngine.Camera.get_pixelHeight                   @
// 0x41F3488  (int  cam)
struct M4 {
  float m[16];
}; // Unity column-major: m[row + col*4]
typedef void (*GetMat_Injected_t)(void *cam, M4 *out);
typedef int (*GetPixel_t)(void *cam);
static GetMat_Injected_t fn_GetView = nullptr;
static GetMat_Injected_t fn_GetProj = nullptr;
static GetPixel_t fn_GetPixelW = nullptr;
static GetPixel_t fn_GetPixelH = nullptr;

// UnityEngine.Component.get_transform @ generic
// We avoid calling this. Instead we read transform.position via Transform
// pointer. Lightbug.CharacterControllerPro.Core.CharacterActor inherits
// PhysicsActor inherits MonoBehaviour. We use transform.position via
// Camera::get_main approach for camera position only.

// === Camera state captured each frame ===
static V3 s_camPos = {0, 0, 0};
static int s_screenW = 1920, s_screenH = 1080;
static bool s_camValid = false;
// VP matrice + dims écran caméra au dernier PreCull, lus depuis Java pour
// projection W2S synchronisée au vsync (élimine le décalage caméra/box).
static float s_vp16[16] = {0};
static int s_camPixW = 0, s_camPixH = 0;
static bool s_vpValid = false;

// === Crosshair / FOV Circle ===
static bool s_crosshair = false; // Crosshair désactivé par défaut
static bool s_crosshairCircle = false;
static int s_crosshairRadius = 150;

// === Active CharacterActor set for current frame ===
static pthread_mutex_t s_mtx = PTHREAD_MUTEX_INITIALIZER;
static std::vector<void *> s_actorsThisFrame;
// Persistent buffer of last-frame snapshot (returned to Java)
static std::vector<EspPed> s_snapshot;

static bool s_enabled = true;
static bool s_dotMode = false;
static bool s_lineEnabled = false;
static bool s_boxEnabled = false;
static bool s_distanceEnabled = false;
static bool s_markerEnabled = false;
static bool s_dynamicColor = false;
static bool s_crosshairEnabled = false;
static bool s_crosshairCircleEnabled = false;
static int s_crosshairCircleRadius = 50;
static bool s_healthEnabled = false;
static bool s_skeletonEnabled = false;  // bone skeleton ESP
static uintptr_t s_libBase = 0;

// === Teleport state ===
// Le pointeur du CharacterActor local est mis à jour à chaque FrameTick
// (= le ped le plus proche de la caméra). Position sauvegardée par
// l'utilisateur.
void *s_localActor = nullptr;
static double s_lastLocalActorNotifyMs = 0.0;
static V3 s_savedPos = {0, 0, 0};
static bool s_hasSavedPos = false;

typedef bool (*Object_op_Implicit_t)(void *exists);
static Object_op_Implicit_t fn_Object_op_Implicit = nullptr;

#include <atomic>
#include <errno.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>


// ============================================================
// FAST RANGE MEMORY VALIDATION via /proc/self/mem & mincore()
// Checks if the memory range [ptr, ptr + size - 1] is readable.
// ============================================================
static int s_memFd = -1;
static bool s_memFdOpenFailed = false;
bool IsValidMemoryFast(void *ptr, size_t size) {
  if (!ptr || size == 0)
    return false;

  if (s_memFd == -1 && !s_memFdOpenFailed) {
    s_memFd = open("/proc/self/mem", O_RDONLY | O_CLOEXEC);
    if (s_memFd < 0) {
      s_memFdOpenFailed = true;
    }
  }

  if (s_memFd >= 0) {
    char buf;
    ssize_t ret1 = pread(s_memFd, &buf, 1, reinterpret_cast<off_t>(ptr));
    if (ret1 != 1)
      return false;
    if (size > 1) {
      ssize_t ret2 =
          pread(s_memFd, &buf, 1, reinterpret_cast<off_t>(ptr) + size - 1);
      return (ret2 == 1);
    }
    return true;
  }

  // Fallback: check all pages containing the range [ptr, ptr + size - 1]
  uintptr_t startAddr = reinterpret_cast<uintptr_t>(ptr);
  uintptr_t endAddr = startAddr + size - 1;

  uintptr_t startPage = startAddr & ~(uintptr_t)(4095);
  uintptr_t endPage = endAddr & ~(uintptr_t)(4095);

  for (uintptr_t page = startPage; page <= endPage; page += 4096) {
    unsigned char vec = 0;
    int r = mincore(reinterpret_cast<void *>(page), 1, &vec);
    if (r != 0) {
      return false;
    }
  }
  return true;
}

bool IsUnityObjectAlive(void *obj) {
  if (!obj)
    return false;
  uintptr_t addr = reinterpret_cast<uintptr_t>(obj);
  if (addr < 0x1000 || (addr % sizeof(void *)) != 0)
    return false;

  // 1. Probe the object header (vtable pointer)
  if (!IsValidMemoryFast(obj, sizeof(void *)))
    return false;

  // 2. Read and validate vtable (klass) pointer
  void *klass = *reinterpret_cast<void **>(obj);
  if (!klass || (reinterpret_cast<uintptr_t>(klass) % sizeof(void *)) != 0)
    return false;
  if (!IsValidMemoryFast(klass, sizeof(void *)))
    return false;

  // 3. Validate name pointer from klass to ensure it's a valid Il2CppClass
  // Il2CppClass.name is at 0x10 on 64-bit
  const char **namePtr =
      reinterpret_cast<const char **>(reinterpret_cast<char *>(klass) + 0x10);
  if (!IsValidMemoryFast(namePtr, sizeof(void *)))
    return false;
  const char *name = *namePtr;
  if (!name || !IsValidMemoryFast((void *)name, 1))
    return false;

  // 4. Validate m_CachedPtr at offset 0x10
  char *objBytes = reinterpret_cast<char *>(obj);
  if (!IsValidMemoryFast(objBytes + 0x10, sizeof(void *)))
    return false;

  void *cachedPtr = *reinterpret_cast<void **>(objBytes + 0x10);
  if (!cachedPtr)
    return false;

  // 5. Native pointer validation: check if the native object is resident in
  // memory
  if (!IsValidMemoryFast(cachedPtr, 1))
    return false;

  return true;
}

static double nowMs() {
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return ts.tv_sec * 1000.0 + ts.tv_nsec / 1e6;
}

// === IL2CPP scene-scan state ===============================================
// We resolve once (lazily) the CharacterActor class + Type object + the
// UnityEngine.Object.FindObjectsOfTypeAll(Type) method, then every ~500 ms we
// rebuild the actor list. This catches BOTH local and remote peds, including
// disabled CharacterActor instances on remote peds.
static bool s_il2cppTried = false;
static bool s_il2cppReady = false;
static void *s_charActorClass = nullptr;
static void *s_charActorTypeObj = nullptr;  // System.Type instance (boxed)
static void *s_findObjectsMethod = nullptr; // Object.FindObjectsOfTypeAll(Type)
static double s_lastScanMs = 0;
static double s_teleportGodModeTime = 0.0;
static std::vector<void *> s_scannedActors;

static void tryInitIl2cppScan() {
  if (s_il2cppReady)
    return;
  if (!g_il2cpp.Init()) {
    return;
  }
  // CharacterActor lives in Assembly-CSharp, namespace
  // Lightbug.CharacterControllerPro.Core.
  s_charActorClass = g_il2cpp.FindClassEverywhere(
      "Lightbug.CharacterControllerPro.Core", "CharacterActor");
  if (!s_charActorClass) {
    return;
  }
  s_charActorTypeObj = g_il2cpp.GetTypeObject(s_charActorClass);
  if (!s_charActorTypeObj) {
    return;
  }
  void *objClass = g_il2cpp.FindClassEverywhere("UnityEngine", "Object");
  if (!objClass) {
    return;
  }
  s_findObjectsMethod = g_il2cpp.GetMethod(objClass, "FindObjectsOfTypeAll", 1);
  if (!s_findObjectsMethod) {
    return;
  }
  s_il2cppReady = true;
  E_LOGI("IL2CPP scene-scan ready: class=%p type=%p method=%p",
         s_charActorClass, s_charActorTypeObj, s_findObjectsMethod);
}

// Calls UnityEngine.Object.FindObjectsOfTypeAll(typeof(CharacterActor)) and
// caches the resulting actor pointers. Throttled to ~2 Hz.
static void scanAllActors() {
  tryInitIl2cppScan();
  if (!s_il2cppReady)
    return;
  double t = nowMs();
  // Scan IL2CPP à ~1 Hz (1000ms). FindObjectsOfTypeAll est coûteux sur le
  // thread Unity → le limiter à 1 Hz élimine les freezes ESP.
  // Les positions sont mises à jour en temps réel par le sniffer réseau
  // (hook_PedDisplacement_Read) donc 1 Hz pour le scan d'acteurs suffit.
  if (t - s_lastScanMs < 1000.0 && !s_scannedActors.empty())
    return;
  s_lastScanMs = t;

  void *params[1] = {s_charActorTypeObj};
  void *arr = g_il2cpp.Invoke(s_findObjectsMethod, nullptr, params);
  if (!arr) {
    E_LOGE("FindObjectsOfTypeAll returned null");
    return;
  }
  uint32_t n = 0;
  if (g_il2cpp.array_length) {
    n = g_il2cpp.array_length(arr);
  } else {
    // Fallback to known IL2CPP array layout (size_t max_length at +0x18).
    n = (uint32_t)(*reinterpret_cast<size_t *>(
        reinterpret_cast<uint8_t *>(arr) + 0x18));
  }
  void **items = Il2CppArray_Items(arr);
  std::vector<void *> actors;
  actors.reserve(n);
  for (uint32_t i = 0; i < n; i++) {
    if (items[i])
      actors.push_back(items[i]);
  }
  pthread_mutex_lock(&s_mtx);
  s_scannedActors = std::move(actors);
  pthread_mutex_unlock(&s_mtx);
  static int dbg = 0;
  if ((dbg++ % 4) == 0) {
    E_LOGI("Scene scan: %u CharacterActor instance(s)", n);
  }
}

// === API ===
void Esp_Init(uintptr_t libBase) {
  s_libBase = libBase;

  // Hooks VehicleOcclusionObject (culling) pour lister tous les véhicules
  // actifs (V17 ECS bypass)
  extern void hook_VehicleOcclusionObject_OnEnable(void *self);
  extern void (*orig_VehicleOcclusionObject_OnEnable)(void *self);
  extern void hook_VehicleOcclusionObject_OnDisable(void *self);
  extern void (*orig_VehicleOcclusionObject_OnDisable)(void *self);

  if (RVA_VehicleOcclusionObject_OnEnable != 0x0) {
    DobbyHook((void *)(s_libBase + RVA_VehicleOcclusionObject_OnEnable),
              (dobby_dummy_func_t)hook_VehicleOcclusionObject_OnEnable,
              (dobby_dummy_func_t *)&orig_VehicleOcclusionObject_OnEnable);
  }
  if (RVA_VehicleOcclusionObject_OnDisable != 0x0) {
    DobbyHook((void *)(s_libBase + RVA_VehicleOcclusionObject_OnDisable),
              (dobby_dummy_func_t)hook_VehicleOcclusionObject_OnDisable,
              (dobby_dummy_func_t *)&orig_VehicleOcclusionObject_OnDisable);
  }
  if (libBase != 0) {
    fn_get_gameObject =
        (Component_get_gameObject_t)(libBase + RVA_GetComponentString);
    fn_get_name = (Object_get_name_t)(libBase + RVA_Object_GetName);
    fn_Camera_get_main = (Camera_get_main_t)(libBase + RVA_Camera_GetMain);
    fn_GetView =
        (GetMat_Injected_t)(libBase + RVA_Camera_GetWorldToCameraMatrix);
    fn_GetProj = (GetMat_Injected_t)(libBase + RVA_Camera_GetProjectionMatrix);
    fn_GetPixelW = (GetPixel_t)(libBase + RVA_Camera_GetPixelWidth);
    fn_GetPixelH = (GetPixel_t)(libBase + RVA_Camera_GetPixelHeight);
  }
  E_LOGI("Esp_Init base=0x%lx get_main=%p getView=%p getProj=%p go=%p name=%p",
         libBase, fn_Camera_get_main, fn_GetView, fn_GetProj, fn_get_gameObject,
         fn_get_name);
}

void Esp_SetScreenSize(int w, int h) {
  s_screenW = w;
  s_screenH = h;
}

void Esp_ScanTick() { scanAllActors(); }

void Esp_SetEnabled(bool en) {
  s_enabled = en;
  E_LOGI("ESP enabled=%d", (int)en);
}

bool Esp_IsEnabled() { return s_enabled; }

void Esp_RegisterActor(void *ca) {
  // Ne pas conditionner à s_enabled : les acteurs sont nécessaires
  // pour le TP, la vitesse, l'aimbot, etc. même si l'ESP visuel est OFF.
  if (!ca)
    return;
  pthread_mutex_lock(&s_mtx);
  s_actorsThisFrame.push_back(ca);
  pthread_mutex_unlock(&s_mtx);
}

// === Read transform.position from a Unity Component ===
typedef void *(*Component_get_transform_t)(void *self);
static Component_get_transform_t fn_get_transform = nullptr;
typedef void (*Transform_get_position_Injected_t)(void *self, V3 *out);
static Transform_get_position_Injected_t fn_get_position = nullptr;

typedef void *(*Transform_get_parent_t)(void *transform);
static Transform_get_parent_t fn_get_parent = nullptr;

typedef void *(*Transform_get_root_t)(void *transform);
static Transform_get_root_t fn_get_root = nullptr;

static void resolveLazy() {
  if (fn_get_transform && fn_get_position && fn_get_parent)
    return;
  if (!s_libBase)
    return;
  if (RVA_Component_GetTransform != 0x0) {
    fn_get_transform =
        (Component_get_transform_t)(s_libBase + RVA_Component_GetTransform);
  }
  // Transform.get_position_Injected @ RVA_Transform_GetPosition  <- READ world
  // position Transform.set_position_Injected @ RVA_Transform_SetPosition  <-
  // WRITE world position
  if (RVA_Transform_GetPosition != 0x0) {
    fn_get_position =
        (Transform_get_position_Injected_t)(s_libBase +
                                            RVA_Transform_GetPosition);
  }
  // fn_get_parent is critical for CustomGetRootTransform — resolve it here!
  if (RVA_Transform_GetParent != 0x0) {
    fn_get_parent =
        (Transform_get_parent_t)(s_libBase + RVA_Transform_GetParent);
  }
}

void Esp_NotifyLocalActor(void *ca) {
  if (!ca || !IsUnityObjectAlive(ca))
    return;
  if (s_localActor == ca) {
    s_lastLocalActorNotifyMs = nowMs();
    return;
  }

  resolveLazy();
  if (fn_get_transform && fn_get_position) {
    void *tr = fn_get_transform(ca);
    if (tr) {
      V3 pos = {0, 0, 0};
      fn_get_position(tr, &pos);
      if (s_camValid) {
        float dx = pos.x - s_camPos.x;
        float dy = pos.y - s_camPos.y;
        float dz = pos.z - s_camPos.z;
        float distSq = dx * dx + dy * dy + dz * dz;
        // Local player is always extremely close to the camera (now relaxed to 40m to allow zoomed out / driving)
        if (distSq < 1600.0f) {
          pthread_mutex_lock(&s_mtx);
          bool shouldUpdate = false;
          if (!s_localActor) {
            shouldUpdate = true;
          } else {
            void *currTr = fn_get_transform(s_localActor);
            if (currTr) {
              V3 currPos = {0, 0, 0};
              fn_get_position(currTr, &currPos);
              float cdx = currPos.x - s_camPos.x;
              float cdy = currPos.y - s_camPos.y;
              float cdz = currPos.z - s_camPos.z;
              float currDistSq = cdx * cdx + cdy * cdy + cdz * cdz;
              if (distSq < currDistSq) {
                shouldUpdate = true;
              }
            } else {
              shouldUpdate = true;
            }
          }
          if (shouldUpdate) {
            E_LOGI("LocalActor identified via PreSimulationUpdate (Camera dist "
                   "%.1fm): %p",
                   sqrtf(distSq), ca);
            s_localActor = ca;
            s_lastLocalActorNotifyMs = nowMs();
          }
          pthread_mutex_unlock(&s_mtx);
        }
      }
    }
  }
}

static bool readActorPosition(void *actor, V3 *out) {
  resolveLazy();
  if (!fn_get_transform || !fn_get_position || !actor)
    return false;
  if (!IsUnityObjectAlive(actor))
    return false;
  void *tr = fn_get_transform(actor);
  if (!tr || !IsUnityObjectAlive(tr))
    return false;
  V3 p = {0, 0, 0};
  fn_get_position(tr, &p);
  *out = p;
  return true;
}

// === Update camera state by calling Unity Camera.main ===
static void updateCameraState() {
  if (!fn_Camera_get_main) {
    s_camValid = false;
    return;
  }
  void *cam = fn_Camera_get_main();
  if (!cam || !IsUnityObjectAlive(cam)) {
    s_camValid = false;
    return;
  }
  V3 cp;
  if (!readActorPosition(cam, &cp)) {
    s_camValid = false;
    return;
  }
  s_camPos = cp;
  s_camValid = true;
}

// === Frame tick: called from end of CharacterActor.PreSimulationUpdate hook
// ===
static double s_lastTickMs = 0;

typedef void (*PhysicsActor_SetPosition_t)(void *self, float x, float y,
                                           float z, float dx, float dy,
                                           float dz);
typedef void (*CharacterActor_SweepAndTeleport_t)(void *self, float x, float y,
                                                  float z);
typedef void (*CharacterActor_Teleport_t)(void *self, float x, float y,
                                          float z);
typedef void (*Transform_set_position_Injected_t)(void *self, V3 *value);
typedef void (*Rigidbody_set_position_Injected_t)(void *self, V3 *value);
typedef void (*Rigidbody_set_velocity_Injected_t)(void *self, V3 *value);
typedef void (*Rigidbody_set_angularVelocity_Injected_t)(void *self, V3 *value);

static PhysicsActor_SetPosition_t fn_set_position = nullptr;
static CharacterActor_SweepAndTeleport_t fn_sweep_tp = nullptr;
static CharacterActor_Teleport_t fn_teleport = nullptr;
static Transform_set_position_Injected_t fn_transform_setpos = nullptr;
static Rigidbody_set_position_Injected_t fn_rigid_setpos = nullptr;
static Rigidbody_set_velocity_Injected_t fn_rigid_setvel = nullptr;
static Rigidbody_set_angularVelocity_Injected_t fn_rigid_setangvel = nullptr;

void *g_LastLocalVehicle = nullptr;

static void resolveTeleport() {
  if (fn_transform_setpos && fn_rigid_setpos && (fn_teleport || fn_sweep_tp) &&
      fn_rigid_setvel && fn_get_parent)
    return;
  if (!s_libBase)
    return;

  if (RVA_Transform_SetPosition != 0x0) {
    fn_transform_setpos =
        (Transform_set_position_Injected_t)(s_libBase +
                                            RVA_Transform_SetPosition);
  }
  if (RVA_Transform_GetPosition != 0x0) {
    fn_get_position =
        (Transform_get_position_Injected_t)(s_libBase +
                                            RVA_Transform_GetPosition);
  }
  if (RVA_Transform_GetParent != 0x0) {
    fn_get_parent =
        (Transform_get_parent_t)(s_libBase + RVA_Transform_GetParent);
  }
  if (RVA_Transform_GetRoot != 0x0) {
    fn_get_root = (Transform_get_root_t)(s_libBase + RVA_Transform_GetRoot);
  }
  // PhysicsActor.SetPosition causes a crash in FixedUpdate due to ABI mismatch.
  // We rely entirely on SweepAndTeleport for both vehicle and player.
  fn_set_position = nullptr;
  if (RVA_SweepAndTeleport != 0x0) {
    fn_sweep_tp =
        (CharacterActor_SweepAndTeleport_t)(s_libBase + RVA_SweepAndTeleport);
  }
  if (RVA_CharacterActor_Teleport != 0x0) {
    fn_teleport =
        (CharacterActor_Teleport_t)(s_libBase + RVA_CharacterActor_Teleport);
  }
  if (RVA_Rigidbody_SetPosition != 0x0) {
    fn_rigid_setpos =
        (Rigidbody_set_position_Injected_t)(s_libBase +
                                            RVA_Rigidbody_SetPosition);
  }
  if (RVA_Rigidbody_SetVelocity != 0x0) {
    fn_rigid_setvel =
        (Rigidbody_set_velocity_Injected_t)(s_libBase +
                                            RVA_Rigidbody_SetVelocity);
  }
  if (RVA_Rigidbody_SetAngularVelocity != 0x0) {
    fn_rigid_setangvel =
        (Rigidbody_set_angularVelocity_Injected_t)(s_libBase +
                                                   RVA_Rigidbody_SetAngularVelocity);
  }
}

// Variables globales pour le teleport véhicule asynchrone
void *g_PendingVehicleRoot = nullptr;
V3 g_PendingVehicleTeleportPos = {0, 0, 0};
bool g_PendingVehicleTeleport = false;

static bool s_autoFollowCar = false;

// === Sticky Car Mode: colle la voiture sur la cible en continu ===
static void *g_StickyCarTarget = nullptr; // actor cible
bool g_StickyCarEnabled = false;          // activé depuis VipMenu

// Indique si le joueur local est actuellement dans un véhicule
bool g_PlayerInVehicle = false;

// === Waypoint stocké pour TP CARTE même en voiture ===

#include <atomic>

static std::atomic<bool> g_isPatchingCheckpoint{false};

extern "C" void Esp_SetPendingWaypoint(float x, float y, float z, bool valid);

void PatchCheckpointInMemory_REMOVED(V3 oldPos, V3 newPos) {
  if (oldPos.x == 0.0f && oldPos.y == 0.0f && oldPos.z == 0.0f)
    return;
  if (newPos.x == 0.0f && newPos.y == 0.0f && newPos.z == 0.0f)
    return;

  float dx = oldPos.x - newPos.x;
  float dy = oldPos.y - newPos.y;
  float dz = oldPos.z - newPos.z;
  if (dx * dx + dy * dy + dz * dz < 1.0f)
    return;

  if (g_isPatchingCheckpoint)
    return;
  g_isPatchingCheckpoint = true;

  // Faire le patch de manière synchrone pour éviter le crash du thread detached
  std::vector<KittyMemory::ProcMap> rwMaps;
  std::vector<KittyMemory::ProcMap> allMaps = KittyMemory::getAllMaps();
  for (const auto &map : allMaps) {
    if (map.readable && map.writeable && !map.executable && map.is_private) {
      if (map.isUnknown() || map.pathname.find("anon") != std::string::npos ||
          map.pathname.find("ashmem") != std::string::npos ||
          map.pathname.find("malloc") != std::string::npos ||
          map.pathname.find("dalvik") != std::string::npos ||
          map.pathname.find("libunity") != std::string::npos ||
          map.pathname.find("libil2cpp") != std::string::npos) {
        rwMaps.push_back(map);
      }
    }
  }

  float targetData[3] = {oldPos.x, oldPos.y, oldPos.z};
  float replacementData[3] = {newPos.x, newPos.y, newPos.z};

  int replacedCount = 0;
  const size_t CHUNK_SIZE = 131072;
  std::vector<char> buffer(CHUNK_SIZE);

  for (const auto &map : rwMaps) {
    if (map.length < sizeof(targetData))
      continue;

    uintptr_t current = map.startAddress;
    uintptr_t end = map.endAddress;

    while (current + sizeof(targetData) <= end) {
      size_t remaining = end - current;
      size_t toRead = std::min(CHUNK_SIZE, remaining);

      size_t bytesRead =
          KittyMemory::syscallMemRead((void *)current, buffer.data(), toRead);
      if (bytesRead >= sizeof(targetData)) {
        for (size_t offset = 0; offset + sizeof(targetData) <= bytesRead;
             offset += 4) {
          float *values = (float *)(buffer.data() + offset);
          if (values[0] == targetData[0] && values[1] == targetData[1] &&
              values[2] == targetData[2]) {
            uintptr_t targetAddr = current + offset;
            if (KittyMemory::syscallMemWrite(
                    (void *)targetAddr, replacementData,
                    sizeof(replacementData)) == sizeof(replacementData)) {
              replacedCount++;
            }
          }
        }
      }

      if (toRead > sizeof(targetData)) {
        current += (toRead - sizeof(targetData) + 1);
      } else {
        break;
      }
    }
  }

  if (replacedCount > 0) {
    E_LOGI("[CheckpointPatch] Found and replaced %d occurrences synchronously!",
           replacedCount);
  }

  g_isPatchingCheckpoint = false;
}

static V3 s_pendingWaypoint = {0.0f, 0.0f, 0.0f};
static bool s_hasPendingWaypoint = false;
extern "C" void Esp_SetPendingWaypoint(float x, float y, float z, bool valid) {
  s_pendingWaypoint = {x, y, z};
  s_hasPendingWaypoint = valid;
}
extern "C" void *g_LastFindGoalInstance;

extern "C" void Esp_ForceRapatrierMarqueurs() {
  extern V3 g_LastExecutedWaypoint;
  float x = s_pendingWaypoint.x;
  float y = s_pendingWaypoint.y;
  float z = s_pendingWaypoint.z;
  bool valid = s_hasPendingWaypoint;
  if (!valid && g_LastExecutedWaypoint.x != 0.0f) {
    x = g_LastExecutedWaypoint.x;
    y = g_LastExecutedWaypoint.y;
    z = g_LastExecutedWaypoint.z;
    valid = true;
  }

  // Strategy 1: use the FindGoal hook instance (only available if a job
  // waypoint was already generated)
  if (valid && g_LastFindGoalInstance &&
      IsValidMemoryFast(g_LastFindGoalInstance, sizeof(void *))) {
    typedef bool (*FindGoal_t)(void *_this, int playerDimension, V3 position,
                               V3 *goal, void *goalEntity, void *method);
    static FindGoal_t fn_FindGoal = nullptr;
    if (!fn_FindGoal && s_libBase && RVA_FindGoal != 0x0) {
      fn_FindGoal = (FindGoal_t)(s_libBase + RVA_FindGoal);
    }
    if (fn_FindGoal) {
      V3 playerPos = {0, 0, 0};
      typedef void *(*Component_get_transform_t)(void *self);
      typedef void (*Transform_get_position_t)(void *self, V3 *out);
      static Component_get_transform_t local_get_transform = nullptr;
      static Transform_get_position_t local_get_position = nullptr;
      if (!local_get_transform && s_libBase) {
        local_get_transform =
            (Component_get_transform_t)(s_libBase + 0x423D17C);
      }
      if (!local_get_position && s_libBase) {
        local_get_position = (Transform_get_position_t)(s_libBase + 0x43063AC);
      }
      if (s_localActor && local_get_transform && local_get_position) {
        void *actorTr = local_get_transform(s_localActor);
        local_get_position(actorTr, &playerPos);
        V3 goal = {0, 0, 0};
        fn_FindGoal(g_LastFindGoalInstance, 0, playerPos, &goal, nullptr,
                    nullptr);
        E_LOGI("ForceRapatrierMarqueurs: called FindGoal at player pos (%.1f, "
               "%.1f, %.1f)",
               playerPos.x, playerPos.y, playerPos.z);
        return; // success
      }
    }
  }

  // Strategy 2: force-activate DragCheckpointToPlayer for a short burst so that
  // when the game's next FindGoal call occurs, the checkpoint snaps to the
  // player. This works even before any manual waypoint exists.
  extern bool g_DragCheckpointToPlayer;
  if (!g_DragCheckpointToPlayer) {
    E_LOGI("ForceRapatrierMarqueurs: no FindGoal instance yet, activating drag "
           "burst");
    g_DragCheckpointToPlayer = true;
    // The main game loop will call FindGoal shortly; the next hook invocation
    // will snap the checkpoint and then we reset the flag after a short delay.
    // Schedule auto-disable after 2 seconds via a simple timestamp.
    extern double g_DragBurstEndTime;
    g_DragBurstEndTime = nowMs() + 2000.0;
  }
}

extern "C" float g_HealLocalAmount;

extern "C" void Esp_HealLocal(float amount) {
  g_HealLocalAmount = amount;
  E_LOGI("Esp_HealLocal: Requested heal to %.1f HP", amount);
}

extern "C" void Esp_GetPendingWaypoint(float *x, float *y, float *z,
                                       bool *valid) {
  if (x)
    *x = s_pendingWaypoint.x;
  if (y)
    *y = s_pendingWaypoint.y;
  if (z)
    *z = s_pendingWaypoint.z;
  if (valid)
    *valid = s_hasPendingWaypoint;
}

typedef int (*Transform_get_childCount_t)(void *self);
typedef void *(*Transform_GetChild_t)(void *self, int index);
typedef void *(*Component_GetComponentString_t)(void *self, void *typeString,
                                                void *method);

static Transform_get_childCount_t fn_get_childCount = nullptr;
static Transform_GetChild_t fn_GetChild = nullptr;
static Component_GetComponentString_t fn_GetComponentString = nullptr;

void *FindRigidbodyRecursively(void *transform) {
  if (!transform || !IsUnityObjectAlive(transform))
    return nullptr;

  void *actorTr = nullptr;
  if (s_localActor && fn_get_transform) {
    actorTr = fn_get_transform(s_localActor);
  }
  if (transform == actorTr) {
    return nullptr; // Players don't use Rigidbody for mod menu teleportation
  }

  // Use a thread-safe cache to avoid repeatedly searching the hierarchy.
  struct CachedRb {
    void *transform;
    void *rigidbody;
  };
  static std::vector<CachedRb> s_rbCache;
  static pthread_mutex_t s_rbMtx = PTHREAD_MUTEX_INITIALIZER;
  static double s_lastCleanupMs = 0.0;

  pthread_mutex_lock(&s_rbMtx);

  // Periodic cleanup: only validate cached entries every 5 seconds,
  // NOT on every call. This avoids per-frame mincore() overhead for stale
  // entries.
  double nowMsVal = nowMs();
  if (nowMsVal - s_lastCleanupMs > 5000.0) {
    s_lastCleanupMs = nowMsVal;
    for (auto it = s_rbCache.begin(); it != s_rbCache.end();) {
      if (!IsUnityObjectAlive(it->transform) ||
          !IsUnityObjectAlive(it->rigidbody)) {
        it = s_rbCache.erase(it);
      } else {
        ++it;
      }
    }
  }

  // Fast lookup in cache (no liveness check — cleanup is periodic)
  for (const auto &entry : s_rbCache) {
    if (entry.transform == transform) {
      void *rb = entry.rigidbody;
      pthread_mutex_unlock(&s_rbMtx);
      return rb;
    }
  }
  pthread_mutex_unlock(&s_rbMtx);

  if (!fn_GetComponentString || !fn_get_childCount || !fn_GetChild ||
      !g_il2cpp.string_new)
    return nullptr;

  void *strObj = g_il2cpp.string_new("Rigidbody");
  if (strObj) {
    void *rb = fn_GetComponentString(transform, strObj, nullptr);
    if (rb && IsUnityObjectAlive(rb)) {
      pthread_mutex_lock(&s_rbMtx);
      // Double check it wasn't added by another thread
      bool exists = false;
      for (const auto &entry : s_rbCache) {
        if (entry.transform == transform) {
          exists = true;
          break;
        }
      }
      if (!exists)
        s_rbCache.push_back({transform, rb});
      pthread_mutex_unlock(&s_rbMtx);
      return rb;
    }
  }

  int count = fn_get_childCount(transform);
  for (int i = 0; i < count; i++) {
    void *child = fn_GetChild(transform, i);
    if (child && IsUnityObjectAlive(child)) {
      void *rb = FindRigidbodyRecursively(
          child); // Recursive call MUST be outside mutex to prevent deadlock
      if (rb) {
        pthread_mutex_lock(&s_rbMtx);
        bool exists = false;
        for (const auto &entry : s_rbCache) {
          if (entry.transform == transform) {
            exists = true;
            break;
          }
        }
        if (!exists)
          s_rbCache.push_back({transform, rb});
        pthread_mutex_unlock(&s_rbMtx);
        return rb;
      }
    }
  }
  return nullptr;
}

void Esp_DirectVehicleTP(void *root, float x, float y, float z,
                         bool playerInside, V3 *customVel) {
  if (!root)
    return;

  V3 targetPos = {x, y, z};
  if (playerInside) {
    s_teleportSpamFrames = 30;
    s_teleportSpamPos = targetPos;
    s_teleportSpamVel = customVel ? *customVel : V3{0, 0, 0};
  }

  E_LOGI("VehicleTP EXEC: root=%p target=(%.2f,%.2f,%.2f) playerInside=%d",
         root, x, y, z, (int)playerInside);

  if (!g_il2cpp.ready)
    g_il2cpp.Init();

  if (!fn_GetComponentString && s_libBase)
    if (RVA_GetComponentString != 0x0) {
      fn_GetComponentString =
          (Component_GetComponentString_t)(s_libBase + RVA_GetComponentString);
    }
  if (!fn_get_childCount && s_libBase)
    if (RVA_Transform_GetChildCount != 0x0) {
      fn_get_childCount =
          (Transform_get_childCount_t)(s_libBase + RVA_Transform_GetChildCount);
    }
  if (!fn_GetChild && s_libBase)
    if (RVA_Transform_GetChild != 0x0) {
      fn_GetChild = (Transform_GetChild_t)(s_libBase + RVA_Transform_GetChild);
    }

  // 1) Teleport Transform (visual + collider basis)
  typedef void (*Transform_set_position_Injected_t)(void *self, V3 *value);
  static Transform_set_position_Injected_t fn_transform_setpos = nullptr;
  if (!fn_transform_setpos && s_libBase)
    if (RVA_Transform_SetPosition != 0x0) {
      fn_transform_setpos =
          (Transform_set_position_Injected_t)(s_libBase +
                                              RVA_Transform_SetPosition);
    }

  if (fn_transform_setpos) {
    fn_transform_setpos(root, &targetPos);
    E_LOGI("VehicleTP: Transform.set_position OK");
  }

  // 2) Teleport Rigidbody (Physics/Network) - crucial for server-side position
  typedef void (*Rigidbody_set_position_Injected_t)(void *self, V3 *value);
  static Rigidbody_set_position_Injected_t fn_rigid_setpos = nullptr;
  if (!fn_rigid_setpos && s_libBase)
    if (RVA_Rigidbody_SetPosition != 0x0) {
      fn_rigid_setpos =
          (Rigidbody_set_position_Injected_t)(s_libBase +
                                              RVA_Rigidbody_SetPosition);
    }

  bool rbMoved = false;
  void *rigid = FindRigidbodyRecursively(root);
  if (rigid && fn_rigid_setpos) {
    fn_rigid_setpos(rigid, &targetPos);

    V3 zeroVal = {0.0f, 0.0f, 0.0f};
    V3 velVal = customVel ? *customVel : zeroVal;
    if (fn_rigid_setvel) {
      fn_rigid_setvel(rigid, &velVal);
    }
    if (fn_rigid_setangvel) {
      fn_rigid_setangvel(rigid, &zeroVal);
    }

    rbMoved = true;
    E_LOGI(
        "VehicleTP: Rigidbody.set_position & velocities cleared OK (rigid=%p)",
        rigid);
  } else {
    E_LOGI("VehicleTP FAILED to find Rigidbody in hierarchy!");
  }

  // Force Unity Physics engine to update colliders to the new transform
  typedef void (*Physics_SyncTransforms_t)();
  static Physics_SyncTransforms_t fn_sync_transforms = nullptr;
  if (!fn_sync_transforms && s_libBase)
    if (RVA_Physics_SyncTransforms != 0x0) {
      fn_sync_transforms =
          (Physics_SyncTransforms_t)(s_libBase + RVA_Physics_SyncTransforms);
    }
  if (fn_sync_transforms)
    fn_sync_transforms();

  E_LOGI("VehicleTP DONE: root=%p rbMoved=%d final=(%.1f, %.1f, %.1f)", root,
         (int)rbMoved, x, y, z);
}

int g_HeistFarmType = 0;
extern float g_HeistFarmDelay;

void ProcessHeistFarm() {
  if (g_HeistFarmType == 0)
    return;
  double now = nowMs();
  static double lastHeistFarmTime = 0;
  static int heistFarmStep = 0;

  if (now - lastHeistFarmTime < (double)(g_HeistFarmDelay * 1000.f))
    return; // custom delay in ms
  lastHeistFarmTime = now;

  if (g_HeistFarmType == 1) { // Harbor Heist (22 steps cycle)
    struct Waypoint {
      float x, y, z;
    };
    static const Waypoint harborSteps[] = {
        {2833.92f, 10.00f, 143.283f}, {2661.549f, 10.010f, 81.918f},
        {2779.09f, 10.01f, -68.98f},  {2661.549f, 10.010f, 81.918f},
        {2891.01f, 10.01f, 71.82f},   {2661.549f, 10.010f, 81.918f},
        {3023.64f, 10.01f, 224.711f}, {2661.549f, 10.010f, 81.918f},
        {2833.51f, 10.00f, 143.165f}, {2661.549f, 10.010f, 81.918f},
        {3023.71f, 10.01f, 224.788f}, {2661.549f, 10.010f, 81.918f},
        {2890.69f, 10.01f, 72.27f},   {2661.549f, 10.010f, 81.918f},
        {2834.82f, 10.00f, 144.582f}, {2661.549f, 10.010f, 81.918f},
        {2779.59f, 10.01f, -69.55f},  {2661.549f, 10.010f, 81.918f},
        {2833.82f, 10.00f, 143.842f}, {2661.549f, 10.010f, 81.918f},
        {2778.98f, 10.01f, -68.89f},  {2661.549f, 10.010f, 81.918f}};
    const int totalSteps = sizeof(harborSteps) / sizeof(harborSteps[0]);
    if (heistFarmStep >= totalSteps) {
      heistFarmStep = 0;
    }
    Waypoint wp = harborSteps[heistFarmStep];
    Teleport_ToPosition(wp.x, wp.y, wp.z, true);
    heistFarmStep++;
    if (heistFarmStep >= totalSteps) {
      heistFarmStep = 0;
    }
  } else if (g_HeistFarmType == 2) { // Arsenal Raid (Gangs)
    if (heistFarmStep == 0) {
      Teleport_ToPosition(1316.0f, 59.38f, 3296.928f, true);
      heistFarmStep = 1;
    } else if (heistFarmStep == 1) {
      Teleport_ToPosition(1620.730f, 44.650f, 3187.800f, true);
      heistFarmStep = 2;
    } else if (heistFarmStep == 2) {
      Teleport_ToPosition(1708.742f, 43.056f, 3309.390f, true);
      heistFarmStep = 3;
    } else if (heistFarmStep == 3) {
      Teleport_ToPosition(1316.0f, 59.38f, 3296.928f, true);
      heistFarmStep = 4;
    } else if (heistFarmStep == 4) {
      Teleport_ToPosition(1459.240f, 51.554f, 3055.512f, true);
      heistFarmStep = 5;
    } else {
      Teleport_ToPosition(1316.0f, 59.38f, 3296.928f, true);
      heistFarmStep = 0;
    }
  }
}

// Atomic re-entrancy guard: prevents the FrameTick from being called
// RECURSIVELY when hook_Rigidbody_SetVelocity -> orig_Rigidbody -> libil2cpp
// -> EventSystem_Update hook re-fires Esp_FrameTick on the same thread.
// This was the root cause of the SIGSEGV crash (double vector allocation /
// re-entrant heap access in FindRigidbodyRecursively).
static std::atomic_flag s_frameTickRunning = ATOMIC_FLAG_INIT;

void Esp_FrameTick() {
  // Non-blocking re-entrancy check: if already running, skip this call entirely
  if (s_frameTickRunning.test_and_set(std::memory_order_acquire)) {
    return; // Prevent recursive re-entry on same thread
  }
  struct FrameTickGuard {
    ~FrameTickGuard() { s_frameTickRunning.clear(std::memory_order_release); }
  } _guard;
  if (s_teleportGodModeTime > 0.0) {
    extern double Esp_GetTimeMs();
    if (Esp_GetTimeMs() > s_teleportGodModeTime) {
      s_teleportGodModeTime = 0.0;
      extern bool g_GodModeEnabled;
      if (!g_GodModeEnabled) {

        SetGodModePatchesState(false);
      }
    }
  }
  extern bool Aimbot_IsEnabled();
  extern bool g_TpCarteToggle;
  extern bool g_StickyCarEnabled;
  extern bool g_MeuDestinoEnabled;
  extern bool g_NoClipEnabled;
  extern int g_SkinReplaceVal;
  extern int g_VehicleReplaceVal;
  extern int g_WeaponReplaceVal;

  extern bool Esp_IsAutoFollowActive();
  extern bool Esp_IsAutoFollowCarActive();
  extern bool g_hasPendingTeleportRequest;
  extern float g_pendingTeleportX;
  extern float g_pendingTeleportY;
  extern float g_pendingTeleportZ;

  extern bool g_DragCheckpointToPlayer;
  bool anyFeature = g_DragCheckpointToPlayer || s_enabled ||
                    Aimbot_IsEnabled() || g_TpCarteToggle ||
                    g_StickyCarEnabled || g_MeuDestinoEnabled ||
                    g_NoClipEnabled || (g_SkinReplaceVal != 0) ||
                    (g_VehicleReplaceVal != 0) || (g_WeaponReplaceVal != 0) ||
                    Esp_IsAutoFollowActive() || Esp_IsAutoFollowCarActive() ||
                    g_hasPendingTeleportRequest || (g_HeistFarmType != 0);
  if (!anyFeature)
    return;

  resolveLazy();

  if (g_hasPendingTeleportRequest) {
    g_hasPendingTeleportRequest = false;
    Teleport_ToPosition(g_pendingTeleportX, g_pendingTeleportY,
                        g_pendingTeleportZ);
  }

  ProcessHeistFarm();

  extern void UpdateLastLocalVehicle();
  UpdateLastLocalVehicle();
  double t = nowMs();

  // === LocalActor Timeout Check (Death / Respawn detector) ===
  if (s_localActor && !IsUnityObjectAlive(s_localActor)) {
    E_LOGI("s_localActor destroyed. Resetting pointer %p", s_localActor);
    s_localActor = nullptr;
    g_LastLocalVehicle = nullptr;
    g_PlayerInVehicle = false;
  }
  if (t - s_lastTickMs < 10.0)
    return; // 100 Hz max throttle
  s_lastTickMs = t;

  // === Per-frame local vehicle tracking & teleport spam ===
  if (s_teleportSpamFrames > 0 && g_LastLocalVehicle &&
      IsUnityObjectAlive(g_LastLocalVehicle)) {
    s_teleportSpamFrames--;

    // Dynamically update s_teleportSpamPos to match the player's actual
    // grounded position!
    if (s_localActor && fn_get_transform && fn_get_position) {
      void *actorTr = fn_get_transform(s_localActor);
      if (actorTr) {
        V3 pPos = {0, 0, 0};
        fn_get_position(actorTr, &pPos);
        float dx = pPos.x - s_teleportSpamPos.x;
        float dy = pPos.y - s_teleportSpamPos.y;
        float dz = pPos.z - s_teleportSpamPos.z;
        float distSq = dx * dx + dy * dy + dz * dz;
        // Only allow height adjustments if we have arrived near target (prevents snapping back to old position)
        if (distSq < 100.0f) {
          if (abs(pPos.y - s_teleportSpamPos.y) > 0.1f) {
            s_teleportSpamPos = pPos;
            s_teleportSpamPos.y += 0.5f; // Keep car at ground level
          }
        }
      }
    }

    s_teleportSpamPos.x += s_teleportSpamVel.x * 0.01f;
    s_teleportSpamPos.y += s_teleportSpamVel.y * 0.01f;
    s_teleportSpamPos.z += s_teleportSpamVel.z * 0.01f;
    Esp_DirectVehicleTP(g_LastLocalVehicle, s_teleportSpamPos.x,
                        s_teleportSpamPos.y, s_teleportSpamPos.z, false,
                        &s_teleportSpamVel);

    // Keep player character position synchronized during the spam loop
    if (s_localActor) {
      if (fn_teleport) {
        fn_teleport(s_localActor, s_teleportSpamPos.x, s_teleportSpamPos.y,
                    s_teleportSpamPos.z);
      } else if (fn_sweep_tp) {
        fn_sweep_tp(s_localActor, s_teleportSpamPos.x, s_teleportSpamPos.y,
                    s_teleportSpamPos.z);
      }
    }
  }

  // === Sticky Car: colle la voiture sur la cible chaque frame ===
  if (g_StickyCarEnabled && g_LastLocalVehicle &&
      IsUnityObjectAlive(g_LastLocalVehicle) && g_StickyCarTarget &&
      IsUnityObjectAlive(g_StickyCarTarget) && fn_get_transform &&
      fn_get_position) {
    // Désactiver le spam loop de TP de départ s'il tourne encore pour éviter
    // tout conflit
    s_teleportSpamFrames = 0;

    void *tgtTr = fn_get_transform(g_StickyCarTarget);
    if (tgtTr && IsUnityObjectAlive(tgtTr)) {
      void *tgtRoot = CustomGetRootTransform(tgtTr);
      if (tgtRoot && tgtRoot != tgtTr) {
        // Target player has entered a vehicle. Skip teleporting to let them
        // drive/sit normally, and turn off sticky car immediately so it doesn't
        // despawn/desync on exit.
        g_StickyCarEnabled = false;
        E_LOGI("StickyCar auto-disabled: target entered vehicle");
      } else {
        V3 tgtPos = {0.0f, 0.0f, 0.0f};
        fn_get_position(tgtTr, &tgtPos);

        V3 vehPos = {0.0f, 0.0f, 0.0f};
        fn_get_position(g_LastLocalVehicle, &vehPos);

        float dx = tgtPos.x - vehPos.x;
        float dy = tgtPos.y - vehPos.y;
        float dz = tgtPos.z - vehPos.z;
        float distSq = dx * dx + dy * dy + dz * dz;

        if (distSq >= 0.25f) { // 0.5m squared
          V3 zero = {0.0f, 0.0f, 0.0f};
          Esp_DirectVehicleTP(g_LastLocalVehicle, tgtPos.x, tgtPos.y, tgtPos.z,
                              false, &zero);
        }
      }
    }
  }

  // Execute AimLock in the EventSystem Update loop to ensure it runs every
  // frame (Moved to CameraPlacementSystem::OnUpdate hook to prevent wobbly
  // rendering glitches) extern void Esp_TickAimLock(); Esp_TickAimLock();

  // === TP CARTE fallback pour voiture: uniquement quand on conduit ===
  // (si FindGoal n'est pas appelé par le moteur pendant la conduite)
  extern bool g_TpCarteToggle;
  extern bool g_PlayerInVehicle;
  if (g_TpCarteToggle && g_PlayerInVehicle && g_LastLocalVehicle &&
      s_hasPendingWaypoint && fn_get_position) {
    static double s_lastCarteTpMs = 0;
    if (t - s_lastCarteTpMs > 500.0) { // tentative toutes les 500ms
      V3 vehPos = {0.0f, 0.0f, 0.0f};
      fn_get_position(g_LastLocalVehicle, &vehPos);
      float vdx = s_pendingWaypoint.x - vehPos.x;
      float vdz = s_pendingWaypoint.z - vehPos.z;
      float vdist2D = vdx * vdx + vdz * vdz;
      if (vdist2D > 0.1f) { // TP exact instead of 5m offset
        s_lastCarteTpMs = t;

        void *actor = s_localActor;
        float targetY = s_pendingWaypoint.y;
        // Player sweep TP removed: do not teleport the player body individually
        // when inside a vehicle

        Esp_DirectVehicleTP(g_LastLocalVehicle, s_pendingWaypoint.x, targetY,
                            s_pendingWaypoint.z, true, nullptr);
      }
    }
  }

  // Skin/Vehicle/Weapon replacements are applied in
  // PedStreamingProvider.Construct / VehiclesStreamingProvider.Construct /
  // PedWeaponCreateSystem.OnUpdate hooks (Main.cpp). No runtime
  // Deconstruct/Construct here to avoid ECS crash on unvalidated offsets.

  UpdateLastLocalVehicle();

  // === AUTO-FOLLOW : téléporte le local au-dessus de la cible aimbot ===
  extern void Esp_TickAutoFollow();
  Esp_TickAutoFollow();

  updateCameraState();

  // IMPORTANT: scanAllActors() (FindObjectsOfTypeAll) est INTERDIT sur le
  // render thread (hook_FireOnPreCull) car c'est un appel IL2CPP lourd qui
  // bloque le render thread → FREEZE complet du jeu.
  // On utilise UNIQUEMENT les actors enregistrés via Esp_RegisterActor()
  // depuis hook_PreSimulationUpdate (game thread, thread-safe).

  pthread_mutex_lock(&s_mtx);
  std::vector<void *> actors = s_actorsThisFrame;
  // On garde aussi le snapshot précédent (scannedActors) pour les frames
  // où PreSimulationUpdate n'a pas tourné (ex: joueur en voiture).
  for (void *a : s_scannedActors) {
    bool found = false;
    for (void *b : actors)
      if (a == b) {
        found = true;
        break;
      }
    if (!found)
      actors.push_back(a);
  }
  s_actorsThisFrame.clear();
  pthread_mutex_unlock(&s_mtx);

  static int dbgCnt = 0;
  if ((dbgCnt++ % 30) == 0) {
    E_LOGI(
        "FrameTick actors=%zu camValid=%d camPos=(%.1f,%.1f,%.1f) screen=%dx%d",
        actors.size(), (int)s_camValid, s_camPos.x, s_camPos.y, s_camPos.z,
        s_screenW, s_screenH);
  }

  // Build camera matrices if available, but DON'T abort if camera is missing.
  // The snapshot (world positions, health, actor ptrs) is valid without a
  // camera. onScreen will be false for all entries when no camera → aimbot uses
  // allowOffscreen=true.
  bool haveCam = false;
  void *cam = nullptr;
  M4 vp{};
  int pixW = s_screenW, pixH = s_screenH;

  if (s_camValid && fn_Camera_get_main && fn_GetView && fn_GetProj) {
    cam = fn_Camera_get_main();
    if (cam && IsUnityObjectAlive(cam)) {
      M4 view{}, proj{};
      fn_GetView(cam, &view);
      fn_GetProj(cam, &proj);

      static int matrixDbg = 0;
      if ((matrixDbg++ % 60) == 0) {
        E_LOGI("View matrix: [%.4f, %.4f, %.4f, %.4f]", view.m[0], view.m[1],
               view.m[2], view.m[3]);
        E_LOGI("Proj matrix: [%.4f, %.4f, %.4f, %.4f]", proj.m[0], proj.m[1],
               proj.m[2], proj.m[3]);
      }

      if (fn_GetPixelW && fn_GetPixelH) {
        int w = fn_GetPixelW(cam);
        int h = fn_GetPixelH(cam);
        if (w > 0 && h > 0) {
          pixW = w;
          pixH = h;
        }
      }

      auto mmul = [](const M4 &A, const M4 &B, M4 &R) {
        for (int c = 0; c < 4; c++)
          for (int r = 0; r < 4; r++) {
            float s = 0.f;
            for (int k = 0; k < 4; k++)
              s += A.m[r + k * 4] * B.m[k + c * 4];
            R.m[r + c * 4] = s;
          }
      };
      mmul(proj, view, vp);
      haveCam = true;
    }
  }

  auto worldToScreen = [&](const V3 &w, float &sx, float &sy,
                           float &sd) -> bool {
    if (!haveCam) {
      sx = 0;
      sy = 0;
      sd = 0;
      return false;
    }
    float cx = vp.m[0] * w.x + vp.m[4] * w.y + vp.m[8] * w.z + vp.m[12];
    float cy = vp.m[1] * w.x + vp.m[5] * w.y + vp.m[9] * w.z + vp.m[13];
    float cw = vp.m[3] * w.x + vp.m[7] * w.y + vp.m[11] * w.z + vp.m[15];
    if (cw <= 0.001f) {
      sd = cw;
      return false;
    }
    float ndcX = cx / cw;
    float ndcY = cy / cw;
    sx = (ndcX * 0.5f + 0.5f) * (float)pixW;
    sy = (1.0f - (ndcY * 0.5f + 0.5f)) * (float)pixH;
    sd = cw;
    return true;
  };

  std::vector<EspPed> out;
  out.reserve(actors.size());

  // Find local: smallest distance to camera
  int localIdx = -1;
  float minDist = 1e9f;

  // First pass: positions + distances
  struct Tmp {
    V3 pos;
    float dist;
  };
  std::vector<Tmp> tmp(actors.size());
  for (size_t i = 0; i < actors.size(); i++) {
    V3 p;
    if (!readActorPosition(actors[i], &p)) {
      tmp[i].dist = 1e9f;
      continue;
    }
    tmp[i].pos = p;
    float dx = p.x - s_camPos.x;
    float dy = p.y - s_camPos.y;
    float dz = p.z - s_camPos.z;
    tmp[i].dist = sqrtf(dx * dx + dy * dy + dz * dz);
    if (tmp[i].dist < minDist) {
      minDist = tmp[i].dist;
      localIdx = (int)i;
    }
  }

  // Second pass: build EspPed entries
  for (size_t i = 0; i < actors.size(); i++) {
    if (tmp[i].dist > 1e8f)
      continue;
    if ((int)i == localIdx || actors[i] == s_localActor)
      continue; // SKIP LOCAL PLAYER!
    EspPed e{};
    e.distance = tmp[i].dist;
    e.worldX = tmp[i].pos.x;
    e.worldY = tmp[i].pos.y;
    e.worldZ = tmp[i].pos.z;
    e.isLocal = false;
    e.pedType = ESP_PED_PLAYER;
    e.health = 1.0f;
    e.name[0] = '\0';
    e.actorPtr = actors[i];

    // 1. Map ESP Actor to Network Entity by closest distance matching
    int bestNetId = -1;
    float bestSqDist = 10.0f; // tolerance of ~3 meters squared
    pthread_mutex_lock(&g_SnifferMtx);
    for (auto &kv : g_PlayerPositions) {
      float dx = kv.second.x - tmp[i].pos.x;
      float dy = kv.second.y - tmp[i].pos.y;
      float dz = kv.second.z - tmp[i].pos.z;
      float sqDist = dx * dx + dy * dy + dz * dz;
      if (sqDist < bestSqDist) {
        bestSqDist = sqDist;
        bestNetId = kv.first;
      }
    }

    // 2. Fetch Sniffed Data
    if (bestNetId != -1) {
      if (g_PlayerNames.find(bestNetId) != g_PlayerNames.end()) {
        strncpy(e.name, g_PlayerNames[bestNetId].c_str(), sizeof(e.name) - 1);
        e.name[sizeof(e.name) - 1] = '\0';
      }
      if (g_PlayerHealth.find(bestNetId) != g_PlayerHealth.end()) {
        e.health = g_PlayerHealth[bestNetId] /
                   100.0f; // Scale to 0..1 for standard ESP health bars
                           // (assuming max health is 100)
      }
    }
    pthread_mutex_unlock(&g_SnifferMtx);

    if (s_skeletonEnabled) {
      typedef void *(*Component_GetComponentString_t)(void *self, void *typeString, void *method);
      static Component_GetComponentString_t fn_GetComponentString = nullptr;
      if (!fn_GetComponentString && s_libBase && RVA_GetComponentString != 0x0) {
        fn_GetComponentString = (Component_GetComponentString_t)(s_libBase + RVA_GetComponentString);
      }
      
      typedef void *(*Animator_GetBoneTransformInternal_t)(void *self, int humanBoneId);
      static Animator_GetBoneTransformInternal_t fn_GetBoneTransform = nullptr;
      if (!fn_GetBoneTransform && s_libBase && RVA_Animator_GetBoneTransformInternal != 0) {
        fn_GetBoneTransform = (Animator_GetBoneTransformInternal_t)(s_libBase + RVA_Animator_GetBoneTransformInternal);
      }

      static void* s_animatorStr = nullptr;
      if (!s_animatorStr && g_il2cpp.ready) s_animatorStr = g_il2cpp.string_new("Animator");

      if (fn_GetComponentString && fn_GetBoneTransform && s_animatorStr && fn_get_position) {
        void *animator = fn_GetComponentString(actors[i], s_animatorStr, nullptr);
        
        // Fallback: If Animator isn't on the CharacterActor directly, try the root.
        if (!animator && fn_get_transform && s_libBase && RVA_Transform_GetRoot != 0x0) {
           void* actorTr = fn_get_transform(actors[i]);
           if (actorTr) {
               typedef void *(*Transform_get_root_t)(void *, void *);
               static Transform_get_root_t fn_gr = (Transform_get_root_t)(s_libBase + RVA_Transform_GetRoot);
               void* rootTr = fn_gr(actorTr, nullptr);
               if (rootTr && rootTr != actorTr) {
                   animator = fn_GetComponentString(rootTr, s_animatorStr, nullptr);
               }
           }
        }

        if (animator) {
          static const int BONES[16] = {10, 9, 7, 0, 13, 15, 17, 14, 16, 18, 1, 3, 5, 2, 4, 6};
          for (int b = 0; b < 16; b++) {
            void* boneTr = fn_GetBoneTransform(animator, BONES[b]);
            if (boneTr) {
              V3 bPos = {0, 0, 0};
              fn_get_position(boneTr, &bPos);
              e.bones[b][0] = bPos.x;
              e.bones[b][1] = bPos.y;
              e.bones[b][2] = bPos.z;
            } else {
              e.bones[b][0] = 0; e.bones[b][1] = 0; e.bones[b][2] = 0;
            }
          }
        }
      }
    }

    V3 feet = tmp[i].pos;
    V3 head = v3(feet.x, feet.y + 1.8f, feet.z);
    float fx, fy, fd, hx, hy, hd;
    bool okF = worldToScreen(feet, fx, fy, fd);
    bool okH = worldToScreen(head, hx, hy, hd);
    e.screenX = fx;
    e.screenY = fy;
    e.screenHeadX = hx;
    e.screenHeadY = hy;
    e.onScreen = (okF && okH);
    // Convert from camera pixel coords (pixW x pixH) to overlay coords
    // (s_screenW x s_screenH)
    if (pixW > 0 && pixH > 0 && (pixW != s_screenW || pixH != s_screenH)) {
      float sx = (float)s_screenW / (float)pixW;
      float sy = (float)s_screenH / (float)pixH;
      e.screenX *= sx;
      e.screenY *= sy;
      e.screenHeadX *= sx;
      e.screenHeadY *= sy;
    }
    out.push_back(e);
  }

  static int dbgOut = 0;
  if ((dbgOut++ % 60) == 0) {
    E_LOGI("Snapshot n=%zu actors_seen=%zu", out.size(), actors.size());
  }
  pthread_mutex_lock(&s_mtx);
  s_snapshot = std::move(out);
  if (localIdx >= 0 && localIdx < (int)actors.size() && minDist < 5.0f) {
    if (s_localActor != actors[localIdx]) {
      E_LOGI("Teleport-ready: s_localActor updated via scan = %p (was %p, "
             "dist=%.1f)",
             actors[localIdx], s_localActor, minDist);
      s_localActor = actors[localIdx];
    }
    s_lastLocalActorNotifyMs = nowMs();
  }
  pthread_mutex_unlock(&s_mtx);
}

// === VEHICLE TRACKING (V17 ECS BYPASS) ===
#include <algorithm>
#include <vector>

static std::vector<void *> s_Vehicles;

void (*orig_VehicleOcclusionObject_OnEnable)(void *self);
void hook_VehicleOcclusionObject_OnEnable(void *self) {
  if (self) {
    if (!IsUnityObjectAlive(self)) {
      if (orig_VehicleOcclusionObject_OnEnable)
        orig_VehicleOcclusionObject_OnEnable(self);
      return;
    }
    // LAZY INIT — do NOT use s_libBase at static declaration time (it equals 0
    // before Esp_Init!)
    typedef void *(*Component_get_transform_t)(void *);
    typedef void *(*Transform_get_root_t)(void *, void *);
    static Component_get_transform_t fn_gt = nullptr;
    static Transform_get_root_t fn_gr = nullptr;
    if (!fn_gt && s_libBase)
      if (RVA_Component_GetTransform != 0x0) {
        fn_gt =
            (Component_get_transform_t)(s_libBase + RVA_Component_GetTransform);
      }
    if (!fn_gr && s_libBase)
      if (RVA_Transform_GetRoot != 0x0) {
        fn_gr = (Transform_get_root_t)(s_libBase + RVA_Transform_GetRoot);
      }

    if (fn_gt && fn_gr) {
      void *transform = fn_gt(self);
      if (transform && IsUnityObjectAlive(transform)) {
        void *root = fn_gr(transform, nullptr);
        if (root && IsUnityObjectAlive(root)) {
          pthread_mutex_lock(&s_mtx);
          if (std::find(s_Vehicles.begin(), s_Vehicles.end(), root) ==
              s_Vehicles.end()) {
            s_Vehicles.push_back(root);
            E_LOGI("VehicleTrack: +root=%p (total=%zu)", root,
                   s_Vehicles.size());
          }
          pthread_mutex_unlock(&s_mtx);
        }
      }
    }
  }
  if (orig_VehicleOcclusionObject_OnEnable)
    orig_VehicleOcclusionObject_OnEnable(self);
}

void (*orig_VehicleOcclusionObject_OnDisable)(void *self);
void hook_VehicleOcclusionObject_OnDisable(void *self) {
  if (self) {
    if (!IsUnityObjectAlive(self)) {
      if (orig_VehicleOcclusionObject_OnDisable)
        orig_VehicleOcclusionObject_OnDisable(self);
      return;
    }
    typedef void *(*Component_get_transform_t)(void *);
    typedef void *(*Transform_get_root_t)(void *, void *);
    static Component_get_transform_t fn_gt = nullptr;
    static Transform_get_root_t fn_gr = nullptr;
    if (!fn_gt && s_libBase)
      if (RVA_Component_GetTransform != 0x0) {
        fn_gt =
            (Component_get_transform_t)(s_libBase + RVA_Component_GetTransform);
      }
    if (!fn_gr && s_libBase)
      if (RVA_Transform_GetRoot != 0x0) {
        fn_gr = (Transform_get_root_t)(s_libBase + RVA_Transform_GetRoot);
      }

    if (fn_gt && fn_gr) {
      void *transform = fn_gt(self);
      if (transform && IsUnityObjectAlive(transform)) {
        void *root = fn_gr(transform, nullptr);
        if (root) {
          pthread_mutex_lock(&s_mtx);
          s_Vehicles.erase(
              std::remove(s_Vehicles.begin(), s_Vehicles.end(), root),
              s_Vehicles.end());
          pthread_mutex_unlock(&s_mtx);
          E_LOGI("VehicleTrack: -root=%p (total=%zu)", root, s_Vehicles.size());
        }
      }
    }
  }
  if (orig_VehicleOcclusionObject_OnDisable)
    orig_VehicleOcclusionObject_OnDisable(self);
}
// ========================================

// État auto-follow (toggle 181)
static bool s_autoFollow = false;
static double s_lastFollowMs = 0.0;
static void *s_lockedFollowTarget = nullptr;

static void Esp_SetAllVehiclesKinematic(bool kinematic) {
  if (!g_il2cpp.ready)
    return;
  typedef void *(*Component_GetComponentString_t)(void *self, void *typeString,
                                                  void *method);
  static Component_GetComponentString_t fn_GetComponentString = nullptr;
  if (!fn_GetComponentString && s_libBase)
    if (RVA_GetComponentString != 0x0) {
      fn_GetComponentString =
          (Component_GetComponentString_t)(s_libBase + RVA_GetComponentString);
    }

  typedef void (*Rigidbody_set_isKinematic_t)(void *self, bool value);
  static Rigidbody_set_isKinematic_t fn_rigid_kinematic = nullptr;
  if (!fn_rigid_kinematic && s_libBase)
    if (RVA_SetIsKinematic != 0x0) {
      fn_rigid_kinematic =
          (Rigidbody_set_isKinematic_t)(s_libBase + RVA_SetIsKinematic);
    }

  if (g_il2cpp.string_new && fn_GetComponentString && fn_rigid_kinematic) {
    void *strObj = g_il2cpp.string_new("Rigidbody");
    if (strObj) {
      pthread_mutex_lock(&s_mtx);
      for (void *vroot : s_Vehicles) {
        if (!vroot || !IsUnityObjectAlive(vroot))
          continue;
        void *rigid = fn_GetComponentString(vroot, strObj, nullptr);
        if (rigid) {
          fn_rigid_kinematic(rigid, kinematic);
        }
      }
      pthread_mutex_unlock(&s_mtx);
    }
  }
}

void Esp_SetAutoFollowCar(bool on) {
  s_autoFollowCar = on;
  s_lockedFollowTarget = nullptr; // Reset lock when toggling
  if (!on) {
    Esp_SetAllVehiclesKinematic(false);
  }
  E_LOGI("AutoFollowCar: %s (lock reset)", on ? "ON" : "OFF");
}
bool Esp_IsAutoFollowCarActive() { return s_autoFollowCar; }

void Esp_SetAutoFollow(bool on) {
  s_autoFollow = on;
  s_lastFollowMs = 0.0;
  s_lockedFollowTarget = nullptr; // Reset lock when toggling
  E_LOGI("AutoFollow: %s (lock reset)", on ? "ON" : "OFF");
}
extern "C" void Esp_ResetAutoFollowLock() {
  float tx, ty, tz;
  void *newTarget = nullptr;
  if (Esp_GetBestTargetWorld(&tx, &ty, &tz, true, &newTarget)) {
    s_lockedFollowTarget = newTarget;
    E_LOGI("AutoFollow: New target locked via button %p", s_lockedFollowTarget);
  } else {
    s_lockedFollowTarget = nullptr;
    E_LOGI("AutoFollow: No target found to lock");
  }
}
bool Esp_IsAutoFollowActive() { return s_autoFollow; }
double Esp_GetTimeMs();
void Esp_TickAutoFollow() {
  if (!s_autoFollow && !s_autoFollowCar)
    return;

  extern void UpdateLastLocalVehicle();
  UpdateLastLocalVehicle();

  float tx = 0, ty = 0, tz = 0;
  bool found = false;

  // 1. Priorité absolue : cible verrouillée par pseudo (Sniffer Réseau)
  if (Aimbot_IsTargetLocked()) {
    extern float g_AimbotTargetPos[3];
    tx = g_AimbotTargetPos[0];
    ty = g_AimbotTargetPos[1];
    tz = g_AimbotTargetPos[2];
    found = true;
  }
  // 2. Sinon, cible IL2CPP verrouillée visuellement
  else if (s_lockedFollowTarget != nullptr) {
    pthread_mutex_lock(&s_mtx);
    auto snap = s_snapshot;
    pthread_mutex_unlock(&s_mtx);

    static double s_lastTargetSeenMs = 0.0;
    for (const auto &p : snap) {
      if (p.actorPtr == s_lockedFollowTarget) {
        tx = p.worldX;
        ty = p.worldY;
        tz = p.worldZ;
        found = true;
        s_lastTargetSeenMs = Esp_GetTimeMs();
        break;
      }
    }

    if (!found) {
      double now = Esp_GetTimeMs();
      if (s_lastTargetSeenMs == 0.0) {
        s_lastTargetSeenMs = now;
      }
      if (now - s_lastTargetSeenMs > 20000.0) {
        s_lockedFollowTarget = nullptr;
        s_lastTargetSeenMs = 0.0;
        E_LOGI("AutoFollow: Target lost for >20000ms, resetting lock");
      }
      return;
    }
  }

  if (!found)
    return;

  pthread_mutex_lock(&s_mtx);
  void *actor = s_localActor;
  pthread_mutex_unlock(&s_mtx);

  V3 myPos = {0, 0, 0};
  if (actor) {
    readActorPosition(actor, &myPos);
  }

  static V3 s_lastAutoFollowPos = {0, 0, 0};

  bool pushed = false;
  // Joystick Push Logic removed temporarily

  if (pushed) {
    return;
  }

  extern float g_AutoFollowHeight;
  extern float g_AutoFollowDistance;

  float dx = myPos.x - tx;
  float dz = myPos.z - tz;
  float dist2D = sqrtf(dx * dx + dz * dz);
  if (dist2D > 0.001f) {
    dx /= dist2D;
    dz /= dist2D;
  } else {
    dx = 1.0f;
    dz = 0.0f;
  }

  myPos.x = tx + dx * g_AutoFollowDistance;
  myPos.z = tz + dz * g_AutoFollowDistance;
  myPos.y = ty + g_AutoFollowHeight;

  extern bool g_PlayerInVehicle;
  extern void *g_LastLocalVehicle;
  if (g_PlayerInVehicle && g_LastLocalVehicle) {
    // Throttle: max 1 TP voiture toutes les 2000ms pour eviter la desync
    // anti-cheat
    static double s_lastVehicleAutoFollowMs = 0.0;
    double now = Esp_GetTimeMs();
    if (now - s_lastVehicleAutoFollowMs >= 2000.0) {
      s_lastVehicleAutoFollowMs = now;
      // Ne teleporter que si la distance est significative (>2m)
      V3 myCurrentPos = {0, 0, 0};
      readActorPosition(actor ? actor : nullptr, &myCurrentPos);
      float ddx = myCurrentPos.x - myPos.x;
      float ddz = myCurrentPos.z - myPos.z;
      float dSq = ddx * ddx + ddz * ddz;
      if (dSq > 4.0f) { // 2m squared
        V3 zero = {0.0f, 0.0f, 0.0f};
        Esp_DirectVehicleTP(g_LastLocalVehicle, myPos.x, myPos.y, myPos.z, true,
                            &zero);
      }
    }
  } else {
    Teleport_ToPosition(myPos.x, myPos.y, myPos.z);
  }
}

bool Esp_GetBestTargetWorld(float *outX, float *outY, float *outZ,
                            bool allowOffscreen, void **outActorPtr) {
  if (!outX || !outY || !outZ)
    return false;

  extern bool g_AimbotHeadshot;
  float heightOffset = g_AimbotHeadshot ? 1.6f : 1.0f;

  if (Aimbot_IsTargetLocked()) {
    extern float g_AimbotTargetPos[3];
    *outX = g_AimbotTargetPos[0];
    *outY = g_AimbotTargetPos[1] + heightOffset;
    *outZ = g_AimbotTargetPos[2];
    if (outActorPtr)
      *outActorPtr = nullptr;
    return true;
  }

  pthread_mutex_lock(&s_mtx);
  auto snap = s_snapshot;
  pthread_mutex_unlock(&s_mtx);

  float scx = s_screenW / 2.0f;
  float scy = s_screenH / 2.0f;

  bool fovFilter = s_crosshairCircle;
  float fovRadSq = (float)s_crosshairRadius * (float)s_crosshairRadius;

  float minDist = 99999999.0f;
  bool found = false;
  for (const auto &p : snap) {
    if (p.isLocal)
      continue;
    float distToOrigin =
        p.worldX * p.worldX + p.worldY * p.worldY + p.worldZ * p.worldZ;
    if (distToOrigin < 1.0f)
      continue;

    bool isOff = !p.onScreen;
    if (isOff && !allowOffscreen)
      continue;

    float score;
    if (!isOff) {
      float dx = p.screenX - scx;
      float dy = p.screenY - scy;
      score = dx * dx + dy * dy;
      if (fovFilter && score > fovRadSq)
        continue;
    } else {
      score = 10000000.0f + p.distance * 1000.0f;
    }

    if (score < minDist) {
      minDist = score;
      *outX = p.worldX;
      *outY = p.worldY + heightOffset;
      *outZ = p.worldZ;
      if (outActorPtr)
        *outActorPtr = p.actorPtr;
      found = true;
    }
  }
  return found;
}

void Esp_ForceScanLocal() {
  scanAllActors();
  if (s_localActor)
    return;

  if (!fn_Camera_get_main || !fn_get_transform || !fn_get_position)
    return;
  void *cam = fn_Camera_get_main();
  if (!cam)
    return;
  void *camTr = fn_get_transform(cam);
  if (!camTr)
    return;
  V3 camPos;
  fn_get_position(camTr, &camPos);

  pthread_mutex_lock(&s_mtx);
  void *bestActor = nullptr;
  float bestDist = 99999.0f;

  for (void *actor : s_scannedActors) {
    void *tr = fn_get_transform(actor);
    if (!tr)
      continue;
    V3 pos;
    fn_get_position(tr, &pos);
    float dx = pos.x - camPos.x;
    float dy = pos.y - camPos.y;
    float dz = pos.z - camPos.z;
    float dist = dx * dx + dy * dy + dz * dz;
    if (dist < bestDist) {
      bestDist = dist;
      bestActor = actor;
    }
  }

  // If an actor is extremely close to the main camera, it is very likely the
  // local player.
  if (bestDist < 10.0f && bestActor) {
    s_localActor = bestActor;
    E_LOGI("Esp_ForceScanLocal: LocalActor fallback identified by Camera "
           "distance! %p (dist=%.2f)",
           bestActor, bestDist);
  }
  pthread_mutex_unlock(&s_mtx);
}

std::vector<EspPed> Esp_GetSnapshot() {
  pthread_mutex_lock(&s_mtx);
  auto snap = s_snapshot;
  pthread_mutex_unlock(&s_mtx);
  return snap;
}

void Esp_TargetVehicle() {}

void Esp_TeleportTargetVehicle(float x, float y, float z) {
  if (!g_PendingVehicleRoot)
    return;

  typedef void (*Transform_setpos_t)(void *, V3 *);
  static Transform_setpos_t fn_setpos = nullptr;
  if (RVA_Transform_SetPosition != 0x0) {
    fn_setpos = (Transform_setpos_t)(s_libBase + RVA_Transform_SetPosition);
  }

  if (fn_setpos) {
    V3 pos = {x, y, z};
    fn_setpos(g_PendingVehicleRoot, &pos);
    E_LOGI("Esp_TeleportTargetVehicle: TP %p to %f %f %f", g_PendingVehicleRoot,
           x, y, z);
  }
}

// Missing ESP Setters and Getters
void Esp_SetDotMode(bool enable) { s_dotMode = enable; }
bool Esp_IsDotMode() { return s_dotMode; }
void Esp_SetLineEnabled(bool enable) { s_lineEnabled = enable; }
bool Esp_IsLineEnabled() { return s_lineEnabled; }
void Esp_SetBoxEnabled(bool enable) { s_boxEnabled = enable; }
bool Esp_IsBoxEnabled() { return s_boxEnabled; }
void Esp_SetDistanceEnabled(bool enable) { s_distanceEnabled = enable; }
bool Esp_IsDistanceEnabled() { return s_distanceEnabled; }
void Esp_SetMarkerEnabled(bool enable) { s_markerEnabled = enable; }
bool Esp_IsMarkerEnabled() { return s_markerEnabled; }
void Esp_SetDynamicColor(bool enable) { s_dynamicColor = enable; }
bool Esp_IsDynamicColor() { return s_dynamicColor; }

void Esp_SetCrosshairEnabled(bool en) { s_crosshair = en; }
bool Esp_IsCrosshairEnabled() { return s_crosshair; }
void Esp_SetCrosshairCircleEnabled(bool en) { s_crosshairCircle = en; }
bool Esp_IsCrosshairCircleEnabled() { return s_crosshairCircle; }
void Esp_SetCrosshairCircleRadius(int r) { s_crosshairRadius = r; }
int Esp_GetCrosshairCircleRadius() { return s_crosshairRadius; }
void Esp_SetHealthEnabled(bool en) { s_healthEnabled = en; }
bool Esp_IsHealthEnabled() { return s_healthEnabled; }

// s_skeletonEnabled declared globally above; setters/getters here
void Esp_SetSkeletonEnabled(bool en) { s_skeletonEnabled = en; }
bool Esp_IsSkeletonEnabled() { return s_skeletonEnabled; }

static int s_teleportSpamFrames_UNUSED = 0;

void *CustomGetRootTransform(void *transform) {
  if (!transform || !fn_get_parent)
    return transform;
  void *current = transform;
  int depth = 0;
  while (depth < 10) {
    void *parent = fn_get_parent(current);
    if (!parent)
      break;
    current = parent;
    depth++;
  }
  return current;
}

void UpdateLastLocalVehicle() {
  resolveLazy();
  resolveTeleport();
  if (!s_localActor || !IsUnityObjectAlive(s_localActor) || !fn_get_transform) {
    s_localActor = nullptr;
    return;
  }

  void *actorTr = fn_get_transform(s_localActor);
  if (!actorTr || !IsUnityObjectAlive(actorTr))
    return;

  bool inVehicle = false;
  void *rootTr = nullptr;

  if (fn_get_parent) {
    void *parentTr = fn_get_parent(actorTr);
    if (parentTr && IsUnityObjectAlive(parentTr)) {
      if (fn_get_root) {
        void *root = fn_get_root(actorTr);
        if (root && root != actorTr && IsUnityObjectAlive(root)) {
          rootTr = root;
          inVehicle = true;
        }
      }
      if (!inVehicle) {
        void *cur = parentTr;
        void *manRoot = parentTr;
        int depth = 0;
        while (depth < 10) {
          void *p = fn_get_parent(cur);
          if (!p || !IsUnityObjectAlive(p))
            break;
          manRoot = p;
          cur = p;
          depth++;
        }
        if (manRoot && manRoot != actorTr) {
          rootTr = manRoot;
          inVehicle = true;
        }
      }
    }
  }

  g_PlayerInVehicle = inVehicle;
  if (inVehicle) {
    g_LastLocalVehicle = rootTr;
  } else {
    // === FALLBACK: Proximity check removed to avoid vehicle desyncs ===
    if (!inVehicle) {
      s_teleportSpamFrames = 0;
    }
  }

  if (g_LastLocalVehicle) {
    if (!IsUnityObjectAlive(g_LastLocalVehicle)) {
      g_LastLocalVehicle = nullptr;
      return;
    }

    void *rigid = FindRigidbodyRecursively(g_LastLocalVehicle);
    if (rigid) {
      typedef void (*Rigidbody_SetIsKinematic_t)(void *self, bool value);
      static Rigidbody_SetIsKinematic_t fn_SetIsKinematic = nullptr;
      if (!fn_SetIsKinematic && s_libBase && RVA_SetIsKinematic != 0x0) {
        fn_SetIsKinematic =
            (Rigidbody_SetIsKinematic_t)(s_libBase + RVA_SetIsKinematic);
      }
      if (fn_SetIsKinematic) {
        extern bool g_VehicleNoClipEnabled;
        extern bool g_StickyCarEnabled;
        bool vehicleNoClip = (g_VehicleNoClipEnabled && inVehicle) ||
                             g_StickyCarEnabled || (s_teleportSpamFrames > 0);
        fn_SetIsKinematic(rigid, vehicleNoClip);
      }
    }
  }
}

// Real Teleport_ToPosition implementation
bool Teleport_ToPosition(float x, float y, float z, bool ignoreCooldown) {
  extern double Esp_GetTimeMs();
  bool isAutoFollow = Esp_IsAutoFollowActive() || Esp_IsAutoFollowCarActive();

  if (!isAutoFollow) {
    s_teleportGodModeTime = Esp_GetTimeMs() + 3000.0;
    SetGodModePatchesState(true);
  }

  resolveLazy();
  resolveTeleport();
  UpdateLastLocalVehicle();
  void *actor = s_localActor;
  if (!actor) {
    E_LOGI("Teleport_ToPosition: s_localActor is null, cannot teleport");
    return false;
  }
  V3 pos = {x, y, z};

  // Get the transform and root
  void *actorTr = nullptr;
  if (fn_get_transform)
    actorTr = fn_get_transform(actor);

  void *rootTr = CustomGetRootTransform(actorTr);

  bool playerInside =
      (rootTr && rootTr != actorTr) ||
      (g_PlayerInVehicle && g_LastLocalVehicle && rootTr == g_LastLocalVehicle);
  void *vehToTp =
      playerInside ? (rootTr ? rootTr : g_LastLocalVehicle) : nullptr;

  if (vehToTp) {
    g_LastLocalVehicle = vehToTp;

    // Teleport the vehicle directly with a slight height offset to avoid clipping into the ground
    // Do NOT call fn_teleport on the player while inside the car, it will eject or glitch them!
    float safeY = y + 2.0f;
    Esp_DirectVehicleTP(vehToTp, x, safeY, z, true);

    // Ensure vehicle wakes up after teleport!
    typedef void (*Rigidbody_set_velocity_Injected_t)(void *self, V3 *value);
    static Rigidbody_set_velocity_Injected_t fn_rigid_setvel = nullptr;
    if (!fn_rigid_setvel && s_libBase && RVA_Rigidbody_SetVelocity != 0x0) {
      fn_rigid_setvel =
          (Rigidbody_set_velocity_Injected_t)(s_libBase +
                                              RVA_Rigidbody_SetVelocity);
    }
    void *rigid = FindRigidbodyRecursively(vehToTp);
    if (rigid && fn_rigid_setvel) {
      V3 zeroVel = {0, -0.1f, 0};
      fn_rigid_setvel(rigid, &zeroVel); // wake up
    }

    E_LOGI("Teleport_ToPosition: Esp_DirectVehicleTP called for vehicle at "
           "safe height: %.2f",
           safeY);
  } else {
    // Player is outside the vehicle. Teleport player first!
    extern bool g_NoClipEnabled;
    extern bool g_FlyEnabled;
    if (g_NoClipEnabled || g_FlyEnabled) {
      Esp_DirectVehicleTP(actorTr, x, y, z, false);
      E_LOGI("Teleport_ToPosition: Player TP via DirectTransform (NoClip/Fly) OK");
    } else {
      Esp_DirectVehicleTP(actorTr, x, y, z, false);
      if (fn_teleport) {
        fn_teleport(actor, x, y, z);
      } else if (fn_sweep_tp) {
        fn_sweep_tp(actor, x, y, z);
      }
      E_LOGI("Teleport_ToPosition: Player TP via Direct + Teleport/Sweep OK");
    }

    // Now get the true grounded position of the player
    V3 groundPos = {x, y, z};
    if (fn_get_position && actorTr) {
      fn_get_position(actorTr, &groundPos);
    }

    // Bring the last vehicle with us, spawned safely next to the player (X
    // offset) at ground level
    if (!isAutoFollow && g_LastLocalVehicle &&
        IsUnityObjectAlive(g_LastLocalVehicle)) {
      Esp_DirectVehicleTP(g_LastLocalVehicle, groundPos.x + 3.0f,
                          groundPos.y + 0.5f, groundPos.z, false);

      // Ensure it wakes up
      typedef void (*Rigidbody_set_velocity_Injected_t)(void *self, V3 *value);
      static Rigidbody_set_velocity_Injected_t fn_rigid_setvel = nullptr;
      if (!fn_rigid_setvel && s_libBase && RVA_Rigidbody_SetVelocity != 0x0) {
        fn_rigid_setvel =
            (Rigidbody_set_velocity_Injected_t)(s_libBase +
                                                RVA_Rigidbody_SetVelocity);
      }
      void *rigid = FindRigidbodyRecursively(g_LastLocalVehicle);
      if (rigid && fn_rigid_setvel) {
        V3 zeroVel = {0, -0.1f, 0};
        fn_rigid_setvel(rigid, &zeroVel); // wake up
      }

      E_LOGI("Teleport_ToPosition: Vehicle TP'd safely next to player at "
             "grounded height");
    }
  }

  return true;
}
void Teleport_LoadSlots() {}
bool Teleport_GoSlot(int slot) { return false; }
bool Esp_IsLocalActor(void *actor) { return actor == s_localActor; }

bool g_hasPendingTeleportRequest = false;
float g_pendingTeleportX = 0.0f;
float g_pendingTeleportY = 0.0f;
float g_pendingTeleportZ = 0.0f;

void Esp_QueueTeleport(float x, float y, float z) {
  g_pendingTeleportX = x;
  g_pendingTeleportY = y;
  g_pendingTeleportZ = z;
  g_hasPendingTeleportRequest = true;
  E_LOGI("Esp_QueueTeleport: Queued teleport to (%.2f, %.2f, %.2f)", x, y, z);
}

double Esp_GetTimeMs() { return nowMs(); }

void *Esp_MakeIl2cppString(const char *str) {
  if (!str)
    return nullptr;
  if (!g_il2cpp.ready && !g_il2cpp.Init())
    return nullptr;
  return g_il2cpp.string_new(str);
}

extern "C" bool Esp_GetCameraVectors(V3 *forward, V3 *right);

void Esp_TickAimLock() {
  // Camera crosshair lock — rotates the main camera to look at the best aimbot
  // target. Uses Transform.Internal_LookAt_Injected (RVA 0x43071BC) to
  // physically turn the crosshair.
  extern bool Aimbot_IsEnabled();
  if (!Aimbot_IsEnabled())
    return;
  extern bool g_AimLockCamera;
  if (!g_AimLockCamera)
    return;
  if (!s_libBase)
    return;

  // Signature: void Internal_LookAt_Injected(Transform* self, ref Vector3
  // worldPosition, ref Vector3 worldUp)
  typedef void (*Transform_Internal_LookAt_Injected_t)(void *self, V3 *worldPos,
                                                       V3 *worldUp);
  static Transform_Internal_LookAt_Injected_t fn_LookAt = nullptr;
  if (!fn_LookAt && s_libBase)
    fn_LookAt = (Transform_Internal_LookAt_Injected_t)(s_libBase + 0x43071BC);
  if (!fn_LookAt)
    return;

  // Lazily resolve Component.get_transform
  typedef void *(*Component_get_transform_t)(void *self);
  static Component_get_transform_t fn_gt = nullptr;
  if (!fn_gt && RVA_Component_GetTransform != 0x0)
    fn_gt = (Component_get_transform_t)(s_libBase + RVA_Component_GetTransform);
  if (!fn_gt)
    return;

  // Get main camera
  if (!fn_Camera_get_main)
    return;
  void *cam = fn_Camera_get_main();
  if (!cam || !IsUnityObjectAlive(cam))
    return;

  // Get camera transform
  void *camTr = fn_gt(cam);
  if (!camTr || !IsUnityObjectAlive(camTr))
    return;

  // Get best target world position
  float tx, ty, tz;
  void *targetActor = nullptr;
  extern bool g_FreecamLock;
  if (!Esp_GetBestTargetWorld(&tx, &ty, &tz, /*allowOffscreen=*/g_FreecamLock,
                              &targetActor)) {
    return;
  }

  // Rotate camera to look at target
  V3 targetPos = {tx, ty, tz};
  extern void *g_MainCameraStorage;
  extern void (*orig_SetCameraLookDirection)(void *, V3, void *);

  if (g_MainCameraStorage && orig_SetCameraLookDirection) {
    typedef void (*Transform_get_position_Injected_t)(void *self, V3 *out);
    static Transform_get_position_Injected_t fn_getPos = nullptr;
    if (!fn_getPos && s_libBase && RVA_Transform_GetPosition != 0x0)
      fn_getPos =
          (Transform_get_position_Injected_t)(s_libBase +
                                              RVA_Transform_GetPosition);

    V3 camPos = {0, 0, 0};
    if (fn_getPos)
      fn_getPos(camTr, &camPos);

    V3 targetDir = {targetPos.x - camPos.x, targetPos.y - camPos.y,
                    targetPos.z - camPos.z};
    float len = sqrtf(targetDir.x * targetDir.x + targetDir.y * targetDir.y +
                      targetDir.z * targetDir.z);
    if (len > 0.001f) {
      targetDir.x /= len;
      targetDir.y /= len;
      targetDir.z /= len;

      V3 finalDir = targetDir;
      extern float g_AimLockSmoothness;
      if (g_AimLockSmoothness > 0.0f && g_AimLockSmoothness < 5.0f) {
        // Get current camera forward vector
        V3 currentForward = {0, 0, 0};
        V3 right = {0, 0, 0};
        Esp_GetCameraVectors(&currentForward, &right);

        float lenF = sqrtf(currentForward.x * currentForward.x +
                           currentForward.y * currentForward.y +
                           currentForward.z * currentForward.z);
        if (lenF > 0.001f) {
          currentForward.x /= lenF;
          currentForward.y /= lenF;
          currentForward.z /= lenF;

          float alpha = g_AimLockSmoothness / 5.0f;
          float distToTarget =
              sqrtf((targetPos.x - camPos.x) * (targetPos.x - camPos.x) +
                    (targetPos.y - camPos.y) * (targetPos.y - camPos.y) +
                    (targetPos.z - camPos.z) * (targetPos.z - camPos.z));
          if (distToTarget < 5.0f)
            alpha *= 0.5f;
          float dot = currentForward.x * targetDir.x +
                      currentForward.y * targetDir.y +
                      currentForward.z * targetDir.z;
          if (dot < 0.965f)
            alpha = std::min(alpha, 0.1f);

          // Simple linear interpolation (Lerp) of direction vectors
          finalDir.x =
              currentForward.x + (targetDir.x - currentForward.x) * alpha;
          finalDir.y =
              currentForward.y + (targetDir.y - currentForward.y) * alpha;
          finalDir.z =
              currentForward.z + (targetDir.z - currentForward.z) * alpha;

          float finalLen =
              sqrtf(finalDir.x * finalDir.x + finalDir.y * finalDir.y +
                    finalDir.z * finalDir.z);
          if (finalLen > 0.001f) {
            finalDir.x /= finalLen;
            finalDir.y /= finalLen;
            finalDir.z /= finalLen;
          } else {
            finalDir = targetDir;
          }
        }
      }

      orig_SetCameraLookDirection(g_MainCameraStorage, finalDir, nullptr);
    }
  } else {
    // Fallback only if g_MainCameraStorage is not captured yet.
    // We only use LookAt on the camera to avoid breaking player physics.
    V3 worldUp = {0.0f, 1.0f, 0.0f};
    fn_LookAt(camTr, &targetPos, &worldUp);
  }
}

bool Esp_TeleportToNextGraffiti() { return false; }
void Esp_AutoMapGraffiti(bool enable) {}

void *Esp_GetLocalTransform() {
  if (!s_localActor)
    return nullptr;
  if (fn_get_transform)
    return fn_get_transform(s_localActor);
  return nullptr;
}

bool Esp_GetLocalPosition(V3 *out) {
  if (!out || !s_localActor || !IsUnityObjectAlive(s_localActor) ||
      !fn_get_transform || !fn_get_position) {
    return false;
  }

  void *actorTr = fn_get_transform(s_localActor);
  if (!actorTr) {
    return false;
  }

  fn_get_position(actorTr, out);
  return true;
}

extern "C" {
void Java_com_android_support_EspBridge_setScreenSize(JNIEnv *env, jclass,
                                                      jint w, jint h) {
  s_camPixW = w;
  s_camPixH = h;
}

void Java_com_android_support_EspBridge_setEnabled(JNIEnv *env, jclass,
                                                   jboolean en) {
  s_enabled = en;
}

jboolean Java_com_android_support_EspBridge_isEnabled(JNIEnv *env, jclass) {
  return s_enabled;
}

jfloatArray Java_com_android_support_EspBridge_getData(JNIEnv *env, jclass) {
  pthread_mutex_lock(&s_mtx);
  auto snap = s_snapshot;
  pthread_mutex_unlock(&s_mtx);

  int count = snap.size();
  if (count == 0) {
    jfloatArray ret = env->NewFloatArray(1);
    float z[1] = {0.0f};
    env->SetFloatArrayRegion(ret, 0, 1, z);
    return ret;
  }

  int stride = 53; // 5 + 16 * 3
  int arrSize = 1 + count * stride;
  float *tmp = new float[arrSize];
  tmp[0] = (float)count;

  for (int i = 0; i < count; i++) {
    int o = 1 + i * stride;
    tmp[o + 0] = snap[i].worldX;
    tmp[o + 1] = snap[i].worldY;
    tmp[o + 2] = snap[i].worldZ;
    tmp[o + 3] = snap[i].distance;
    tmp[o + 4] = snap[i].isLocal ? 1.0f : 0.0f;
    
    // Copy 16 bones * 3 floats
    for (int b = 0; b < 16; b++) {
      tmp[o + 5 + b * 3 + 0] = snap[i].bones[b][0];
      tmp[o + 5 + b * 3 + 1] = snap[i].bones[b][1];
      tmp[o + 5 + b * 3 + 2] = snap[i].bones[b][2];
    }
  }

  jfloatArray ret = env->NewFloatArray(arrSize);
  env->SetFloatArrayRegion(ret, 0, arrSize, tmp);
  delete[] tmp;
  return ret;
}

jfloatArray Java_com_android_support_EspBridge_getViewProjection(JNIEnv *env,
                                                                 jclass) {
  jfloatArray ret = env->NewFloatArray(18);
  float tmp[18];
  bool ok = false;
  pthread_mutex_lock(&s_mtx);
  if (s_vpValid) {
    memcpy(tmp, s_vp16, 16 * sizeof(float));
    tmp[16] = (float)s_camPixW;
    tmp[17] = (float)s_camPixH;
    ok = true;
  }
  pthread_mutex_unlock(&s_mtx);

  if (ok) {
    env->SetFloatArrayRegion(ret, 0, 18, tmp);
  }
  return ret;
}

jobjectArray Java_com_android_support_EspBridge_getPedNames(JNIEnv *env,
                                                            jclass) {
  pthread_mutex_lock(&s_mtx);
  auto snap = s_snapshot;
  pthread_mutex_unlock(&s_mtx);

  jclass stringClass = env->FindClass("java/lang/String");
  int count = snap.size();
  if (count == 0) {
    jstring emptyStr = env->NewStringUTF("");
    jobjectArray ret = env->NewObjectArray(0, stringClass, emptyStr);
    env->DeleteLocalRef(emptyStr);
    return ret;
  }
  jstring initStr = env->NewStringUTF("");
  jobjectArray ret = env->NewObjectArray(count, stringClass, initStr);
  env->DeleteLocalRef(initStr);

  for (int i = 0; i < count; i++) {
    jstring nameStr = env->NewStringUTF(snap[i].name);
    env->SetObjectArrayElement(ret, i, nameStr);
    env->DeleteLocalRef(nameStr);
  }
  return ret;
}

jboolean Java_com_android_support_EspBridge_isDotMode(JNIEnv *env, jclass) {
  return s_dotMode;
}
jboolean Java_com_android_support_EspBridge_isLineEnabled(JNIEnv *, jclass) {
  return s_lineEnabled;
}
jboolean Java_com_android_support_EspBridge_isBoxEnabled(JNIEnv *, jclass) {
  return s_boxEnabled;
}
jboolean Java_com_android_support_EspBridge_isDistanceEnabled(JNIEnv *,
                                                              jclass) {
  return s_distanceEnabled;
}
jboolean Java_com_android_support_EspBridge_isMarkerEnabled(JNIEnv *, jclass) {
  return s_markerEnabled;
}
jboolean Java_com_android_support_EspBridge_isDynamicColor(JNIEnv *, jclass) {
  return s_dynamicColor;
}
jboolean Java_com_android_support_EspBridge_isCrosshairEnabled(JNIEnv *,
                                                               jclass) {
  return s_crosshairEnabled;
}
jboolean Java_com_android_support_EspBridge_isCrosshairCircleEnabled(JNIEnv *,
                                                                     jclass) {
  return s_crosshairCircleEnabled;
}
jint Java_com_android_support_EspBridge_getCrosshairCircleRadius(JNIEnv *,
                                                                 jclass) {
  return s_crosshairCircleRadius;
}
jboolean Java_com_android_support_EspBridge_isHealthEnabled(JNIEnv *, jclass) {
  return s_healthEnabled;
}
jboolean Java_com_android_support_EspBridge_isSkeletonEnabled(JNIEnv *, jclass) {
  return s_skeletonEnabled;
}

extern "C" void Esp_TPCarToTarget() {
  float tx = 0, ty = 0, tz = 0;
  void *targetActor = nullptr; // cible principale (scope fonction)
  if (Esp_GetBestTargetWorld(&tx, &ty, &tz, true, &targetActor)) {
    pthread_mutex_lock(&s_mtx);
    void *actor = s_localActor;
    auto vehicles = s_Vehicles; // copy under lock
    pthread_mutex_unlock(&s_mtx);

    if (!actor) {
      E_LOGI("Esp_TPCarToTarget: s_localActor is null");
      return;
    }

    V3 myPos = {0, 0, 0};
    readActorPosition(actor, &myPos);

    resolveLazy();
    resolveTeleport();

    void *actorTr = nullptr;
    if (fn_get_transform)
      actorTr = fn_get_transform(actor);

    // Get root transform using parent-walking
    void *rootTr = CustomGetRootTransform(actorTr);

    // Determine the vehicle to teleport: ONLY local player's vehicle
    void *vehicleToTP = nullptr;
    bool playerInside = false;

    if (rootTr && rootTr != actorTr) {
      // Player is currently inside a vehicle — eject them and use the vehicle!
      vehicleToTP = rootTr;
      playerInside = false; // player no longer inside
      g_LastLocalVehicle = rootTr;
      E_LOGI("Esp_TPCarToTarget: Player is INSIDE vehicle %p, ejecting them",
             vehicleToTP);

      typedef void (*Transform_SetParent_t)(void *self, void *parent);
      static Transform_SetParent_t fn_SetParent = nullptr;
      if (!fn_SetParent && s_libBase) {
        fn_SetParent =
            (Transform_SetParent_t)(s_libBase + 0x4306594); // Transform.SetParent(Transform
                                                            // parent)
      }
      if (fn_SetParent) {
        fn_SetParent(actorTr, nullptr);
        E_LOGI("Esp_TPCarToTarget: Ejected player by setting parent to null!");
      }
    } else if (g_LastLocalVehicle) {
      if (IsUnityObjectAlive(g_LastLocalVehicle)) {
        vehicleToTP = g_LastLocalVehicle;
        playerInside = false;
        E_LOGI("Esp_TPCarToTarget: Using last local vehicle %p", vehicleToTP);
      } else {
        g_LastLocalVehicle = nullptr;
      }
    }

    // Last resort: if still no vehicle, pick the nearest one to the player
    // (but skip the target player's vehicle to avoid desyncing others)
    if (!vehicleToTP && !vehicles.empty()) {
      E_LOGI("Esp_TPCarToTarget: No tracked local vehicle, finding nearest to "
             "player");
      float dummyX2, dummyY2, dummyZ2;
      void *fallbackActor = nullptr;
      Esp_GetBestTargetWorld(&dummyX2, &dummyY2, &dummyZ2, true,
                             &fallbackActor);
      if (!targetActor)
        targetActor = fallbackActor; // utiliser pour sticky
      void *targetRootTr = nullptr;
      void *actorForRoot = targetActor ? targetActor : fallbackActor;
      if (actorForRoot && fn_get_transform) {
        void *targetTr = fn_get_transform(actorForRoot);
        if (targetTr)
          targetRootTr = CustomGetRootTransform(targetTr);
      }

      float minDist = 50.0f; // Only pick up nearby vehicles (within 50m)
      for (void *v : vehicles) {
        if (!v || !IsUnityObjectAlive(v) || v == targetRootTr)
          continue; // skip target's vehicle
        V3 vpos = {0, 0, 0};
        if (fn_get_position) {
          fn_get_position(v, &vpos);
          float dx = vpos.x - myPos.x;
          float dy = vpos.y - myPos.y;
          float dz = vpos.z - myPos.z;
          float dist = sqrtf(dx * dx + dy * dy + dz * dz);
          if (dist < minDist) {
            minDist = dist;
            vehicleToTP = v;
            g_LastLocalVehicle = v; // remember it
          }
        }
      }
      if (vehicleToTP)
        E_LOGI("Esp_TPCarToTarget: Fallback: nearest vehicle %p at dist %.1f",
               vehicleToTP, minDist);
    }

    if (vehicleToTP) {
      // Mode sticky: accroche la voiture sur la cible en continu
      g_StickyCarEnabled = true;
      g_StickyCarTarget = targetActor; // traqué chaque frame dans Esp_FrameTick
      g_LastLocalVehicle = vehicleToTP;
      // Téléportation initiale immédiate sur la cible
      V3 zero = {0.0f, 0.0f, 0.0f};
      Esp_DirectVehicleTP(vehicleToTP, tx, ty + 2.5f, tz, playerInside, &zero);
      E_LOGI("Esp_TPCarToTarget: Vehicle TP executed. root=%p playerInside=%d "
             "target=(%.1f, %.1f, %.1f)",
             vehicleToTP, (int)playerInside, tx, ty, tz);
    } else {
      E_LOGI("Esp_TPCarToTarget: No vehicle found! Enter a vehicle first, or "
             "get within 50m of yours.");
    }
  } else {
    E_LOGI("Esp_TPCarToTarget: No target found");
  }
}

extern "C" bool Esp_GetCameraVectors(V3 *forward, V3 *right) {
  bool ok = false;
  pthread_mutex_lock(&s_mtx);
  if (s_vpValid) {
    extern V3 g_EspCamFwd;
    extern V3 g_EspCamRight;
    *forward = g_EspCamFwd;
    *right = g_EspCamRight;
    ok = true;
  }
  pthread_mutex_unlock(&s_mtx);
  return ok;
}
} // extern "C"

V3 g_EspCamFwd = {0, 0, 0};
V3 g_EspCamRight = {0, 0, 0};
V3 g_EspCamPos = {0, 0, 0};

extern "C" bool Esp_GetCameraPosition(V3 *pos) {
  bool ok = false;
  pthread_mutex_lock(&s_mtx);
  if (s_vpValid) {
    *pos = g_EspCamPos;
    ok = true;
  }
  pthread_mutex_unlock(&s_mtx);
  return ok;
}

extern "C" void Esp_CaptureCameraMatrix(void *cam) {
  if (!cam || !IsUnityObjectAlive(cam) || !fn_GetView || !fn_GetProj)
    return;

  M4 view{}, proj{};
  fn_GetView(cam, &view);
  fn_GetProj(cam, &proj);

  int pixW = s_screenW, pixH = s_screenH;
  if (fn_GetPixelW && fn_GetPixelH) {
    int w = fn_GetPixelW(cam);
    int h = fn_GetPixelH(cam);
    if (w > 0 && h > 0) {
      pixW = w;
      pixH = h;
    }
  }

  auto mmul = [](const M4 &A, const M4 &B, M4 &R) {
    for (int c = 0; c < 4; c++) {
      for (int r = 0; r < 4; r++) {
        float s = 0.f;
        for (int k = 0; k < 4; k++) {
          s += A.m[r + k * 4] * B.m[k + c * 4];
        }
        R.m[r + c * 4] = s;
      }
    }
  };
  M4 vp{};
  mmul(proj, view, vp);

  pthread_mutex_lock(&s_mtx);
  memcpy(s_vp16, vp.m, sizeof(s_vp16));

  g_EspCamRight.x = view.m[0];
  g_EspCamRight.y = view.m[4];
  g_EspCamRight.z = view.m[8];

  g_EspCamFwd.x = -view.m[2];
  g_EspCamFwd.y = -view.m[6];
  g_EspCamFwd.z = -view.m[10];

  if (pixW > 0 && pixH > 0) {
    s_camPixW = pixW;
    s_camPixH = pixH;
  }
  s_vpValid = true;
  
  // Extract camera position from inverse view matrix
  // Pos = -(R^T * T) where T is translation vector
  float tx = view.m[12];
  float ty = view.m[13];
  float tz = view.m[14];
  g_EspCamPos.x = -(view.m[0] * tx + view.m[1] * ty + view.m[2] * tz);
  g_EspCamPos.y = -(view.m[4] * tx + view.m[5] * ty + view.m[6] * tz);
  g_EspCamPos.z = -(view.m[8] * tx + view.m[9] * ty + view.m[10] * tz);

  pthread_mutex_unlock(&s_mtx);
}
