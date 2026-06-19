// ============================================================================
// Emanuel_IL2CPP.cpp — Reflection-based feature layer
// Voir Emanuel_IL2CPP.h pour la rationale complète.
// ============================================================================

#include "Emanuel_IL2CPP.h"
#include "il2cpp_bridge.h"
#include "Includes/Logger.h"

#include <atomic>
#include <chrono>
#include <thread>
#include <cstring>
#include <cstdint>

#define EM_TAG "EmanuelIL2CPP"

// ---------------------------------------------------------------------------
// IL2CPP API supplémentaires non exposées par il2cpp_bridge.h.
// On les résout par dlsym à la demande.
// ---------------------------------------------------------------------------
typedef void*       (*il2cpp_class_get_fields_t)(void* klass, void** iter);
typedef const char* (*il2cpp_field_get_name_t)(void* field);
typedef uint32_t    (*il2cpp_field_get_offset_t)(void* field);
typedef void*       (*il2cpp_field_get_type_t)(void* field);
typedef const char* (*il2cpp_type_get_name_t)(void* type);
typedef size_t      (*il2cpp_image_get_class_count_t)(void* image);
typedef void*       (*il2cpp_image_get_class_t)(void* image, size_t index);
typedef const char* (*il2cpp_class_get_name_t)(void* klass);
typedef const char* (*il2cpp_class_get_namespace_t)(void* klass);
typedef const char* (*il2cpp_image_get_name_t)(void* image);

// ---------------------------------------------------------------------------
// Dump des fields d'une classe identifiée par ns::name.
// ---------------------------------------------------------------------------
int EmanuelIL2CPP_DumpClass(const char* ns, const char* name) {
    if (!g_il2cpp.ready && !g_il2cpp.Init()) {
        LOGE(EM_TAG ": IL2CPP API unavailable");
        return -1;
    }
    void* klass = g_il2cpp.FindClassEverywhere(ns ? ns : "", name);
    if (!klass) {
        LOGE(EM_TAG ": class not found: %s::%s", ns ? ns : "", name);
        return -1;
    }

    void* h = g_il2cpp.handle;
    auto get_fields = (il2cpp_class_get_fields_t)dlsym(h, "il2cpp_class_get_fields");
    auto f_get_name = (il2cpp_field_get_name_t)  dlsym(h, "il2cpp_field_get_name");
    auto f_get_off  = (il2cpp_field_get_offset_t)dlsym(h, "il2cpp_field_get_offset");
    auto f_get_type = (il2cpp_field_get_type_t)  dlsym(h, "il2cpp_field_get_type");
    auto t_get_name = (il2cpp_type_get_name_t)   dlsym(h, "il2cpp_type_get_name");

    if (!get_fields || !f_get_name || !f_get_off) {
        LOGE(EM_TAG ": missing IL2CPP field API symbols");
        return -1;
    }

    LOGI(EM_TAG ": ===== CLASS %s::%s =====", ns ? ns : "", name);
    int count = 0;
    void* iter = nullptr;
    while (void* field = get_fields(klass, &iter)) {
        const char* fname = f_get_name(field);
        uint32_t off = f_get_off(field);
        const char* tname = "?";
        if (f_get_type && t_get_name) {
            void* type = f_get_type(field);
            if (type) {
                const char* tn = t_get_name(type);
                if (tn) tname = tn;
            }
        }
        LOGI(EM_TAG ": FIELD %s offset=0x%X type=%s",
             fname ? fname : "<null>", off, tname);
        count++;
    }
    LOGI(EM_TAG ": ===== END %s (%d fields) =====", name, count);
    return count;
}

// ---------------------------------------------------------------------------
// Énumère TOUTES les classes IL2CPP de toutes les assemblies, et logge
// celles dont le nom (ou le namespace) contient `keyword`.
// ---------------------------------------------------------------------------
int EmanuelIL2CPP_DumpClassesByKeyword(const char* keyword, int maxResults) {
    if (!keyword || !*keyword) return 0;
    if (!g_il2cpp.ready && !g_il2cpp.Init()) {
        LOGE(EM_TAG ": IL2CPP API unavailable");
        return -1;
    }
    void* h = g_il2cpp.handle;
    auto img_class_count = (il2cpp_image_get_class_count_t)dlsym(h, "il2cpp_image_get_class_count");
    auto img_class       = (il2cpp_image_get_class_t)      dlsym(h, "il2cpp_image_get_class");
    auto cls_name        = (il2cpp_class_get_name_t)       dlsym(h, "il2cpp_class_get_name");
    auto cls_ns          = (il2cpp_class_get_namespace_t)  dlsym(h, "il2cpp_class_get_namespace");
    auto img_name        = (il2cpp_image_get_name_t)       dlsym(h, "il2cpp_image_get_name");

    if (!img_class_count || !img_class || !cls_name) {
        LOGE(EM_TAG ": missing image/class enum API");
        return -1;
    }

    void* domain = g_il2cpp.domain_get();
    size_t n_asm = 0;
    void** assemblies = g_il2cpp.domain_get_assemblies(domain, &n_asm);
    if (!assemblies || n_asm == 0) {
        LOGE(EM_TAG ": no assemblies");
        return -1;
    }

    LOGI(EM_TAG ": ===== SCAN keyword='%s' across %zu assemblies =====", keyword, n_asm);
    int found = 0;
    for (size_t i = 0; i < n_asm; ++i) {
        void* image = g_il2cpp.assembly_get_image(assemblies[i]);
        if (!image) continue;
        const char* iname = img_name ? img_name(image) : "?";
        size_t n_cls = img_class_count(image);
        for (size_t j = 0; j < n_cls; ++j) {
            void* klass = img_class(image, j);
            if (!klass) continue;
            const char* name = cls_name(klass);
            const char* ns = cls_ns ? cls_ns(klass) : "";
            if (!name) continue;
            bool match = strstr(name, keyword) != nullptr ||
                         (ns && strstr(ns, keyword) != nullptr);
            if (!match) continue;
            LOGI(EM_TAG ": HIT [%s] %s::%s", iname, ns ? ns : "", name);
            found++;
            if (maxResults > 0 && found >= maxResults) {
                LOGI(EM_TAG ": ===== STOP at maxResults=%d =====", maxResults);
                return found;
            }
        }
    }
    LOGI(EM_TAG ": ===== SCAN done, %d hits =====", found);
    return found;
}

// ---------------------------------------------------------------------------
// Bootstrap : dump les classes critiques d'OneState RP avec leurs vrais
// noms extraits du code C# décompilé (Assembly-CSharp).
// ---------------------------------------------------------------------------
void EmanuelIL2CPP_BootstrapDump() {
    LOGI(EM_TAG ": Bootstrap dump START");

    // -- Composants ECS Morpeh (struct IComponent — pas trouvable via
    // FindObjectsOfTypeAll, mais le dump des fields est utile). --
    EmanuelIL2CPP_DumpClass(
        "dev.Scripts.ECS.Morpeh.Features.ElementsFeature.Components", "Health");
    EmanuelIL2CPP_DumpClass(
        "dev.Scripts.ECS.Morpeh.Features.ElementsFeature.Components", "DealDamage");
    EmanuelIL2CPP_DumpClass(
        "dev.Scripts.ECS.Morpeh.Features.ElementsFeature.Components", "ElementDealDamage");
    EmanuelIL2CPP_DumpClass(
        "dev.Scripts.ECS.Morpeh.Features.ElementsFeature.Components", "VehicleHealthUpdate");

    // -- Providers (MonoProvider<T> = MonoBehaviour, FINDABLE par reflection) --
    EmanuelIL2CPP_DumpClass(
        "dev.Scripts.ECS.Morpeh.Features.HudLifeValuesFeature.Providers",
        "HudHealthViewProvider");
    EmanuelIL2CPP_DumpClass(
        "dev.Scripts.ECS.Morpeh.Features.HudLifeValuesFeature.Providers",
        "HudStaminaViewProvider");

    // -- Services (MonoBehaviour ou plain class) --
    EmanuelIL2CPP_DumpClass(
        "dev.Scripts.ECS.Morpeh.Features.ItemUpgradeScreenFeature.Services",
        "WeaponParametersService");
    EmanuelIL2CPP_DumpClass(
        "dev.Scripts.ECS.Morpeh.Features.WeaponShopFeature.Systems",
        "FactionWeaponShopService");

    // -- Movement (Lightbug — MonoBehaviour confirmé : 86 fields trouvés) --
    EmanuelIL2CPP_DumpClass("Lightbug.CharacterControllerPro.Core", "CharacterActor");

    // -- Scan global pour trouver les vrais Manager/Service/Provider liés
    // aux features qu'on veut moder. --
    EmanuelIL2CPP_DumpClassesByKeyword("MoneyTransfer", 50);
    EmanuelIL2CPP_DumpClassesByKeyword("Wallet", 30);
    EmanuelIL2CPP_DumpClassesByKeyword("Reputation", 30);
    EmanuelIL2CPP_DumpClassesByKeyword("Stamina", 30);
    EmanuelIL2CPP_DumpClassesByKeyword("LocalPlayer", 80);

    LOGI(EM_TAG ": Bootstrap dump END (filter logcat with grep EmanuelIL2CPP)");
}

// ---------------------------------------------------------------------------
// GodMode reflection-based.
//
// AVERTISSEMENT IMPORTANT : OneState utilise Morpeh ECS. La majorité des
// composants gameplay sont des `struct IComponent` stockés dans des Stash<T>
// internes au World, PAS des UnityEngine.Object. Donc
// `Object.FindObjectsOfTypeAll(typeof(Health))` retourne ZÉRO instance.
//
// Pour vraiment moder via reflection, il faudrait :
//   - Résoudre Scellecs.Morpeh.World et son `Default` static
//   - Appeler World.GetStash<Health>()
//   - Itérer les entités et écrire dans le stash
// → c'est faisable mais complexe. Pour l'instant ce thread tente quand même
//   FindObjectsOfTypeAll sur HudHealthViewProvider (qui EST un MonoBehaviour)
//   pour démontrer que la reflection marche, mais ne peut pas vraiment
//   donner GodMode. Garder pour démonstration uniquement.
// ---------------------------------------------------------------------------
static std::atomic<bool> g_GodReflectionRunning{false};
static std::thread g_GodReflectionThread;

static void GodReflectionLoop() {
    LOGI(EM_TAG ": GodMode reflection thread START (note: ECS limitations)");

    while (g_GodReflectionRunning && !g_il2cpp.ready && !g_il2cpp.Init()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
    if (!g_il2cpp.ready) {
        LOGE(EM_TAG ": GodMode: IL2CPP never ready, abort");
        return;
    }

    void* domain = g_il2cpp.domain_get ? g_il2cpp.domain_get() : nullptr;
    void* attached = nullptr;
    if (domain && g_il2cpp.thread_attach) {
        attached = g_il2cpp.thread_attach(domain);
    }

    // Tente de trouver HudHealthViewProvider (MonoBehaviour) — on logge juste
    // combien d'instances vivent. Pas de modification de gameplay car le
    // Provider ne stocke pas la santé réelle.
    void* provKlass = g_il2cpp.FindClassEverywhere(
        "dev.Scripts.ECS.Morpeh.Features.HudLifeValuesFeature.Providers",
        "HudHealthViewProvider");
    if (!provKlass) {
        LOGE(EM_TAG ": GodMode: HudHealthViewProvider class not found");
        if (attached && g_il2cpp.thread_detach) g_il2cpp.thread_detach(attached);
        return;
    }
    void* typeObj = g_il2cpp.GetTypeObject(provKlass);
    void* objClass = g_il2cpp.GetClass("UnityEngine", "Object");
    void* findAll = objClass ? g_il2cpp.GetMethod(objClass, "FindObjectsOfTypeAll", 1) : nullptr;
    if (!findAll || !typeObj) {
        LOGE(EM_TAG ": GodMode: missing FindObjectsOfTypeAll");
        if (attached && g_il2cpp.thread_detach) g_il2cpp.thread_detach(attached);
        return;
    }

    while (g_GodReflectionRunning) {
        void* params[1] = { typeObj };
        void* arr = g_il2cpp.runtime_invoke(findAll, nullptr, params, nullptr);
        if (arr && g_il2cpp.array_length) {
            uint32_t n = g_il2cpp.array_length(arr);
            LOGI(EM_TAG ": GodMode tick: %u HudHealthViewProvider instances "
                 "alive (no gameplay effect, ECS limitation)", n);
        }
        std::this_thread::sleep_for(std::chrono::seconds(2));
    }

    if (attached && g_il2cpp.thread_detach) g_il2cpp.thread_detach(attached);
    LOGI(EM_TAG ": GodMode reflection thread STOP");
}

void EmanuelIL2CPP_StartGodModeReflection() {
    if (g_GodReflectionRunning.exchange(true)) return;
    g_GodReflectionThread = std::thread(GodReflectionLoop);
    g_GodReflectionThread.detach();
}

void EmanuelIL2CPP_StopGodModeReflection() {
    g_GodReflectionRunning = false;
}
