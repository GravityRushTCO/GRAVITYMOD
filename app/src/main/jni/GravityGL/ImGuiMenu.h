#pragma once

class ImGuiMenu {
public:
    static ImGuiMenu& get() {
        static ImGuiMenu instance;
        return instance;
    }
    
    void render();
    bool onTouch(int action, float x, float y);
    void drawEspOverlay(); // Will handle ESP rendering via ImGui later

private:
    ImGuiMenu() = default;
    
    bool m_open = false;
};
