#include "HttpUtils.h"
#include "GravityServerConfig.h"
#include "LicenseSystem.h"
#include <thread>
#include <atomic>
#include <android/log.h>
#include "Includes/obfuscate.h"

#define LOG_TAG "GravityHTTP"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,  LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

static JavaVM* s_jvm = nullptr;

// URL active courante (peut être changée en cours d'exécution)
static std::string s_serverUrl = "";

namespace HttpUtils {

void Init(JavaVM* jvm) {
    s_jvm = jvm;
    // Démarrer avec l'URL publique, fallback local
    s_serverUrl = GRAVITY_SERVER_PUBLIC_URL;
    LOGI("HttpUtils: server URL = %s", s_serverUrl.c_str());
}

void SetServerUrl(const std::string& url) {
    s_serverUrl = url;
    LOGI("HttpUtils: URL updated → %s", url.c_str());
}

const std::string& GetServerUrl() {
    return s_serverUrl;
}

// ─── Requête HTTP interne ───────────────────────────────────────────────────
static std::string DoHttpRequest(const std::string& urlStr,
                                  const std::string& method,
                                  const std::string& jsonBody) {
    if (!s_jvm) { LOGE("HttpUtils non initialisé."); return ""; }

    JNIEnv* env = nullptr;
    bool didAttach = false;
    int status = s_jvm->GetEnv((void**)&env, JNI_VERSION_1_6);
    if (status == JNI_EDETACHED) {
        if (s_jvm->AttachCurrentThread(&env, nullptr) != 0) {
            LOGE("Failed to attach thread"); return "";
        }
        didAttach = true;
    }

    std::string responseData;

    // ── Try/catch via JNI exception handling ───────────────────────────────
    auto cleanup = [&]() {
        if (env->ExceptionCheck()) env->ExceptionClear();
        if (didAttach) s_jvm->DetachCurrentThread();
    };

    jclass urlClass = env->FindClass("java/net/URL");
    if (!urlClass) { cleanup(); return ""; }

    jmethodID urlInit = env->GetMethodID(urlClass, "<init>", "(Ljava/lang/String;)V");
    if (!urlInit || env->ExceptionCheck()) { cleanup(); return ""; }

    jstring jUrlStr = env->NewStringUTF(urlStr.c_str());
    if (!jUrlStr) { cleanup(); return ""; }

    jobject urlObj = env->NewObject(urlClass, urlInit, jUrlStr);
    if (!urlObj || env->ExceptionCheck()) { env->ExceptionClear(); cleanup(); return ""; }

    jmethodID openConn = env->GetMethodID(urlClass, "openConnection", "()Ljava/net/URLConnection;");
    if (!openConn) { cleanup(); return ""; }

    jobject connObj = env->CallObjectMethod(urlObj, openConn);
    if (!connObj || env->ExceptionCheck()) { env->ExceptionClear(); cleanup(); return ""; }

    jclass connClass = env->GetObjectClass(connObj);

    // Set timeout
    jmethodID setConnTimeout = env->GetMethodID(connClass, "setConnectTimeout", "(I)V");
    jmethodID setReadTimeout  = env->GetMethodID(connClass, "setReadTimeout",    "(I)V");
    if (setConnTimeout) env->CallVoidMethod(connObj, setConnTimeout, GRAVITY_HTTP_TIMEOUT_MS);
    if (setReadTimeout)  env->CallVoidMethod(connObj, setReadTimeout,  GRAVITY_HTTP_TIMEOUT_MS);
    if (env->ExceptionCheck()) env->ExceptionClear();

    jmethodID setMethod = env->GetMethodID(connClass, "setRequestMethod", "(Ljava/lang/String;)V");
    jstring jMethod = env->NewStringUTF(method.c_str());
    if (setMethod && jMethod) env->CallVoidMethod(connObj, setMethod, jMethod);
    if (env->ExceptionCheck()) env->ExceptionClear();

    jmethodID setProp = env->GetMethodID(connClass, "setRequestProperty", "(Ljava/lang/String;Ljava/lang/String;)V");
    if (setProp) {
        if (method == "POST" && !jsonBody.empty()) {
            jstring k1 = env->NewStringUTF("Content-Type");
            jstring v1 = env->NewStringUTF("application/json; charset=utf-8");
            env->CallVoidMethod(connObj, setProp, k1, v1);
            env->DeleteLocalRef(k1); env->DeleteLocalRef(v1);
        }

        // Supabase Authentication Header injection (for any request method)
        if (urlStr.find((const char*)OBFUSCATE("supabase.co")) != std::string::npos) {
            std::string anon_key = License::SUPABASE_ANON_KEY;

            jstring k2 = env->NewStringUTF((const char*)OBFUSCATE("apikey"));
            jstring v2 = env->NewStringUTF(anon_key.c_str());
            env->CallVoidMethod(connObj, setProp, k2, v2);
            env->DeleteLocalRef(k2); env->DeleteLocalRef(v2);

            // Omit Authorization header unless the key starts with "eyJ" (JWT format)
            if (anon_key.size() >= 3 && anon_key[0] == 'e' && anon_key[1] == 'y' && anon_key[2] == 'J') {
                jstring k3 = env->NewStringUTF((const char*)OBFUSCATE("Authorization"));
                std::string bearer = std::string((const char*)OBFUSCATE("Bearer ")) + anon_key;
                jstring v3 = env->NewStringUTF(bearer.c_str());
                env->CallVoidMethod(connObj, setProp, k3, v3);
                env->DeleteLocalRef(k3); env->DeleteLocalRef(v3);
            }
        }
    }

    if (method == "POST" && !jsonBody.empty()) {
        jmethodID setDoOut = env->GetMethodID(connClass, "setDoOutput", "(Z)V");
        if (setDoOut) env->CallVoidMethod(connObj, setDoOut, JNI_TRUE);

        jmethodID getOut = env->GetMethodID(connClass, "getOutputStream", "()Ljava/io/OutputStream;");
        jobject outStream = getOut ? env->CallObjectMethod(connObj, getOut) : nullptr;
        if (env->ExceptionCheck()) { env->ExceptionClear(); outStream = nullptr; }
        if (outStream) {
            jclass outCls = env->GetObjectClass(outStream);
            jmethodID write = env->GetMethodID(outCls, "write", "([B)V");
            jmethodID flush = env->GetMethodID(outCls, "flush", "()V");
            jmethodID close = env->GetMethodID(outCls, "close", "()V");
            jbyteArray jBytes = env->NewByteArray(jsonBody.size());
            env->SetByteArrayRegion(jBytes, 0, jsonBody.size(), (const jbyte*)jsonBody.data());
            if (write) env->CallVoidMethod(outStream, write, jBytes);
            if (flush) env->CallVoidMethod(outStream, flush);
            if (close) env->CallVoidMethod(outStream, close);
            if (env->ExceptionCheck()) env->ExceptionClear();
            env->DeleteLocalRef(jBytes); env->DeleteLocalRef(outCls); env->DeleteLocalRef(outStream);
        }
    }

    jmethodID getRespCode = env->GetMethodID(connClass, "getResponseCode", "()I");
    int respCode = 0;
    if (getRespCode) {
        respCode = env->CallIntMethod(connObj, getRespCode);
        if (env->ExceptionCheck()) { env->ExceptionClear(); respCode = 0; }
    }

    if (respCode >= 200 && respCode < 300) {
        jmethodID getIn = env->GetMethodID(connClass, "getInputStream", "()Ljava/io/InputStream;");
        jobject inStream = getIn ? env->CallObjectMethod(connObj, getIn) : nullptr;
        if (env->ExceptionCheck()) { env->ExceptionClear(); inStream = nullptr; }
        if (inStream) {
            jclass inCls = env->GetObjectClass(inStream);
            jmethodID readM  = env->GetMethodID(inCls, "read",  "([B)I");
            jmethodID closeM = env->GetMethodID(inCls, "close", "()V");
            jbyteArray buf = env->NewByteArray(4096);
            int n;
            while (readM && (n = env->CallIntMethod(inStream, readM, buf)) > 0) {
                jbyte* b = env->GetByteArrayElements(buf, nullptr);
                responseData.append((char*)b, n);
                env->ReleaseByteArrayElements(buf, b, JNI_ABORT);
                if (env->ExceptionCheck()) { env->ExceptionClear(); break; }
            }
            if (closeM) env->CallVoidMethod(inStream, closeM);
            if (env->ExceptionCheck()) env->ExceptionClear();
            env->DeleteLocalRef(buf); env->DeleteLocalRef(inCls); env->DeleteLocalRef(inStream);
        }
    } else if (respCode > 0) {
        LOGE("HTTP %s → %d", urlStr.c_str(), respCode);
        jmethodID getErr = env->GetMethodID(connClass, "getErrorStream", "()Ljava/io/InputStream;");
        jobject errStream = getErr ? env->CallObjectMethod(connObj, getErr) : nullptr;
        if (env->ExceptionCheck()) { env->ExceptionClear(); errStream = nullptr; }
        if (errStream) {
            jclass inCls = env->GetObjectClass(errStream);
            jmethodID readM  = env->GetMethodID(inCls, "read",  "([B)I");
            jmethodID closeM = env->GetMethodID(inCls, "close", "()V");
            jbyteArray buf = env->NewByteArray(4096);
            int n;
            while (readM && (n = env->CallIntMethod(errStream, readM, buf)) > 0) {
                jbyte* b = env->GetByteArrayElements(buf, nullptr);
                responseData.append((char*)b, n);
                env->ReleaseByteArrayElements(buf, b, JNI_ABORT);
                if (env->ExceptionCheck()) { env->ExceptionClear(); break; }
            }
            if (closeM) env->CallVoidMethod(errStream, closeM);
            if (env->ExceptionCheck()) env->ExceptionClear();
            env->DeleteLocalRef(buf); env->DeleteLocalRef(inCls); env->DeleteLocalRef(errStream);
        }
    }

    if (jMethod) env->DeleteLocalRef(jMethod);
    env->DeleteLocalRef(connClass);
    env->DeleteLocalRef(connObj);
    env->DeleteLocalRef(jUrlStr);
    env->DeleteLocalRef(urlClass);

    cleanup();
    return responseData;
}

// ─── API publique ───────────────────────────────────────────────────────────
void GetAsync(const std::string& url, void (*cb)(const std::string&)) {
    std::thread([url, cb]() {
        std::string r = DoHttpRequest(url, "GET", "");
        // Fallback: si URL publique échoue et que c'est bien la publique, retry local
        if (r.empty() && url.find("192.168.") == std::string::npos &&
            url.find("localhost") == std::string::npos) {
            std::string localUrl = url;
            // Reconstruire avec URL locale
            std::string path = "/";
            auto pos = url.find('/', url.find("://") + 3);
            if (pos != std::string::npos) path = url.substr(pos);
            localUrl = GRAVITY_SERVER_LOCAL_URL + path;
            LOGI("Public URL failed, retrying local: %s", localUrl.c_str());
            r = DoHttpRequest(localUrl, "GET", "");
        }
        if (cb) cb(r);
    }).detach();
}

void DeleteAsync(const std::string& url, void (*cb)(const std::string&)) {
    std::thread([url, cb]() {
        std::string r = DoHttpRequest(url, "DELETE", "");
        if (cb) cb(r);
    }).detach();
}

void PostAsync(const std::string& url, const std::string& body, void (*cb)(const std::string&)) {
    std::thread([url, body, cb]() {
        std::string r = DoHttpRequest(url, "POST", body);
        if (r.empty() && url.find("192.168.") == std::string::npos &&
            url.find("localhost") == std::string::npos) {
            std::string path = "/";
            auto pos = url.find('/', url.find("://") + 3);
            if (pos != std::string::npos) path = url.substr(pos);
            std::string localUrl = GRAVITY_SERVER_LOCAL_URL + path;
            r = DoHttpRequest(localUrl, "POST", body);
        }
        if (cb) cb(r);
    }).detach();
}

void SendDebugLog(const std::string& level, const std::string& message, const std::string& context) {
    // Escape quotes in message and context for JSON
    std::string escapedMsg = message;
    size_t pos = 0;
    while ((pos = escapedMsg.find("\"", pos)) != std::string::npos) {
        escapedMsg.replace(pos, 1, "\\\"");
        pos += 2;
    }
    
    std::string escapedCtx = context;
    pos = 0;
    while ((pos = escapedCtx.find("\"", pos)) != std::string::npos) {
        escapedCtx.replace(pos, 1, "\\\"");
        pos += 2;
    }

    // Call GetDeviceHWID from Main.cpp if needed, but for simplicity here we just use a generic or pass it.
    // To avoid dependency loops, we'll format a basic JSON.
    extern const char* GetDeviceHWID(); 
    std::string hwid = "Unknown";
    try { hwid = GetDeviceHWID(); } catch(...) {}

    std::string jsonBody = "{"
                           "\"device\":\"" + hwid + "\","
                           "\"level\":\"" + level + "\","
                           "\"message\":\"" + escapedMsg + "\","
                           "\"context\":\"" + escapedCtx + "\""
                           "}";
                           
    PostAsync(GetServerUrl() + "/api/logs", jsonBody);
}

} // namespace HttpUtils
