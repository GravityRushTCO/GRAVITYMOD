import re

with open('ImGuiMenu.cpp', 'r', encoding='utf-8') as f:
    text = f.read()

start_idx = text.find('if (physicalTab == 0) {')
end_idx = text.find('} else if (physicalTab == 3) {')

content = text[start_idx:end_idx]

with open('features_code.txt', 'w', encoding='utf-8') as f:
    f.write(content)
