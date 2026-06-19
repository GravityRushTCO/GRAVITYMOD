#ifndef EMANUEL_IL2CPP_H
#define EMANUEL_IL2CPP_H

// ============================================================================
// Emanuel_IL2CPP : Reflection-based feature layer
//
// Pourquoi ce fichier ?
//   Le mod menu Emanuel n'utilise PAS de byte-patches statiques (vérifié :
//   1 seul DobbyHook, aucun CodePatch sur libil2cpp.so). Il utilise la
//   reflection IL2CPP runtime : il résout les classes par nom, scanne les
//   instances vivantes via Object.FindObjectsOfTypeAll, et modifie les
//   fields directement chaque frame.
//
// Avantage de cette approche :
//   - Résiste aux mises à jour du jeu (offsets binaires obsolètes ne le sont
//     pas : on cherche par NOM de classe et NOM de champ).
//   - Pas de patch destructif sur le code → moins de risques de casser une
//     chaîne damage/sync comme avec patch_InfAmmoInv.
//
// Usage :
//   - EmanuelIL2CPP_DumpClass("ns","Name") : log dans logcat tous les fields
//     d'une classe avec leur offset et type. À appeler une fois pour
//     identifier les vrais noms des fields privés du jeu (ex: "_health" vs
//     "health" vs "m_currentHealth").
//   - EmanuelIL2CPP_StartGodModeReflection() / Stop() : démarre un thread
//     qui scanne les PedHealthComponent / ElementHealthSync et force health
//     au max chaque frame.
//   - EmanuelIL2CPP_BootstrapDump() : appelé une fois au démarrage, dump les
//     classes critiques (Health, Weapon, Stamina, Money) pour qu'on voie
//     leurs vrais fields dans logcat.
// ============================================================================

#ifdef __cplusplus
extern "C" {
#endif

// Dump tous les fields d'une classe IL2CPP dans logcat.
// Format de chaque ligne : "FIELD <ns>::<class> name=<fname> offset=0x<XX> type=<tname>"
// Retourne le nombre de fields dumpés, ou -1 si la classe est introuvable.
int EmanuelIL2CPP_DumpClass(const char* ns, const char* name);

// Bootstrap : dump les classes critiques d'OneState RP avec leurs VRAIS
// namespaces (extraits du code C# décompilé Assembly-CSharp).
// OneState est en grande partie Morpeh ECS : la plupart des "components"
// sont des struct IComponent, donc INVISIBLES à FindObjectsOfTypeAll.
// On dump quand même pour voir les fields, et on scan par keyword pour
// trouver les vrais Services/Managers MonoBehaviour.
void EmanuelIL2CPP_BootstrapDump();

// Énumère toutes les classes IL2CPP de toutes les assemblies, et logge
// celles dont le nom contient `keyword` (case-sensitive). Permet de
// découvrir les noms exacts des Services, Managers, Providers du jeu.
// Ex: "Health" → liste tous les Health*, *HealthView*, etc.
// `maxResults` : limite (0 = pas de limite).
int EmanuelIL2CPP_DumpClassesByKeyword(const char* keyword, int maxResults);

// GodMode reflection-based : scanne ElementHealthSync chaque seconde et set
// le field "Value" (ou équivalent identifié par le dump) au max.
// Démarre un thread bg ; appel idempotent.
void EmanuelIL2CPP_StartGodModeReflection();
void EmanuelIL2CPP_StopGodModeReflection();

#ifdef __cplusplus
}
#endif

#endif // EMANUEL_IL2CPP_H
