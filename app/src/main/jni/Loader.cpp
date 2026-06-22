#include <jni.h>
#include <dlfcn.h>
#include <string>
#include <fstream>
#include <android/log.h>
#include <thread>
#include <sys/stat.h>
#include <unistd.h>

#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, "ModLoader", __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, "ModLoader", __VA_ARGS__)

static JavaVM* g_jvm = nullptr;

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* vm, void* reserved) {
    g_jvm = vm;
    LOGI("ModLoader JNI_OnLoad called.");
    
    // Check for fake ID reset marker
    if (access("/data/data/com.onestate.global/reset_pending", F_OK) == 0) {
        LOGI("Fake ID was active on close. Resetting PlayerPrefs...");
        system("rm -rf /data/data/com.onestate.global/shared_prefs/*");
        remove("/data/data/com.onestate.global/reset_pending");
    }

    // Load the bundled libCore.so directly from the app's native library directory.
    // The OS linker automatically resolves "libCore.so" since it's packaged in the APK.
    void* handle = dlopen("libCore.so", RTLD_NOW | RTLD_GLOBAL);
    if (!handle) {
        LOGE("dlopen failed: %s", dlerror());
        return JNI_ERR;
    }

    // Call the core JNI_OnLoad synchronously so that classes can be found
    typedef jint (*JNI_OnLoadFunc)(JavaVM*, void*);
    JNI_OnLoadFunc CoreOnLoad = (JNI_OnLoadFunc)dlsym(handle, "JNI_OnLoad");
    
    if (CoreOnLoad) {
        LOGI("Payload loaded successfully. Forwarding JNI_OnLoad...");
        return CoreOnLoad(vm, reserved);
    } else {
        LOGE("dlsym failed: Could not find JNI_OnLoad in libCore.so");
    }

    return JNI_VERSION_1_6;
}
