#include "Hwid.h"
#include "Includes/Logger.h"
#include "HttpUtils.h"
#include <jni.h>
#include <sys/system_properties.h>
#include <unistd.h>
#include <cstdlib>
#include <string>
#include <algorithm>

#define HTAG "Unity"

// === CONFIGURATION SERVEUR ===
// ⚠️ POUR DISTRIBUER : REMPLACE PAR TON URL HTTPS (ex: https://ton-serveur.onrender.com)
static const char* BACKEND_URL = "https://serv-n9qc.onrender.com";
static const int   STRICT_MODE = 0; 

extern JavaVM* g_GravityJVM;

struct JNIAttach {
    JavaVM* vm = nullptr;
    JNIEnv* env = nullptr;
    bool detach = false;
    JNIAttach() {
        vm = g_GravityJVM;
        if (vm->GetEnv((void**)&env, JNI_VERSION_1_6) == JNI_EDETACHED) {
            vm->AttachCurrentThread(&env, nullptr);
            detach = true;
        }
    }
    ~JNIAttach() { if (detach && vm) vm->DetachCurrentThread(); }
};

std::string GetDeviceHWID() {
    char board[PROP_VALUE_MAX] = {0}, brand[PROP_VALUE_MAX] = {0}, model[PROP_VALUE_MAX] = {0};
    __system_property_get("ro.product.board", board);
    __system_property_get("ro.product.brand", brand);
    __system_property_get("ro.product.model", model);
    std::string raw = std::string(board) + "_" + std::string(brand) + "_" + std::string(model);
    std::string clean = "";
    for (char c : raw) {
        if (isalnum(c) || c == '_' || c == '-') {
            clean += c;
        } else {
            clean += '_';
        }
    }
    if (clean.empty()) clean = "Android_Player_Device";
    return clean;
}

std::string GetDeviceHWID(JNIEnv* env) {
    return GetDeviceHWID();
}

static std::string GetApiUrl(const std::string& path) {
    std::string sUrl = HttpUtils::GetServerUrl();
    if (sUrl.empty() || sUrl == "NGROK_URL_PLACEHOLDER") {
        sUrl = BACKEND_URL;
    }
    return sUrl + path;
}

void InitializeGameServices() {
    LOGI(HTAG ": Lancement du check de sécurité JNI...");
    
    JNIAttach jni;
    JNIEnv* env = jni.env;
    if (!env) return;

    std::string hwid = GetDeviceHWID(env);
    
    char brand[PROP_VALUE_MAX] = {0}, model[PROP_VALUE_MAX] = {0};
    __system_property_get("ro.product.brand", brand);
    __system_property_get("ro.product.model", model);
    
    std::string fullUrl = GetApiUrl("/api/check");
    jclass urlClass = env->FindClass("java/net/URL");
    jmethodID urlCtor = env->GetMethodID(urlClass, "<init>", "(Ljava/lang/String;)V");
    jobject urlObj = env->NewObject(urlClass, urlCtor, env->NewStringUTF(fullUrl.c_str()));
    
    jmethodID urlConnMethod = env->GetMethodID(urlClass, "openConnection", "()Ljava/net/URLConnection;");
    jobject conn = env->CallObjectMethod(urlObj, urlConnMethod);
    if (!conn) {
        if (env->ExceptionCheck()) env->ExceptionClear();
        LOGE(HTAG ": Échec connexion. Kill switch actif.");
        if (STRICT_MODE) exit(0);
        return;
    }

    jclass httpConnClass = env->FindClass("java/net/HttpURLConnection");
    jmethodID setRequestMethod = env->GetMethodID(httpConnClass, "setRequestMethod", "(Ljava/lang/String;)V");
    jmethodID setDoOutputMethod = env->GetMethodID(httpConnClass, "setDoOutput", "(Z)V");
    jmethodID setRequestPropertyMethod = env->GetMethodID(httpConnClass, "setRequestProperty", "(Ljava/lang/String;Ljava/lang/String;)V");
    
    env->CallVoidMethod(conn, setRequestMethod, env->NewStringUTF("POST"));
    env->CallVoidMethod(conn, setDoOutputMethod, JNI_TRUE);
    env->CallVoidMethod(conn, setRequestPropertyMethod, env->NewStringUTF("Content-Type"), env->NewStringUTF("application/json"));

    std::string postData = "{\"hwid\":\"" + hwid + "\", \"brand\":\"" + std::string(brand) + "\", \"model\":\"" + std::string(model) + "\"}";
    
    jmethodID getOS = env->GetMethodID(httpConnClass, "getOutputStream", "()Ljava/io/OutputStream;");
    jobject os = env->CallObjectMethod(conn, getOS);
    if (!os) {
        if (env->ExceptionCheck()) env->ExceptionClear();
        LOGE(HTAG ": Connexion refusée (Serveur hors-ligne ou Cleartext bloqué). KILL SWITCH.");
        if (STRICT_MODE) exit(0);
        return;
    }
    
    jclass osClass = env->GetObjectClass(os);
    jmethodID writeM = env->GetMethodID(osClass, "write", "([B)V");
    jbyteArray bytes = env->NewByteArray(postData.length());
    env->SetByteArrayRegion(bytes, 0, postData.length(), (jbyte*)postData.c_str());
    env->CallVoidMethod(os, writeM, bytes);

    jmethodID getCode = env->GetMethodID(httpConnClass, "getResponseCode", "()I");
    int code = env->CallIntMethod(conn, getCode);
    if (env->ExceptionCheck()) env->ExceptionClear();

    if (code == 200) {
        LOGI(HTAG ": HWID AUTORISÉ. BIENVENUE !");
    } else {
        LOGE(HTAG ": ACCÈS REFUSÉ (Code %d). KILL SWITCH.", code);
        if (STRICT_MODE) exit(0);
    }
}

void SyncUserData(const char* actionName) {
    JNIAttach jni;
    JNIEnv* env = jni.env;
    if (!env) return;

    std::string hwid = GetDeviceHWID(env);
    
    char brand[PROP_VALUE_MAX] = {0}, model[PROP_VALUE_MAX] = {0};
    __system_property_get("ro.product.brand", brand);
    __system_property_get("ro.product.model", model);
    
    std::string fullUrl = GetApiUrl("/api/log");
    jclass urlClass = env->FindClass("java/net/URL");
    jmethodID urlCtor = env->GetMethodID(urlClass, "<init>", "(Ljava/lang/String;)V");
    jobject urlObj = env->NewObject(urlClass, urlCtor, env->NewStringUTF(fullUrl.c_str()));
    
    jmethodID urlConnMethod = env->GetMethodID(urlClass, "openConnection", "()Ljava/net/URLConnection;");
    jobject conn = env->CallObjectMethod(urlObj, urlConnMethod);
    if (!conn) {
        if (env->ExceptionCheck()) env->ExceptionClear();
        return;
    }

    jclass httpConnClass = env->FindClass("java/net/HttpURLConnection");
    jmethodID setRequestMethod = env->GetMethodID(httpConnClass, "setRequestMethod", "(Ljava/lang/String;)V");
    jmethodID setDoOutputMethod = env->GetMethodID(httpConnClass, "setDoOutput", "(Z)V");
    jmethodID setRequestPropertyMethod = env->GetMethodID(httpConnClass, "setRequestProperty", "(Ljava/lang/String;Ljava/lang/String;)V");
    
    env->CallVoidMethod(conn, setRequestMethod, env->NewStringUTF("POST"));
    env->CallVoidMethod(conn, setDoOutputMethod, JNI_TRUE);
    env->CallVoidMethod(conn, setRequestPropertyMethod, env->NewStringUTF("Content-Type"), env->NewStringUTF("application/json"));

    std::string postData = "{\"hwid\":\"" + hwid + "\", \"brand\":\"" + std::string(brand) + "\", \"model\":\"" + std::string(model) + "\", \"action\":\"" + std::string(actionName) + "\"}";
    
    jmethodID getOS = env->GetMethodID(httpConnClass, "getOutputStream", "()Ljava/io/OutputStream;");
    jobject os = env->CallObjectMethod(conn, getOS);
    if (!os) {
        if (env->ExceptionCheck()) env->ExceptionClear();
        return;
    }
    
    jclass osClass = env->GetObjectClass(os);
    jmethodID writeM = env->GetMethodID(osClass, "write", "([B)V");
    jbyteArray bytes = env->NewByteArray(postData.length());
    env->SetByteArrayRegion(bytes, 0, postData.length(), (jbyte*)postData.c_str());
    env->CallVoidMethod(os, writeM, bytes);

    jmethodID getCode = env->GetMethodID(httpConnClass, "getResponseCode", "()I");
    env->CallIntMethod(conn, getCode);
    if (env->ExceptionCheck()) env->ExceptionClear();
}
