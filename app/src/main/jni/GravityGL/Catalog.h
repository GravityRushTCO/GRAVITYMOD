#ifndef CATALOG_H
#define CATALOG_H

struct CatalogItem {
    const char* displayName;
    int idVal;
};

#include <GLES2/gl2.h>
#include <sys/stat.h>
#include <dirent.h>
#include <cstdio>
#include <cstring>
#include <cstdint>
#include <android/log.h>
#include <unordered_map>
#include <string>
#include <mutex>
#include <algorithm>
#include <vector>


#define TCACHE_TAG "TexCache"
#define TCACHE_LOG(...) __android_log_print(ANDROID_LOG_INFO, TCACHE_TAG, __VA_ARGS__)

// -------------------------------------------------------
// Global texture map (spriteName -> OpenGL texture ID)
// -------------------------------------------------------
inline std::unordered_map<std::string, unsigned int> g_GameTextures;
inline std::mutex g_GameTexturesMutex;

// Cache directory set once by hack_thread at startup
inline char g_TexCacheDir[256] = {};

inline void Catalog_SetCacheDir(const char* dir) {
    snprintf(g_TexCacheDir, sizeof(g_TexCacheDir), "%s", dir);
    mkdir(g_TexCacheDir, 0777);
}

// -------------------------------------------------------
// Save a captured GL texture to disk as raw RGBA file
// Header: [uint32 width][uint32 height][RGBA bytes...]
// -------------------------------------------------------
inline void SaveTextureToDisk(const std::string& name, unsigned int glId) {
    if (!g_TexCacheDir[0]) return;

    // Bind the texture and read its dimensions
    int prevTex = 0;
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &prevTex);
    glBindTexture(GL_TEXTURE_2D, glId);

    // On GLES2 glGetTexLevelParameteriv is not available; use FBO readback
    // Create a temp FBO and blit the texture into it, then glReadPixels
    GLuint fbo = 0;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, glId, 0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glDeleteFramebuffers(1, &fbo);
        glBindTexture(GL_TEXTURE_2D, prevTex);
        return;
    }

    // Read back viewport as proxy for texture size (safe for 256x256 icons)
    GLint vp[4];
    glGetIntegerv(GL_VIEWPORT, vp);
    int w = vp[2] > 0 ? vp[2] : 256;
    int h = vp[3] > 0 ? vp[3] : 256;
    if (w > 512) w = 512;
    if (h > 512) h = 512;

    std::vector<uint8_t> pixels(w * h * 4);
    glReadPixels(0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDeleteFramebuffers(1, &fbo);
    glBindTexture(GL_TEXTURE_2D, prevTex);

    // Build a safe filename (replace spaces / special chars)
    std::string safeName = name;
    for (auto& c : safeName) {
        if (c == ' ' || c == '/' || c == '\\' || c == ':' || c == '*' || c == '?') c = '_';
    }

    char path[512];
    snprintf(path, sizeof(path), "%s/%s.rgba", g_TexCacheDir, safeName.c_str());

    FILE* f = fopen(path, "wb");
    if (!f) return;
    uint32_t uw = (uint32_t)w, uh = (uint32_t)h;
    fwrite(&uw, 4, 1, f);
    fwrite(&uh, 4, 1, f);
    fwrite(pixels.data(), 1, pixels.size(), f);
    fclose(f);
    TCACHE_LOG("Saved texture '%s' (%dx%d) -> %s", name.c_str(), w, h, path);
}

// -------------------------------------------------------
// Register + persist
// -------------------------------------------------------
inline void RegisterGameTexture(const std::string& spriteName, unsigned int glId) {
    {
        std::lock_guard<std::mutex> lock(g_GameTexturesMutex);
        // Don't re-capture if already cached
        if (g_GameTextures.count(spriteName) && g_GameTextures[spriteName] == glId) return;
        g_GameTextures[spriteName] = glId;
    }
    // Save to disk asynchronously (called from GL thread so direct save is fine)
    SaveTextureToDisk(spriteName, glId);
}

// -------------------------------------------------------
// Load all persisted textures from disk into OpenGL
// Must be called from GL thread (e.g. first drawMenu frame)
// -------------------------------------------------------
inline void LoadCachedTextures() {
    if (!g_TexCacheDir[0]) return;

    DIR* dir = opendir(g_TexCacheDir);
    if (!dir) return;

    struct dirent* ent;
    while ((ent = readdir(dir)) != nullptr) {
        const char* fname = ent->d_name;
        size_t len = strlen(fname);
        if (len < 6 || strcmp(fname + len - 5, ".rgba") != 0) continue;

        char path[512];
        snprintf(path, sizeof(path), "%s/%s", g_TexCacheDir, fname);
        FILE* f = fopen(path, "rb");
        if (!f) continue;

        uint32_t w = 0, h = 0;
        if (fread(&w, 4, 1, f) != 1 || fread(&h, 4, 1, f) != 1 || w == 0 || h == 0 || w > 4096 || h > 4096) {
            fclose(f); continue;
        }

        std::vector<uint8_t> pixels(w * h * 4);
        size_t read = fread(pixels.data(), 1, pixels.size(), f);
        fclose(f);
        if (read != pixels.size()) continue;

        // Create GL texture
        GLuint tex = 0;
        glGenTextures(1, &tex);
        if (!tex) continue;
        glBindTexture(GL_TEXTURE_2D, tex);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
        glBindTexture(GL_TEXTURE_2D, 0);

        // Derive sprite name from filename (strip .rgba), keep underscores as-is
        std::string spriteName(fname, len - 5);
        // Also store a space-version for backward compat
        std::string spriteNameSpaces = spriteName;
        for (auto& c : spriteNameSpaces) if (c == '_') c = ' ';

        std::lock_guard<std::mutex> lock(g_GameTexturesMutex);
        if (!g_GameTextures.count(spriteName)) {
            g_GameTextures[spriteName] = tex;
            TCACHE_LOG("Loaded cached texture '%s' (%dx%d) glId=%u", spriteName.c_str(), w, h, tex);
        }
        if (!g_GameTextures.count(spriteNameSpaces)) {
            g_GameTextures[spriteNameSpaces] = tex;
        }
    }
    closedir(dir);
}

inline unsigned int FindGameTextureFuzzy(const std::string& itemName) {
    std::lock_guard<std::mutex> lock(g_GameTexturesMutex);
    if (itemName.empty()) return 0;
    
    // Build a cleaned search key from itemName
    auto makeKey = [](std::string s) -> std::string {
        // lowercase
        std::transform(s.begin(), s.end(), s.begin(), ::tolower);
        // Remove parenthesized words like (homme), (fille), (sdf)
        for (const auto& word : {" (homme)", " (fille)", " (sdf)", " (travail m)",
                                  "(homme)", "(fille)", "(sdf)"}) {
            size_t pos = s.find(word);
            if (pos != std::string::npos) s.erase(pos, strlen(word));
        }
        // Remove LOD suffixes
        for (const auto& sfx : {" lod01", " lod2", " norm", " mask", " complete",
                                  "_lod01", "_lod2", "_norm", "_mask", "_complete",
                                  " clothes", "_clothes", " organic", "_organic"}) {
            size_t pos = s.find(sfx);
            if (pos != std::string::npos) s.erase(pos);
        }
        // Unify separators: spaces/underscores both become '_'
        for (auto& c : s) if (c == ' ') c = '_';
        // Remove trailing underscores
        while (!s.empty() && s.back() == '_') s.pop_back();
        return s;
    };
    
    std::string searchKey = makeKey(itemName);
    if (searchKey.empty()) return 0;
    
    // First pass: exact key match
    for (const auto& pair : g_GameTextures) {
        std::string key = makeKey(pair.first);
        if (key == searchKey) return pair.second;
    }
    
    // Second pass: substring match (key contains search or vice versa)
    for (const auto& pair : g_GameTextures) {
        std::string key = makeKey(pair.first);
        if (!key.empty() && (key.find(searchKey) != std::string::npos || 
                              searchKey.find(key) != std::string::npos)) {
            // Prefer textures with lod01 (highest quality)
            if (pair.first.find("lod01") != std::string::npos ||
                pair.first.find("complete") != std::string::npos) {
                return pair.second;
            }
        }
    }
    
    // Third pass: less strict - any partial match
    for (const auto& pair : g_GameTextures) {
        std::string key = makeKey(pair.first);
        if (!key.empty() && (key.find(searchKey) != std::string::npos || 
                              searchKey.find(key) != std::string::npos)) {
            return pair.second;
        }
    }
    
    return 0;
}



// VRAIS SKINS DU JEU (Enum PedClothes de dump_v18.cs)
static const CatalogItem skinCatalog[] = {
    {"Par defaut", -1},
    {"John (Homme)", 0},
    {"Eve (Fille)", 1},
    {"Michael (Homme)", 2},
    {"College (Homme)", 3},
    {"Nancy (Fille)", 4},
    {"Carlos (Homme)", 5},
    {"Emily (Fille)", 6},
    {"Alex (Homme)", 7},
    {"Jack (Homme)", 8},
    {"Skater (Homme)", 9},
    {"Steve (Homme)", 10},
    {"Baddie (Fille)", 11},
    {"Carl (Homme)", 12},
    {"Cowboy (Homme)", 13},
    {"Homeless (SDF)", 14},
    {"Housewife (Fille)", 15},
    {"Jayden (Homme)", 16},
    {"Liam (Homme)", 17},
    {"Lumberjack (Homme)", 18},
    {"Mason (Homme)", 19},
    {"McWinner (Homme)", 20},
    {"Mike (Homme)", 21},
    {"Nick (Homme)", 22},
    {"White Collar (Homme)", 23},
    {"Young Pike (Homme)", 24},
    {"Larry (Homme)", 25},
    {"John Smith (Homme)", 26},
    {"Redneck (Homme)", 27},
    {"Casual (Homme)", 28},
    {"Clerk (Fille)", 29},
    {"Yacht (Homme)", 30},
    {"Lebowski (Homme)", 31},
    {"Pablo (Homme)", 32},
    {"Yakuza (Homme)", 33},
    {"Chessplayer (Fille)", 34},
    {"Latino (Fille)", 35},
    {"Rob (Homme)", 36},
    {"Tourist (Homme)", 37},
    {"Ray (Homme)", 38},
    {"Juan (Homme)", 39},
    {"Kosei (Fille)", 40},
    {"Cheerleader (Fille)", 41},
    {"Grunge (Fille)", 42},
    {"Stefanny (Fille)", 43},
    {"Hangover (Homme)", 44},
    {"Pimp (Homme)", 45},
    {"Metalhead (Homme)", 46},
    {"Tony Ricci (Homme)", 47},
    {"Baby (Fille)", 48},
    {"Rose (Fille)", 49},
    {"Angel (Fille)", 50},
    {"Old Legend (Homme)", 51},
    {"Future (Homme)", 52},
    {"Gopnik (Homme)", 53},
    {"Hick (Homme)", 54},
    {"Barber (Homme)", 55},
    {"Presenter (Fille)", 56},
    {"Hop Artist (Homme)", 57},
    {"IT Investor (Homme)", 58},
    {"Fashion (Fille)", 59},
    {"Carrier (Homme)", 60},
    {"Beach (Homme)", 61},
    {"Oxford (Homme)", 62},
    {"Sensei (Homme)", 63},
    {"Afterparty (Homme)", 64},
    {"China Tourist (Homme)", 65},
    {"London (Homme)", 66},
    {"Drifter (Homme)", 67},
    {"Politic (Fille)", 68},
    {"Luna (Homme)", 69},
    {"Singer (Fille)", 70},
    {"Italy Crime (Homme)", 71},
    {"Dragon (Homme)", 72},
    {"Famous (Fille)", 73},
    {"Rap Style (Homme)", 74},
    {"Street Legend (Homme)", 75},
    {"Tuff Guy (Homme)", 76},
    {"Black Curly (Fille)", 77},
    {"Drift (Fille)", 78},
    {"Bodyguard (Homme)", 79},
    {"Black Sport (Homme)", 80},
    {"Casual Street Boy", 81},
    {"Bone (Homme)", 82},
    {"Black Cowboy (Homme)", 83},
    {"Clown (Homme)", 84},
    {"Snake (Homme)", 85},
    {"Biker (Homme)", 86},
    {"Raven (Homme)", 87},
    {"Sport (Homme)", 88},
    {"Scary (Fille)", 89},
    {"Nerd (Homme)", 90},
    {"Santa (Homme)", 91},
    {"Winter (Homme)", 92},
    {"Frozen (Homme)", 93},
    {"Christmas (Fille)", 94},
    {"Red (Homme)", 95},
    {"Triad (Fille)", 96},
    {"Triad (Homme)", 97},
    {"Gangster (Homme)", 98},
    {"Loco (Fille)", 99},
    {"Loco (Homme)", 100},
    {"Gangster (Fille)", 101},
    {"Abibas (Homme)", 102},
    {"Slav (Homme)", 103},
    {"Slav (Fille)", 104},
    {"Abibas (Fille)", 105},
    {"Bunny (Homme)", 106},
    {"Demon (Homme)", 107},
    {"Kawaii (Fille)", 108},
    {"Charming (Fille)", 109},
    {"Bouncer (Homme)", 110},
    {"Kingpin (Homme)", 111},
    {"Glamour (Fille)", 112},
    {"White Fox (Homme)", 113},
    {"Ronin (Homme)", 114},
    {"Sakura (Fille)", 115},
    {"Rebel (Homme)", 116},
    {"Blaze (Homme)", 117},
    {"Big Hustle (Homme)", 118},
    {"Casey (Homme)", 119},
    {"Stargirl (Fille)", 120},
    {"Lux Paulinho (Homme)", 121},
    {"Paulinho (Homme)", 122},
    {"Ghoul (Homme)", 123},
    {"Rapid (Homme)", 124},
    {"Marcus (Homme)", 125},
    {"Onyx (Homme)", 126},
    {"Old Tourist (Homme)", 127},
    {"Seabreeze (Fille)", 128},
    {"Red Butterfly (Fille)", 129},
    {"Texan (Homme)", 130},
    {"Elf (Fille)", 131},
    {"Edge (Homme)", 132},
    {"Bad S (Homme)", 133},
    {"Urban Tiger (Homme)", 134},
    {"Desert Hawk (Homme)", 135},
    {"Red Cipher (Homme)", 136},
    {"Easter Bunny (Homme)", 137},
    {"Brick Soul (Homme)", 138},
    {"Kimono (Homme)", 139},
    {"Black Vendetta (Homme)", 140},
    {"Court Shade (Homme)", 141},
    {"Baseball (Homme)", 142},
    {"Outlaw (Homme)", 143},
    {"Contender (Homme)", 144},
    {"Shadow Suit (Homme)", 145},
    {"Velocity Gear (Homme)", 146},
    {"Void Dancer (Fille)", 147},
    {"Camo Drip (Homme)", 148},
    {"Robe de Mariee (Fille)", 149},
    {"Costume Marie (Homme)", 150},
    {"Prime Attire (Homme)", 151},
    {"USA Look (Homme)", 152},
    {"USA Look (Fille)", 153},
    {"Silent Agent (Homme)", 154},
    {"Grim Specialist (Homme)", 155},
    
    // Factions & Uniformes
    {"Police F1", 1000},
    {"Police F2", 1001},
    {"Police F3", 1002},
    {"Police F4", 1003},
    {"Police M1", 1004},
    {"Police M2", 1005},
    {"Police M3", 1006},
    {"Police M4", 1007},
    {"Police M5", 1008},
    {"Soldat M1", 1009},
    {"Soldat M2", 1010},
    {"Soldat M3", 1011},
    {"Soldat M4", 1012},
    {"Soldat F1", 1013},
    {"Soldat F2", 1014},
    {"Soldat F3", 1015},
    {"Soldat F4", 1016},
    {"Medecin M1", 1017},
    {"Medecin M2", 1018},
    {"Medecin M3", 1019},
    {"Medecin M4", 1020},
    {"Medecin F1", 1030},
    {"Medecin F2", 1031},
    {"Medecin F3", 1032},
    {"Medecin F4", 1033},
    {"Police Cadette", 1045},
    {"Police Cadet M", 1047},
    {"Triade Blanche F", 3000},
    {"Triade Noire F", 3001},
    {"Triade Noire M", 3010},
    {"Triade Blanche M", 3011},
    {"Slav Rouge F", 3003},
    {"Loco Bleu F", 3004},
    {"Loco Rose F", 3005},
    {"Loco Bleu M", 3014},
    {"Loco Rose M", 3015},
    {"Abibas Noir M", 3018},
    {"Abibas Vert M", 3019},
    {"Abibas Noir F", 3008},
    {"Abibas Vert F", 3009},
    {"Miner (Travail M)", 2002},
    {"Hazmat (Travail M)", 2010}
};
static const int skinCatalogSize = sizeof(skinCatalog) / sizeof(skinCatalog[0]);

// VRAIES ARMES DU JEU (Enum InventoryItem de dump_v18.cs)
static const CatalogItem weaponCatalog[] = {
    {"Par defaut", -1},
    {"AK47 Standard", 5000},
    {"M4A1 Standard", 5001},
    {"KTR Standard", 5002},
    {"Pistolet Colt", 5003},
    {"Pistolet Glock", 5004},
    {"Stungun (Taser)", 5005},
    {"Pistolet Walter 9", 5006},
    {"Desert Eagle", 5007},
    {"Beretta 92", 5008},
    {"MP5 Standard", 5009},
    {"Taser de Clan", 5010},
    
    // Skins d'armes premium
    {"AK47 Scarlet", 5011},
    {"M4A1 Starlight", 5012},
    {"Deagle Gold", 5013},
    {"AK47 Frostbite", 5014},
    {"KTR Festive", 5015},
    {"M4A1 Scorpio", 5016},
    {"MP5 Yumi", 5017},
    {"MP5 Unit-01", 5018},
    {"KTR Firestorm", 5019},
    {"M4A1 Havoc", 5020},
    {"AK47 Zero Eclipse", 5021},
    {"KTR Jungle Camo", 5023},
    {"M4A1 Candy Cane", 5024},
    {"M4A1 Jungle Camo", 5025},
    {"AK47 Snow Reaper", 5026},
    {"AK47 Redeemer", 5027},
    {"KTR Dragon", 5028},
    {"AK47 Belladonna", 5029},
    {"MP5 Veins", 5030},
    {"AK47 Disorder", 5031},
    {"KTR Oversight", 5032},
    {"Baton de Combat", 5100}
};
static const int weaponCatalogSize = sizeof(weaponCatalog) / sizeof(weaponCatalog[0]);

// CATALOGUE VEHICULES DU JEU (IDs d'assets addressables)
static const CatalogItem vehicleCatalog[] = {
    {"Par defaut", -1},
    {"Moto Sport", 10006},
    {"Moto Scooter", 10005},
    {"Moto Chopper", 10007},
    {"Moto Cross", 10008},
    {"Lafera Supercar", 10011},
    {"Aventador Supercar", 10012},
    {"Chiron Hypercar", 10013},
    {"Viper", 10014},
    {"Mustang Coupe", 10015},
    {"Police Cruiser", 10016},
    {"Camaro Sport", 10017},
    {"Police Interceptor", 10018},
    {"Police SUV", 10019},
    {"Ambulance", 10020},
    {"Camion Pompier", 10021},
    {"Bus de la Ville", 10022},
    {"Taxi Cab", 10023},
    {"Tesla Model S", 10024},
    {"Cube SUV (G-Wagon)", 10025},
    {"Jeep Wrangler", 10026},
    {"Range Rover SUV", 10027},
    {"Audi R8", 10028},
    {"Porsche 911", 10029},
    {"BMW M5 Sport", 10030},
    {"Mercedes E63", 10031},
    {"Supra MK4 Classic", 10032},
    {"Civic Type R", 10033},
    {"Skyline R34 Tuner", 10034},
    {"Nissan GTR R35", 10069},
    {"Hummer H1 Offroad", 10080},
    {"Camion Remorque", 10035},
    {"Monster Truck", 10036},
    {"Karting de Course", 10037}
};
static const int vehicleCatalogSize = sizeof(vehicleCatalog) / sizeof(vehicleCatalog[0]);

#endif // CATALOG_H
