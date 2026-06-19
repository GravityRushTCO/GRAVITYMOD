#pragma once
#include <vector>
#include <string>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <android/log.h>
#include "../il2cpp_bridge.h"

#define E_LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "GravityGL", __VA_ARGS__))
#define E_LOGE(...) ((void)__android_log_print(ANDROID_LOG_ERROR, "GravityGL", __VA_ARGS__))

// Structure representing a memory region
struct VipMemoryRegion {
    uintptr_t start;
    uintptr_t end;
};

// Safe memory write using /proc/self/mem or mprotect fallback
inline bool VipHacks_SafeWrite(void* addr, const void* data, size_t len) {
    static int mem_fd = -1;
    if (mem_fd == -1) {
        mem_fd = open("/proc/self/mem", O_WRONLY);
    }
    if (mem_fd == -1) {
        long pageSize = sysconf(_SC_PAGESIZE);
        uintptr_t pageStart = ((uintptr_t)addr) & ~(pageSize - 1);
        if (mprotect((void*)pageStart, pageSize * 2, PROT_READ | PROT_WRITE | PROT_EXEC) != 0) {
            return false;
        }
        memcpy(addr, data, len);
        return true;
    }
    ssize_t bytes = pwrite(mem_fd, data, len, (off_t)addr);
    if (bytes == (ssize_t)len) return true;

    long pageSize = sysconf(_SC_PAGESIZE);
    uintptr_t pageStart = ((uintptr_t)addr) & ~(pageSize - 1);
    if (mprotect((void*)pageStart, pageSize * 2, PROT_READ | PROT_WRITE | PROT_EXEC) == 0) {
        memcpy(addr, data, len);
        return true;
    }
    return false;
}

// Caching addresses for memory scanning patterns
static uintptr_t s_speedRunCached = 0;
static uintptr_t s_bigJumpCached1 = 0;
static uintptr_t s_bigJumpCached2 = 0;
static uintptr_t s_wallHackCached = 0;
static uintptr_t s_noRecoilCached = 0;
static uintptr_t s_superRecoilCached = 0;
static uintptr_t s_staminaCached = 0;
static uintptr_t s_moveToVehicleCached = 0;
static uintptr_t s_speedOfMovementCached = 0;

static std::vector<uintptr_t> s_globalSpeedCached;
static std::vector<uintptr_t> s_cameraDistanceCached;
static std::vector<uintptr_t> s_graphicsQualityCached;

// Scan and replace using /proc/self/mem
inline int VipHacks_ScanAndReplace(const uint8_t* findPattern, const char* mask, const uint8_t* replacePattern, size_t patternLen, uintptr_t& cachedAddr) {
    if (cachedAddr != 0) {
        if (VipHacks_SafeWrite((void*)cachedAddr, replacePattern, patternLen)) {
            return 1;
        }
        cachedAddr = 0;
    }
    
    int mem_fd = open("/proc/self/mem", O_RDONLY);
    if (mem_fd == -1) return 0;
    
    std::vector<VipMemoryRegion> regions;
    FILE *f = fopen("/proc/self/maps", "r");
    if (f) {
        char line[512];
        while (fgets(line, sizeof(line), f)) {
            if (strstr(line, "r-") || strstr(line, "rw-")) {
                char path[256] = {0};
                uintptr_t start = 0, end = 0;
                int fields = sscanf(line, "%lx-%lx %*s %*s %*s %*s %255s", &start, &end, path);
                bool isCodeOrLib = false;
                if (fields == 3) {
                    if (strstr(path, ".so") || strstr(path, ".apk") || strstr(path, ".jar") || 
                        strstr(path, ".odex") || strstr(path, ".oat") || strstr(path, "/system/") || 
                        strstr(path, "/apex/")) {
                        isCodeOrLib = true;
                    }
                }
                if (!isCodeOrLib) {
                    regions.push_back({start, end});
                }
            }
        }
        fclose(f);
    }
    
    int replaceCount = 0;
    std::vector<uint8_t> buffer(4096 * 16); // 64KB scan buffer
    for (const auto& reg : regions) {
        uintptr_t cur = reg.start;
        while (cur < reg.end) {
            size_t toRead = reg.end - cur;
            if (toRead > buffer.size()) toRead = buffer.size();
            
            ssize_t bytes = pread(mem_fd, buffer.data(), toRead, (off_t)cur);
            if (bytes <= 0) {
                cur += 4096;
                continue;
            }
            if ((size_t)bytes < patternLen) {
                cur += bytes;
                continue;
            }
            
            for (size_t i = 0; i <= (size_t)bytes - patternLen; i++) {
                bool matched = true;
                for (size_t p = 0; p < patternLen; p++) {
                    if (mask[p] == 'x' && buffer[i + p] != findPattern[p]) {
                        matched = false;
                        break;
                    }
                }
                if (matched) {
                    uintptr_t addr = cur + i;
                    if (VipHacks_SafeWrite((void*)addr, replacePattern, patternLen)) {
                        cachedAddr = addr;
                        replaceCount++;
                    }
                }
            }
            cur += bytes - patternLen + 1;
        }
    }
    close(mem_fd);
    return replaceCount;
}

inline bool IsVtableForClass(int mem_fd, uintptr_t ptr, uintptr_t classPtrVal) {
    if (ptr == classPtrVal) return true;
    if (ptr < 0x10000000ULL || ptr > 0xFFFFFFFFFFFFULL) return false;
    uintptr_t deref = 0;
    if (pread(mem_fd, &deref, sizeof(deref), (off_t)ptr) == sizeof(deref)) {
        if (deref == classPtrVal) return true;
    }
    return false;
}

// IL2CPP instances modification via dynamic heap class pointer scanning using /proc/self/mem
inline void VipHacks_SetClassFieldFloat(const char* ns, const char* className, uint32_t offset, float value) {
    if (!g_il2cpp.ready && !g_il2cpp.Init()) return;
    void* klass = g_il2cpp.FindClassEverywhere(ns, className);
    if (!klass) {
        E_LOGE("VipHacks: Class %s not found", className);
        return;
    }
    
    int mem_fd = open("/proc/self/mem", O_RDONLY);
    if (mem_fd == -1) return;
    
    std::vector<VipMemoryRegion> regions;
    FILE *f = fopen("/proc/self/maps", "r");
    if (f) {
        char line[512];
        while (fgets(line, sizeof(line), f)) {
            if (strstr(line, "rw-p")) {
                char path[256] = {0};
                uintptr_t start = 0, end = 0;
                int fields = sscanf(line, "%lx-%lx %*s %*s %*s %*s %255s", &start, &end, path);
                bool isCodeOrLib = false;
                if (fields == 3) {
                    if (strstr(path, ".so") || strstr(path, ".apk") || strstr(path, ".jar") || 
                        strstr(path, ".odex") || strstr(path, ".oat") || strstr(path, "/system/") || 
                        strstr(path, "/apex/")) {
                        isCodeOrLib = true;
                    }
                }
                if (!isCodeOrLib) {
                    regions.push_back({start, end});
                }
            }
        }
        fclose(f);
    }
    
    uintptr_t classPtrVal = (uintptr_t)klass;
    int writeCount = 0;
    
    std::vector<uintptr_t> buffer(4096); // 32KB pointer buffer
    for (const auto& reg : regions) {
        uintptr_t cur = reg.start;
        while (cur < reg.end) {
            size_t toRead = reg.end - cur;
            if (toRead > buffer.size() * sizeof(uintptr_t)) toRead = buffer.size() * sizeof(uintptr_t);
            
            ssize_t bytes = pread(mem_fd, buffer.data(), toRead, (off_t)cur);
            if (bytes <= 0) {
                cur += 4096;
                continue;
            }
            
            size_t words = bytes / sizeof(uintptr_t);
            for (size_t i = 0; i < words; i++) {
                if (IsVtableForClass(mem_fd, buffer[i], classPtrVal)) {
                    uintptr_t objAddr = cur + i * sizeof(uintptr_t);
                    if (objAddr + offset + sizeof(float) <= reg.end) {
                        if (VipHacks_SafeWrite((void*)(objAddr + offset), &value, sizeof(value))) {
                            writeCount++;
                        }
                    }
                }
            }
            cur += bytes;
        }
    }
    close(mem_fd);
    E_LOGI("VipHacks: Written float %f to %d instances of class %s at offset 0x%X", value, writeCount, className, offset);
}

inline int VipHacks_ScanAndEditOffsetFloat(const uint8_t* findPattern, const char* mask, size_t patternLen, size_t offset, float value, std::vector<uintptr_t>& cachedAddrs) {
    if (!cachedAddrs.empty()) {
        int writeCount = 0;
        for (uintptr_t addr : cachedAddrs) {
            if (VipHacks_SafeWrite((void*)addr, &value, sizeof(value))) {
                writeCount++;
            }
        }
        if (writeCount == (int)cachedAddrs.size()) {
            return writeCount;
        }
        cachedAddrs.clear();
    }
    
    int mem_fd = open("/proc/self/mem", O_RDONLY);
    if (mem_fd == -1) return 0;
    
    std::vector<VipMemoryRegion> regions;
    FILE *f = fopen("/proc/self/maps", "r");
    if (f) {
        char line[512];
        while (fgets(line, sizeof(line), f)) {
            if (strstr(line, "r-") || strstr(line, "rw-")) {
                char path[256] = {0};
                uintptr_t start = 0, end = 0;
                int fields = sscanf(line, "%lx-%lx %*s %*s %*s %*s %255s", &start, &end, path);
                bool isCodeOrLib = false;
                if (fields == 3) {
                    if (strstr(path, ".so") || strstr(path, ".apk") || strstr(path, ".jar") || 
                        strstr(path, ".odex") || strstr(path, ".oat") || strstr(path, "/system/") || 
                        strstr(path, "/apex/")) {
                        isCodeOrLib = true;
                    }
                }
                if (!isCodeOrLib) {
                    regions.push_back({start, end});
                }
            }
        }
        fclose(f);
    }
    
    int replaceCount = 0;
    std::vector<uint8_t> buffer(4096 * 16);
    for (const auto& reg : regions) {
        uintptr_t cur = reg.start;
        while (cur < reg.end) {
            size_t toRead = reg.end - cur;
            if (toRead > buffer.size()) toRead = buffer.size();
            
            ssize_t bytes = pread(mem_fd, buffer.data(), toRead, (off_t)cur);
            if (bytes <= 0) {
                cur += 4096;
                continue;
            }
            if ((size_t)bytes < patternLen) {
                cur += bytes;
                continue;
            }
            
            for (size_t i = 0; i <= (size_t)bytes - patternLen; i++) {
                bool matched = true;
                for (size_t p = 0; p < patternLen; p++) {
                    if (mask[p] == 'x' && buffer[i + p] != findPattern[p]) {
                        matched = false;
                        break;
                    }
                }
                if (matched) {
                    uintptr_t addr = cur + i + offset;
                    if (VipHacks_SafeWrite((void*)addr, &value, sizeof(value))) {
                        cachedAddrs.push_back(addr);
                        replaceCount++;
                    }
                }
            }
            cur += bytes - patternLen + 1;
        }
    }
    close(mem_fd);
    return replaceCount;
}

inline int VipHacks_ScanAndEditOffsetDword(const uint8_t* findPattern, const char* mask, size_t patternLen, size_t offset, int32_t value, std::vector<uintptr_t>& cachedAddrs) {
    if (!cachedAddrs.empty()) {
        int writeCount = 0;
        for (uintptr_t addr : cachedAddrs) {
            if (VipHacks_SafeWrite((void*)addr, &value, sizeof(value))) {
                writeCount++;
            }
        }
        if (writeCount == (int)cachedAddrs.size()) {
            return writeCount;
        }
        cachedAddrs.clear();
    }
    
    int mem_fd = open("/proc/self/mem", O_RDONLY);
    if (mem_fd == -1) return 0;
    
    std::vector<VipMemoryRegion> regions;
    FILE *f = fopen("/proc/self/maps", "r");
    if (f) {
        char line[512];
        while (fgets(line, sizeof(line), f)) {
            if (strstr(line, "r-") || strstr(line, "rw-")) {
                char path[256] = {0};
                uintptr_t start = 0, end = 0;
                int fields = sscanf(line, "%lx-%lx %*s %*s %*s %*s %255s", &start, &end, path);
                bool isCodeOrLib = false;
                if (fields == 3) {
                    if (strstr(path, ".so") || strstr(path, ".apk") || strstr(path, ".jar") || 
                        strstr(path, ".odex") || strstr(path, ".oat") || strstr(path, "/system/") || 
                        strstr(path, "/apex/")) {
                        isCodeOrLib = true;
                    }
                }
                if (!isCodeOrLib) {
                    regions.push_back({start, end});
                }
            }
        }
        fclose(f);
    }
    
    int replaceCount = 0;
    std::vector<uint8_t> buffer(4096 * 16);
    for (const auto& reg : regions) {
        uintptr_t cur = reg.start;
        while (cur < reg.end) {
            size_t toRead = reg.end - cur;
            if (toRead > buffer.size()) toRead = buffer.size();
            
            ssize_t bytes = pread(mem_fd, buffer.data(), toRead, (off_t)cur);
            if (bytes <= 0) {
                cur += 4096;
                continue;
            }
            if ((size_t)bytes < patternLen) {
                cur += bytes;
                continue;
            }
            
            for (size_t i = 0; i <= (size_t)bytes - patternLen; i++) {
                bool matched = true;
                for (size_t p = 0; p < patternLen; p++) {
                    if (mask[p] == 'x' && buffer[i + p] != findPattern[p]) {
                        matched = false;
                        break;
                    }
                }
                if (matched) {
                    uintptr_t addr = cur + i + offset;
                    if (VipHacks_SafeWrite((void*)addr, &value, sizeof(value))) {
                        cachedAddrs.push_back(addr);
                        replaceCount++;
                    }
                }
            }
            cur += bytes - patternLen + 1;
        }
    }
    close(mem_fd);
    return replaceCount;
}

// Global state variables declarations
extern bool g_VipSpeedRun;
extern bool g_VipBigJump;
extern bool g_VipWallHack;
extern bool g_VipNoRecoil;
extern bool g_VipSuperRecoil;
extern bool g_VipStaminaInfinie;
extern bool g_VipMoveToVehicle;
extern bool g_VipSpeedOfMovement;

extern float g_VipVehicleSpeedVal;
extern float g_VipVehicleAngleVal;
extern bool g_VipVehicleMaxBrake;
extern float g_VipVehicleForwardVal;
extern bool g_VipVehicleNoDamage;
extern bool g_VipVehicleInfFuel;
extern float g_VipVehicleSlippingVal;
extern float g_VipVehicleWheelsVal;
extern float g_VipGlobalSpeedVal;
extern float g_VipCameraDistanceVal;
extern float g_VipGraphicsQualityVal;
extern float g_VipWheelAmountVal;
extern bool g_DragCheckpointToPlayer;
extern float g_DragCheckpointDelay;
extern float g_HeistFarmDelay;

// Toggles implementation
inline void VipHacks_ToggleSpeedRun(bool on) {
    uint8_t find[] = {0x80, 0x4F, 0xC3, 0xC7, 0xF0, 0x23, 0x74, 0xC9, 0x00, 0x00, 0xF0, 0x3F};
    uint8_t repl[] = {0x80, 0x4F, 0xC3, 0xC7, 0xF0, 0x23, 0x74, 0xC9, 0x00, 0x00, 0x00, 0x40};
    if (on) {
        VipHacks_ScanAndReplace(find, "xxxxxxxxxxxx", repl, 12, s_speedRunCached);
    } else {
        if (s_speedRunCached != 0) {
            VipHacks_SafeWrite((void*)s_speedRunCached, find, 12);
        } else {
            VipHacks_ScanAndReplace(repl, "xxxxxxxxxxxx", find, 12, s_speedRunCached);
        }
    }
}

inline void VipHacks_ToggleBigJump(bool on) {
    uint8_t find1[] = {0x00, 0x00, 0xC0, 0x40, 0x00, 0x00, 0x00, 0x00, 0xCD, 0xCC, 0x4C, 0x3D};
    uint8_t repl1[] = {0x00, 0x00, 0x20, 0x41, 0x00, 0x00, 0x00, 0x00, 0xCD, 0xCC, 0x4C, 0x3D};
    
    uint8_t find2[] = {0x00, 0x00, 0xE0, 0x40, 0x00, 0x00, 0x00, 0x41, 0x00, 0x00, 0x00, 0x40};
    uint8_t repl2[] = {0x00, 0x00, 0x70, 0x41, 0x00, 0x00, 0x00, 0x41, 0x00, 0x00, 0x00, 0x40};
    
    if (on) {
        VipHacks_ScanAndReplace(find1, "xxxxxxxxxxxx", repl1, 12, s_bigJumpCached1);
        VipHacks_ScanAndReplace(find2, "xxxxxxxxxxxx", repl2, 12, s_bigJumpCached2);
    } else {
        if (s_bigJumpCached1 != 0) {
            VipHacks_SafeWrite((void*)s_bigJumpCached1, find1, 12);
        } else {
            VipHacks_ScanAndReplace(repl1, "xxxxxxxxxxxx", find1, 12, s_bigJumpCached1);
        }
        if (s_bigJumpCached2 != 0) {
            VipHacks_SafeWrite((void*)s_bigJumpCached2, find2, 12);
        } else {
            VipHacks_ScanAndReplace(repl2, "xxxxxxxxxxxx", find2, 12, s_bigJumpCached2);
        }
    }
}

inline void VipHacks_ToggleWallHack(bool on) {
    uint8_t find[] = {0x00, 0x00, 0x48, 0x43, 0x02, 0x00, 0x00, 0x00, 0x0A, 0x00, 0x00, 0x00};
    uint8_t repl[] = {0x00, 0x00, 0xC8, 0xC2, 0x02, 0x00, 0x00, 0x00, 0x0A, 0x00, 0x00, 0x00};
    if (on) {
        VipHacks_ScanAndReplace(find, "xxxxxxxxxxxx", repl, 12, s_wallHackCached);
    } else {
        if (s_wallHackCached != 0) {
            VipHacks_SafeWrite((void*)s_wallHackCached, find, 12);
        } else {
            VipHacks_ScanAndReplace(repl, "xxxxxxxxxxxx", find, 12, s_wallHackCached);
        }
    }
}

inline void VipHacks_ToggleNoRecoil(bool on) {
    uint8_t find[] = {0xCD, 0xCC, 0xCC, 0x3D, 0x66, 0x66, 0x66, 0x3F, 0x00, 0x00, 0x00, 0x41, 0x00, 0x00, 0x00, 0x00};
    uint8_t repl[] = {0x00, 0x00, 0x00, 0x00, 0x66, 0x66, 0x66, 0x3F, 0x00, 0x00, 0x00, 0x41, 0x00, 0x00, 0x00, 0x00};
    if (on) {
        VipHacks_ScanAndReplace(find, "xxxxxxxxxxxxxxxx", repl, 16, s_noRecoilCached);
    } else {
        if (s_noRecoilCached != 0) {
            VipHacks_SafeWrite((void*)s_noRecoilCached, find, 16);
        } else {
            VipHacks_ScanAndReplace(repl, "xxxxxxxxxxxxxxxx", find, 16, s_noRecoilCached);
        }
    }
}

inline void VipHacks_ToggleSuperRecoil(bool on) {
    uint8_t find[] = {0xCD, 0xCC, 0xCC, 0x3D, 0x66, 0x66, 0x66, 0x3F, 0x00, 0x00, 0x00, 0x41, 0x00, 0x00, 0x00, 0x00};
    uint8_t repl[] = {0xCD, 0xCC, 0xCC, 0x3D, 0x00, 0x00, 0x40, 0x40, 0x00, 0x00, 0x00, 0x41, 0x00, 0x00, 0x00, 0x00};
    if (on) {
        VipHacks_ScanAndReplace(find, "xxxxxxxxxxxxxxxx", repl, 16, s_superRecoilCached);
    } else {
        if (s_superRecoilCached != 0) {
            VipHacks_SafeWrite((void*)s_superRecoilCached, find, 16);
        } else {
            VipHacks_ScanAndReplace(repl, "xxxxxxxxxxxxxxxx", find, 16, s_superRecoilCached);
        }
    }
}

inline void VipHacks_ToggleStaminaInfinie(bool on) {
    uint8_t find[] = {0x00, 0x00, 0x30, 0xC1, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    uint8_t repl[] = {0x00, 0x00, 0xC6, 0x42, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    if (on) {
        VipHacks_ScanAndReplace(find, "xxxxxxxxxxxxxxxx", repl, 16, s_staminaCached);
    } else {
        if (s_staminaCached != 0) {
            VipHacks_SafeWrite((void*)s_staminaCached, find, 16);
        } else {
            VipHacks_ScanAndReplace(repl, "xxxxxxxxxxxxxxxx", find, 16, s_staminaCached);
        }
    }
}

inline void VipHacks_ToggleMoveToVehicle(bool on) {
    uint8_t find[] = {0x00, 0x00, 0x00, 0x3F, 0x9A, 0x99, 0x19, 0x3E, 0x9A, 0x99, 0x59, 0x3F, 0x00, 0x00, 0xC0, 0x40};
    uint8_t repl[] = {0x00, 0x00, 0xC6, 0x42, 0x00, 0x00, 0xC6, 0x42, 0x9A, 0x99, 0x59, 0x3F, 0x00, 0x00, 0xC0, 0x40};
    if (on) {
        VipHacks_ScanAndReplace(find, "xxxxxxxxxxxxxxxx", repl, 16, s_moveToVehicleCached);
    } else {
        if (s_moveToVehicleCached != 0) {
            VipHacks_SafeWrite((void*)s_moveToVehicleCached, find, 16);
        } else {
            VipHacks_ScanAndReplace(repl, "xxxxxxxxxxxxxxxx", find, 16, s_moveToVehicleCached);
        }
    }
}

inline void VipHacks_ToggleSpeedOfMovement(bool on) {
    uint8_t find[] = {0x66, 0x66, 0xA6, 0x3F, 0x00, 0x00, 0x20, 0x40, 0x00, 0x00, 0x20, 0x41, 0x00, 0x00, 0x00, 0x40};
    uint8_t repl[] = {0x28, 0x6B, 0x6E, 0x4E, 0x00, 0x00, 0x20, 0x40, 0x00, 0x00, 0x20, 0x41, 0x00, 0x00, 0x00, 0x40};
    if (on) {
        VipHacks_ScanAndReplace(find, "xxxxxxxxxxxxxxxx", repl, 16, s_speedOfMovementCached);
    } else {
        if (s_speedOfMovementCached != 0) {
            VipHacks_SafeWrite((void*)s_speedOfMovementCached, find, 16);
        } else {
            VipHacks_ScanAndReplace(repl, "xxxxxxxxxxxxxxxx", find, 16, s_speedOfMovementCached);
        }
    }
}

inline void VipHacks_ToggleVehicleMaxBrake(bool on) {
    if (on) {
        VipHacks_SetClassFieldFloat("", "VehicleViewParams", 0x88, 9999999.0f);
    } else {
        VipHacks_SetClassFieldFloat("", "VehicleViewParams", 0x88, 10.0f);
    }
}

inline void VipHacks_ToggleVehicleNoDamage(bool on) {
    if (on) {
        VipHacks_SetClassFieldFloat("", "VehicleViewParams", 0x12C, 999999999999.0f);
    } else {
        VipHacks_SetClassFieldFloat("", "VehicleViewParams", 0x12C, 1000.0f);
    }
}

inline void VipHacks_ToggleVehicleInfFuel(bool on) {
    if (on) {
        VipHacks_SetClassFieldFloat("", "VehicleParametersItemSettingsValuesModel", 0x18, -99999999999999999999.0f);
    } else {
        VipHacks_SetClassFieldFloat("", "VehicleParametersItemSettingsValuesModel", 0x18, 60.0f);
    }
}
