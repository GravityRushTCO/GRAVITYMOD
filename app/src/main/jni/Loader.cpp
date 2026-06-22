#include <jni.h>
#include <dlfcn.h>
#include <string>
#include <fstream>
#include <android/log.h>
#include <thread>
#include <sys/stat.h>
#include <unistd.h>
#include <ctime>

#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, "ModLoader", __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, "ModLoader", __VA_ARGS__)

static JavaVM* g_jvm = nullptr;

std::string GetAppDataDir() {
    char path[256] = {0};
    FILE* f = fopen("/proc/self/cmdline", "rb");
    if (f) {
        fread(path, 1, sizeof(path) - 1, f);
        fclose(f);
        for (int i = 0; i < sizeof(path); i++) {
            if (path[i] == ':') {
                path[i] = 0;
                break;
            }
        }
        return std::string("/data/data/") + path + "/files/";
    }
    return "";
}

bool DownloadFileJNI(const std::string& urlStr, const std::string& outPath) {
    JNIEnv* env = nullptr;
    bool didAttach = false;
    if (g_jvm->GetEnv((void**)&env, JNI_VERSION_1_6) == JNI_EDETACHED) {
        if (g_jvm->AttachCurrentThread(&env, nullptr) != 0) return false;
        didAttach = true;
    }

    bool success = false;
    jclass urlClass = env->FindClass("java/net/URL");
    if (urlClass) {
        jmethodID urlInit = env->GetMethodID(urlClass, "<init>", "(Ljava/lang/String;)V");
        jstring jUrlStr = env->NewStringUTF(urlStr.c_str());
        jobject urlObj = env->NewObject(urlClass, urlInit, jUrlStr);
        if (env->ExceptionCheck()) { env->ExceptionDescribe(); env->ExceptionClear(); }

        jmethodID openConn = env->GetMethodID(urlClass, "openConnection", "()Ljava/net/URLConnection;");
        jobject connObj = env->CallObjectMethod(urlObj, openConn);
        if (env->ExceptionCheck()) { env->ExceptionDescribe(); env->ExceptionClear(); }

        if (connObj) {
            jclass connClass = env->GetObjectClass(connObj);
            
            jmethodID setReqProp = env->GetMethodID(connClass, "setRequestProperty", "(Ljava/lang/String;Ljava/lang/String;)V");
            if (setReqProp) {
                jstring key1 = env->NewStringUTF("Cache-Control");
                jstring val1 = env->NewStringUTF("no-cache, no-store, must-revalidate");
                env->CallVoidMethod(connObj, setReqProp, key1, val1);
                env->DeleteLocalRef(key1);
                env->DeleteLocalRef(val1);
            }

            jmethodID setConnTimeout = env->GetMethodID(connClass, "setConnectTimeout", "(I)V");
            jmethodID setReadTimeout  = env->GetMethodID(connClass, "setReadTimeout", "(I)V");
            if (setConnTimeout) env->CallVoidMethod(connObj, setConnTimeout, 15000);
            if (setReadTimeout) env->CallVoidMethod(connObj, setReadTimeout, 15000);

            jmethodID getRespCode = env->GetMethodID(connClass, "getResponseCode", "()I");
            int respCode = getRespCode ? env->CallIntMethod(connObj, getRespCode) : 0;
            if (env->ExceptionCheck()) { env->ExceptionDescribe(); env->ExceptionClear(); }

            if (respCode >= 200 && respCode < 300) {
                jmethodID getIn = env->GetMethodID(connClass, "getInputStream", "()Ljava/io/InputStream;");
                jobject inStream = getIn ? env->CallObjectMethod(connObj, getIn) : nullptr;
                if (inStream) {
                    std::ofstream outFile(outPath, std::ios::binary);
                    if (outFile.is_open()) {
                        jclass inCls = env->GetObjectClass(inStream);
                        jmethodID readM = env->GetMethodID(inCls, "read", "([B)I");
                        jmethodID closeM = env->GetMethodID(inCls, "close", "()V");
                        jbyteArray buf = env->NewByteArray(16384);
                        int n;
                        while (readM && (n = env->CallIntMethod(inStream, readM, buf)) > 0) {
                            jbyte* b = env->GetByteArrayElements(buf, nullptr);
                            outFile.write((char*)b, n);
                            env->ReleaseByteArrayElements(buf, b, JNI_ABORT);
                            if (env->ExceptionCheck()) { env->ExceptionDescribe(); env->ExceptionClear(); break; }
                        }
                        outFile.close();
                        success = true;
                        if (closeM) env->CallVoidMethod(inStream, closeM);
                    }
                }
            } else {
                LOGE("HTTP GET failed with code: %d", respCode);
            }
        }
    }
    
    if (didAttach) g_jvm->DetachCurrentThread();
    return success;
}

void BackgroundUpdateThread(std::string payloadPath, std::string serverUrl) {
    LOGI("Checking for background payload update...");
    std::string tmpPath = payloadPath + ".tmp";
    if (DownloadFileJNI(serverUrl, tmpPath)) {
        LOGI("Background update downloaded successfully. Applying for next launch.");
        chmod(tmpPath.c_str(), S_IRWXU);
        unlink(payloadPath.c_str());
        rename(tmpPath.c_str(), payloadPath.c_str());
    } else {
        LOGE("Background update failed.");
        remove(tmpPath.c_str());
    }
}

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* vm, void* reserved) {
    g_jvm = vm;
    LOGI("ModLoader JNI_OnLoad called.");

    // Check for fake ID reset marker
    if (access("/data/data/com.onestate.global/reset_pending", F_OK) == 0) {
        LOGI("Fake ID was active on close. Resetting PlayerPrefs...");
        system("rm -rf /data/data/com.onestate.global/shared_prefs/*");
        remove("/data/data/com.onestate.global/reset_pending");
    }
    
    std::string filesDir = GetAppDataDir();
    if (filesDir.empty()) {
        LOGE("Could not determine app data dir");
        return JNI_ERR;
    }

    std::string payloadPath = filesDir + "libCore.so";
    // GitHub Raw URL for free, reliable hosting. 
    // Add timestamp to bypass GitHub Raw CDN caching
    std::string serverUrl = "https://raw.githubusercontent.com/GravityRushTCO/GRAVITYMOD/main/Update/libCore.so";
    serverUrl += "?v=" + std::to_string(time(nullptr)); 

    // If payload does not exist, we MUST block and download it now so JNI can register methods
    if (access(payloadPath.c_str(), R_OK) != 0) {
        LOGI("First launch detected. Downloading payload synchronously...");
        
        // We must spawn a thread and join it to bypass Android's NetworkOnMainThreadException
        bool downloadSuccess = false;
        std::thread dlThread([&]() {
            downloadSuccess = DownloadFileJNI(serverUrl, payloadPath);
        });
        dlThread.join();

        if (!downloadSuccess) {
            LOGE("Synchronous download failed! Mod will not work.");
            return JNI_ERR;
        }
        chmod(payloadPath.c_str(), S_IRWXU);
    } else {
        LOGI("Local payload found. Launching background update thread.");
        std::thread(BackgroundUpdateThread, payloadPath, serverUrl).detach();
    }

    LOGI("Loading core from %s...", payloadPath.c_str());
    void* handle = dlopen(payloadPath.c_str(), RTLD_NOW | RTLD_GLOBAL);
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
