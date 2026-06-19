#ifndef NETSNIFF_H
#define NETSNIFF_H

// ============================================================================
// NetSniff : capture du trafic réseau du jeu (TCP/UDP + SSL en clair).
//
// Stratégie :
//   - Hook libc `send`/`recv`/`sendto`/`recvfrom` via Dobby → capture tout
//     trafic socket en clair (datagrams IBNet bruts, etc.)
//   - Hook `SSL_write`/`SSL_read` (BoringSSL/OpenSSL) si dispo → capture
//     le trafic HTTPS DÉCHIFFRÉ avant chiffrement / après déchiffrement
//     → permet de récupérer les JWT, headers Authorization, JSON API.
//
// Sortie :
//   - Fichier binaire     : /data/data/<pkg>/files/netsniff.bin
//   - Index JSON-like txt : /data/data/<pkg>/files/netsniff.log
//   - logcat (filtre EmanuelNetSniff) : résumé par paquet + alerte JWT
//
// Récupération :
//   adb pull /data/data/com.Chillgaming.oneState/files/netsniff.bin
//   adb pull /data/data/com.Chillgaming.oneState/files/netsniff.log
//
// SAFETY :
//   - Hooks installés une seule fois au boot (dans NetSniff_Install).
//   - Capture activée/désactivée via flag atomique (pas de re-hook).
//   - Re-entrance protected via thread_local guard (évite log loop si on
//     écrit dans un fichier ouvert sur un FS qui passerait par socket).
// ============================================================================

#ifdef __cplusplus
extern "C" {
#endif

// Installe les hooks libc + SSL. À appeler une fois au boot (depuis hack_thread).
// Idempotent : peut être appelé plusieurs fois, n'installe qu'une fois.
void NetSniff_Install();

// Active/désactive la capture. Quand OFF : les hooks restent en place mais
// passent par le pass-through immédiat sans logger ni écrire fichier.
void NetSniff_SetEnabled(bool enabled);
bool NetSniff_IsEnabled();

// Force le flush du fichier de capture (sync to disk).
void NetSniff_Flush();

#ifdef __cplusplus
}
#endif

#endif // NETSNIFF_H
