package com.android.support;

import java.util.HashMap;
import java.util.Map;

/**
 * LanguageManager — Gravity Mod Menu
 * Manages UI language with persistent preference storage.
 * Keys MUST match exactly what GetFeatureList() returns.
 */
public class LanguageManager {
    public static String currentLanguage = "fr"; // Default

    // 8 supported languages
    public static final String[] LANG_CODES   = {"fr","en","ar","es","pt","de","tr","ru"};
    public static final String[] LANG_NAMES   = {"Français","English","العربية","Español","Português","Deutsch","Türkçe","Русский"};
    public static final String[] LANG_FLAGS   = {"🇫🇷","🇬🇧","🇸🇦","🇪🇸","🇧🇷","🇩🇪","🇹🇷","🇷🇺"};

    private static final Map<String, Map<String, String>> dicts = new HashMap<>();

    static {
        // ── CATEGORIES ──────────────────────────────────────────────────
        add("🛡️ DEFENSE",                                "🛡️ DEFENSE",            "🛡️ الدفاع",             "🛡️ DEFENSA",
            "🛡️ DEFESA",             "🛡️ VERTEIDIGUNG",     "🛡️ SAVUNMA",           "🛡️ ЗАЩИТА");
        add("👁️ ESP / AIMBOT",                           "👁️ ESP / AIMBOT",        "👁️ الرادار / التصويب",  "👁️ ESP / AIMBOT",
            "👁️ ESP / MIRA",          "👁️ ESP / AIMBOT",     "👁️ ESP / NIŞAN",       "👁️ ESP / ПРИЦЕЛ");
        add("🔫 ARMES",                                  "🔫 WEAPONS",             "🔫 الأسلحة",            "🔫 ARMAS",
            "🔫 ARMAS",               "🔫 WAFFEN",            "🔫 SİLAHLAR",          "🔫 ОРУЖИЕ");
        add("📷 VISION",                                 "📷 VISION",              "📷 الرؤية",             "📷 VISIÓN",
            "📷 VISÃO",               "📷 SICHT",             "📷 GÖRÜŞ",              "📷 ЗРЕНИЕ");
        add("🏃 MOUVEMENT",                              "🏃 MOVEMENT",            "🏃 الحركة",             "🏃 MOVIMIENTO",
            "🏃 MOVIMENTO",           "🏃 BEWEGUNG",          "🏃 HAREKET",            "🏃 ДВИЖЕНИЕ");
        add("📍 TELEPORT",                               "📍 TELEPORT",            "📍 الانتقال الآني",      "📍 TELETRANSPORTE",
            "📍 TELETRANSPORTE",      "📍 TELEPORT",          "📍 IŞINLANMA",          "📍 ТЕЛЕПОРТ");
        add("👑 ADMIN",                                  "👑 ADMIN",               "👑 المشرف",             "👑 ADMIN",
            "👑 ADMIN",               "👑 ADMIN",             "👑 YÖNETİCİ",           "👑 ADMIN");
        add("🛡️ ANTI-BAN",                              "🛡️ ANTI-BAN",           "🛡️ ضد الحظر",          "🛡️ ANTI-BAN",
            "🛡️ ANTI-BAN",           "🛡️ ANTI-BAN",         "🛡️ BAN-KORUMASI",      "🛡️ ANTI-BAN");

        // ── FEATURES (labels only) ─────────
        add("🏃 NoClip [traverse les murs]",
            "🏃 NoClip [walk through walls]",
            "🏃 عبور الجدران",
            "🏃 NoClip [atraviesa paredes]",
            "🏃 NoClip [atravessa paredes]",
            "🏃 NoClip [durch Wände]",
            "🏃 NoClip [duvardan geç]",
            "🏃 NoClip [сквозь стены]");

        add("Anti-Desync Véhicules [permet de les pousser]",
            "Anti-Desync Vehicles [allows pushing]",
            "مضاد عدم التزامن للمركبات [يسمح بالدفع]",
            "Anti-Desync Vehículos [permite empujar]",
            "Anti-Desync Veículos [permite empurrar]",
            "Anti-Desync Fahrzeuge [erlaubt Schieben]",
            "Anti-Desync Araçlar [itmeye izin verir]",
            "Анти-рассинхрон машин [позволяет толкать]");

        add("ESP + Aimbot + Dot Mode [vise automatiquement les joueurs]",
            "ESP + Aimbot + Dot Mode [auto-aim at players]",
            "ESP + تصويب تلقائي + نقطة [يستهدف اللاعبين]",
            "ESP + Aimbot + Modo Punto [apunta jugadores]",
            "ESP + Aimbot + Ponto [mira automática]",
            "ESP + Aimbot + Punkt [auto-zielen]",
            "ESP + Aimbot + Nokta [otomatik nişan]",
            "ESP + Aimbot + Точка [авто-прицел]");

        add("Wall-Bang [tire a travers les murs]",
            "Wall-Bang [shoot through walls]",
            "Wall-Bang [يطلق عبر الجدران]",
            "Wall-Bang [dispara por paredes]",
            "Wall-Bang [dispara pelas paredes]",
            "Wall-Bang [durch Wände schießen]",
            "Wall-Bang [duvardan ateş et]",
            "Wall-Bang [стрельба сквозь стены]");

        add("Pas de Rechargement",
            "No Reload",
            "بدون إعادة تحميل",
            "Sin Recarga",
            "Sem Recarga",
            "Kein Nachladen",
            "Yeniden Şarj Yok",
            "Без перезарядки");

        add("Camera FOV 60 a 140",
            "Camera FOV 60 to 140",
            "زاوية الكاميرا 60 إلى 140",
            "Cámara FOV 60 a 140",
            "FOV Câmera 60 a 140",
            "Kamera FOV 60 bis 140",
            "Kamera FOV 60-140",
            "FOV камеры 60-140");

        add("Vitesse 1.0x a 3.0x (valeur/10)",
            "Speed 1.0x to 3.0x (value/10)",
            "السرعة 1.0x إلى 3.0x (القيمة÷10)",
            "Velocidad 1.0x a 3.0x (valor/10)",
            "Velocidade 1.0x a 3.0x (valor/10)",
            "Geschwindigkeit 1.0x bis 3.0x (Wert/10)",
            "Hız 1.0x-3.0x (değer/10)",
            "Скорость 1.0x до 3.0x (значение/10)");

        add("📍 TP Joueur a Waypoint (Carte)",
            "📍 TP Player to Waypoint",
            "📍 انتقال اللاعب إلى العلامة",
            "📍 TP Jugador a Marcador",
            "📍 TP Jogador para Waypoint",
            "📍 TP Spieler zu Wegpunkt",
            "📍 Oyuncuyu Işınla",
            "📍 ТП игрока к точке");

        add("🚗 TP Voiture a Waypoint (Carte)",
            "🚗 TP Car to Waypoint",
            "🚗 انتقال السيارة إلى العلامة",
            "🚗 TP Coche a Marcador",
            "🚗 TP Carro para Waypoint",
            "🚗 TP Auto zu Wegpunkt",
            "🚗 Arabayı Işınla",
            "🚗 ТП машины к точке");

        add("🎯 S'accrocher a la cible (1 fois)",
            "🎯 Grapple to target (once)",
            "🎯 التثبت بالهدف (مرة واحدة)",
            "🎯 Enganchar al objetivo",
            "🎯 Agarrar o alvo",
            "🎯 Ziel greifen (1x)",
            "🎯 Hedefe tutun",
            "🎯 Захват цели (1 раз)");

        add("🚁 Auto-Suivre la cible (vol stationnaire)",
            "🚁 Auto-Follow target (hover)",
            "🚁 المتابعة التلقائية (طيران)",
            "🚁 Auto-Seguir objetivo",
            "🚁 Auto-Seguir alvo",
            "🎯 Ziel automatisch folgen",
            "🚁 Hedefe otomatik tak",
            "🚁 Авто-следование за целью");

        add("📍 Memoriser lieu actuel -> Favori 1",
            "📍 Save location → Fav 1",
            "📍 حفظ الموقع → مفضلة 1",
            "📍 Guardar ubicación → Fav 1",
            "📍 Salvar local → Fav 1",
            "📍 Ort speichern → Fav 1",
            "📍 Konumu kaydet → Fav 1",
            "📍 Сохранить место → Изб 1");
        add("🚀 Me teleporter au Favori 1",
            "🚀 Teleport to Fav 1",
            "🚀 الانتقال إلى مفضلة 1",
            "🚀 Teletransportarse a Fav 1",
            "🚀 Teletransportar a Fav 1",
            "🚀 Zum Favoriten 1 teleportieren",
            "🚀 Fav 1'e ışınlan",
            "🚀 Телепорт к Изб 1");
        add("📍 Memoriser lieu actuel -> Favori 2",
            "📍 Save location → Fav 2","📍 حفظ → مفضلة 2","📍 Guardar → Fav 2",
            "📍 Salvar → Fav 2","📍 Ort → Fav 2","📍 Kaydet → Fav 2","📍 Сохранить → Изб 2");
        add("🚀 Me teleporter au Favori 2",
            "🚀 Teleport to Fav 2","🚀 الانتقال إلى مفضلة 2","🚀 TP a Fav 2",
            "🚀 TP Fav 2","🚀 Fav 2 TP","🚀 Fav 2 ışınlan","🚀 ТП к Изб 2");
        add("📍 Memoriser lieu actuel -> Favori 3",
            "📍 Save location → Fav 3","📍 حفظ → مفضلة 3","📍 Guardar → Fav 3",
            "📍 Salvar → Fav 3","📍 Ort → Fav 3","📍 Kaydet → Fav 3","📍 Сохранить → Изб 3");
        add("🚀 Me teleporter au Favori 3",
            "🚀 Teleport to Fav 3","🚀 الانتقال إلى مفضلة 3","🚀 TP a Fav 3",
            "🚀 TP Fav 3","🚀 Fav 3 TP","🚀 Fav 3 ışınlan","🚀 ТП к Изб 3");

        add("Niveau Admin: 4",
            "Admin Level: 4","مستوى المشرف: 4","Nivel Admin: 4",
            "Nível Admin: 4","Admin-Level: 4","Yönetici Seviyesi: 4","Уровень Admin: 4");

        add("⚠️ Anti-Ban [EN CAS DE BAN UNIQUEMENT - supprime le compte invite actuel]",
            "⚠️ Anti-Ban [IF BANNED ONLY - removes guest account]",
            "⚠️ ضد الحظر [فقط عند الحظر]",
            "⚠️ Anti-Ban [SOLO SI BANEADO]",
            "⚠️ Anti-Ban [APENAS SE BANIDO]",
            "⚠️ Anti-Ban [NUR BEI BAN]",
            "⚠️ Anti-Ban [YALNIZCA BAN'DA]",
            "⚠️ Anti-Ban [ТОЛЬКО ПРИ БАНЕ]");
        add("Regenerer Device ID [nouveau GUID + Model + Name]",
            "Regenerate Device ID [new GUID + Model + Name]",
            "تجديد معرف الجهاز",
            "Regenerar ID de dispositivo",
            "Regenerar ID do dispositivo",
            "Geräte-ID regenerieren",
            "Cihaz ID'sini yenile",
            "Обновить ID устройства");

        add("MINIMIZE",
            "MINIMIZE","تصغير","MINIMIZAR",
            "MINIMIZAR","MINIMIEREN","KÜÇÜLT","СВЕРНУТЬ");
        add("Langue / Language",
            "Language","اللغة","Idioma",
            "Idioma","Sprache","Dil","Язык");
    }

    private static void add(String fr,
        String enV, String arV, String esV,
        String ptV, String deV, String trV,
        String ruV) {
        
        Map<String,String> en = dicts.get("en"); if(en==null){en=new HashMap<>();dicts.put("en",en);}
        Map<String,String> ar = dicts.get("ar"); if(ar==null){ar=new HashMap<>();dicts.put("ar",ar);}
        Map<String,String> es = dicts.get("es"); if(es==null){es=new HashMap<>();dicts.put("es",es);}
        Map<String,String> pt = dicts.get("pt"); if(pt==null){pt=new HashMap<>();dicts.put("pt",pt);}
        Map<String,String> de = dicts.get("de"); if(de==null){de=new HashMap<>();dicts.put("de",de);}
        Map<String,String> tr = dicts.get("tr"); if(tr==null){tr=new HashMap<>();dicts.put("tr",tr);}
        Map<String,String> ru = dicts.get("ru"); if(ru==null){ru=new HashMap<>();dicts.put("ru",ru);}
        
        en.put(fr, enV); ar.put(fr, arV); es.put(fr, esV);
        pt.put(fr, ptV); de.put(fr, deV); tr.put(fr, trV); ru.put(fr, ruV);
    }

    public static String translateLabel(String label) {
        if ("fr".equals(currentLanguage) || label == null) return label;
        Map<String,String> dict = dicts.get(currentLanguage);
        if (dict == null) return label;
        String t = dict.get(label);
        return t != null ? t : label;
    }

    public static String translateFeatureString(String raw) {
        if ("fr".equals(currentLanguage) || raw == null) return raw;

        String prefix = "";
        String suffix = "";
        String label = raw;

        if (label.endsWith("_True")) {
            suffix = "_True";
            label = label.substring(0, label.length() - 5);
        }

        if (label.startsWith("CollapseAdd_")) {
            prefix = "CollapseAdd_";
            label = label.substring(12);
        }

        String[] firstSplit = label.split("_", 2);
        if (firstSplit.length > 0 && firstSplit[0].matches("^-?\\d+$")) {
            prefix += firstSplit[0] + "_";
            label = firstSplit.length > 1 ? firstSplit[1] : "";
        }

        String[] typeSplit = label.split("_", 2);
        if (typeSplit.length < 2) return raw;

        String type = typeSplit[0];
        String rest = typeSplit[1];
        prefix += type + "_";

        if ("SeekBar".equals(type)) {
            int last = rest.lastIndexOf('_');
            if (last > 0) {
                int secondLast = rest.lastIndexOf('_', last - 1);
                if (secondLast >= 0) {
                    suffix = rest.substring(secondLast) + suffix;
                    rest = rest.substring(0, secondLast);
                }
            }
        }

        return prefix + translateLabel(rest) + suffix;
    }

    public static String getText(String key) {
        return translateLabel(key);
    }
}
