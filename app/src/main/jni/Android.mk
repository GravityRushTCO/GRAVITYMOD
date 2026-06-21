LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

# Kittymemory
KITTYMEMORY_PATH = KittyMemory
include $(CLEAR_VARS)
LOCAL_MODULE    := Keystone
LOCAL_SRC_FILES := $(KITTYMEMORY_PATH)/Deps/Keystone/libs-android/$(TARGET_ARCH_ABI)/libkeystone.a
include $(PREBUILT_STATIC_LIBRARY)

# Dobby
include $(CLEAR_VARS)
LOCAL_MODULE := Dobby
LOCAL_SRC_FILES := Dobby/${TARGET_ARCH_ABI}/libdobby.a
include $(PREBUILT_STATIC_LIBRARY)

# Here is the name of your lib.
# When you change the lib name, change also on System.loadLibrary("") under OnCreate method on StaticActivity.java
# Both must have same name
include $(CLEAR_VARS)
LOCAL_MODULE    := Payload
LOCAL_SRC_FILES := Loader.cpp
LOCAL_LDLIBS    := -llog -landroid
include $(BUILD_SHARED_LIBRARY)

# ----------------- CORE MOD (THE PAYLOAD) -----------------
# This is the actual mod library that will be downloaded by the Loader.
# It should be built and uploaded to your Lovable server.
include $(CLEAR_VARS)
LOCAL_MODULE    := Core

# -std=c++17 is required to support AIDE app with NDK
LOCAL_CFLAGS := -w -Wno-error=format-security -fvisibility=hidden -fpermissive -fexceptions -g
LOCAL_CPPFLAGS := -w -Wno-error=format-security -fvisibility=hidden -Werror -std=c++17 -g
LOCAL_CPPFLAGS += -Wno-error=c++11-narrowing -fpermissive -Wall -fexceptions
LOCAL_LDFLAGS += -Wl,--gc-sections,-llog
LOCAL_LDLIBS := -llog -landroid -lEGL -lGLESv3 -lGLESv2
LOCAL_ARM_MODE := arm

LOCAL_C_INCLUDES += $(LOCAL_PATH)
LOCAL_C_INCLUDES += $(LOCAL_PATH)/Includes/
LOCAL_C_INCLUDES += $(LOCAL_PATH)/Dobby/
LOCAL_C_INCLUDES += $(LOCAL_PATH)/imgui/

# Here you add the cpp file to compile
# ✅ FICHIERS AJOUTÉS:
# - missing_implementations.cpp (Teleport + JNI functions)
# - device_id_faker.cpp (Anti-ban Device ID spoofing)

LOCAL_SRC_FILES := Main.cpp \
    Hwid.cpp \
    HttpUtils.cpp \
    imgui/imgui.cpp \
    imgui/imgui_demo.cpp \
    imgui/imgui_draw.cpp \
    imgui/imgui_tables.cpp \
    imgui/imgui_widgets.cpp \
    imgui/imgui_impl_android.cpp \
    imgui/imgui_impl_opengl3.cpp \
    GravityGL/GravityGL.cpp \
    GravityGL/VipMenu.cpp \
    GravityGL/ImGuiMenu.cpp \
    GravityGL/ModelRenderer.cpp \
    GravityGL/HexagonBackground.cpp \
    GravityGL/PlanetRenderer.cpp \
    GravityGL/GravityOverlay.cpp \
    Esp.cpp \
    Aimbot.cpp \
    Menu/Jni.cpp \
    Menu/Menu.cpp \
    Menu/Setup.cpp \
    Includes/Utils.cpp \
	KittyMemory/KittyAsm.cpp \
	KittyMemory/KittyIOFile.cpp \
    LicenseSystem.cpp \
    Protect.cpp \
    KittyMemory/KittyMemory.cpp \
    KittyMemory/KittyPtrValidator.cpp \
    KittyMemory/KittyScanner.cpp \
    KittyMemory/KittyUtils.cpp \
    KittyMemory/MemoryBackup.cpp \
    KittyMemory/MemoryPatch.cpp \
	xDL/xdl.c \
	xDL/xdl_iterate.c \
	xDL/xdl_linker.c \
	xDL/xdl_lzma.c \
	xDL/xdl_util.c \

LOCAL_STATIC_LIBRARIES := Keystone Dobby

include $(BUILD_SHARED_LIBRARY)