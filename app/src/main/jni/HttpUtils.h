#pragma once
#include <jni.h>
#include <string>

namespace HttpUtils {
    void Init(JavaVM* jvm);
    void SetServerUrl(const std::string& url);
    const std::string& GetServerUrl();
    void GetAsync(const std::string& url, void (*callback)(const std::string& response));
    void DeleteAsync(const std::string& url, void (*callback)(const std::string& response) = nullptr);
    void PostAsync(const std::string& url, const std::string& jsonBody, void (*callback)(const std::string& response) = nullptr);
    void SendDebugLog(const std::string& level, const std::string& message, const std::string& context = "");
}
