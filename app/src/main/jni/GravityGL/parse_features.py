import re
import json

with open('ImGuiMenu.cpp', 'r', encoding='utf-8') as f:
    text = f.read()

features = []
# Match checkboxes
for match in re.finditer(r'CustomCheckbox\("([^"]+)"', text):
    # Try to find the nearest TriggerChange
    idx = match.end()
    trigger_match = re.search(r'TriggerChange\((\d+)', text[idx:idx+200])
    if trigger_match:
        fid = int(trigger_match.group(1))
        # Ensure we don't have duplicates
        if not any(f['id'] == fid for f in features):
            features.append({"id": fid, "label": match.group(1), "type": "checkbox"})

# Match sliders
for match in re.finditer(r'CustomSliderFloat\("([^"]+)"', text):
    idx = match.end()
    trigger_match = re.search(r'TriggerChange\((\d+)', text[idx:idx+200])
    if trigger_match:
        fid = int(trigger_match.group(1))
        if not any(f['id'] == fid for f in features):
            features.append({"id": fid, "label": match.group(1), "type": "slider"})

# Match combos
for match in re.finditer(r'CustomCombo\("([^"]+)"', text):
    idx = match.end()
    trigger_match = re.search(r'TriggerChange\((\d+)', text[idx:idx+200])
    if trigger_match:
        fid = int(trigger_match.group(1))
        if not any(f['id'] == fid for f in features):
            features.append({"id": fid, "label": match.group(1), "type": "combo"})

print(json.dumps(features, indent=2))
