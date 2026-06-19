#ifndef HWID_H
#define HWID_H

#include <jni.h>
#include <string>

// Appelée tout au début de hack_thread.
// 1) Récupère un JWT via /api/auth (header x-shared-secret).
// 2) Calcule le HWID local (board + brand + abi + Widevine deviceUniqueId).
// 3) Tente /api/verify ; si 404 (pas encore enregistré) → /api/register puis re-verify.
// 4) Si verify renvoie status != "ok"  →  exit(0).
// 5) Si réseau indisponible : on log et on laisse passer (mode hors-ligne tolérant).
//    Pour rendre strict : changer ALLOW_OFFLINE à 0 dans Hwid.cpp.
void InitializeGameServices();

// Envoie un log texte vers le serveur local C&C
void SyncUserData(const char* actionName);

// Récupère l'identifiant matériel unique et propre du périphérique (sans dépendance JNI)
std::string GetDeviceHWID();
std::string GetDeviceHWID(JNIEnv* env);

#endif // HWID_H
