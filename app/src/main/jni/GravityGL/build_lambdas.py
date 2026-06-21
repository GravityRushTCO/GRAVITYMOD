import re

with open('features_code.txt', 'r', encoding='utf-8') as f:
    code = f.read()

# I will write a script that generates the `RenderFeature` switch based on a dictionary of code snippets.
# But since I didn't extract 500+ features from `features_code.txt`, let me grab the entire file first.
with open('ImGuiMenu.cpp', 'r', encoding='utf-8') as f:
    full_text = f.read()

# Instead of hardcoding dictionaries, I'll extract EVERY feature block using regex.
# A feature block starts with `static` or `bool` or `if (IsFeatureVisible(` and contains `TriggerChange(ID`.
# The regex must capture the entire code for that feature.
# This is tricky in regex because of nested braces. 

# Let's just create a dynamic dispatcher using python script that replaces the static feature declarations inline!
# No, we need a switch statement `switch(id)` where `id` is the feature ID.
# What if we just provide the C++ rewrite manually? It's much safer.

def extract_feature(text, feature_id):
    # Find TriggerChange(feature_id
    pattern = r'TriggerChange\(' + str(feature_id) + r'(?:,|\))'
    match = re.search(pattern, text)
    if not match:
        return ""
    
    # We trace back to find the start of the feature. Usually `static ` or `if (IsFeatureVisible`
    start_idx = text.rfind('static ', 0, match.start())
    alt_start = text.rfind('if (IsFeatureVisible', 0, match.start())
    alt_start2 = text.rfind('bool crosshair =', 0, match.start())
    
    # Take the maximum valid start index
    starts = [s for s in [start_idx, alt_start, alt_start2] if s != -1]
    if not starts:
        return ""
    start = max(starts)

    # Now we need to find the end of the block.
    # It usually ends with `TriggerChange(feature_id, false);\n      }`
    end_pattern = r'TriggerChange\(' + str(feature_id) + r', false\);\s*\}'
    end_match = re.search(end_pattern, text[start:])
    
    if end_match:
        end = start + end_match.end()
    else:
        # Some don't have else if (var) { var=false; ... }
        # They end right after TriggerChange
        end_brace = text.find('}', match.end())
        end = end_brace + 1

    return text[start:end].strip()

# IDs to extract
feature_ids = [
    120, 132, 184, 185, 183, 205, 300, 301, 302,
    121, 194, 195, 196, 241, 197, 199, 240, 193,
    109, 308, 13, 228, 400, 110, 306, 153, 305, 304,
    500, 501, 502, 503, 504, 505, 506, 507, 143, 510, 511, 512, 513, 514, 515, 516, 517
]

switch_cases = ""
for fid in feature_ids:
    code_block = extract_feature(full_text, fid)
    if code_block:
        switch_cases += f"            case {fid}: {{\n                {code_block.replace(chr(10), chr(10)+'                ')}\n                break;\n            }}\n"

dynamic_engine = f"""
    auto RenderFeature = [&](int id) {{
        switch(id) {{
{switch_cases}
        }}
    }};

    // MAPPAGE DES ONGLETS DYNAMIQUES
    int physicalTab = currentTab;
    if (g_ConfigLoaded && g_DynamicConfig.contains("tabs") && g_DynamicConfig["tabs"].is_array()) {{
        int i = 0;
        for (auto& tab : g_DynamicConfig["tabs"]) {{
            if (i == currentTab) {{
                if (tab.contains("id")) {{
                    std::string id = tab["id"];
                    if (id == "combat") physicalTab = 0;
                    else if (id == "visuels") physicalTab = 1;
                    else if (id == "monde") physicalTab = 2;
                    else if (id == "catalogue") physicalTab = 3;
                    else if (id == "teleports") physicalTab = 4;
                    else if (id == "couleurs") physicalTab = 5;
                }}
            }}
            i++;
        }}
    }}
"""

# We now replace the contents of physicalTab 0, 1, 2 with the dynamic loops.
# Let's find the start and end of physicalTab == 0, 1, 2.

start_str = "    // Replace currentTab checks with physicalTab\n    if (physicalTab == 0) {"
end_str = "    } else if (physicalTab == 3) {"

start_idx = full_text.find(start_str)
end_idx = full_text.find(end_str)

if start_idx == -1 or end_idx == -1:
    print("Could not find bounds", start_idx, end_idx)
    exit(1)

replacement = """    // Replace currentTab checks with physicalTab
    if (physicalTab == 0 || physicalTab == 1 || physicalTab == 2) {
        if (g_ConfigLoaded && g_DynamicConfig.contains("tabs") && g_DynamicConfig["tabs"].is_array()) {
            for (auto& tab : g_DynamicConfig["tabs"]) {
                std::string tid = tab["id"];
                if ((physicalTab == 0 && tid == "combat") ||
                    (physicalTab == 1 && tid == "visuels") ||
                    (physicalTab == 2 && tid == "monde")) {
                    
                    if (tab.contains("label")) {
                        CenterText(GetGradientColorU32(0.5f), tab["label"].get<std::string>().c_str());
                        ImGui::Dummy(ImVec2(0, 5));
                    }
                    if (tab.contains("items") && tab["items"].is_array()) {
                        for (auto& item : tab["items"]) {
                            if (item.contains("id")) {
                                RenderFeature(item["id"].get<int>());
                            }
                        }
                    }
                }
            }
        } else {
            // Fallback rendering
            if (physicalTab == 0) {
                CenterText(GetGradientColorU32(0.3f), "AIMBOT & ASSISTANCE"); ImGui::Dummy(ImVec2(0, 5));
                RenderFeature(120); RenderFeature(132); RenderFeature(184); RenderFeature(185); RenderFeature(183); RenderFeature(205); RenderFeature(300); RenderFeature(301); RenderFeature(302);
            } else if (physicalTab == 1) {
                CenterText(GetGradientColorU32(0.5f), "ESP & VISUELS"); ImGui::Dummy(ImVec2(0, 5));
                RenderFeature(121); RenderFeature(194); RenderFeature(195); RenderFeature(196); RenderFeature(241); RenderFeature(197); RenderFeature(199); RenderFeature(240); RenderFeature(193);
            } else if (physicalTab == 2) {
                CenterText(GetGradientColorU32(0.7f), "MONDE & DIVERS"); ImGui::Dummy(ImVec2(0, 5));
                RenderFeature(109); RenderFeature(308); RenderFeature(13); RenderFeature(228); RenderFeature(400); RenderFeature(110); RenderFeature(306); RenderFeature(153);
                // Also 500+ features
                int worldFeatures[] = {500, 501, 502, 503, 504, 505, 506, 507, 143, 510, 511, 512, 513, 514, 515, 516, 517};
                for(int fid : worldFeatures) RenderFeature(fid);
            }
        }
"""

new_text = full_text[:start_idx] + replacement + full_text[end_idx:]

# Also we need to inject dynamic_engine right before `int physicalTab = currentTab;`
engine_start = new_text.find('    // --- MAPPAGE DES ONGLETS DYNAMIQUES ---')
engine_end = new_text.find('    // Replace currentTab checks with physicalTab')

if engine_start != -1 and engine_end != -1:
    new_text = new_text[:engine_start] + dynamic_engine + new_text[engine_end:]
else:
    print("Could not find engine bounds")
    exit(1)

with open('ImGuiMenu.cpp', 'w', encoding='utf-8') as f:
    f.write(new_text)

print("ImGuiMenu refactored with lambdas!")
