import re

with open('ImGuiMenu.cpp', 'r', encoding='utf-8') as f:
    text = f.read()

start_str = "    if (currentTab == 0) {"
end_str = "    ImGui::EndChild();\n    ImGui::PopStyleColor();\n    ImGui::PopStyleVar();"
start_idx = text.find(start_str)
end_idx = text.find(end_str, start_idx)

if start_idx == -1 or end_idx == -1:
    print("Could not find bounds", start_idx, end_idx)
    exit(1)

print("Found bounds!")
