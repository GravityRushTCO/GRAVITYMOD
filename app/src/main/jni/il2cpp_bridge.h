#pragma once
#include <cstdint>
#include <cstring>
#include <dlfcn.h>

// IL2CPP API minimal - via dlsym
typedef void* (*il2cpp_domain_get_t)();
typedef void** (*il2cpp_domain_get_assemblies_t)(void* domain, size_t* size);
typedef void* (*il2cpp_assembly_get_image_t)(void* assembly);
typedef void* (*il2cpp_class_from_name_t)(void* image, const char* ns, const char* name);
typedef void* (*il2cpp_class_get_method_from_name_t)(void* klass, const char* name, int argsCount);
typedef void* (*il2cpp_class_get_type_t)(void* klass);
typedef void* (*il2cpp_type_get_object_t)(void* type);
typedef void* (*il2cpp_runtime_invoke_t)(void* method, void* obj, void** params, void** exc);
typedef void* (*il2cpp_object_unbox_t)(void* obj);
typedef uint32_t (*il2cpp_array_length_t)(void* arr);
typedef void* (*il2cpp_string_new_t)(const char* str);
typedef void* (*il2cpp_thread_attach_t)(void* domain);
typedef void (*il2cpp_thread_detach_t)(void* thread);
typedef void* (*il2cpp_resolve_icall_t)(const char* name);

// IL2CPP Array layout:
//   +0x10 bounds (ptr)
//   +0x18 max_length (size_t)
//   +0x20 first element (vector[0])
struct Il2CppArrayBounds { /* opaque */ };
struct Il2CppArray {
    // header (klass, monitor)  -- 0x10 bytes
    void* klass;
    void* monitor;
    Il2CppArrayBounds* bounds;
    size_t max_length;
    // followed by vector[]
};
static inline void** Il2CppArray_Items(void* arr) {
    return reinterpret_cast<void**>(reinterpret_cast<uint8_t*>(arr) + 0x20);
}

struct IL2CPP {
    bool ready = false;
    void* handle = nullptr;
    
    il2cpp_domain_get_t domain_get = nullptr;
    il2cpp_domain_get_assemblies_t domain_get_assemblies = nullptr;
    il2cpp_assembly_get_image_t assembly_get_image = nullptr;
    il2cpp_class_from_name_t class_from_name = nullptr;
    il2cpp_class_get_method_from_name_t class_get_method_from_name = nullptr;
    il2cpp_class_get_type_t class_get_type = nullptr;
    il2cpp_type_get_object_t type_get_object = nullptr;
    il2cpp_runtime_invoke_t runtime_invoke = nullptr;
    il2cpp_object_unbox_t object_unbox = nullptr;
    il2cpp_array_length_t array_length = nullptr;
    il2cpp_string_new_t string_new = nullptr;
    il2cpp_thread_attach_t thread_attach = nullptr;
    il2cpp_thread_detach_t thread_detach = nullptr;
    il2cpp_resolve_icall_t resolve_icall = nullptr;
    typedef void* (*il2cpp_array_new_t)(void* klass, size_t count);
    il2cpp_array_new_t array_new = nullptr;

    bool Init() {
        handle = dlopen("libil2cpp.so", RTLD_NOLOAD);
        if (!handle) return false;
        
        domain_get = (il2cpp_domain_get_t)dlsym(handle, "il2cpp_domain_get");
        domain_get_assemblies = (il2cpp_domain_get_assemblies_t)dlsym(handle, "il2cpp_domain_get_assemblies");
        assembly_get_image = (il2cpp_assembly_get_image_t)dlsym(handle, "il2cpp_assembly_get_image");
        class_from_name = (il2cpp_class_from_name_t)dlsym(handle, "il2cpp_class_from_name");
        class_get_method_from_name = (il2cpp_class_get_method_from_name_t)dlsym(handle, "il2cpp_class_get_method_from_name");
        class_get_type = (il2cpp_class_get_type_t)dlsym(handle, "il2cpp_class_get_type");
        type_get_object = (il2cpp_type_get_object_t)dlsym(handle, "il2cpp_type_get_object");
        runtime_invoke = (il2cpp_runtime_invoke_t)dlsym(handle, "il2cpp_runtime_invoke");
        object_unbox = (il2cpp_object_unbox_t)dlsym(handle, "il2cpp_object_unbox");
        array_length = (il2cpp_array_length_t)dlsym(handle, "il2cpp_array_length");
        string_new = (il2cpp_string_new_t)dlsym(handle, "il2cpp_string_new");
        thread_attach = (il2cpp_thread_attach_t)dlsym(handle, "il2cpp_thread_attach");
        thread_detach = (il2cpp_thread_detach_t)dlsym(handle, "il2cpp_thread_detach");
        resolve_icall = (il2cpp_resolve_icall_t)dlsym(handle, "il2cpp_resolve_icall");
        array_new = (il2cpp_array_new_t)dlsym(handle, "il2cpp_array_new");
        
        if (!domain_get || !class_from_name || !class_get_method_from_name || !runtime_invoke) {
            return false;
        }
        
        ready = true;
        return true;
    }

    // Get System.Type object for a managed class (used to call FindObjectsOfTypeAll(Type)).
    void* GetTypeObject(void* klass) {
        if (!ready || !klass || !class_get_type || !type_get_object) return nullptr;
        void* type = class_get_type(klass);
        if (!type) return nullptr;
        return type_get_object(type);
    }

    // Look up a class by name in any loaded assembly image (not just mscorlib).
    // Useful for game-specific classes like Lightbug.CharacterControllerPro.Core.CharacterActor
    // which live in Assembly-CSharp.
    void* FindClassEverywhere(const char* ns, const char* name) {
        if (!ready || !domain_get_assemblies || !assembly_get_image) return nullptr;
        size_t n = 0;
        void** assemblies = domain_get_assemblies(domain_get(), &n);
        if (!assemblies) return nullptr;
        for (size_t i = 0; i < n; i++) {
            void* img = assembly_get_image(assemblies[i]);
            if (!img) continue;
            void* k = class_from_name(img, ns, name);
            if (k) return k;
        }
        return nullptr;
    }
    
    void* GetClass(const char* ns, const char* name) {
        if (!ready) return nullptr;
        return class_from_name(nullptr, ns, name);
    }
    
    void* GetMethod(void* klass, const char* name, int argsCount) {
        if (!ready) return nullptr;
        return class_get_method_from_name(klass, name, argsCount);
    }

    // Dynamic field offset lookup
    uint32_t GetFieldOffset(void* klass, const char* name) {
        if (!handle || !klass) return 0;
        typedef void* (*il2cpp_class_get_fields_t)(void* klass, void** iter);
        typedef const char* (*il2cpp_field_get_name_t)(void* field);
        typedef uint32_t (*il2cpp_field_get_offset_t)(void* field);
        
        auto get_fields = (il2cpp_class_get_fields_t)dlsym(handle, "il2cpp_class_get_fields");
        auto f_get_name = (il2cpp_field_get_name_t)dlsym(handle, "il2cpp_field_get_name");
        auto f_get_offset = (il2cpp_field_get_offset_t)dlsym(handle, "il2cpp_field_get_offset");
        
        if (!get_fields || !f_get_name || !f_get_offset) return 0;
        
        void* iter = nullptr;
        while (void* field = get_fields(klass, &iter)) {
            const char* fname = f_get_name(field);
            if (fname && strcmp(fname, name) == 0) {
                return f_get_offset(field);
            }
        }
        return 0;
    }
    
    void* Invoke(void* method, void* obj, void** params) {
        if (!ready) return nullptr;
        void* exc = nullptr;
        return runtime_invoke(method, obj, params, &exc);
    }
    
    // === CAMERA HELPERS ===
    void* GetMainCamera() {
        void* camClass = FindClassEverywhere("UnityEngine", "Camera");
        if (!camClass) return nullptr;
        void* getMain = GetMethod(camClass, "get_main", 0);
        if (!getMain) return nullptr;
        return Invoke(getMain, nullptr, nullptr);
    }
    
    void GetWorldToCameraMatrix(void* cam, float outMat[16]) {
        void* camClass = FindClassEverywhere("UnityEngine", "Camera");
        if (!camClass || !cam) return;
        void* getView = GetMethod(camClass, "get_worldToCameraMatrix", 0);
        if (!getView) return;
        void* boxed = Invoke(getView, cam, nullptr);
        if (boxed && object_unbox) {
            void* raw = object_unbox(boxed);
            if (raw) memcpy(outMat, raw, 64);
        }
    }
    
    void GetProjectionMatrix(void* cam, float outMat[16]) {
        void* camClass = FindClassEverywhere("UnityEngine", "Camera");
        if (!camClass || !cam) return;
        void* getProj = GetMethod(camClass, "get_projectionMatrix", 0);
        if (!getProj) return;
        void* boxed = Invoke(getProj, cam, nullptr);
        if (boxed && object_unbox) {
            void* raw = object_unbox(boxed);
            if (raw) memcpy(outMat, raw, 64);
        }
    }
};

static IL2CPP g_il2cpp;
