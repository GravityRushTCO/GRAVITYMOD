import re

with open('ImGuiMenu.cpp', 'r', encoding='utf-8') as f:
    text = f.read()

# Add the maps at the top
header = """
// --- Remote Sync ---
#include <map>
static std::map<int, bool> g_RemoteToggles;
static std::map<int, bool> g_RemoteTogglesPending;
static std::map<int, float> g_RemoteSliders;
static std::map<int, bool> g_RemoteSlidersPending;
"""

text = text.replace('bool g_ConfigLoaded = false;', header + '\nbool g_ConfigLoaded = false;')

# Inject ExecuteRemoteCommand parser
cmd_parser = """  } else if (command == "toggle" || command == "feature" || command == "set_value") {
    int f_id = -1;
    if (cmd.contains("id")) f_id = cmd["id"].get<int>();
    else if (cmd.contains("feature_id")) f_id = cmd["feature_id"].get<int>();
    if (f_id != -1 && cmd.contains("value")) {
      if (cmd["value"].is_boolean()) {
        g_RemoteToggles[f_id] = cmd["value"].get<bool>();
        g_RemoteTogglesPending[f_id] = true;
      } else if (cmd["value"].is_number()) {
        g_RemoteSliders[f_id] = cmd["value"].get<float>();
        g_RemoteSlidersPending[f_id] = true;
      }
    }
"""
text = text.replace('  } else if (command == "unfreeze") {', cmd_parser + '  } else if (command == "unfreeze") {')

# Toggle replacement
def repl_toggle(m):
    var_name = m.group(1)
    feat_id = m.group(2)
    sync_code = f"if (g_RemoteTogglesPending[{feat_id}]) {{ {var_name} = g_RemoteToggles[{feat_id}]; g_RemoteTogglesPending[{feat_id}] = false; TriggerChange({feat_id}, {var_name}); }}\n      "
    return m.group(0).replace(f"if (IsFeatureVisible({feat_id}))", sync_code + f"if (IsFeatureVisible({feat_id}))")

text = re.sub(r'static bool\s+([a-zA-Z0-9_]+)\s*=\s*(?:false|true);\s*if\s*\(IsFeatureVisible\((\d+)\)\)', repl_toggle, text)

# Slider replacement
def repl_slider(m):
    var_name = m.group(1)
    feat_id = m.group(2)
    sync_code = f"if (g_RemoteSlidersPending[{feat_id}]) {{ {var_name} = g_RemoteSliders[{feat_id}]; g_RemoteSlidersPending[{feat_id}] = false; TriggerChange({feat_id}, false, (int){var_name}); }}\n      "
    return m.group(0).replace(f"if (IsFeatureVisible({feat_id}))", sync_code + f"if (IsFeatureVisible({feat_id}))")

text = re.sub(r'static float\s+([a-zA-Z0-9_]+)\s*=\s*[\d\.]+f?;\s*if\s*\(IsFeatureVisible\((\d+)\)\)', repl_slider, text)

with open('ImGuiMenu.cpp', 'w', encoding='utf-8') as f:
    f.write(text)

print('Patched successfully!')
