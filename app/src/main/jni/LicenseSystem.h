#pragma once
#include <string>
#include <atomic>

// ═══════════════════════════════════════════════════════════════════
//  GRAVITY MOD — License System
//
//  Format clé : GRVTY-XXXX-XXXX-XXXX-XXXX (base36, 20 chars utiles)
//  Validation : HMAC-SHA256 tronqué signé avec GRAVITY_LICENSE_SECRET
//  Vérification serveur : POST /api/license/verify (HWID binding)
//
//  ÉTATS :
//    PENDING  → Pas encore vérifié (affiche login screen)
//    VALID    → Clé acceptée côté serveur → mod déverrouillé
//    INVALID  → Clé rejetée → lockdown immédiat
//    BANNED   → Appareil/clé blacklisté → lockdown immédiat
// ═══════════════════════════════════════════════════════════════════

namespace License {

    enum class State {
        PENDING,   // Attente de saisie/vérification
        CHECKING,  // Requête serveur en cours
        VALID,     // Clé valide et vérifiée
        INVALID,   // Clé invalide (format ou serveur)
        BANNED,    // Blacklisté (lockdown)
        OFFLINE,   // Serveur inaccessible — utilisé le cache local
    };

    // Initialise le système — à appeler depuis hack_thread avant tout
    // Lit la clé sauvegardée (si elle existe) et lance la vérification
    void Init(const std::string& appDataDir, const std::string& hwid);

    // État courant de la licence
    State GetState();

    // Retourne true tant que l'écran de login doit bloquer le menu
    bool IsLocked();

    // Déverrouille manuellement / auto
    void SetUnlocked(bool unlocked);

    // Soumettre une clé depuis l'UI (saisie utilisateur)
    // Lance la vérification asynchrone côté serveur
    void SubmitKey(const std::string& key);

    // Pour l'UI : récupère le message d'erreur/statut affiché
    const char* GetStatusMessage();

    // Pour l'UI : animation de chargement (0.0 à 1.0)
    float GetCheckingProgress();

    // Récupérer la date d'expiration pour l'affichage (depuis Supabase)
    const char* GetExpirationDate();
    void SetExpirationDate(const std::string& dateStr);
    std::string GetRemainingTime();

    // Vérification de version
    bool IsNewVersionAvailable();
    const char* GetLatestVersion();

    extern const std::string SUPABASE_ANON_KEY;

} // namespace License
