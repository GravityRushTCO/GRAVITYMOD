// ============================================================================
// NetSniff.cpp — hook libc / SSL pour capture trafic réseau + JWT.
// Voir NetSniff.h pour la rationale.
// ============================================================================

#include "NetSniff.h"
#include "Includes/Logger.h"
#include "Dobby/dobby.h"

#include <atomic>
#include <mutex>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstdint>
#include <dlfcn.h>
#include <errno.h>
#include <elf.h>
#include <link.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>

#define NS_TAG "EmanuelNetSniff"
#define NS_LOG_MAX_HEX 64    // bytes hex à logguer par paquet en logcat
#define NS_FILE_MAX_BYTES (200 * 1024 * 1024) // 200 MB cap pour éviter de remplir le FS

// Forward decls : NS_STATUS écrit dans logcat + fichier persistant (le
// fichier survit aux overflows du ring buffer logcat).
static FILE* g_StatusFile;
#define NS_STATUS(...) do { \
    LOGI(__VA_ARGS__); \
    if (g_StatusFile) { fprintf(g_StatusFile, __VA_ARGS__); fputc('\n', g_StatusFile); fflush(g_StatusFile); } \
} while(0)

// ---------------------------------------------------------------------------
// État global
// ---------------------------------------------------------------------------
static std::atomic<bool> g_NetSniffEnabled{false};
static std::atomic<bool> g_NetSniffInstalled{false};
static std::atomic<uint64_t> g_BytesWritten{0};
static std::mutex g_FileMutex;
static FILE* g_BinFile = nullptr;
static FILE* g_LogFile = nullptr;
static char g_BinPath[512] = {0};
static char g_LogPath[512] = {0};

// Re-entrance guard (per-thread). Si un hook est appelé depuis l'intérieur
// de notre propre code (ex: socket call dans fwrite via syslog → fwrite →
// socket → hook), on laisse passer sans capturer.
static thread_local int g_InHook = 0;

// ---------------------------------------------------------------------------
// Pointeurs vers les fonctions originales (post-Dobby hook).
// ---------------------------------------------------------------------------
typedef ssize_t (*send_t)(int, const void*, size_t, int);
typedef ssize_t (*recv_t)(int, void*, size_t, int);
typedef ssize_t (*sendto_t)(int, const void*, size_t, int,
                            const struct sockaddr*, socklen_t);
typedef ssize_t (*recvfrom_t)(int, void*, size_t, int,
                              struct sockaddr*, socklen_t*);
typedef int (*SSL_write_t)(void* ssl, const void* buf, int num);
typedef int (*SSL_read_t)(void* ssl, void* buf, int num);

static send_t       orig_send       = nullptr;
static recv_t       orig_recv       = nullptr;
static sendto_t     orig_sendto     = nullptr;
static recvfrom_t   orig_recvfrom   = nullptr;
static SSL_write_t  orig_SSL_write  = nullptr;
static SSL_read_t   orig_SSL_read   = nullptr;

// ---------------------------------------------------------------------------
// Helpers : path resolution + JWT scan
// ---------------------------------------------------------------------------
// Résout un dossier où on peut écrire ET que adb pull peut récupérer.
// Stratégie : essaye plusieurs chemins, garde le premier qui marche.
//   1. /sdcard/Android/data/<pkg>/files  (Scoped Storage app-specific, accessible adb)
//   2. /storage/emulated/0/Android/data/<pkg>/files
//   3. /data/data/<pkg>/files  (private, accessible seulement via run-as si debuggable)
static void resolveAppDir(char* out, size_t outSize) {
    out[0] = 0;
    FILE* f = fopen("/proc/self/cmdline", "rb");
    if (!f) return;
    char pkg[256] = {0};
    size_t r = fread(pkg, 1, sizeof(pkg) - 1, f);
    fclose(f);
    if (r == 0) return;
    pkg[sizeof(pkg) - 1] = 0;
    for (size_t i = 0; i < r; i++) if (pkg[i] == ':') { pkg[i] = 0; break; }
    if (!pkg[0]) return;

    // Essai 1 : /sdcard/Android/data/<pkg>/files (accessible adb shell sans root)
    char candidate[400];
    const char* sdRoots[] = {
        "/sdcard/Android/data",
        "/storage/emulated/0/Android/data",
        "/storage/self/primary/Android/data",
        nullptr
    };
    for (int i = 0; sdRoots[i]; ++i) {
        snprintf(candidate, sizeof(candidate), "%s/%s/files", sdRoots[i], pkg);
        mkdir(candidate, 0775); // créer si manquant
        // Test d'écriture
        char testFile[480];
        snprintf(testFile, sizeof(testFile), "%s/.netsniff_test", candidate);
        FILE* t = fopen(testFile, "w");
        if (t) {
            fclose(t);
            unlink(testFile);
            strncpy(out, candidate, outSize - 1);
            out[outSize - 1] = 0;
            return;
        }
    }
    // Fallback : private data dir (debuggable seulement)
    snprintf(out, outSize, "/data/data/%s/files", pkg);
    mkdir(out, 0775);
}

// JWT format = base64(header).base64(payload).base64(signature).
// Header commence quasi toujours par {"alg":"... → encodé "eyJ".
// On cherche "eyJ" suivi de chars b64 puis '.'. Si trouvé, log alert.
static void scanForJwt(const char* tag, const uint8_t* buf, size_t len) {
    if (len < 30) return;
    for (size_t i = 0; i + 30 < len; ++i) {
        if (buf[i] != 'e' || buf[i+1] != 'y' || buf[i+2] != 'J') continue;
        // Vérifie qu'on a au moins 20 chars b64-safe puis un '.'
        size_t j = i + 3;
        size_t end = (i + 512 < len) ? i + 512 : len;
        bool hasDot = false;
        for (; j < end; ++j) {
            uint8_t c = buf[j];
            bool b64 = (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z')
                    || (c >= '0' && c <= '9') || c == '-' || c == '_'
                    || c == '+' || c == '/' || c == '=';
            if (c == '.') { hasDot = (j > i + 20); break; }
            if (!b64) break;
        }
        if (!hasDot) continue;
        // Trouvé un JWT plausible. Log les ~200 premiers chars.
        size_t showLen = (j - i + 200 < len - i) ? j - i + 200 : len - i;
        if (showLen > 400) showLen = 400;
        char preview[512];
        size_t cp = 0;
        for (size_t k = 0; k < showLen && cp < sizeof(preview) - 1; ++k) {
            uint8_t c = buf[i + k];
            preview[cp++] = (c >= 0x20 && c < 0x7F) ? c : '.';
        }
        preview[cp] = 0;
        LOGI(NS_TAG ": 🔑 JWT detected in %s @ offset %zu : %s",
             tag, i, preview);
        return; // un seul JWT par buffer suffit
    }
}

// Détection HTTP simple pour log lisible
static bool looksLikeHttp(const uint8_t* buf, size_t len) {
    if (len < 6) return false;
    static const char* prefixes[] = {
        "GET ", "POST ", "PUT ", "HEAD ", "DELETE ", "PATCH ",
        "HTTP/1.", "HTTP/2"
    };
    for (auto p : prefixes) {
        size_t pl = strlen(p);
        if (len >= pl && memcmp(buf, p, pl) == 0) return true;
    }
    return false;
}

// ---------------------------------------------------------------------------
// Capture : écrit dans le fichier binaire + résume dans logcat.
// Format binaire (par paquet) :
//   [u8 dir]      0=send, 1=recv, 2=sendto, 3=recvfrom, 4=SSL_write, 5=SSL_read
//   [u8 socktype] inferred 'T' for TCP, 'U' for UDP, 'S' for SSL, '?' unknown
//   [u32 fd]      socket fd ou pointeur SSL tronqué
//   [u32 len]     longueur des données qui suivent
//   [byte * len]  data
// ---------------------------------------------------------------------------
static void captureWrite(uint8_t dir, uint8_t st, uint32_t id,
                         const void* data, size_t len) {
    if (!g_NetSniffEnabled.load(std::memory_order_relaxed)) return;
    if (!data || len == 0) return;
    if (g_BytesWritten.load(std::memory_order_relaxed) > NS_FILE_MAX_BYTES) return;

    // Logcat résumé
    const char* tag;
    switch (dir) {
        case 0: tag = "send"; break;
        case 1: tag = "recv"; break;
        case 2: tag = "sendto"; break;
        case 3: tag = "recvfrom"; break;
        case 4: tag = "SSL_write"; break;
        case 5: tag = "SSL_read"; break;
        default: tag = "?"; break;
    }

    // Hex preview des N premiers bytes
    char hex[NS_LOG_MAX_HEX * 3 + 4] = {0};
    size_t hl = 0;
    size_t show = (len < NS_LOG_MAX_HEX) ? len : NS_LOG_MAX_HEX;
    const uint8_t* b = (const uint8_t*)data;
    for (size_t i = 0; i < show && hl + 4 < sizeof(hex); ++i) {
        snprintf(hex + hl, sizeof(hex) - hl, "%02X ", b[i]);
        hl += 3;
    }

    if (looksLikeHttp(b, len)) {
        // Pour HTTP : log les premiers 256 chars en ASCII
        char ascii[260];
        size_t al = 0;
        size_t showA = (len < 256) ? len : 256;
        for (size_t i = 0; i < showA && al < sizeof(ascii) - 1; ++i) {
            uint8_t c = b[i];
            ascii[al++] = (c >= 0x20 && c < 0x7F) ? c : (c == '\n' ? '\\' : '.');
        }
        ascii[al] = 0;
        LOGI(NS_TAG ": [%s/%c id=%u len=%zu] HTTP: %s",
             tag, st, id, len, ascii);
    } else {
        LOGI(NS_TAG ": [%s/%c id=%u len=%zu] hex: %s%s",
             tag, st, id, len, hex, (len > NS_LOG_MAX_HEX) ? "..." : "");
    }

    // JWT scan (pour SSL_write/read principalement, mais aussi clear text)
    scanForJwt(tag, b, len);

    // Écriture fichier
    {
        std::lock_guard<std::mutex> lock(g_FileMutex);
        if (!g_BinFile) {
            if (g_BinPath[0] == 0) {
                char dir[256];
                resolveAppDir(dir, sizeof(dir));
                if (dir[0] == 0) return;
                snprintf(g_BinPath, sizeof(g_BinPath), "%s/netsniff.bin", dir);
                snprintf(g_LogPath, sizeof(g_LogPath), "%s/netsniff.log", dir);
            }
            g_BinFile = fopen(g_BinPath, "ab");
            g_LogFile = fopen(g_LogPath, "ab");
            if (g_BinFile) chmod(g_BinPath, 0644);
            if (g_LogFile) chmod(g_LogPath, 0644);
        }
        if (g_BinFile) {
            uint32_t L = (uint32_t)len;
            fwrite(&dir, 1, 1, g_BinFile);
            fwrite(&st,  1, 1, g_BinFile);
            fwrite(&id,  1, 4, g_BinFile);
            fwrite(&L,   1, 4, g_BinFile);
            fwrite(data, 1, len, g_BinFile);
            g_BytesWritten.fetch_add(len + 10, std::memory_order_relaxed);
        }
        if (g_LogFile) {
            fprintf(g_LogFile, "%s/%c id=%u len=%zu hex=%s\n",
                    tag, st, id, len, hex);
        }
    }
}

// ---------------------------------------------------------------------------
// Hooks
// ---------------------------------------------------------------------------
static ssize_t hook_send(int fd, const void* buf, size_t n, int flags) {
    ssize_t r = orig_send(fd, buf, n, flags);
    if (g_InHook == 0 && r > 0) {
        g_InHook = 1;
        captureWrite(0, 'T', (uint32_t)fd, buf, (size_t)r);
        g_InHook = 0;
    }
    return r;
}
static ssize_t hook_recv(int fd, void* buf, size_t n, int flags) {
    ssize_t r = orig_recv(fd, buf, n, flags);
    if (g_InHook == 0 && r > 0) {
        g_InHook = 1;
        captureWrite(1, 'T', (uint32_t)fd, buf, (size_t)r);
        g_InHook = 0;
    }
    return r;
}
static ssize_t hook_sendto(int fd, const void* buf, size_t n, int flags,
                           const struct sockaddr* a, socklen_t al) {
    ssize_t r = orig_sendto(fd, buf, n, flags, a, al);
    if (g_InHook == 0 && r > 0) {
        g_InHook = 1;
        captureWrite(2, 'U', (uint32_t)fd, buf, (size_t)r);
        g_InHook = 0;
    }
    return r;
}
static ssize_t hook_recvfrom(int fd, void* buf, size_t n, int flags,
                             struct sockaddr* a, socklen_t* al) {
    ssize_t r = orig_recvfrom(fd, buf, n, flags, a, al);
    if (g_InHook == 0 && r > 0) {
        g_InHook = 1;
        captureWrite(3, 'U', (uint32_t)fd, buf, (size_t)r);
        g_InHook = 0;
    }
    return r;
}
static int hook_SSL_write(void* ssl, const void* buf, int num) {
    int r = orig_SSL_write(ssl, buf, num);
    if (g_InHook == 0 && r > 0) {
        g_InHook = 1;
        captureWrite(4, 'S', (uint32_t)((uintptr_t)ssl & 0xFFFFFFFF), buf, (size_t)r);
        g_InHook = 0;
    }
    return r;
}
static int hook_SSL_read(void* ssl, void* buf, int num) {
    int r = orig_SSL_read(ssl, buf, num);
    if (g_InHook == 0 && r > 0) {
        g_InHook = 1;
        captureWrite(5, 'S', (uint32_t)((uintptr_t)ssl & 0xFFFFFFFF), buf, (size_t)r);
        g_InHook = 0;
    }
    return r;
}

// ---------------------------------------------------------------------------
// Installation
// ---------------------------------------------------------------------------
static int hookSym(const char* libCandidate, const char* sym,
                   void* hookFn, void** origFn) {
    void* h = dlopen(libCandidate, RTLD_NOLOAD);
    if (!h) h = dlopen(libCandidate, RTLD_NOW);
    if (!h) return -1;
    void* addr = dlsym(h, sym);
    if (!addr) return -2;
    int rc = DobbyHook(addr, (dobby_dummy_func_t)hookFn,
                       (dobby_dummy_func_t*)origFn);
    NS_STATUS(NS_TAG ": hook %s@%s addr=%p rc=%d", sym, libCandidate, addr, rc);
    return rc;
}

// -------------------------------------------------------------------------
// Résolution de symboles dans une .so chargée mais pas dlopen-able (apex
// path bloqué sur Android 13+). On utilise dl_iterate_phdr pour trouver la
// base address, puis on parse le fichier .so en lisant SHT_DYNSYM.
// -------------------------------------------------------------------------
struct ElfFindCtx {
    const char* nameNeedle; // ex: "libssl.so"
    uintptr_t baseAddr;
    char fullPath[512];
};

static int elf_findCallback(struct dl_phdr_info* info, size_t /*sz*/, void* data) {
    ElfFindCtx* ctx = (ElfFindCtx*)data;
    if (!info->dlpi_name || !*info->dlpi_name) return 0;
    if (strstr(info->dlpi_name, ctx->nameNeedle)) {
        ctx->baseAddr = info->dlpi_addr;
        strncpy(ctx->fullPath, info->dlpi_name, sizeof(ctx->fullPath) - 1);
        return 1; // stop
    }
    return 0;
}

// Parse l'ELF du fichier on disk pour trouver l'offset du symbole `name`
// dans .dynsym, puis retourne base + offset.
static void* resolveSymbolInLoadedSo(const char* nameNeedle, const char* sym) {
    ElfFindCtx ctx = { nameNeedle, 0, {0} };
    if (!dl_iterate_phdr(elf_findCallback, &ctx) || ctx.baseAddr == 0) {
        NS_STATUS(NS_TAG ": resolveSymbol : %s NOT loaded in process", nameNeedle);
        return nullptr;
    }
    NS_STATUS(NS_TAG ": resolveSymbol : %s loaded @ 0x%lx (path=%s)",
         nameNeedle, ctx.baseAddr, ctx.fullPath);

    int fd = open(ctx.fullPath, O_RDONLY);
    if (fd < 0) {
        LOGE(NS_TAG ": cannot open %s (errno=%d)", ctx.fullPath, errno);
        return nullptr;
    }
    struct stat st;
    if (fstat(fd, &st) < 0) { close(fd); return nullptr; }
    void* fileMap = mmap(nullptr, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    close(fd);
    if (fileMap == MAP_FAILED) {
        LOGE(NS_TAG ": mmap %s failed", ctx.fullPath);
        return nullptr;
    }

    Elf64_Ehdr* ehdr = (Elf64_Ehdr*)fileMap;
    if (memcmp(ehdr->e_ident, ELFMAG, SELFMAG) != 0) {
        munmap(fileMap, st.st_size);
        return nullptr;
    }
    Elf64_Shdr* shdrs = (Elf64_Shdr*)((uint8_t*)fileMap + ehdr->e_shoff);
    Elf64_Sym* symtab = nullptr;
    const char* strtab = nullptr;
    size_t nsyms = 0;
    for (int i = 0; i < ehdr->e_shnum; ++i) {
        if (shdrs[i].sh_type == SHT_DYNSYM) {
            symtab = (Elf64_Sym*)((uint8_t*)fileMap + shdrs[i].sh_offset);
            nsyms = shdrs[i].sh_size / sizeof(Elf64_Sym);
            Elf64_Shdr& sl = shdrs[shdrs[i].sh_link];
            strtab = (const char*)fileMap + sl.sh_offset;
            break;
        }
    }
    if (!symtab || !strtab) {
        LOGE(NS_TAG ": no DYNSYM in %s", ctx.fullPath);
        munmap(fileMap, st.st_size);
        return nullptr;
    }
    void* result = nullptr;
    for (size_t i = 0; i < nsyms; ++i) {
        const char* sname = strtab + symtab[i].st_name;
        if (sname && strcmp(sname, sym) == 0 && symtab[i].st_value != 0) {
            result = (void*)(ctx.baseAddr + symtab[i].st_value);
            break;
        }
    }
    munmap(fileMap, st.st_size);
    if (!result) {
        NS_STATUS(NS_TAG ": symbol %s NOT found in %s", sym, nameNeedle);
    } else {
        NS_STATUS(NS_TAG ": resolved %s @ %p (via ELF parse of %s)",
             sym, result, nameNeedle);
    }
    return result;
}

static int hookViaElfResolver(const char* libNeedle, const char* sym,
                              void* hookFn, void** origFn) {
    void* addr = resolveSymbolInLoadedSo(libNeedle, sym);
    if (!addr) return -1;
    int rc = DobbyHook(addr, (dobby_dummy_func_t)hookFn,
                       (dobby_dummy_func_t*)origFn);
    NS_STATUS(NS_TAG ": ELF hook %s@%s addr=%p rc=%d", sym, libNeedle, addr, rc);
    return rc;
}

// Dump toutes les libs chargées dans le process via /proc/self/maps.
// Permet d'identifier les .so SSL/crypto chargés par Unity à runtime.
// Écrit dans /data/data/<pkg>/files/netsniff_libs.txt (FILE, pas logcat,
// pour éviter l'overflow du ring buffer logcat sous la pression du trafic).
static void dumpLoadedLibsForSslDetection() {
    // Résout le dossier files de l'app
    char appDir[256];
    resolveAppDir(appDir, sizeof(appDir));
    if (appDir[0] == 0) {
        LOGE(NS_TAG ": cannot resolve app data dir");
        return;
    }

    char outPath[512];
    snprintf(outPath, sizeof(outPath), "%s/netsniff_libs.txt", appDir);
    FILE* out = fopen(outPath, "w");
    if (!out) {
        LOGE(NS_TAG ": cannot write %s (errno=%d)", outPath, errno);
        return;
    }
    chmod(outPath, 0644);

    FILE* f = fopen("/proc/self/maps", "r");
    if (!f) {
        LOGE(NS_TAG ": cannot open /proc/self/maps");
        fclose(out);
        return;
    }
    char line[1024];
    char prevSo[512] = {0};
    int total = 0;
    int sslish = 0;
    fprintf(out, "===== LOADED .SO LIBRARIES =====\n");
    while (fgets(line, sizeof(line), f)) {
        size_t L = strlen(line);
        while (L > 0 && (line[L-1] == '\n' || line[L-1] == '\r')) line[--L] = 0;
        const char* path = strrchr(line, ' ');
        if (!path) continue;
        path++;
        if (path[0] != '/') continue;
        const char* dotso = strstr(path, ".so");
        if (!dotso) continue;
        if (strcmp(prevSo, path) == 0) continue;
        strncpy(prevSo, path, sizeof(prevSo) - 1);
        total++;
        bool ssl = (strstr(path, "ssl") || strstr(path, "crypto")
                 || strstr(path, "tls") || strstr(path, "curl")
                 || strstr(path, "Boring") || strstr(path, "boring")
                 || strstr(path, "TLS") || strstr(path, "http")
                 || strstr(path, "net") || strstr(path, "Chill")
                 || strstr(path, "il2cpp"));
        fprintf(out, "%s %s\n", ssl ? "[SSL?]" : "      ", path);
        if (ssl) {
            sslish++;
            LOGI(NS_TAG ": SSL-candidate: %s", path);
        }
    }
    fclose(f);
    fprintf(out, "\n===== %d total .so, %d SSL-candidates =====\n", total, sslish);
    fflush(out);
    fclose(out);

    LOGI(NS_TAG ": libs dumped to %s (%d libs, %d SSL-candidates)",
         outPath, total, sslish);
}

// Path du fichier status (toujours append-able, écrit en parallèle du
// logcat). À récupérer avec :
//   adb pull /sdcard/Android/data/<pkg>/files/netsniff_status.txt
static char g_StatusPath[512] = {0};

void NetSniff_Install() {
    if (g_NetSniffInstalled.exchange(true)) return;

    // ENABLED NETSNIFF HOOKS FOR ECONOMIC EXPLORATION
    NS_STATUS(NS_TAG ": NetSniff hooks installing...");
    // hookSym("libc.so", "send", (void*)hook_send, (void**)&orig_send);
    // hookSym("libc.so", "recv", (void*)hook_recv, (void**)&orig_recv);
    // hookSym("libc.so", "sendto", (void*)hook_sendto, (void**)&orig_sendto);
    // hookSym("libc.so", "recvfrom", (void*)hook_recvfrom, (void**)&orig_recvfrom);
}

void NetSniff_SetEnabled(bool enabled) {
    g_NetSniffEnabled.store(enabled, std::memory_order_relaxed);
    LOGI(NS_TAG ": capture %s (file=%s)",
         enabled ? "ON" : "OFF",
         g_BinPath[0] ? g_BinPath : "(not yet opened)");
    if (!enabled) NetSniff_Flush();
}

bool NetSniff_IsEnabled() {
    return g_NetSniffEnabled.load(std::memory_order_relaxed);
}

void NetSniff_Flush() {
    std::lock_guard<std::mutex> lock(g_FileMutex);
    if (g_BinFile) fflush(g_BinFile);
    if (g_LogFile) fflush(g_LogFile);
    LOGI(NS_TAG ": flushed (%llu bytes total)",
         (unsigned long long)g_BytesWritten.load());
}
