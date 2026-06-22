#include "Jni.hpp"
#include <pthread.h>
#include <unistd.h>
#include "Includes/Logger.h"
#include "Includes/obfuscate.h"

extern void* hack_thread(void*);

void setText(JNIEnv *env, jobject obj, const char* text) {
    jclass textViewClass = env->FindClass(OBFUSCATE("android/widget/TextView"));
    jmethodID setTextMethod = env->GetMethodID(textViewClass, OBFUSCATE("setText"), OBFUSCATE("(Ljava/lang/CharSequence;)V"));
    jclass htmlClass = env->FindClass(OBFUSCATE("android/text/Html"));
    jmethodID fromHtmlMethod = env->GetStaticMethodID(htmlClass, OBFUSCATE("fromHtml"), OBFUSCATE("(Ljava/lang/String;)Landroid/text/Spanned;"));
    jobject spannedText = env->CallStaticObjectMethod(htmlClass, fromHtmlMethod, env->NewStringUTF(text));
    env->CallVoidMethod(obj, setTextMethod, spannedText);
}

void Toast(JNIEnv *env, jobject ctx, const char *text, int length) {
    jclass toastClass = env->FindClass(OBFUSCATE("android/widget/Toast"));
    jmethodID makeTextMethod = env->GetStaticMethodID(toastClass, OBFUSCATE("makeText"), OBFUSCATE("(Landroid/content/Context;Ljava/lang/CharSequence;I)Landroid/widget/Toast;"));
    jobject toastText = env->NewStringUTF(text);
    jobject toastObj = env->CallStaticObjectMethod(toastClass, makeTextMethod, ctx, toastText, length);
    jmethodID showMethod = env->GetMethodID(toastClass, OBFUSCATE("show"), OBFUSCATE("()V"));
    env->CallVoidMethod(toastObj, showMethod);
}

void Dialog(JNIEnv *env, jobject ctx, const char *title, const char *message, const char *openBtn, const char *closeBtn, int sec, const char *url) {
    LOGI("JNI: Dialog called: %s", title);
}

void startService(JNIEnv *env, jobject ctx) {
    jclass contextClass = env->FindClass(OBFUSCATE("android/content/Context"));
    jmethodID startServiceMethod = env->GetMethodID(contextClass, OBFUSCATE("startService"), OBFUSCATE("(Landroid/content/Intent;)Landroid/content/ComponentName;"));
    
    jclass intentClass = env->FindClass(OBFUSCATE("android/content/Intent"));
    jmethodID intentConstructor = env->GetMethodID(intentClass, OBFUSCATE("<init>"), OBFUSCATE("(Landroid/content/Context;Ljava/lang/Class;)V"));
    
    jclass serviceClass = env->FindClass(OBFUSCATE("com/android/support/Launcher"));
    jobject intent = env->NewObject(intentClass, intentConstructor, ctx, serviceClass);
    
    env->CallObjectMethod(ctx, startServiceMethod, intent);
}

void CheckOverlayPermission(JNIEnv *env, jclass clazz, jobject ctx) {
    LOGI("JNI: CheckOverlayPermission called (Payload implementation)");
    
    jclass mainClass = env->FindClass(OBFUSCATE("com/android/support/Main"));
    jmethodID requestPermissionMethod = env->GetStaticMethodID(mainClass, OBFUSCATE("requestPermission"), OBFUSCATE("(Landroid/content/Context;)V"));
    env->CallStaticVoidMethod(mainClass, requestPermissionMethod, ctx);

    static bool threadStarted = false;
    if (!threadStarted) {
        LOGI("JNI: Starting Gravity hack thread via pthread...");
        pthread_t t;
        pthread_create(&t, nullptr, hack_thread, nullptr);
        threadStarted = true;
    }

    // LOGI("JNI: Starting service...");
    // startService(env, ctx);
}

int get_api_sdk(JNIEnv *env) {
    jclass versionClass = env->FindClass(OBFUSCATE("android/os/Build$VERSION"));
    jfieldID sdkIntField = env->GetStaticFieldID(versionClass, OBFUSCATE("SDK_INT"), OBFUSCATE("I"));
    return env->GetStaticIntField(versionClass, sdkIntField);
}