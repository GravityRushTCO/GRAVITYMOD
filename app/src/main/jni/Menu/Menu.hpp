#ifndef MENU_HPP
#define MENU_HPP

#include "Jni.hpp"

// Déclaration ici car setText est implémenté dans Jni.cpp et utilisé dans
// Menu.cpp
void setText(JNIEnv *env, jobject obj, const char *text);

#ifdef __cplusplus
extern "C" {
#endif

void Init(JNIEnv *env, jobject thiz, jobject ctx, jobject title,
          jobject subtitle);

jstring Icon(JNIEnv *env, jobject thiz);

jstring IconWebViewData(JNIEnv *env, jobject thiz);

jobjectArray SettingsList(JNIEnv *env, jobject activityObject);

jobjectArray GetFeatureList(JNIEnv *env, jobject context);

void Changes(JNIEnv *env, jclass clazz, jobject obj, jint featNum,
             jstring featName, jint value, jlong Lvalue, jboolean isOn,
             jstring text);

#ifdef __cplusplus
}
#endif

#endif // MENU_HPP