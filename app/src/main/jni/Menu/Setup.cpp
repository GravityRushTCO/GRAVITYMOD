#include "Includes/obfuscate.h"
#include "Menu/Menu.hpp"
#include "Utils.hpp"
#include "Includes/Logger.h"
#include "Menu/Jni.hpp"

int RegisterMenu(JNIEnv *env) {
    JNINativeMethod methods[] = {
            {OBFUSCATE("Icon"),            OBFUSCATE("()Ljava/lang/String;"),                                                           reinterpret_cast<void *>(Icon)},
            {OBFUSCATE("IconWebViewData"), OBFUSCATE("()Ljava/lang/String;"),                                                           reinterpret_cast<void *>(IconWebViewData)},
            {OBFUSCATE("IsGameLibLoaded"), OBFUSCATE("()Z"),                                                                            reinterpret_cast<void *>(isGameLibLoaded)},
            {OBFUSCATE("Init"),            OBFUSCATE("(Landroid/content/Context;Landroid/widget/TextView;Landroid/widget/TextView;)V"), reinterpret_cast<void *>(Init)},
            {OBFUSCATE("SettingsList"),    OBFUSCATE("()[Ljava/lang/String;"),                                                          reinterpret_cast<void *>(SettingsList)},
            {OBFUSCATE("GetFeatureList"),  OBFUSCATE("()[Ljava/lang/String;"),                                                          reinterpret_cast<void *>(GetFeatureList)},
    };

    jclass clazz = env->FindClass(OBFUSCATE("com/android/support/Menu"));
    if (!clazz) {
        LOGE("JNI: Class Menu not found!");
        return JNI_ERR;
    }
    if (env->RegisterNatives(clazz, methods, sizeof(methods) / sizeof(methods[0])) != 0) {
        LOGE("JNI: RegisterNatives for Menu failed!");
        return JNI_ERR;
    }
    LOGI("JNI: Menu registered successfully");
    return JNI_OK;
}

int RegisterPreferences(JNIEnv *env) {
    JNINativeMethod methods[] = {
            {OBFUSCATE("Changes"), OBFUSCATE("(Landroid/content/Context;ILjava/lang/String;IJZLjava/lang/String;)V"), reinterpret_cast<void *>(Changes)},
    };
    jclass clazz = env->FindClass(OBFUSCATE("com/android/support/Preferences"));
    if (!clazz) {
        LOGE("JNI: Class Preferences not found!");
        return JNI_ERR;
    }
    if (env->RegisterNatives(clazz, methods, sizeof(methods) / sizeof(methods[0])) != 0) {
        LOGE("JNI: RegisterNatives for Preferences failed!");
        return JNI_ERR;
    }
    LOGI("JNI: Preferences registered successfully");
    return JNI_OK;
}

int RegisterMain(JNIEnv *env) {
    JNINativeMethod methods[] = {
            {OBFUSCATE("CheckOverlayPermission"), OBFUSCATE("(Landroid/content/Context;)V"),
             reinterpret_cast<void *>(CheckOverlayPermission)},
    };
    jclass clazz = env->FindClass(OBFUSCATE("com/android/support/Main"));
    if (!clazz) {
        LOGE("JNI: Class Main not found!");
        return JNI_ERR;
    }
    if (env->RegisterNatives(clazz, methods, sizeof(methods) / sizeof(methods[0])) != 0) {
        LOGE("JNI: RegisterNatives for Main failed!");
        return JNI_ERR;
    }
    LOGI("JNI: Main registered successfully");
    return JNI_OK;
}

extern "C" {
    void Java_com_android_support_EspBridge_setScreenSize(JNIEnv* env, jclass, jint w, jint h);
    void Java_com_android_support_EspBridge_setEnabled(JNIEnv* env, jclass, jboolean en);
    jboolean Java_com_android_support_EspBridge_isEnabled(JNIEnv* env, jclass);
    jfloatArray Java_com_android_support_EspBridge_getData(JNIEnv* env, jclass);
    jfloatArray Java_com_android_support_EspBridge_getViewProjection(JNIEnv* env, jclass);
    jobjectArray Java_com_android_support_EspBridge_getPedNames(JNIEnv* env, jclass);
    jboolean Java_com_android_support_EspBridge_isDotMode(JNIEnv* env, jclass);
    jboolean Java_com_android_support_EspBridge_isLineEnabled(JNIEnv*, jclass);
    jboolean Java_com_android_support_EspBridge_isBoxEnabled(JNIEnv*, jclass);
    jboolean Java_com_android_support_EspBridge_isDistanceEnabled(JNIEnv*, jclass);
    jboolean Java_com_android_support_EspBridge_isMarkerEnabled(JNIEnv*, jclass);
    jboolean Java_com_android_support_EspBridge_isDynamicColor(JNIEnv*, jclass);
    jboolean Java_com_android_support_EspBridge_isCrosshairEnabled(JNIEnv*, jclass);
    jboolean Java_com_android_support_EspBridge_isCrosshairCircleEnabled(JNIEnv*, jclass);
    jint Java_com_android_support_EspBridge_getCrosshairCircleRadius(JNIEnv*, jclass);
    jboolean Java_com_android_support_EspBridge_isHealthEnabled(JNIEnv*, jclass);
}

int RegisterEspBridge(JNIEnv *env) {
    JNINativeMethod methods[] = {
        {OBFUSCATE("setScreenSize"), OBFUSCATE("(II)V"), reinterpret_cast<void *>(Java_com_android_support_EspBridge_setScreenSize)},
        {OBFUSCATE("setEnabled"), OBFUSCATE("(Z)V"), reinterpret_cast<void *>(Java_com_android_support_EspBridge_setEnabled)},
        {OBFUSCATE("isEnabled"), OBFUSCATE("()Z"), reinterpret_cast<void *>(Java_com_android_support_EspBridge_isEnabled)},
        {OBFUSCATE("getData"), OBFUSCATE("()[F"), reinterpret_cast<void *>(Java_com_android_support_EspBridge_getData)},
        {OBFUSCATE("getViewProjection"), OBFUSCATE("()[F"), reinterpret_cast<void *>(Java_com_android_support_EspBridge_getViewProjection)},
        {OBFUSCATE("getPedNames"), OBFUSCATE("()[Ljava/lang/String;"), reinterpret_cast<void *>(Java_com_android_support_EspBridge_getPedNames)},
        {OBFUSCATE("isDotMode"), OBFUSCATE("()Z"), reinterpret_cast<void *>(Java_com_android_support_EspBridge_isDotMode)},
        {OBFUSCATE("isLineEnabled"), OBFUSCATE("()Z"), reinterpret_cast<void *>(Java_com_android_support_EspBridge_isLineEnabled)},
        {OBFUSCATE("isBoxEnabled"), OBFUSCATE("()Z"), reinterpret_cast<void *>(Java_com_android_support_EspBridge_isBoxEnabled)},
        {OBFUSCATE("isDistanceEnabled"), OBFUSCATE("()Z"), reinterpret_cast<void *>(Java_com_android_support_EspBridge_isDistanceEnabled)},
        {OBFUSCATE("isMarkerEnabled"), OBFUSCATE("()Z"), reinterpret_cast<void *>(Java_com_android_support_EspBridge_isMarkerEnabled)},
        {OBFUSCATE("isDynamicColor"), OBFUSCATE("()Z"), reinterpret_cast<void *>(Java_com_android_support_EspBridge_isDynamicColor)},
        {OBFUSCATE("isCrosshairEnabled"), OBFUSCATE("()Z"), reinterpret_cast<void *>(Java_com_android_support_EspBridge_isCrosshairEnabled)},
        {OBFUSCATE("isCrosshairCircleEnabled"), OBFUSCATE("()Z"), reinterpret_cast<void *>(Java_com_android_support_EspBridge_isCrosshairCircleEnabled)},
        {OBFUSCATE("getCrosshairCircleRadius"), OBFUSCATE("()I"), reinterpret_cast<void *>(Java_com_android_support_EspBridge_getCrosshairCircleRadius)},
        {OBFUSCATE("isHealthEnabled"), OBFUSCATE("()Z"), reinterpret_cast<void *>(Java_com_android_support_EspBridge_isHealthEnabled)},
    };
    jclass clazz = env->FindClass(OBFUSCATE("com/android/support/EspBridge"));
    if (!clazz) {
        LOGE("JNI: Class EspBridge not found!");
        return JNI_ERR;
    }
    if (env->RegisterNatives(clazz, methods, sizeof(methods) / sizeof(methods[0])) != 0) {
        LOGE("JNI: RegisterNatives for EspBridge failed!");
        return JNI_ERR;
    }
    LOGI("JNI: EspBridge registered successfully");
    return JNI_OK;
}

extern "C" {
    jboolean Java_com_chillbase_games_ChillbaseActivity_nativeTouch(JNIEnv *env, jclass clazz, jint action, jfloat x, jfloat y, jfloat viewW, jfloat viewH);
}

int RegisterChillbaseActivity(JNIEnv *env) {
    JNINativeMethod methods[] = {
            {OBFUSCATE("nativeTouch"), OBFUSCATE("(IFFFF)Z"), reinterpret_cast<void *>(Java_com_chillbase_games_ChillbaseActivity_nativeTouch)},
    };
    jclass clazz = env->FindClass(OBFUSCATE("com/chillbase/games/ChillbaseActivity"));
    if (!clazz) {
        env->ExceptionClear();
        LOGE("JNI: Class ChillbaseActivity not found!");
        return JNI_ERR;
    }
    if (env->RegisterNatives(clazz, methods, sizeof(methods) / sizeof(methods[0])) != 0) {
        env->ExceptionClear();
        LOGE("JNI: RegisterNatives for ChillbaseActivity failed!");
        return JNI_ERR;
    }
    LOGI("JNI: ChillbaseActivity registered successfully");
    return JNI_OK;
}

// Stocke le pointeur JavaVM globalement pour les modules qui doivent attacher
// un thread non-Java (Hwid.cpp). JNI_GetCreatedJavaVMs n'est PAS exporté par
// les libs NDK Android, on se sert donc de cette variable.
JavaVM* g_GravityJVM = nullptr;

// Exported function to be called by Cloud Loader
extern "C"
JNIEXPORT void JNICALL
Payload_Init(JNIEnv *env, jobject ctx) {
    JavaVM* vm;
    env->GetJavaVM(&vm);
    g_GravityJVM = vm;
    
    LOGI("Payload_Init called by Cloud Loader!");
    
    // Enregistrer les JNI natives du mod menu réel
    RegisterMenu(env);
    RegisterPreferences(env);
    // On n'enregistre plus Main (CheckOverlayPermission) car c'est le Loader qui a reçu cet appel !
    // RegisterMain(env); 
    RegisterEspBridge(env);
    RegisterChillbaseActivity(env);
    
    jclass localClass = env->FindClass("com/android/support/DialogHelper");
    if (localClass != nullptr) {
        extern jclass g_DialogHelperClass;
        g_DialogHelperClass = (jclass)env->NewGlobalRef(localClass);
        env->DeleteLocalRef(localClass);
    } else {
        env->ExceptionClear();
    }
    
    // Au lieu que Java appelle CheckOverlayPermission sur notre lib, le Loader nous passe l'appel.
    // On doit donc appeler manuellement la logique d'origine de CheckOverlayPermission.
    jclass mainClass = env->FindClass(OBFUSCATE("com/android/support/Main"));
    CheckOverlayPermission(env, mainClass, ctx);
    
    extern void setupGravityOverlay();
    setupGravityOverlay();

    LOGI("Payload_Init finished successfully");
}
extern void* hack_thread(void*);

jclass g_DialogHelperClass = nullptr;

extern "C" JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* vm, void* reserved) {
    JNIEnv* env;
    if (vm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6) != JNI_OK) {
        return JNI_ERR;
    }
    g_GravityJVM = vm;
    RegisterMenu(env);
    RegisterPreferences(env);
    RegisterMain(env);
    RegisterEspBridge(env);
    RegisterChillbaseActivity(env);

    jclass localClass = env->FindClass("com/android/support/DialogHelper");
    if (localClass != nullptr) {
        g_DialogHelperClass = (jclass)env->NewGlobalRef(localClass);
        env->DeleteLocalRef(localClass);
    } else {
        env->ExceptionClear();
    }

    static bool threadStarted = false;
    if (!threadStarted) {
        LOGI("JNI: Starting Gravity hack thread from JNI_OnLoad...");
        pthread_t t;
        pthread_create(&t, nullptr, hack_thread, nullptr);
        threadStarted = true;
    }

    extern void setupGravityOverlay();
    setupGravityOverlay();

    return JNI_VERSION_1_6;
}
