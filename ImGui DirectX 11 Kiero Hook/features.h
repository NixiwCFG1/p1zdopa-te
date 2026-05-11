#pragma once
#include "game.h"
#include "render3d.h"
#include "imgui/imgui.h"
#include "blur.h"
#include "spread.h"
#include <algorithm>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <cstring>
#include <string>
#include <system_error>
#include <unordered_map>
#include <vector>
#include <wininet.h>
#include <shellapi.h>
#include <ctime>
#define _CRT_SECURE_NO_WARNINGS
#pragma comment(lib, "wininet.lib")
#pragma comment(lib, "Shell32.lib")

namespace Settings
{
    inline bool esp_enabled = false;
    inline bool esp_box = false;
    inline int  esp_box_style = 0;
    inline bool esp_health = false;
    inline bool esp_armor = false;
    inline bool esp_name = false;
    inline bool esp_distance = false;
    inline bool esp_skeleton = false;
    inline bool esp_head_dot = false;
    inline bool esp_snaplines = false;
    inline bool esp_team = false;
    inline float esp_color_enemy[4] = { 1.0f, 0.0f, 0.0f, 1.0f };
    inline float esp_color_team[4] = { 0.0f, 0.4f, 1.0f, 1.0f };
    inline float esp_color_skeleton[4] = { 0.0f, 1.0f, 0.0f, 1.0f };
    inline bool esp_glow = false;
    inline float esp_glow_color_enemy[4] = { 1.0f, 0.0f, 0.0f, 1.0f };
    inline float esp_glow_color_team[4] = { 0.0f, 0.4f, 1.0f, 1.0f };
    inline bool aim_enabled = false;
    inline int  aim_key = VK_RBUTTON;
    inline float aim_fov = 10.0f;
    inline float aim_smooth = 5.0f;
    inline int  aim_bone = Game::BONE_HEAD;
    inline bool aim_team = false;
    inline bool aim_visible_only = false;
    inline bool aim_fov_circle = true;
    inline bool trigger_enabled = false;
    inline int  trigger_key = VK_LMENU;
    inline int  trigger_delay = 0;
    inline bool trigger_team = false;
    inline bool trigger_prospread = false;
    inline bool bhop_enabled = false;
    inline bool autostrafe = true;
    inline bool noflash_enabled = false;
    inline float noflash_alpha = 0.0f;
    inline bool radar_hack = false;
    inline float bitcoin_price = 0.0f;
    inline std::string bitcoin_last_update = "Never";
    inline float nigeria_temp = 0.0f;
    inline std::string nigeria_condition = "Unknown";
    inline std::string nigeria_last_update = "Never";
    inline bool warning_enabled = false;
    inline bool thirdperson = false;
    inline int  thirdperson_dist = 150;
    inline bool auto_accept = false;
    inline bool auto_buy = false;
    inline int  auto_buy_preset = 0;
    inline bool watermark = true;
    inline bool show_hotkeys = true;
    inline bool show_bombtimer = true;
    inline bool spectator_enemy = false;
    inline int  spectator_mode = 0;
    inline int  spectator_enemy_index = -1;
    inline uint32_t spectator_enemy_list[64];
    inline int  spectator_enemy_count = 0;
    inline bool spectator_only_enemies = true;
    inline bool crosshair_enabled = false;
    inline int  crosshair_size = 5;
    inline int  crosshair_gap = 2;
    inline int  crosshair_thickness = 1;
    inline float crosshair_color[4] = { 0.0f, 1.0f, 0.0f, 1.0f };
    inline bool fov_changer = false;
    inline int  fov_value = 90;
    inline bool no_visual_recoil = false;
    inline bool custom_hud = false;
    inline bool grenade_tracer = false;
    inline float grenade_tracer_thickness = 2.0f;
    inline float grenade_trail_duration = 5.0f;
    inline bool grenade_prediction = false;
    inline bool bullet_tracer = false;
    inline float bullet_tracer_duration = 3.0f;
    inline float bullet_tracer_color[4] = { 1.0f, 0.85f, 0.3f, 1.0f };
    inline bool spectator_list = true;
    inline bool rain_enabled = false;
    inline int  rain_intensity = 300;
    inline float rain_speed = 1.0f;
    inline float rain_wind = 0.15f;
    inline float rain_alpha = 0.25f;
    inline bool world_modulation = false;
    inline float world_color[4] = { 1.0f, 1.0f, 1.0f, 0.15f };
    inline int world_mod_mode = 0;
    inline bool trashtalk_enabled = false;

    //inline char cheat_tag[32] = "PIZDEC (c) rapira";

}

namespace Hotkeys
{
    inline ImVec2 pos = ImVec2(-1, -1);
    inline bool pos_init = false;
    inline float anim_alpha = 0.0f;
    static const char* CATEGORY_COMBAT = "Combat";
    static const char* CATEGORY_VISUALS = "Visuals";
    static const char* CATEGORY_MISC = "Misc";
    struct Bind { const char* name; bool* toggle; int vk; int mode; const char* category; };
    inline Bind binds[32] = {};
    inline int bind_count = 0;
    struct CatAnim { float height; float alpha; };
    inline CatAnim cat_anims[8] = {};
    inline int cat_count = 0;
    inline const char* cat_names[8] = {};
    inline int rebind_idx = -1;
    inline int ctx_idx = -1;
    inline bool casino_mode = true;
    inline bool waiting_for_lmb = false;

    inline int FindBind(bool* toggle) {
        for (int i = 0; i < bind_count; i++)
            if (binds[i].toggle == toggle) return i;
        return -1;
    }

    inline int FindBindByName(const char* name) {
        if (!name) return -1;
        for (int i = 0; i < bind_count; i++) {
            if (binds[i].name && strcmp(binds[i].name, name) == 0)
                return i;
        }
        return -1;
    }

    inline void RegisterBind(const char* name, bool* toggle, int default_vk = 0, int mode = 0, const char* category = CATEGORY_MISC) {
        int idx = FindBind(toggle);
        if (idx >= 0) { binds[idx].name = name; binds[idx].category = category; return; }
        if (bind_count >= 32) return;
        binds[bind_count++] = { name, toggle, default_vk, mode, category };
    }

    inline void SetKey(bool* toggle, int vk) {
        int idx = FindBind(toggle);
        if (idx >= 0) binds[idx].vk = vk;
    }

    inline void RemoveKey(bool* toggle) {
        int idx = FindBind(toggle);
        if (idx >= 0) binds[idx].vk = 0;
    }
    inline void StartRebind(bool* toggle) {
        int idx = FindBind(toggle);
        if (idx >= 0) rebind_idx = idx;
    }

    inline const char* VkName(int vk) {
        switch (vk) {
        case 0: return nullptr;
        case VK_LBUTTON: return "LMB";
        case VK_RBUTTON: return "RMB";
        case VK_MBUTTON: return "MMB";
        case VK_XBUTTON1: return "X1";
        case VK_XBUTTON2: return "X2";
        case VK_LMENU: return "LAlt";
        case VK_RMENU: return "RAlt";
        case VK_LSHIFT: return "LShift";
        case VK_RSHIFT: return "RShift";
        case VK_LCONTROL: return "LCtrl";
        case VK_RCONTROL: return "RCtrl";
        case VK_SPACE: return "Space";
        case VK_INSERT: return "INS";
        case VK_DELETE: return "DEL";
        case VK_HOME: return "Home";
        case VK_END: return "End";
        case VK_CAPITAL: return "Caps";
        case VK_TAB: return "Tab";
        default: {
            if (vk >= 0x30 && vk <= 0x39) { static char d[2]; d[0] = (char)vk; d[1] = 0; return d; }
            if (vk >= 0x41 && vk <= 0x5A) { static char c[2]; c[0] = (char)vk; c[1] = 0; return c; }
            if (vk >= VK_F1 && vk <= VK_F12) { static char f[4]; sprintf_s(f, "F%d", vk - VK_F1 + 1); return f; }
            static char _kb[8]; sprintf_s(_kb, "0x%02X", vk); return _kb;
        }
        }
    }
    inline void ProcessRebind() {
        if (rebind_idx < 0) {
            if (waiting_for_lmb && (GetAsyncKeyState(VK_LBUTTON) & 1)) {
                waiting_for_lmb = false;
                rebind_idx = ctx_idx;
            }
            return;
        }

        if (GetAsyncKeyState(VK_ESCAPE) & 0x8000) { rebind_idx = -1; waiting_for_lmb = false; return; }
        if (GetAsyncKeyState(VK_DELETE) & 0x8000) { binds[rebind_idx].vk = 0; rebind_idx = -1; waiting_for_lmb = false; return; }

        if (casino_mode) {
            static const int scan[] = {
                VK_LBUTTON, VK_RBUTTON, VK_MBUTTON, VK_XBUTTON1, VK_XBUTTON2,
                VK_LMENU, VK_RMENU, VK_LSHIFT, VK_RSHIFT, VK_LCONTROL, VK_RCONTROL,
                VK_SPACE, VK_INSERT, VK_HOME, VK_END, VK_CAPITAL, VK_TAB,
                0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39,
                0x41,0x42,0x43,0x44,0x45,0x46,0x47,0x48,0x49,0x4A,0x4B,0x4C,0x4D,
                0x4E,0x4F,0x50,0x51,0x52,0x53,0x54,0x55,0x56,0x57,0x58,0x59,0x5A,
                VK_F1,VK_F2,VK_F3,VK_F4,VK_F5,VK_F6,VK_F7,VK_F8,VK_F9,VK_F10,VK_F11,VK_F12
            };
            for (int k : scan) {
                if (GetAsyncKeyState(k) & 1) {
                    binds[rebind_idx].vk = k;
                    rebind_idx = -1;
                    return;
                }
            }
        } else {
            static const int scan[] = {
                VK_RBUTTON, VK_MBUTTON, VK_XBUTTON1, VK_XBUTTON2,
                VK_LMENU, VK_RMENU, VK_LSHIFT, VK_RSHIFT, VK_LCONTROL, VK_RCONTROL,
                VK_SPACE, VK_INSERT, VK_HOME, VK_END, VK_CAPITAL, VK_TAB,
                0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39,
                0x41,0x42,0x43,0x44,0x45,0x46,0x47,0x48,0x49,0x4A,0x4B,0x4C,0x4D,
                0x4E,0x4F,0x50,0x51,0x52,0x53,0x54,0x55,0x56,0x57,0x58,0x59,0x5A,
                VK_F1,VK_F2,VK_F3,VK_F4,VK_F5,VK_F6,VK_F7,VK_F8,VK_F9,VK_F10,VK_F11,VK_F12
            };
            for (int k : scan) {
                if (GetAsyncKeyState(k) & 1) {
                    binds[rebind_idx].vk = k;
                    rebind_idx = -1;
                    return;
                }
            }
        }
    }

    inline const char* ModeName(int mode) {
        switch (mode) { case 0: return "Activate"; case 1: return "Toggle"; case 2: return "Hold"; case 3: return "Activator"; default: return "Activate"; }
    }
    inline void ProcessKeys(bool menuOpen) {
        if (menuOpen) return;
        for (int i = 0; i < bind_count; i++) {
            if (binds[i].vk == 0 || !binds[i].toggle) continue;
            switch (binds[i].mode) {
            case 0:
                if (GetAsyncKeyState(binds[i].vk) & 1)
                    *binds[i].toggle = true;
                break;
            case 1:
                if (GetAsyncKeyState(binds[i].vk) & 1)
                    *binds[i].toggle = !*binds[i].toggle;
                break;
            case 2:
                *binds[i].toggle = (GetAsyncKeyState(binds[i].vk) & 0x8000) != 0;
                break;
            case 3:
                *binds[i].toggle = true;
                break;
            }
        }
    }

    inline bool IsKeyPressed(bool* toggle) {
        int idx = FindBind(toggle);
        if (idx < 0 || binds[idx].vk == 0) return false;
        return (GetAsyncKeyState(binds[idx].vk) & 0x8000) != 0;
    }
    inline void RenderContextMenu() {
        if (ctx_idx < 0 || ctx_idx >= bind_count) return;
        ImGui::PushStyleColor(ImGuiCol_PopupBg, ImVec4(0.05f, 0.06f, 0.09f, 0.95f));
        ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.2f, 0.23f, 0.28f, 0.6f));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8, 6));
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(6, 4));
        if (ImGui::BeginPopup("##hk_ctx")) {
            ImGui::TextColored(ImVec4(0.0f, 0.7f, 1.0f, 1.0f), "%s", binds[ctx_idx].name);
            ImGui::Separator();
            if (ImGui::Selectable("Bind key...", false)) { rebind_idx = ctx_idx; }
            ImGui::Separator();
            bool m0 = binds[ctx_idx].mode == 0, m1 = binds[ctx_idx].mode == 1, m2 = binds[ctx_idx].mode == 2;
            if (ImGui::Selectable("Activate", m0)) binds[ctx_idx].mode = 0;
            if (ImGui::Selectable("Toggle", m1)) binds[ctx_idx].mode = 1;
            if (ImGui::Selectable("Hold", m2)) binds[ctx_idx].mode = 2;
            if (binds[ctx_idx].vk != 0) {
                ImGui::Separator();
                if (ImGui::Selectable("Remove bind", false)) binds[ctx_idx].vk = 0;
            }
            ImGui::EndPopup();
        }
        ImGui::PopStyleVar(2);
        ImGui::PopStyleColor(2);
    }

    inline void DrawCategoryPanel(ImDrawList* dl, float px, float py, float panelW, float drawH, float alpha,
        const char* catName, int entryCount, const char** names, const int* vks)
    {
        ImGuiIO& io = ImGui::GetIO();
        int a_bg = (int)(185 * alpha), a_brd = (int)(100 * alpha);

        auto srv = Blur::GetSRV();
        if (srv && io.DisplaySize.x > 0 && alpha > 0.05f) {
            ImVec2 u0(px / io.DisplaySize.x, py / io.DisplaySize.y);
            ImVec2 u1((px + panelW) / io.DisplaySize.x, (py + drawH) / io.DisplaySize.y);
            dl->AddImageRounded((ImTextureID)srv,
                ImVec2(px, py), ImVec2(px + panelW, py + drawH),
                u0, u1, IM_COL32(255, 255, 255, (int)(255 * alpha)), 10.0f);
        }
        dl->AddRectFilled(ImVec2(px, py), ImVec2(px + panelW, py + drawH),
            IM_COL32(12, 12, 12, a_bg), 10.0f);
        dl->AddRect(ImVec2(px, py), ImVec2(px + panelW, py + drawH),
            IM_COL32(45, 45, 45, a_brd), 10.0f);

        ImVec2 tsz = ImGui::CalcTextSize(catName);
        dl->AddText(ImVec2(px + (panelW - tsz.x) * 0.5f, py + 5),
            IM_COL32(165, 211, 50, (int)(255 * alpha)), catName);
        dl->AddLine(ImVec2(px + 10, py + 28),
            ImVec2(px + panelW - 10, py + 28), IM_COL32(165, 211, 50, (int)(120 * alpha)));

        for (int i = 0; i < entryCount; i++) {
            float ey = py + 28 + 10 + i * 22.0f;
            if (ey + 20 > py + drawH) break;

            bool pressed = (vks[i] != 0) && (GetAsyncKeyState(vks[i]) & 0x8000);
            int name_a = pressed ? (int)(255 * alpha) : (int)(195 * alpha);
            int key_a = pressed ? (int)(240 * alpha) : (int)(160 * alpha);
            ImU32 nameCol = pressed ? IM_COL32(240, 245, 255, name_a) : IM_COL32(180, 185, 195, name_a);
            ImU32 keyCol = pressed ? IM_COL32(165, 211, 50, key_a) : IM_COL32(138, 143, 158, key_a);

            dl->AddText(ImVec2(px + 14, ey), nameCol, names[i]);
            const char* kn = VkName(vks[i]);
            if (kn) {
                char kb[32]; sprintf_s(kb, "[%s]", kn);
                ImVec2 ksz = ImGui::CalcTextSize(kb);
                dl->AddText(ImVec2(px + panelW - ksz.x - 14, ey), keyCol, kb);
            }
        }
    }

    inline void Render(bool menuOpen) {
        if (!Settings::show_hotkeys) { 
            for (int i = 0; i < cat_count; i++) {
                cat_anims[i].alpha = 0;
                cat_anims[i].height = 0;
            }
            return; 
        }

        ProcessRebind();
        ProcessKeys(menuOpen);
        const char* allCats[] = { CATEGORY_COMBAT, CATEGORY_VISUALS, CATEGORY_MISC };
        int numCats = 3;

        const char* catNames[8];
        int catCounts[8] = {};
        const char* catNamesList[8][32];
        int catVksList[8][32];
        int totalCats = 0;

        for (int c = 0; c < numCats; c++) {
            const char* catName = allCats[c];
            int count = 0;
            for (int i = 0; i < bind_count; i++) {
                if (binds[i].vk == 0) continue;
                if (strcmp(binds[i].category, catName) != 0) continue;
                catNamesList[totalCats][count] = binds[i].name;
                catVksList[totalCats][count] = binds[i].vk;
                count++;
                if (count >= 32) break;
            }
            if (count > 0) {
                catNames[totalCats] = catName;
                catCounts[totalCats] = count;
                totalCats++;
            }
        }

        if (totalCats == 0) { 
            for (int i = 0; i < cat_count; i++) {
                cat_anims[i].alpha = 0;
                cat_anims[i].height = 0;
            }
            return; 
        }

        ImGuiIO& io = ImGui::GetIO();
        float dt = io.DeltaTime;
        float panelW = 210.0f;
        float gap = 10.0f;

        if (!pos_init) { 
            pos = ImVec2(io.DisplaySize.x - panelW - 15, 45); 
            pos_init = true; 
        }
        for (int c = 0; c < totalCats; c++) {
            float targetH = 28.0f + catCounts[c] * 22.0f + 12.0f;
            cat_anims[c].height += (targetH - cat_anims[c].height) * dt * 12.0f;
            float targetA = 1.0f;
            cat_anims[c].alpha += (targetA - cat_anims[c].alpha) * dt * 10.0f;
        }
        for (int c = totalCats; c < 8; c++) {
            cat_anims[c].alpha = 0;
            cat_anims[c].height = 0;
        }
        cat_count = totalCats;
        memcpy(cat_names, catNames, sizeof(cat_names));

        if (menuOpen) {
            ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0, 0, 0, 0));
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0, 0, 0, 0));
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
            ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0);
            
            float totalH = 0;
            for (int c = 0; c < totalCats; c++) {
                totalH += cat_anims[c].height + (c > 0 ? gap : 0);
            }
            
            ImGui::SetNextWindowPos(pos, ImGuiCond_Once);
            ImGui::SetNextWindowSize(ImVec2(panelW, totalH));
            ImGui::Begin("##hk_drag", nullptr,
                ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse |
                ImGuiWindowFlags_NoBackground);
            pos = ImGui::GetWindowPos();
            
            float currentY = 0;
            for (int c = 0; c < totalCats; c++) {
                DrawCategoryPanel(ImGui::GetWindowDrawList(), pos.x, pos.y + currentY, 
                    panelW, cat_anims[c].height, cat_anims[c].alpha, 
                    catNames[c], catCounts[c], catNamesList[c], catVksList[c]);
                currentY += cat_anims[c].height + gap;
            }
            
            ImGui::End();
            ImGui::PopStyleVar(2);
            ImGui::PopStyleColor(2);
        } else {
            float currentY = pos.y;
            for (int c = 0; c < totalCats; c++) {
                DrawCategoryPanel(ImGui::GetBackgroundDrawList(), pos.x, currentY, 
                    panelW, cat_anims[c].height, cat_anims[c].alpha, 
                    catNames[c], catCounts[c], catNamesList[c], catVksList[c]);
                currentY += cat_anims[c].height + gap;
            }
            }
        }
    }

namespace Config
{
    inline std::filesystem::path config_dir;
    inline std::vector<std::string> config_names;
    inline int selected_index = -1;
    inline char name_buffer[64] = "default";
    inline std::string active_config;
    inline std::string status_message = "Ready";
    inline bool initialized = false;

    inline std::string Trim(const std::string& value) {
        size_t begin = 0;
        size_t end = value.size();
        while (begin < end && (value[begin] == ' ' || value[begin] == '\t' || value[begin] == '\r' || value[begin] == '\n'))
            ++begin;
        while (end > begin && (value[end - 1] == ' ' || value[end - 1] == '\t' || value[end - 1] == '\r' || value[end - 1] == '\n'))
            --end;
        return value.substr(begin, end - begin);
    }

    inline std::string NormalizeName(std::string name) {
        name = Trim(name);
        if (name.size() >= 4) {
            std::string ext = name.substr(name.size() - 4);
            if (_stricmp(ext.c_str(), ".cfg") == 0)
                name.resize(name.size() - 4);
        }

        std::string out;
        out.reserve(name.size());
        for (char ch : name) {
            bool ok = (ch >= 'A' && ch <= 'Z') ||
                      (ch >= 'a' && ch <= 'z') ||
                      (ch >= '0' && ch <= '9') ||
                      ch == ' ' || ch == '_' || ch == '-' || ch == '.';
            out.push_back(ok ? ch : '_');
        }

        while (!out.empty() && (out.front() == ' ' || out.front() == '.'))
            out.erase(out.begin());
        while (!out.empty() && (out.back() == ' ' || out.back() == '.'))
            out.pop_back();
        if (out.size() > 48)
            out.resize(48);
        while (!out.empty() && (out.back() == ' ' || out.back() == '.'))
            out.pop_back();
        if (out.empty())
            out = "default";
        return out;
    }

    inline void SetInputName(const std::string& name) {
        std::string normalized = NormalizeName(name);
        strcpy_s(name_buffer, normalized.c_str());
    }

    inline std::filesystem::path& GetConfigDirectory() {
        if (config_dir.empty()) {
            char appdata[MAX_PATH] = {};
            DWORD len = GetEnvironmentVariableA("APPDATA", appdata, MAX_PATH);
            if (len > 0 && len < MAX_PATH) {
                config_dir = std::filesystem::path(appdata) / "pizdopasta" / "configs";
            } else {
                std::error_code ec;
                config_dir = std::filesystem::current_path(ec);
                if (ec)
                    config_dir = ".";
                config_dir /= "pizdopasta";
                config_dir /= "configs";
            }
        }
        return config_dir;
    }

    inline std::string GetConfigDirectoryString() {
        return GetConfigDirectory().string();
    }

    inline std::filesystem::path GetConfigPath(const std::string& name) {
        return GetConfigDirectory() / (NormalizeName(name) + ".cfg");
    }

    inline void SetStatus(const std::string& text) {
        status_message = text;
    }

    inline bool OpenFolder() {
        std::error_code ec;
        std::filesystem::create_directories(GetConfigDirectory(), ec);

        const std::string folder = GetConfigDirectoryString();
        HINSTANCE result = ShellExecuteA(nullptr, "open", folder.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
        if ((INT_PTR)result <= 32) {
            SetStatus("Open folder failed");
            return false;
        }

        SetStatus("Opened config folder");
        return true;
    }

    inline int FindConfigIndex(const std::string& name) {
        for (int i = 0; i < (int)config_names.size(); i++) {
            if (_stricmp(config_names[i].c_str(), name.c_str()) == 0)
                return i;
        }
        return -1;
    }

    inline void SelectByIndex(int index) {
        if (index < 0 || index >= (int)config_names.size()) {
            selected_index = -1;
            return;
        }
        selected_index = index;
        SetInputName(config_names[index]);
    }

    inline void SelectByName(const std::string& name) {
        int idx = FindConfigIndex(name);
        if (idx >= 0) {
            SelectByIndex(idx);
        } else {
            selected_index = -1;
            SetInputName(name);
        }
    }

    inline bool ParseBool(const std::string& value, bool fallback) {
        if (value == "1" || _stricmp(value.c_str(), "true") == 0 || _stricmp(value.c_str(), "yes") == 0 || _stricmp(value.c_str(), "on") == 0)
            return true;
        if (value == "0" || _stricmp(value.c_str(), "false") == 0 || _stricmp(value.c_str(), "no") == 0 || _stricmp(value.c_str(), "off") == 0)
            return false;
        return fallback;
    }

    inline int ParseInt(const std::string& value, int fallback) {
        try {
            return std::stoi(value);
        } catch (...) {
            return fallback;
        }
    }

    inline float ParseFloat(const std::string& value, float fallback) {
        try {
            return std::stof(value);
        } catch (...) {
            return fallback;
        }
    }

    inline bool ParseFloat4(const std::string& value, float out[4]) {
        float a = 0.0f, b = 0.0f, c = 0.0f, d = 0.0f;
        if (sscanf_s(value.c_str(), "%f,%f,%f,%f", &a, &b, &c, &d) != 4)
            return false;
        out[0] = a;
        out[1] = b;
        out[2] = c;
        out[3] = d;
        return true;
    }

    inline void WriteSetting(std::ofstream& file, const char* key, bool value) {
        file << key << '=' << (value ? 1 : 0) << '\n';
    }

    inline void WriteSetting(std::ofstream& file, const char* key, int value) {
        file << key << '=' << value << '\n';
    }

    inline void WriteSetting(std::ofstream& file, const char* key, float value) {
        file << key << '=' << value << '\n';
    }

    inline void WriteSetting(std::ofstream& file, const char* key, const float* value) {
        file << key << '=' << value[0] << ',' << value[1] << ',' << value[2] << ',' << value[3] << '\n';
    }

    inline void ApplySetting(const std::unordered_map<std::string, std::string>& values, const char* key, bool& value) {
        auto it = values.find(key);
        if (it != values.end())
            value = ParseBool(it->second, value);
    }

    inline void ApplySetting(const std::unordered_map<std::string, std::string>& values, const char* key, int& value) {
        auto it = values.find(key);
        if (it != values.end())
            value = ParseInt(it->second, value);
    }

    inline void ApplySetting(const std::unordered_map<std::string, std::string>& values, const char* key, float& value) {
        auto it = values.find(key);
        if (it != values.end())
            value = ParseFloat(it->second, value);
    }

    inline void ApplySetting(const std::unordered_map<std::string, std::string>& values, const char* key, float* value) {
        auto it = values.find(key);
        if (it != values.end())
            ParseFloat4(it->second, value);
    }

    inline std::unordered_map<std::string, std::string> ReadKeyValues(const std::filesystem::path& path) {
        std::unordered_map<std::string, std::string> values;
        std::ifstream file(path);
        if (!file.is_open())
            return values;

        std::string line;
        while (std::getline(file, line)) {
            line = Trim(line);
            if (line.empty() || line[0] == '#')
                continue;
            if (line.size() > 1 && line[0] == '/' && line[1] == '/')
                continue;

            size_t eq = line.find('=');
            if (eq == std::string::npos)
                continue;

            std::string key = Trim(line.substr(0, eq));
            std::string value = Trim(line.substr(eq + 1));
            if (!key.empty())
                values[key] = value;
        }
        return values;
    }

    inline std::string GetSelectedName();

    inline void Refresh() {
        std::string keep_name = GetSelectedName();
        config_names.clear();

        std::error_code ec;
        std::filesystem::create_directories(GetConfigDirectory(), ec);

        if (std::filesystem::exists(GetConfigDirectory(), ec)) {
            for (std::filesystem::directory_iterator it(GetConfigDirectory(), std::filesystem::directory_options::skip_permission_denied, ec), end; it != end && !ec; it.increment(ec)) {
                if (!it->is_regular_file(ec))
                    continue;

                std::filesystem::path path = it->path();
                if (_stricmp(path.extension().string().c_str(), ".cfg") != 0)
                    continue;
                config_names.push_back(path.stem().string());
            }
        }

        std::sort(config_names.begin(), config_names.end(), [](const std::string& a, const std::string& b) {
            return _stricmp(a.c_str(), b.c_str()) < 0;
        });

        if (!keep_name.empty()) {
            int idx = FindConfigIndex(keep_name);
            if (idx >= 0) {
                selected_index = idx;
                return;
            }
        }

        if (selected_index >= 0 && selected_index < (int)config_names.size())
            return;

        selected_index = config_names.empty() ? -1 : 0;
    }

    inline std::string GetSelectedName() {
        if (selected_index >= 0 && selected_index < (int)config_names.size())
            return config_names[selected_index];
        return {};
    }

    inline void SaveSettings(std::ofstream& file) {
#define CFG_BOOL(field) WriteSetting(file, "settings." #field, Settings::field)
#define CFG_INT(field) WriteSetting(file, "settings." #field, Settings::field)
#define CFG_FLOAT(field) WriteSetting(file, "settings." #field, Settings::field)
#define CFG_FLOAT4(field) WriteSetting(file, "settings." #field, Settings::field)
        CFG_BOOL(esp_enabled);
        CFG_BOOL(esp_box);
        CFG_INT(esp_box_style);
        CFG_BOOL(esp_health);
        CFG_BOOL(esp_armor);
        CFG_BOOL(esp_name);
        CFG_BOOL(esp_distance);
        CFG_BOOL(esp_skeleton);
        CFG_BOOL(esp_head_dot);
        CFG_BOOL(esp_snaplines);
        CFG_BOOL(esp_team);
        CFG_FLOAT4(esp_color_enemy);
        CFG_FLOAT4(esp_color_team);
        CFG_FLOAT4(esp_color_skeleton);
        CFG_BOOL(esp_glow);
        CFG_FLOAT4(esp_glow_color_enemy);
        CFG_FLOAT4(esp_glow_color_team);
        CFG_BOOL(aim_enabled);
        CFG_INT(aim_key);
        CFG_FLOAT(aim_fov);
        CFG_FLOAT(aim_smooth);
        CFG_INT(aim_bone);
        CFG_BOOL(aim_team);
        CFG_BOOL(aim_visible_only);
        CFG_BOOL(aim_fov_circle);
        CFG_BOOL(trigger_enabled);
        CFG_INT(trigger_key);
        CFG_INT(trigger_delay);
        CFG_BOOL(trigger_team);
        CFG_BOOL(trigger_prospread);
        CFG_BOOL(bhop_enabled);
        CFG_BOOL(autostrafe);
        CFG_BOOL(noflash_enabled);
        CFG_FLOAT(noflash_alpha);
        CFG_BOOL(radar_hack);
        CFG_BOOL(warning_enabled);
        CFG_BOOL(thirdperson);
        CFG_INT(thirdperson_dist);
        CFG_BOOL(auto_accept);
        CFG_BOOL(auto_buy);
        CFG_INT(auto_buy_preset);
        CFG_BOOL(watermark);
        CFG_BOOL(show_hotkeys);
        CFG_BOOL(show_bombtimer);
        CFG_BOOL(spectator_enemy);
        CFG_INT(spectator_mode);
        CFG_BOOL(spectator_only_enemies);
        CFG_BOOL(crosshair_enabled);
        CFG_INT(crosshair_size);
        CFG_INT(crosshair_gap);
        CFG_INT(crosshair_thickness);
        CFG_FLOAT4(crosshair_color);
        CFG_BOOL(fov_changer);
        CFG_INT(fov_value);
        CFG_BOOL(no_visual_recoil);
        CFG_BOOL(custom_hud);
        CFG_BOOL(grenade_tracer);
        CFG_FLOAT(grenade_tracer_thickness);
        CFG_FLOAT(grenade_trail_duration);
        CFG_BOOL(grenade_prediction);
        CFG_BOOL(bullet_tracer);
        CFG_FLOAT(bullet_tracer_duration);
        CFG_FLOAT4(bullet_tracer_color);
        CFG_BOOL(spectator_list);
        CFG_BOOL(rain_enabled);
        CFG_INT(rain_intensity);
        CFG_FLOAT(rain_speed);
        CFG_FLOAT(rain_wind);
        CFG_FLOAT(rain_alpha);
        CFG_BOOL(world_modulation);
        CFG_FLOAT4(world_color);
        CFG_INT(world_mod_mode);
        CFG_BOOL(trashtalk_enabled);
#undef CFG_BOOL
#undef CFG_INT
#undef CFG_FLOAT
#undef CFG_FLOAT4
    }

    inline void LoadSettings(const std::unordered_map<std::string, std::string>& values) {
#define CFG_BOOL(field) ApplySetting(values, "settings." #field, Settings::field)
#define CFG_INT(field) ApplySetting(values, "settings." #field, Settings::field)
#define CFG_FLOAT(field) ApplySetting(values, "settings." #field, Settings::field)
#define CFG_FLOAT4(field) ApplySetting(values, "settings." #field, Settings::field)
        CFG_BOOL(esp_enabled);
        CFG_BOOL(esp_box);
        CFG_INT(esp_box_style);
        CFG_BOOL(esp_health);
        CFG_BOOL(esp_armor);
        CFG_BOOL(esp_name);
        CFG_BOOL(esp_distance);
        CFG_BOOL(esp_skeleton);
        CFG_BOOL(esp_head_dot);
        CFG_BOOL(esp_snaplines);
        CFG_BOOL(esp_team);
        CFG_FLOAT4(esp_color_enemy);
        CFG_FLOAT4(esp_color_team);
        CFG_FLOAT4(esp_color_skeleton);
        CFG_BOOL(esp_glow);
        CFG_FLOAT4(esp_glow_color_enemy);
        CFG_FLOAT4(esp_glow_color_team);
        CFG_BOOL(aim_enabled);
        CFG_INT(aim_key);
        CFG_FLOAT(aim_fov);
        CFG_FLOAT(aim_smooth);
        CFG_INT(aim_bone);
        CFG_BOOL(aim_team);
        CFG_BOOL(aim_visible_only);
        CFG_BOOL(aim_fov_circle);
        CFG_BOOL(trigger_enabled);
        CFG_INT(trigger_key);
        CFG_INT(trigger_delay);
        CFG_BOOL(trigger_team);
        CFG_BOOL(trigger_prospread);
        CFG_BOOL(bhop_enabled);
        CFG_BOOL(autostrafe);
        CFG_BOOL(noflash_enabled);
        CFG_FLOAT(noflash_alpha);
        CFG_BOOL(radar_hack);
        CFG_BOOL(warning_enabled);
        CFG_BOOL(thirdperson);
        CFG_INT(thirdperson_dist);
        CFG_BOOL(auto_accept);
        CFG_BOOL(auto_buy);
        CFG_INT(auto_buy_preset);
        CFG_BOOL(watermark);
        CFG_BOOL(show_hotkeys);
        CFG_BOOL(show_bombtimer);
        CFG_BOOL(spectator_enemy);
        CFG_INT(spectator_mode);
        CFG_BOOL(spectator_only_enemies);
        CFG_BOOL(crosshair_enabled);
        CFG_INT(crosshair_size);
        CFG_INT(crosshair_gap);
        CFG_INT(crosshair_thickness);
        CFG_FLOAT4(crosshair_color);
        CFG_BOOL(fov_changer);
        CFG_INT(fov_value);
        CFG_BOOL(no_visual_recoil);
        CFG_BOOL(custom_hud);
        CFG_BOOL(grenade_tracer);
        CFG_FLOAT(grenade_tracer_thickness);
        CFG_FLOAT(grenade_trail_duration);
        CFG_BOOL(grenade_prediction);
        CFG_BOOL(bullet_tracer);
        CFG_FLOAT(bullet_tracer_duration);
        CFG_FLOAT4(bullet_tracer_color);
        CFG_BOOL(spectator_list);
        CFG_BOOL(rain_enabled);
        CFG_INT(rain_intensity);
        CFG_FLOAT(rain_speed);
        CFG_FLOAT(rain_wind);
        CFG_FLOAT(rain_alpha);
        CFG_BOOL(world_modulation);
        CFG_FLOAT4(world_color);
        CFG_INT(world_mod_mode);
        CFG_BOOL(trashtalk_enabled);
#undef CFG_BOOL
#undef CFG_INT
#undef CFG_FLOAT
#undef CFG_FLOAT4
    }

    inline void SaveHotkeys(std::ofstream& file) {
        WriteSetting(file, "hotkeys.casino_mode", Hotkeys::casino_mode);
        for (int i = 0; i < Hotkeys::bind_count; i++) {
            if (!Hotkeys::binds[i].name)
                continue;

            std::string prefix = std::string("hotkey.") + Hotkeys::binds[i].name;
            WriteSetting(file, (prefix + ".vk").c_str(), Hotkeys::binds[i].vk);
            WriteSetting(file, (prefix + ".mode").c_str(), Hotkeys::binds[i].mode);
        }
    }

    inline void LoadHotkeys(const std::unordered_map<std::string, std::string>& values) {
        ApplySetting(values, "hotkeys.casino_mode", Hotkeys::casino_mode);
        for (int i = 0; i < Hotkeys::bind_count; i++) {
            if (!Hotkeys::binds[i].name)
                continue;

            std::string prefix = std::string("hotkey.") + Hotkeys::binds[i].name;
            ApplySetting(values, (prefix + ".vk").c_str(), Hotkeys::binds[i].vk);
            ApplySetting(values, (prefix + ".mode").c_str(), Hotkeys::binds[i].mode);
        }
        Hotkeys::rebind_idx = -1;
        Hotkeys::waiting_for_lmb = false;
    }

    inline bool Save(const std::string& raw_name) {
        std::string name = NormalizeName(raw_name);
        std::error_code ec;
        std::filesystem::create_directories(GetConfigDirectory(), ec);

        std::ofstream file(GetConfigPath(name), std::ios::out | std::ios::trunc);
        if (!file.is_open()) {
            SetStatus("Save failed: " + name);
            return false;
        }

        file << "# PIZDOPASTA config\n";
        file << "version=1\n";
        SaveSettings(file);
        SaveHotkeys(file);
        file.close();

        active_config = name;
        SetInputName(name);
        Refresh();
        SelectByName(name);
        SetStatus("Saved: " + name);
        return true;
    }

    inline bool Load(const std::string& raw_name) {
        std::string name = NormalizeName(raw_name);
        std::filesystem::path path = GetConfigPath(name);
        std::error_code ec;
        if (!std::filesystem::exists(path, ec)) {
            SetStatus("Load failed: " + name);
            Refresh();
            return false;
        }

        std::unordered_map<std::string, std::string> values = ReadKeyValues(path);
        LoadSettings(values);
        LoadHotkeys(values);
        active_config = name;
        SetInputName(name);
        Refresh();
        SelectByName(name);
        SetStatus("Loaded: " + name);
        return true;
    }

    inline bool Delete(const std::string& raw_name) {
        std::string name = NormalizeName(raw_name);
        std::filesystem::path path = GetConfigPath(name);
        std::error_code ec;
        bool existed = std::filesystem::exists(path, ec);
        bool removed = false;
        if (existed) {
            std::filesystem::remove(path, ec);
            removed = !ec;
        }

        int keep_index = selected_index;
        if (removed && _stricmp(active_config.c_str(), name.c_str()) == 0)
            active_config.clear();
        Refresh();

        if (!config_names.empty()) {
            if (keep_index < 0 || keep_index >= (int)config_names.size())
                keep_index = 0;
            SelectByIndex(keep_index);
        } else {
            selected_index = -1;
            SetInputName("default");
        }

        if (removed) {
            SetStatus("Deleted: " + name);
            return true;
        }

        SetStatus("Delete failed: " + name);
        return false;
    }

    inline void Initialize() {
        config_dir.clear();
        GetConfigDirectory();
        initialized = true;
        Refresh();
    }
}

namespace ESP
{
    inline ImU32 ColorFromFloat(float c[4]) {
        return IM_COL32((int)(c[0] * 255), (int)(c[1] * 255), (int)(c[2] * 255), (int)(c[3] * 255));
    }

    inline void DrawCornerBox(ImDrawList* dl, float x, float y, float w, float h, ImU32 col, float t = 1.5f) {
        float l = w * 0.25f;
        dl->AddLine(ImVec2(x, y), ImVec2(x + l, y), col, t);
        dl->AddLine(ImVec2(x, y), ImVec2(x, y + l), col, t);
        dl->AddLine(ImVec2(x + w, y), ImVec2(x + w - l, y), col, t);
        dl->AddLine(ImVec2(x + w, y), ImVec2(x + w, y + l), col, t);
        dl->AddLine(ImVec2(x, y + h), ImVec2(x + l, y + h), col, t);
        dl->AddLine(ImVec2(x, y + h), ImVec2(x, y + h - l), col, t);
        dl->AddLine(ImVec2(x + w, y + h), ImVec2(x + w - l, y + h), col, t);
        dl->AddLine(ImVec2(x + w, y + h), ImVec2(x + w, y + h - l), col, t);
    }

    inline void Render() {
        if (!Settings::esp_enabled || !CS2::initialized) return;

        ImDrawList* dl = ImGui::GetBackgroundDrawList();
        ImGuiIO& io = ImGui::GetIO();
        float sw = io.DisplaySize.x, sh = io.DisplaySize.y;

        for (int i = 0; i < Game::playerCount; i++) {
            Game::PlayerInfo& p = Game::players[i];
            if (!p.valid || !p.alive) continue;
            if (!Settings::esp_team && p.team == Game::localTeam) continue;

            bool isEnemy = (p.team != Game::localTeam);
            ImU32 col = isEnemy ? ColorFromFloat(Settings::esp_color_enemy) : ColorFromFloat(Settings::esp_color_team);

            Vec2 headScreen, feetScreen;
            Vec3 headTop = p.headPos;
            headTop.z += 8.0f;

            if (!Game::WorldToScreen(headTop, headScreen, sw, sh)) continue;
            if (!Game::WorldToScreen(p.origin, feetScreen, sw, sh)) continue;

            float boxH = feetScreen.y - headScreen.y;
            if (boxH < 4.0f) continue;
            float boxW = boxH * 0.45f;
            float boxX = headScreen.x - boxW * 0.5f;
            float boxY = headScreen.y;
            if (Settings::esp_box) {
                ImU32 outline = IM_COL32(0, 0, 0, 180);
                if (Settings::esp_box_style == 0) {
                    dl->AddRect(ImVec2(boxX - 1, boxY - 1), ImVec2(boxX + boxW + 1, boxY + boxH + 1), outline, 0, 0, 2.5f);
                    dl->AddRect(ImVec2(boxX, boxY), ImVec2(boxX + boxW, boxY + boxH), col, 0, 0, 1.5f);
                } else {
                    DrawCornerBox(dl, boxX, boxY, boxW, boxH, outline, 2.5f);
                    DrawCornerBox(dl, boxX, boxY, boxW, boxH, col, 1.5f);
                }
            }
            if (Settings::esp_health) {
                float barW = 3.0f;
                float barX = boxX - barW - 3;
                float hpFrac = (float)p.health / 100.0f;
                if (hpFrac > 1.0f) hpFrac = 1.0f;
                float filledH = boxH * hpFrac;

                dl->AddRectFilled(ImVec2(barX - 1, boxY - 1), ImVec2(barX + barW + 1, boxY + boxH + 1), IM_COL32(0, 0, 0, 180));
                ImU32 hpCol = IM_COL32(
                    (int)((1.0f - hpFrac) * 255),
                    (int)(hpFrac * 255), 0, 255);
                dl->AddRectFilled(ImVec2(barX, boxY + boxH - filledH), ImVec2(barX + barW, boxY + boxH), hpCol);

                if (p.health < 100) {
                    char hpStr[8]; sprintf_s(hpStr, "%d", p.health);
                    ImVec2 ts = ImGui::CalcTextSize(hpStr);
                    dl->AddText(ImVec2(barX - ts.x - 2, boxY + boxH - filledH - ts.y * 0.5f), IM_COL32(255, 255, 255, 220), hpStr);
                }
            }
            if (Settings::esp_armor && p.armor > 0) {
                float barW = 3.0f;
                float barX = boxX + boxW + 3;
                float armorFrac = (float)p.armor / 100.0f;
                float filledH = boxH * armorFrac;
                dl->AddRectFilled(ImVec2(barX - 1, boxY - 1), ImVec2(barX + barW + 1, boxY + boxH + 1), IM_COL32(0, 0, 0, 180));
                dl->AddRectFilled(ImVec2(barX, boxY + boxH - filledH), ImVec2(barX + barW, boxY + boxH), IM_COL32(80, 140, 255, 255));
            }
            if (Settings::esp_name) {
                ImVec2 ts = ImGui::CalcTextSize(p.name);
                float nx = headScreen.x - ts.x * 0.5f;
                dl->AddText(ImVec2(nx + 1, boxY - ts.y - 3), IM_COL32(0, 0, 0, 200), p.name);
                dl->AddText(ImVec2(nx, boxY - ts.y - 4), IM_COL32(255, 255, 255, 240), p.name);
            }
            if (Settings::esp_distance) {
                float dist = Game::Distance(Game::localPos, p.origin) / 100.0f;
                char dStr[16]; sprintf_s(dStr, "%.0fm", dist);
                ImVec2 ts = ImGui::CalcTextSize(dStr);
                dl->AddText(ImVec2(headScreen.x - ts.x * 0.5f, boxY + boxH + 2), IM_COL32(200, 200, 200, 200), dStr);
            }
            if (Settings::esp_head_dot) {
                Vec2 hScreen;
                if (Game::WorldToScreen(p.headPos, hScreen, sw, sh)) {
                    dl->AddCircleFilled(ImVec2(hScreen.x, hScreen.y), 3.0f, col);
                }
            }
            if (Settings::esp_skeleton) {
                ImU32 skelCol = ColorFromFloat(Settings::esp_color_skeleton);
                struct BoneConnection { int from, to; };
                BoneConnection conns[] = {
                    { Game::BONE_HEAD, Game::BONE_NECK },
                    { Game::BONE_NECK, Game::BONE_CHEST },
                    { Game::BONE_CHEST, Game::BONE_PELVIS },
                    { Game::BONE_CHEST, Game::BONE_LEFT_SHOULDER },
                    { Game::BONE_CHEST, Game::BONE_RIGHT_SHOULDER },
                    { Game::BONE_LEFT_SHOULDER, Game::BONE_LEFT_HAND },
                    { Game::BONE_RIGHT_SHOULDER, Game::BONE_RIGHT_HAND },
                    { Game::BONE_PELVIS, Game::BONE_LEFT_KNEE },
                    { Game::BONE_PELVIS, Game::BONE_RIGHT_KNEE },
                    { Game::BONE_LEFT_KNEE, Game::BONE_LEFT_FOOT },
                    { Game::BONE_RIGHT_KNEE, Game::BONE_RIGHT_FOOT },
                };
                for (auto& c : conns) {
                    Vec3 from = Game::GetBonePosition(p.pawn, c.from);
                    Vec3 to = Game::GetBonePosition(p.pawn, c.to);
                    Vec2 s1, s2;
                    if (Game::WorldToScreen(from, s1, sw, sh) && Game::WorldToScreen(to, s2, sw, sh)) {
                        dl->AddLine(ImVec2(s1.x, s1.y), ImVec2(s2.x, s2.y), IM_COL32(0, 0, 0, 150), 3.0f);
                        dl->AddLine(ImVec2(s1.x, s1.y), ImVec2(s2.x, s2.y), skelCol, 1.5f);
                    }
                }
            }
            if (Settings::esp_snaplines) {
                dl->AddLine(ImVec2(sw * 0.5f, sh), ImVec2(feetScreen.x, feetScreen.y), col, 1.0f);
            }
        }
    }
}

namespace Aimbot
{
    inline int targetIndex = -1;

    inline void Run() {
        if (!Settings::aim_enabled || !CS2::initialized) return;
        
        int bind_idx = Hotkeys::FindBind(&Settings::aim_enabled);
        if (bind_idx >= 0 && Hotkeys::binds[bind_idx].vk != 0) {
            if (!(GetAsyncKeyState(Hotkeys::binds[bind_idx].vk) & 0x8000)) {
                targetIndex = -1;
                return;
            }
        } else if (Settings::aim_key != 0) {
            if (Settings::aim_key != 0 && !(GetAsyncKeyState(Settings::aim_key) & 0x8000)) {
                targetIndex = -1;
                return;
            }
        }

        ImGuiIO& io = ImGui::GetIO();
        float sw = io.DisplaySize.x, sh = io.DisplaySize.y;

        uintptr_t localPawn = CS2::GetLocalPlayerPawn();
        if (!localPawn) return;

        Vec3 eyePos = Game::localPos;
        int flags = CS2::Read<int>(localPawn + Game::entity::m_fFlags);
        bool isCrouching = (flags & 0x4) != 0;
        eyePos.z += isCrouching ? 46.0f : 64.0f;

        Vec3 viewAngles = Game::localViewAngles;
        Vec3 aimPunch = Game::localAimPunch;
        float bestFov = Settings::aim_fov;
        int bestIdx = -1;
        Vec3 bestAngle = {};

        for (int i = 0; i < Game::playerCount; i++) {
            Game::PlayerInfo& p = Game::players[i];
            if (!p.valid || !p.alive) continue;
            if (!Settings::aim_team && p.team == Game::localTeam) continue;

            Vec3 targetBone;
            switch (Settings::aim_bone) {
            case Game::BONE_HEAD:   targetBone = Game::GetBonePosition(p.pawn, Game::BONE_HEAD); break;
            case Game::BONE_NECK:   targetBone = Game::GetBonePosition(p.pawn, Game::BONE_NECK); break;
            case Game::BONE_CHEST:  targetBone = Game::GetBonePosition(p.pawn, Game::BONE_CHEST); break;
            default:                targetBone = Game::GetBonePosition(p.pawn, Game::BONE_HEAD); break;
            }

            if (targetBone.x == 0 && targetBone.y == 0 && targetBone.z == 0) continue;

            Vec3 angle = Game::CalcAngle(eyePos, targetBone);
            float fov = Game::GetFov(viewAngles, angle);

            if (fov < bestFov) {
                bestFov = fov;
                bestIdx = i;
                bestAngle = angle;
            }
        }

        targetIndex = bestIdx;

        if (bestIdx >= 0) {
            Vec3 finalAngle = bestAngle;
            float smooth = Settings::aim_smooth;
            if (smooth < 1.0f) smooth = 1.0f;

            Vec3 delta;
            delta.x = finalAngle.x - viewAngles.x;
            delta.y = finalAngle.y - viewAngles.y;
            delta.z = 0;
            while (delta.y > 180.0f) delta.y -= 360.0f;
            while (delta.y < -180.0f) delta.y += 360.0f;

            Vec3 newAngle;
            newAngle.x = viewAngles.x + delta.x / smooth;
            newAngle.y = viewAngles.y + delta.y / smooth;
            newAngle.z = 0;
            if (newAngle.x > 89.0f) newAngle.x = 89.0f;
            if (newAngle.x < -89.0f) newAngle.x = -89.0f;
            while (newAngle.y > 180.0f) newAngle.y -= 360.0f;
            while (newAngle.y < -180.0f) newAngle.y += 360.0f;
            if (CS2::clientBase) {
                CS2::Write<Vec3>(CS2::clientBase + CS2::offsets::dwViewAngles, newAngle);
            }
        }
    }

    inline void DrawFOVCircle() {
        if (!Settings::aim_enabled || !Settings::aim_fov_circle) return;
        ImGuiIO& io = ImGui::GetIO();
        ImDrawList* dl = ImGui::GetBackgroundDrawList();

        float radius = tanf(Settings::aim_fov * 3.14159265f / 180.0f) * (io.DisplaySize.x * 0.5f);
        ImVec2 center(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f);
        dl->AddCircle(center, radius, IM_COL32(255, 255, 255, 60), 64, 1.0f);
    }
}

namespace Triggerbot
{
    inline DWORD lastShot = 0;

    inline void Run() {
        if (!Settings::trigger_enabled || !CS2::initialized) return;

        int bind_idx = Hotkeys::FindBind(&Settings::trigger_enabled);
        if (bind_idx >= 0 && Hotkeys::binds[bind_idx].vk != 0) {
            if (!(GetAsyncKeyState(Hotkeys::binds[bind_idx].vk) & 0x8000)) return;
        } else if (Settings::trigger_key != 0) {
            if (!(GetAsyncKeyState(Settings::trigger_key) & 0x8000)) return;
        }

        uintptr_t localPawn = CS2::GetLocalPlayerPawn();
        if (!localPawn) return;

        bool shouldShoot = false;

        if (Settings::trigger_prospread) {
            shouldShoot = Spread::TriggerSpreadProSpread();
        } else {
            int entIndex = CS2::Read<int>(localPawn + Game::pawn::m_iIDEntIndex);
            if (entIndex <= 0) return;
            uintptr_t targetEnt = CS2::GetEntityByIndex(entIndex);
            if (!targetEnt) return;
            int entHealth = CS2::Read<int>(targetEnt + Game::entity::m_iHealth);
            if (entHealth <= 0) return;
            int entTeam = (int)CS2::Read<uint8_t>(targetEnt + Game::entity::m_iTeamNum);
            if (!Settings::trigger_team && entTeam == Game::localTeam) return;
            shouldShoot = true;
        }

        if (!shouldShoot) return;

        DWORD now = GetTickCount();
        if (now - lastShot < (DWORD)Settings::trigger_delay) return;
        lastShot = now;

        if (CS2::clientBase)
            CS2::Write<int>(CS2::clientBase + Game::buttons::dwForceAttack, 65537);

        Sleep(1);

        if (CS2::clientBase)
            CS2::Write<int>(CS2::clientBase + Game::buttons::dwForceAttack, 256);
    }
}

namespace BunnyHop
{
    inline float NormalizeYaw(float y) {
        while (y > 180.0f) y -= 360.0f;
        while (y < -180.0f) y += 360.0f;
        return y;
    }

    inline void Run() {
        if (!Settings::bhop_enabled || !CS2::initialized) return;
        if (!(GetAsyncKeyState(VK_SPACE) & 0x8000)) return;
        if (!CS2::clientBase) return;

        uintptr_t localPawn = CS2::GetLocalPlayerPawn();
        if (!localPawn) return;
        uintptr_t moveSvc = CS2::Read<uintptr_t>(localPawn + Game::pawn::m_pMovementServices);
        if (moveSvc) {
            CS2::Write<float>(moveSvc + Game::movement::m_flStamina, 0.0f);
        }

        int flags = CS2::Read<int>(localPawn + Game::entity::m_fFlags);
        bool onGround = (flags & (1 << 0)) != 0;

        if (onGround) {
            CS2::Write<int>(CS2::clientBase + Game::buttons::dwForceJump, 65537);
        } else {
            CS2::Write<int>(CS2::clientBase + Game::buttons::dwForceJump, 256);

            if (Settings::autostrafe) {
                float curYaw = Game::localViewAngles.y;

                Vec3 vel = CS2::Read<Vec3>(localPawn + Game::entity::m_vecAbsVelocity);
                float speed = sqrtf(vel.x * vel.x + vel.y * vel.y);

                if (speed > 30.0f) {
                    float velAngle = atan2f(vel.y, vel.x) * (180.0f / 3.14159265f);
                    float diff = NormalizeYaw(velAngle - curYaw);

                    if (diff > 2.0f) {
                        CS2::Write<int>(CS2::clientBase + Game::buttons::dwForceLeft, 65537);
                        CS2::Write<int>(CS2::clientBase + Game::buttons::dwForceRight, 256);
                    } else if (diff < -2.0f) {
                        CS2::Write<int>(CS2::clientBase + Game::buttons::dwForceRight, 65537);
                        CS2::Write<int>(CS2::clientBase + Game::buttons::dwForceLeft, 256);
                    } else {
                        CS2::Write<int>(CS2::clientBase + Game::buttons::dwForceLeft, 256);
                        CS2::Write<int>(CS2::clientBase + Game::buttons::dwForceRight, 256);
                    }
                }
            }
        }
    }
}

namespace API
{
    namespace ApiData
    {
        inline bool TryParseFloat(const std::string& s, float& out)
        {
            try {
                size_t idx = 0;
                out = std::stof(s, &idx);
                return idx > 0;
            }
            catch (...) {
                return false;
            }
        }

        inline bool TryParseInt(const std::string& s, int& out)
        {
            try {
                size_t idx = 0;
                out = std::stoi(s, &idx);
                return idx > 0;
            }
            catch (...) {
                return false;
            }
        }
    }

    inline std::string HttpGet(const char* url)
    {
        HINTERNET hInternet = InternetOpenA("Mozilla/5.0", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
        if (!hInternet) return "";

        HINTERNET hConnect = InternetOpenUrlA(hInternet, url, NULL, 0, INTERNET_FLAG_RELOAD, 0);
        if (!hConnect)
        {
            InternetCloseHandle(hInternet);
            return "";
        }

        std::string result;
        char buffer[1024];
        DWORD bytesRead = 0;
        while (InternetReadFile(hConnect, buffer, sizeof(buffer) - 1, &bytesRead) && bytesRead > 0)
        {
            buffer[bytesRead] = 0;
            result += buffer;
        }

        InternetCloseHandle(hConnect);
        InternetCloseHandle(hInternet);
        return result;
    }

    inline void UpdateBitcoinPrice()
    {
        static time_t lastUpdate = 0;
        time_t now = time(nullptr);
        if (now - lastUpdate < 60) return;
        lastUpdate = now;

        std::string response = HttpGet("https://api.coingecko.com/api/v3/simple/price?ids=bitcoin&vs_currencies=usd");
        if (response.empty())
        {
            response = HttpGet("https://api.binance.com/api/v3/ticker/price?symbol=BTCUSDT");
            if (!response.empty())
            {
                size_t pos = response.find("\"price\":\"");
                if (pos != std::string::npos)
                {
                    pos += 9;
                    size_t endPos = response.find("\"", pos);
                    if (endPos != std::string::npos)
                    {
                        std::string priceStr = response.substr(pos, endPos - pos);
                        float parsedPrice = 0.0f;
                        if (ApiData::TryParseFloat(priceStr, parsedPrice)) {
                            Settings::bitcoin_price = parsedPrice;
                            time_t t = time(nullptr);
                            tm timeInfo{};
                            char timeStr[64];
                            localtime_s(&timeInfo, &t);
                            strftime(timeStr, sizeof(timeStr), "%H:%M:%S", &timeInfo);
                            Settings::bitcoin_last_update = timeStr;
                        }
                    }
                }
            }
        }
        else
        {
            size_t pos = response.find("\"usd\":");
            if (pos != std::string::npos)
            {
                pos += 6;
                size_t endPos = response.find("}", pos);
                if (endPos != std::string::npos)
                {
                    std::string priceStr = response.substr(pos, endPos - pos);
                    float parsedPrice = 0.0f;
                    if (ApiData::TryParseFloat(priceStr, parsedPrice)) {
                        Settings::bitcoin_price = parsedPrice;
                        time_t t = time(nullptr);
                        tm timeInfo{};
                        char timeStr[64];
                        localtime_s(&timeInfo, &t);
                        strftime(timeStr, sizeof(timeStr), "%H:%M:%S", &timeInfo);
                        Settings::bitcoin_last_update = timeStr;
                    }
                }
            }
        }
    }

    inline void UpdateNigeriaWeather()
    {
        static time_t lastUpdate = 0;
        time_t now = time(nullptr);
        if (now - lastUpdate < 300) return;
        lastUpdate = now;

        std::string response = HttpGet("https://api.open-meteo.com/v1/forecast?latitude=9.0820&longitude=8.6753&current_weather=true");
        if (!response.empty())
        {
            size_t tempPos = response.find("\"temperature\":");
            if (tempPos != std::string::npos)
            {
                tempPos += 14;
                size_t endPos = response.find(",", tempPos);
                if (endPos != std::string::npos)
                {
                    std::string tempStr = response.substr(tempPos, endPos - tempPos);
                    try {
                        Settings::nigeria_temp = std::stof(tempStr);
                    } catch (...) {
                        Settings::nigeria_temp = 0.0f;
                    }
                }
            }

            size_t codePos = response.find("\"weathercode\":");
            if (codePos != std::string::npos)
            {
                codePos += 14;
                size_t endPos = response.find("}", codePos);
                if (endPos != std::string::npos)
                {
                    try {
                        int code = std::stoi(response.substr(codePos, endPos - codePos));
                        switch(code)
                        {
                            case 0: Settings::nigeria_condition = "Clear"; break;
                            case 1: case 2: case 3: Settings::nigeria_condition = "Partly Cloudy"; break;
                            case 45: case 48: Settings::nigeria_condition = "Foggy"; break;
                            case 51: case 53: case 55: Settings::nigeria_condition = "Drizzle"; break;
                            case 61: case 63: case 65: Settings::nigeria_condition = "Rain"; break;
                            case 71: case 73: case 75: Settings::nigeria_condition = "Snow"; break;
                            case 80: case 81: case 82: Settings::nigeria_condition = "Showers"; break;
                            case 95: case 96: case 99: Settings::nigeria_condition = "Thunderstorm"; break;
                            default: Settings::nigeria_condition = "Unknown";
                        }
                    } catch (...) {
                        Settings::nigeria_condition = "Unknown";
                    }
                }
            }

            time_t t = time(nullptr);
            struct tm timeInfo;
            localtime_s(&timeInfo, &t);
            char timeStr[64];
            strftime(timeStr, sizeof(timeStr), "%H:%M:%S", &timeInfo);
            Settings::nigeria_last_update = timeStr;
        }
    }

    inline void UpdateAll()
    {
        UpdateBitcoinPrice();
        UpdateNigeriaWeather();
    }
}

namespace Misc
{
    inline Vec3 oldPunch = {};

    inline void NoFlash() {
        if (!Settings::noflash_enabled || !CS2::initialized) return;
        uintptr_t localPawn = CS2::GetLocalPlayerPawn();
        if (!localPawn) return;
        CS2::Write<float>(localPawn + Game::pawn::m_flFlashMaxAlpha, Settings::noflash_alpha);
    }

    inline void RadarHack() {
        if (!Settings::radar_hack || !CS2::initialized) return;
        for (int i = 0; i < Game::playerCount; i++) {
            Game::PlayerInfo& p = Game::players[i];
            if (!p.valid || !p.alive) continue;
            if (p.team == Game::localTeam) continue;
            CS2::Write<bool>(p.pawn + Game::pawn::m_entitySpottedState + Game::pawn::m_bSpotted, true);
        }
    }

    inline void NoVisualRecoil() {
        if (!Settings::no_visual_recoil || !CS2::initialized) return;
        uintptr_t localPawn = CS2::GetLocalPlayerPawn();
        if (!localPawn) return;
        uintptr_t cameraSvc = CS2::Read<uintptr_t>(localPawn + Game::pawn::m_pCameraServices);
        if (!cameraSvc) return;
        Vec3 zero = { 0, 0, 0 };
        CS2::Write<Vec3>(cameraSvc + Game::camera::m_vecCsViewPunchAngle, zero);
    }

    inline void FOVChanger() {
        if (!Settings::fov_changer || !CS2::initialized) return;
        uintptr_t localPawn = CS2::GetLocalPlayerPawn();
        if (!localPawn) return;
        bool scoped = CS2::Read<bool>(localPawn + Game::pawn::m_bIsScoped);
        if (!scoped) {
            uintptr_t cameraSvc = CS2::Read<uintptr_t>(localPawn + Game::pawn::m_pCameraServices);
            if (cameraSvc) {
                CS2::Write<uint32_t>(cameraSvc + Game::camera::m_iFOV, (uint32_t)Settings::fov_value);
                CS2::Write<uint32_t>(cameraSvc + Game::camera::m_iFOVStart, (uint32_t)Settings::fov_value);
            }
        }
    }

    inline void ThirdPerson() {
        if (!CS2::initialized || !CS2::clientBase) return;
        if (!Game::localAlive) return;

        uintptr_t localPawn = CS2::GetLocalPlayerPawn();
        if (!localPawn) return;

        uintptr_t observerServices = CS2::Read<uintptr_t>(localPawn + Game::pawn::m_pObserverServices);
        if (!observerServices) return;

        if (Settings::thirdperson) {
            CS2::Write<uint8_t>(observerServices + Game::observer::m_iObserverMode, 1);
        } else {
            uint8_t curMode = CS2::Read<uint8_t>(observerServices + Game::observer::m_iObserverMode);
            if (curMode != 0) {
                CS2::Write<uint8_t>(observerServices + Game::observer::m_iObserverMode, 0);
            }
        }
    }
    inline uintptr_t GetLocalObserverPawn() {
        uintptr_t localPawn = CS2::GetLocalPlayerPawn();
        if (localPawn) return localPawn;

        uintptr_t localCtrl = CS2::GetLocalPlayerController();
        if (!localCtrl) return 0;

        uintptr_t entityList = CS2::GetEntityList();
        if (!entityList) return 0;

        uint32_t pawnHandle = CS2::Read<uint32_t>(localCtrl + CS2::controller::m_hPlayerPawn);
        if (!pawnHandle) return 0;

        uintptr_t pChunk = CS2::Read<uintptr_t>(entityList + 8 * ((pawnHandle & 0x7FFF) >> 9) + 0x10);
        if (!pChunk) return 0;

        return CS2::Read<uintptr_t>(pChunk + CS2::ENTRY_SIZE * (pawnHandle & 0x1FF));
    }

    inline void SpectatorEnemy() {
        if (!CS2::initialized || !CS2::clientBase) return;
        if (Game::localAlive) {
            Settings::spectator_enemy_count = 0;
            Settings::spectator_enemy_index = -1;
            return;
        }

        if (!Settings::spectator_enemy) {
            Settings::spectator_enemy_count = 0;
            Settings::spectator_enemy_index = -1;
            return;
        }

        uintptr_t observerPawn = GetLocalObserverPawn();
        if (!observerPawn) return;

        uintptr_t observerServices = CS2::Read<uintptr_t>(observerPawn + Game::pawn::m_pObserverServices);
        if (!observerServices) return;
        if (Settings::spectator_mode == 2) {
            CS2::Write<uint8_t>(observerServices + Game::observer::m_iObserverMode, 6);
            Settings::spectator_enemy_count = 0;
            Settings::spectator_enemy_index = -1;
            return;
        }
        uintptr_t entityList = CS2::GetEntityList();
        if (!entityList) return;

        uintptr_t ctrlListEntry = CS2::Read<uintptr_t>(entityList + 0x10);
        if (!ctrlListEntry) return;

        Settings::spectator_enemy_count = 0;
        for (int i = 0; i < 64; i++) {
            uintptr_t ctrl = CS2::Read<uintptr_t>(ctrlListEntry + CS2::ENTRY_SIZE * (i & 0x1FF));
            if (!ctrl) continue;
            if (Settings::spectator_only_enemies) {
                int team = (int)CS2::Read<uint8_t>(ctrl + Game::entity::m_iTeamNum);
                if (team == Game::localTeam) continue;
            }

            uint32_t pawnHandle = CS2::Read<uint32_t>(ctrl + CS2::controller::m_hPlayerPawn);
            if (!pawnHandle || pawnHandle == 0xFFFFFFFF) continue;

            uintptr_t pChunk = CS2::Read<uintptr_t>(entityList + 8 * ((pawnHandle & 0x7FFF) >> 9) + 0x10);
            if (!pChunk) continue;
            uintptr_t pawn = CS2::Read<uintptr_t>(pChunk + CS2::ENTRY_SIZE * (pawnHandle & 0x1FF));
            if (!pawn || pawn == observerPawn) continue;

            int health = CS2::Read<int>(pawn + Game::entity::m_iHealth);
            if (health <= 0) continue;

            Settings::spectator_enemy_list[Settings::spectator_enemy_count++] = pawnHandle;
            if (Settings::spectator_enemy_count >= 64) break;
        }

        if (Settings::spectator_enemy_count == 0) {
            Settings::spectator_enemy_index = -1;
            return;
        }
        static bool prevLMB = false, prevRMB = false;
        bool curLMB = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;
        bool curRMB = (GetAsyncKeyState(VK_RBUTTON) & 0x8000) != 0;
        bool shouldSwitch = false;

        if (curLMB && !prevLMB) {
            Settings::spectator_enemy_index++;
            if (Settings::spectator_enemy_index >= Settings::spectator_enemy_count)
                Settings::spectator_enemy_index = 0;
            shouldSwitch = true;
        }
        if (curRMB && !prevRMB) {
            Settings::spectator_enemy_index--;
            if (Settings::spectator_enemy_index < 0)
                Settings::spectator_enemy_index = Settings::spectator_enemy_count - 1;
            shouldSwitch = true;
        }
        prevLMB = curLMB;
        prevRMB = curRMB;
        if (Settings::spectator_enemy_index < 0 || Settings::spectator_enemy_index >= Settings::spectator_enemy_count) {
            Settings::spectator_enemy_index = 0;
            shouldSwitch = true;
        }

        if (!shouldSwitch) return;

        uint32_t selectedHandle = Settings::spectator_enemy_list[Settings::spectator_enemy_index];
        uint8_t obsMode = (Settings::spectator_mode == 1) ? 5 : 4;
        CS2::Write<uint8_t>(observerServices + Game::observer::m_iObserverMode, obsMode);
        CS2::Write<uint32_t>(observerServices + Game::observer::m_hObserverTarget, selectedHandle);
    }

    inline void Run() {
        NoFlash();
        RadarHack();
        NoVisualRecoil();
        FOVChanger();
        ThirdPerson();
        SpectatorEnemy();
        API::UpdateAll();
    }
}

namespace Prikol
{
    inline void SendChatMessage(const char* message) {
        OpenClipboard(NULL);
        EmptyClipboard();
        HGLOBAL hglb = GlobalAlloc(GMEM_MOVEABLE, strlen(message) + 1);
        char* lptstr = (char*)GlobalLock(hglb);
        //strcpy(lptstr, message);
        GlobalUnlock(hglb);
        SetClipboardData(CF_TEXT, hglb);
        CloseClipboard();

        keybd_event(0x59, 0, 0, 0);
        keybd_event(0x59, 0, KEYEVENTF_KEYUP, 0);
        Sleep(100);

        keybd_event(VK_CONTROL, 0, 0, 0);
        keybd_event(0x56, 0, 0, 0);
        keybd_event(0x56, 0, KEYEVENTF_KEYUP, 0);
        keybd_event(VK_CONTROL, 0, KEYEVENTF_KEYUP, 0);
        Sleep(100);

        keybd_event(VK_RETURN, 0, 0, 0);
        keybd_event(VK_RETURN, 0, KEYEVENTF_KEYUP, 0);
    }

    inline void CheckTrashtalk() {
        if (!Settings::trashtalk_enabled || !Game::localAlive) return;

        uintptr_t entityList = CS2::GetEntityList();
        if (!entityList) return;

        uintptr_t listEntry = CS2::Read<uintptr_t>(entityList + 0x10);
        if (!listEntry) return;

        for (int i = 0; i < 64; i++) {
            uintptr_t ctrl = CS2::Read<uintptr_t>(listEntry + CS2::ENTRY_SIZE * (i & 0x1FF));
            if (!ctrl) {
                Game::previousHealth[i] = 0;
                continue;
            }
            uint32_t pawnHandle = CS2::Read<uint32_t>(ctrl + CS2::controller::m_hPlayerPawn);
            if (!pawnHandle) {
                Game::previousHealth[i] = 0;
                continue;
            }

            uintptr_t pChunk = CS2::Read<uintptr_t>(entityList + 8 * ((pawnHandle & 0x7FFF) >> 9) + 0x10);
            if (!pChunk) {
                Game::previousHealth[i] = 0;
                continue;
            }

            uintptr_t pawnEnt = CS2::Read<uintptr_t>(pChunk + CS2::ENTRY_SIZE * (pawnHandle & 0x1FF));
            if (!pawnEnt) {
                Game::previousHealth[i] = 0;
                continue;
            }

            int health = CS2::Read<int>(pawnEnt + Game::entity::m_iHealth);
            int team = CS2::Read<uint8_t>(pawnEnt + Game::entity::m_iTeamNum);

            if (team != Game::localTeam) {
                if (Game::previousHealth[i] > 0 && health <= 0) {
                    SendChatMessage("1 свинья");
                }
                Game::previousHealth[i] = health;
            }
        }
    }
}

namespace AutoBuy
{
    inline bool boughtThisRound = false;
    inline bool lastFreeze = false;
    inline void SendBuyCommand(const char* cmd) {
        for (int i = 0; cmd[i]; i++) {
            SHORT vk = VkKeyScanA(cmd[i]);
            BYTE key = (BYTE)(vk & 0xFF);
            bool shift = (vk >> 8) & 1;
            if (shift) keybd_event(VK_SHIFT, 0, 0, 0);
            keybd_event(key, 0, 0, 0);
            keybd_event(key, 0, KEYEVENTF_KEYUP, 0);
            if (shift) keybd_event(VK_SHIFT, 0, KEYEVENTF_KEYUP, 0);
            Sleep(5);
        }
        keybd_event(VK_RETURN, 0, 0, 0);
        keybd_event(VK_RETURN, 0, KEYEVENTF_KEYUP, 0);
    }

    inline void OpenConsoleAndBuy(const char* cmd) {
        keybd_event(VK_OEM_3, 0, 0, 0);
        keybd_event(VK_OEM_3, 0, KEYEVENTF_KEYUP, 0);
        Sleep(50);
        SendBuyCommand(cmd);
        Sleep(30);
        keybd_event(VK_OEM_3, 0, 0, 0);
        keybd_event(VK_OEM_3, 0, KEYEVENTF_KEYUP, 0);
    }

    inline void SendChatMessage(const char* message) {
        keybd_event(0x59, 0, 0, 0);
        keybd_event(0x59, 0, KEYEVENTF_KEYUP, 0);
        Sleep(50);
        for (int i = 0; message[i]; i++) {
            SHORT vk = VkKeyScanA(message[i]);
            BYTE key = (BYTE)(vk & 0xFF);
            bool shift = (vk >> 8) & 1;
            if (shift) keybd_event(VK_SHIFT, 0, 0, 0);
            keybd_event(key, 0, 0, 0);
            keybd_event(key, 0, KEYEVENTF_KEYUP, 0);
            if (shift) keybd_event(VK_SHIFT, 0, KEYEVENTF_KEYUP, 0);
            Sleep(5);
        }
        keybd_event(VK_RETURN, 0, 0, 0);
        keybd_event(VK_RETURN, 0, KEYEVENTF_KEYUP, 0);
    }

    inline void BuyPreset(int preset) {
        const char* commands[] = { nullptr };

        switch (preset) {
        case 0:
        {
            const char* cmds[] = {
                "buy vesthelm", "buy ak47", "buy m4a1",
                "buy smokegrenade", "buy flashbang",
                "buy hegrenade", "buy molotov", "buy incgrenade",
                "buy defuser", nullptr
            };
            for (int i = 0; cmds[i]; i++) {
                OpenConsoleAndBuy(cmds[i]);
                Sleep(50);
            }
            break;
        }
        case 1:
        {
            const char* cmds[] = {
                "buy vesthelm", "buy awp",
                "buy smokegrenade", "buy flashbang",
                "buy defuser", nullptr
            };
            for (int i = 0; cmds[i]; i++) {
                OpenConsoleAndBuy(cmds[i]);
                Sleep(50);
            }
            break;
        }
        case 2:
        {
            const char* cmds[] = {
                "buy vest", "buy mp9", "buy mac10",
                "buy flashbang", nullptr
            };
            for (int i = 0; cmds[i]; i++) {
                OpenConsoleAndBuy(cmds[i]);
                Sleep(50);
            }
            break;
        }
        }
    }

    inline void Run() {
        if (!Settings::auto_buy || !CS2::initialized) return;

        uintptr_t gr = CS2::GetGameRules();
        if (!gr) return;

        bool freeze = CS2::Read<bool>(gr + CS2::gamerules::m_bFreezePeriod);
        bool warmup = CS2::Read<bool>(gr + CS2::gamerules::m_bWarmupPeriod);
        if (freeze && !lastFreeze && !warmup) {
            boughtThisRound = false;
        }
        lastFreeze = freeze;
        if (freeze && !boughtThisRound && !warmup) {
            uintptr_t localPawn = CS2::GetLocalPlayerPawn();
            if (localPawn) {
                uint8_t lifeState = CS2::Read<uint8_t>(localPawn + Game::entity::m_lifeState);
                if (lifeState == 0) {
                    Sleep(500);
                    BuyPreset(Settings::auto_buy_preset);
                    boughtThisRound = true;
                }
            }
        }
        if (!freeze) {
            boughtThisRound = false;
        }
    }
}

namespace AutoAccept
{
    inline bool wasSearching = false;
    inline DWORD lastCheckTime = 0;

    inline void Run() {
        if (!Settings::auto_accept) return;

        DWORD now = GetTickCount();
        if (now - lastCheckTime < 1000) return;
        lastCheckTime = now;
        HWND hwnd = FindWindowA(nullptr, "Counter-Strike 2");
        if (!hwnd) return;
        if (!CS2::initialized) return;
        uintptr_t localPawn = CS2::GetLocalPlayerPawn();
        uintptr_t localCtrl = CS2::GetLocalPlayerController();
        if (!localPawn && CS2::clientBase) {
            HWND fg = GetForegroundWindow();
            if (fg == hwnd) {
                INPUT input[2] = {};
                input[0].type = INPUT_KEYBOARD;
                input[0].ki.wVk = VK_RETURN;
                input[1].type = INPUT_KEYBOARD;
                input[1].ki.wVk = VK_RETURN;
                input[1].ki.dwFlags = KEYEVENTF_KEYUP;
                SendInput(2, input, sizeof(INPUT));
            }
        }
    }
}

namespace GrenadeTracer
{
    enum GrenadeType : uint8_t {
        GREN_UNKNOWN = 0,
        GREN_FLASHBANG,
        GREN_HE,
        GREN_SMOKE,
        GREN_MOLOTOV,
        GREN_DECOY
    };

    struct Trail {
        Vec3 points[1024];
        int pointCount = 0;
        int entityIndex = -1;
        GrenadeType type = GREN_UNKNOWN;
        bool alive = false;
        float deathTime = 0.0f;
    };

    inline Trail trails[32];
    inline int trailCount = 0;

    inline float GetTime() {
        static ULONGLONG startTick = GetTickCount64();
        return (float)(GetTickCount64() - startTick) / 1000.0f;
    }

    inline ImU32 GetColor(GrenadeType t, float alpha = 1.0f) {
        uint8_t a = (uint8_t)(alpha * 255);
        switch (t) {
            case GREN_FLASHBANG: return IM_COL32(255, 255, 100, a);
            case GREN_HE:       return IM_COL32(255, 100, 30, a);
            case GREN_SMOKE:    return IM_COL32(100, 160, 255, a);
            case GREN_MOLOTOV:  return IM_COL32(255, 50, 20, a);
            case GREN_DECOY:    return IM_COL32(100, 255, 100, a);
            default:            return IM_COL32(255, 255, 255, a);
        }
    }

    inline const char* GetLabel(GrenadeType t) {
        switch (t) {
            case GREN_FLASHBANG: return "FLASH";
            case GREN_HE:       return "HE";
            case GREN_SMOKE:    return "SMOKE";
            case GREN_MOLOTOV:  return "MOLOTOV";
            case GREN_DECOY:    return "DECOY";
            default:            return "?";
        }
    }

    inline GrenadeType ClassifyGrenade(const char* name) {
        if (strstr(name, "flashbang"))   return GREN_FLASHBANG;
        if (strstr(name, "hegrenade"))   return GREN_HE;
        if (strstr(name, "smokegrenade"))return GREN_SMOKE;
        if (strstr(name, "molotov"))     return GREN_MOLOTOV;
        if (strstr(name, "incendiary"))  return GREN_MOLOTOV;
        if (strstr(name, "decoy"))       return GREN_DECOY;
        return GREN_UNKNOWN;
    }
    inline int dbg_highIdx = 0;
    inline int dbg_entFound = 0;
    inline int dbg_nameFound = 0;
    inline int dbg_projFound = 0;
    inline char dbg_lastName[48] = {};
    inline char dbg_allNames[32][48] = {};
    inline int dbg_allNameCount = 0;

    inline void Update() {
        if (!Settings::grenade_tracer || !CS2::initialized) return;

        float curTime = GetTime();
        for (int t = 0; t < trailCount; t++)
            trails[t].alive = false;

        uintptr_t entityList = CS2::GetEntityList();
        if (!entityList) return;
        int highIdx = 1024;
        dbg_highIdx = highIdx;
        dbg_entFound = 0;
        dbg_nameFound = 0;
        dbg_projFound = 0;
        dbg_allNameCount = 0;

        for (int i = 64; i <= highIdx; i++) {
            uintptr_t listEntry = CS2::Read<uintptr_t>(entityList + 8 * ((i & 0x7FFF) >> 9) + 0x10);
            if (!listEntry) continue;
            uintptr_t ent = CS2::Read<uintptr_t>(listEntry + CS2::ENTRY_SIZE * (i & 0x1FF));
            if (!ent) continue;

            dbg_entFound++;
            uintptr_t identity = CS2::Read<uintptr_t>(ent + 0x10);
            if (!identity) continue;
            char name[48] = {};
            bool validStr = false;

            for (int nameOff = 0x18; nameOff <= 0x20; nameOff += 0x8) {
                uintptr_t nameAddr = CS2::Read<uintptr_t>(identity + nameOff);
                if (!nameAddr || nameAddr < 0x10000) continue;

                __try {
                    char first = *(char*)nameAddr;
                    if (first >= ' ' && first <= '~') {
                        for (int c = 0; c < 47; c++) {
                            name[c] = *(char*)(nameAddr + c);
                            if (!name[c]) break;
                        }
                        if (name[0] != 0) { validStr = true; break; }
                    }
                } __except (EXCEPTION_EXECUTE_HANDLER) { continue; }
            }

            if (!validStr) continue;

            dbg_nameFound++;
            memcpy(dbg_lastName, name, 48);
            if (dbg_allNameCount < 32) {
                bool dup = false;
                for (int n = 0; n < dbg_allNameCount; n++) {
                    if (strcmp(dbg_allNames[n], name) == 0) { dup = true; break; }
                }
                if (!dup) memcpy(dbg_allNames[dbg_allNameCount++], name, 48);
            }
            if (!strstr(name, "_projectile")) continue;

            dbg_projFound++;

            GrenadeType gtype = ClassifyGrenade(name);
            if (gtype == GREN_UNKNOWN) {
                gtype = GREN_HE;
            }
            uintptr_t node = CS2::Read<uintptr_t>(ent + Game::entity::m_pGameSceneNode);
            if (!node) continue;
            Vec3 pos = CS2::Read<Vec3>(node + Game::scene::m_vecAbsOrigin);
            if (pos.x == 0 && pos.y == 0 && pos.z == 0) continue;

            int trailIdx = -1;
            for (int t = 0; t < trailCount; t++) {
                if (trails[t].entityIndex == i) { trailIdx = t; break; }
            }
            if (trailIdx < 0 && trailCount < 32) {
                trailIdx = trailCount++;
                trails[trailIdx] = {};
                trails[trailIdx].entityIndex = i;
                trails[trailIdx].type = gtype;
            }
            if (trailIdx < 0) continue;

            trails[trailIdx].alive = true;

            bool addPoint = (trails[trailIdx].pointCount == 0);
            if (!addPoint) {
                Vec3& last = trails[trailIdx].points[trails[trailIdx].pointCount - 1];
                float dx = pos.x - last.x, dy = pos.y - last.y, dz = pos.z - last.z;
                if (dx * dx + dy * dy + dz * dz > 4.0f) addPoint = true;
            }
            if (addPoint && trails[trailIdx].pointCount < 1024) {
                trails[trailIdx].points[trails[trailIdx].pointCount++] = pos;
            }
        }
        for (int t = 0; t < trailCount; t++) {
            if (!trails[t].alive && trails[t].deathTime == 0.0f)
                trails[t].deathTime = curTime;
        }
        for (int t = trailCount - 1; t >= 0; t--) {
            if (!trails[t].alive && trails[t].deathTime > 0 &&
                (curTime - trails[t].deathTime) > Settings::grenade_trail_duration) {
                trails[t] = trails[--trailCount];
            }
        }
    }

    inline void GetColorF(GrenadeType t, float alpha, float& r, float& g, float& b) {
        switch (t) {
            case GREN_FLASHBANG: r = 1.0f;  g = 1.0f;  b = 0.4f;  break;
            case GREN_HE:       r = 1.0f;  g = 0.4f;  b = 0.12f; break;
            case GREN_SMOKE:    r = 0.4f;  g = 0.63f; b = 1.0f;  break;
            case GREN_MOLOTOV:  r = 1.0f;  g = 0.2f;  b = 0.08f; break;
            case GREN_DECOY:    r = 0.4f;  g = 1.0f;  b = 0.4f;  break;
            default:            r = 1.0f;  g = 1.0f;  b = 1.0f;  break;
        }
    }

    inline void Render() {
        if (!Settings::grenade_tracer || !CS2::initialized || !Render3D::initialized) return;

        Render3D::lineThickness = Settings::grenade_tracer_thickness;
        float curTime = GetTime();

        for (int t = 0; t < trailCount; t++) {
            Trail& tr = trails[t];
            if (tr.pointCount < 2) continue;
            float globalFade = 1.0f;
            if (!tr.alive && tr.deathTime > 0) {
                float elapsed = curTime - tr.deathTime;
                globalFade = 1.0f - (elapsed / Settings::grenade_trail_duration);
                if (globalFade <= 0) continue;
            }

            float cr, cg, cb;
            GetColorF(tr.type, 1.0f, cr, cg, cb);

            int n = tr.pointCount;
            for (int p = 1; p < n; p++) {
                Render3D::DrawLine3D(tr.points[p - 1], tr.points[p], cr, cg, cb, globalFade);
            }
        }
    }
}
namespace GrenadePrediction
{
    enum WeaponID : int {
        WEAPON_FLASHBANG   = 43,
        WEAPON_HEGRENADE   = 44,
        WEAPON_SMOKEGRENADE = 45,
        WEAPON_MOLOTOV     = 46,
        WEAPON_DECOY       = 47,
        WEAPON_INCENDIARY  = 48,
    };

    inline bool IsGrenadeWeapon(int defIdx) {
        return defIdx == WEAPON_FLASHBANG || defIdx == WEAPON_HEGRENADE ||
               defIdx == WEAPON_SMOKEGRENADE || defIdx == WEAPON_MOLOTOV ||
               defIdx == WEAPON_DECOY || defIdx == WEAPON_INCENDIARY;
    }

    inline GrenadeTracer::GrenadeType GetGrenadeType(int defIdx) {
        switch (defIdx) {
            case WEAPON_FLASHBANG:    return GrenadeTracer::GREN_FLASHBANG;
            case WEAPON_HEGRENADE:    return GrenadeTracer::GREN_HE;
            case WEAPON_SMOKEGRENADE: return GrenadeTracer::GREN_SMOKE;
            case WEAPON_MOLOTOV:
            case WEAPON_INCENDIARY:   return GrenadeTracer::GREN_MOLOTOV;
            case WEAPON_DECOY:        return GrenadeTracer::GREN_DECOY;
            default:                  return GrenadeTracer::GREN_UNKNOWN;
        }
    }

    constexpr float GRAVITY   = 400.0f;
    constexpr float TICK_RATE = 1.0f / 64.0f;
    constexpr float THROW_SPEED = 750.0f;
    constexpr int   MAX_STEPS  = 300;
    constexpr float BOUNCE_MOD = 0.45f;
    constexpr float DRAG = 0.0f;

    inline Vec3 predPoints[MAX_STEPS + 1];
    inline int  predCount = 0;
    inline GrenadeTracer::GrenadeType predType = GrenadeTracer::GREN_UNKNOWN;
    inline bool predActive = false;
    inline float TraceLine(Vec3 start, Vec3 end, Vec3& hitNormal) {
        float dz = end.z - start.z;
        if (dz >= 0) return 1.0f;
        float floorZ = Game::localPos.z - 10.0f;
        if (end.z > floorZ) return 1.0f;
        if (start.z <= floorZ) return 1.0f;

        float frac = (start.z - floorZ) / (start.z - end.z);
        if (frac < 0) frac = 0;
        if (frac > 1) frac = 1;
        hitNormal = { 0, 0, 1 };
        return frac;
    }
    inline Vec3 Reflect(Vec3 vel, Vec3 normal, float mod) {
        float dot = vel.x * normal.x + vel.y * normal.y + vel.z * normal.z;
        return {
            vel.x - (1.0f + mod) * dot * normal.x,
            vel.y - (1.0f + mod) * dot * normal.y,
            vel.z - (1.0f + mod) * dot * normal.z
        };
    }

    inline void Update() {
        predActive = false;
        predCount = 0;
        if (!Settings::grenade_prediction || !CS2::initialized || !Game::localAlive) return;

        uintptr_t localPawn = CS2::GetLocalPlayerPawn();
        if (!localPawn) return;
        uintptr_t weapSvc = CS2::Read<uintptr_t>(localPawn + Game::weapon::m_pWeaponServices);
        if (!weapSvc) return;
        uint32_t weapHandle = CS2::Read<uint32_t>(weapSvc + Game::weapon::m_hActiveWeapon);
        if (!weapHandle) return;

        uintptr_t entityList = CS2::GetEntityList();
        if (!entityList) return;
        uintptr_t wChunk = CS2::Read<uintptr_t>(entityList + 8 * ((weapHandle & 0x7FFF) >> 9) + 0x10);
        if (!wChunk) return;
        uintptr_t weapEnt = CS2::Read<uintptr_t>(wChunk + CS2::ENTRY_SIZE * (weapHandle & 0x1FF));
        if (!weapEnt) return;

        int defIdx = CS2::Read<uint16_t>(weapEnt + Game::weapon::m_iItemDefinitionIndex);
        if (!IsGrenadeWeapon(defIdx)) return;

        predType = GetGrenadeType(defIdx);
        Vec3 eyePos = Game::localPos;
        eyePos.z += 64.12f;
        Vec3 va = Game::localViewAngles;
        float pitch = va.x * (3.14159265f / 180.0f);
        float yaw   = va.y * (3.14159265f / 180.0f);
        Vec3 forward = {
            cosf(pitch) * cosf(yaw),
            cosf(pitch) * sinf(yaw),
            -sinf(pitch)
        };
        float throwSpeed = THROW_SPEED;

        Vec3 vel = {
            forward.x * throwSpeed,
            forward.y * throwSpeed,
            forward.z * throwSpeed
        };
        Vec3 pos = {
            eyePos.x + forward.x * 16.0f,
            eyePos.y + forward.y * 16.0f,
            eyePos.z + forward.z * 16.0f
        };

        predPoints[0] = pos;
        predCount = 1;
        for (int step = 0; step < MAX_STEPS; step++) {
            vel.z -= GRAVITY * TICK_RATE;
            Vec3 next = {
                pos.x + vel.x * TICK_RATE,
                pos.y + vel.y * TICK_RATE,
                pos.z + vel.z * TICK_RATE
            };
            Vec3 hitN;
            float frac = TraceLine(pos, next, hitN);
            if (frac < 1.0f) {
                next.x = pos.x + (next.x - pos.x) * frac;
                next.y = pos.y + (next.y - pos.y) * frac;
                next.z = pos.z + (next.z - pos.z) * frac;
                vel = Reflect(vel, hitN, BOUNCE_MOD);
                vel.x *= 0.6f;
                vel.y *= 0.6f;
                vel.z *= 0.6f;
                float speed2 = vel.x*vel.x + vel.y*vel.y + vel.z*vel.z;
                if (speed2 < 100.0f) {
                    predPoints[predCount++] = next;
                    break;
                }
            }

            pos = next;
            predPoints[predCount++] = pos;
            if (predCount >= MAX_STEPS) break;
        }

        predActive = (predCount >= 2);
    }

    inline void Render() {
        if (!Settings::grenade_prediction || !predActive || !Render3D::initialized) return;

        float cr, cg, cb;
        GrenadeTracer::GetColorF(predType, 1.0f, cr, cg, cb);

        Render3D::lineThickness = Settings::grenade_tracer_thickness;

        for (int i = 1; i < predCount; i++) {
            float t = (float)i / (float)(predCount - 1);
            float alpha = 1.0f - t * 0.7f;
            Render3D::DrawLine3D(predPoints[i - 1], predPoints[i], cr, cg, cb, alpha);
        }
        if (predCount > 1) {
            Vec2 endScreen;
            float endDepth;
            if (Game::WorldToScreenDepth(predPoints[predCount - 1], endScreen, endDepth,
                    Render3D::screenW, Render3D::screenH)) {
                ImDrawList* dl = ImGui::GetForegroundDrawList();
                ImU32 col = GrenadeTracer::GetColor(predType, 0.8f);
                dl->AddCircle(ImVec2(endScreen.x, endScreen.y), 8.0f, col, 16, 2.0f);
                dl->AddCircleFilled(ImVec2(endScreen.x, endScreen.y), 3.0f, col);
            }
        }
    }
}
namespace BulletTracer
{
    struct BulletTrail {
        Vec3 start;
        Vec3 end;
        float spawnTime;
    };

    constexpr int MAX_BULLETS = 128;
    inline BulletTrail bullets[MAX_BULLETS];
    inline int bulletCount = 0;
    inline int prevShotsFired = 0;
    constexpr float BULLET_RANGE = 8192.0f;

    inline float GetTime() {
        static ULONGLONG startTick = GetTickCount64();
        return (float)(GetTickCount64() - startTick) / 1000.0f;
    }

    inline void Update() {
        if (!Settings::bullet_tracer || !CS2::initialized || !Game::localAlive) {
            prevShotsFired = 0;
            return;
        }

        float curTime = GetTime();
        for (int i = bulletCount - 1; i >= 0; i--) {
            if ((curTime - bullets[i].spawnTime) > Settings::bullet_tracer_duration) {
                bullets[i] = bullets[--bulletCount];
            }
        }

        uintptr_t localPawn = CS2::GetLocalPlayerPawn();
        if (!localPawn) { prevShotsFired = 0; return; }

        int shotsFired = CS2::Read<int>(localPawn + Game::pawn::m_iShotsFired);
        if (shotsFired > prevShotsFired && prevShotsFired >= 0) {
            Vec3 eyePos = Game::localPos;
            eyePos.z += 64.12f;
            Vec3 va = Game::localViewAngles;
            Vec3 punch = Game::localAimPunch;
            float pitch = (va.x + punch.x * 2.0f) * (3.14159265f / 180.0f);
            float yaw   = (va.y + punch.y * 2.0f) * (3.14159265f / 180.0f);

            Vec3 dir = {
                cosf(pitch) * cosf(yaw),
                cosf(pitch) * sinf(yaw),
                -sinf(pitch)
            };

            Vec3 endPos = {
                eyePos.x + dir.x * BULLET_RANGE,
                eyePos.y + dir.y * BULLET_RANGE,
                eyePos.z + dir.z * BULLET_RANGE
            };

            if (bulletCount < MAX_BULLETS) {
                bullets[bulletCount++] = { eyePos, endPos, curTime };
            }
        }

        prevShotsFired = shotsFired;
        if (shotsFired == 0) prevShotsFired = 0;
    }

    inline void Render() {
        if (!Settings::bullet_tracer || !Render3D::initialized) return;

        float curTime = GetTime();
        float cr = Settings::bullet_tracer_color[0];
        float cg = Settings::bullet_tracer_color[1];
        float cb = Settings::bullet_tracer_color[2];

        Render3D::lineThickness = 2.0f;

        for (int i = 0; i < bulletCount; i++) {
            float elapsed = curTime - bullets[i].spawnTime;
            float t = elapsed / Settings::bullet_tracer_duration;
            if (t >= 1.0f) continue;
            float fade = (t < 0.7f) ? 1.0f : (1.0f - (t - 0.7f) / 0.3f);
            fade *= Settings::bullet_tracer_color[3];
            Render3D::DrawLine3D(bullets[i].start, bullets[i].end, cr, cg, cb, fade);
        }
    }
}

namespace Visuals
{
    inline void DrawCrosshair() {
        if (!Settings::crosshair_enabled) return;
        ImDrawList* dl = ImGui::GetBackgroundDrawList();
        ImGuiIO& io = ImGui::GetIO();
        float cx = io.DisplaySize.x * 0.5f;
        float cy = io.DisplaySize.y * 0.5f;
        int s = Settings::crosshair_size;
        int g = Settings::crosshair_gap;
        float t = (float)Settings::crosshair_thickness;
        ImU32 col = ESP::ColorFromFloat(Settings::crosshair_color);
        ImU32 outline = IM_COL32(0, 0, 0, 180);
        dl->AddLine(ImVec2(cx - s - g, cy), ImVec2(cx - g, cy), outline, t + 2);
        dl->AddLine(ImVec2(cx + g, cy), ImVec2(cx + s + g, cy), outline, t + 2);
        dl->AddLine(ImVec2(cx, cy - s - g), ImVec2(cx, cy - g), outline, t + 2);
        dl->AddLine(ImVec2(cx, cy + g), ImVec2(cx, cy + s + g), outline, t + 2);
        dl->AddLine(ImVec2(cx - s - g, cy), ImVec2(cx - g, cy), col, t);
        dl->AddLine(ImVec2(cx + g, cy), ImVec2(cx + s + g, cy), col, t);
        dl->AddLine(ImVec2(cx, cy - s - g), ImVec2(cx, cy - g), col, t);
        dl->AddLine(ImVec2(cx, cy + g), ImVec2(cx, cy + s + g), col, t);
    }

    inline void DrawWatermark() {
        if (!Settings::watermark) return;
        ImDrawList* dl = ImGui::GetBackgroundDrawList();
        ImGuiIO& io = ImGui::GetIO();
        float fps = io.Framerate;

        char buf[128];
        sprintf_s(buf, "PIZDOPASTA | %.0f fps", fps);
        ImVec2 ts = ImGui::CalcTextSize(buf);
        float px = io.DisplaySize.x - ts.x - 15;
        float py = 10;

        dl->AddRectFilled(ImVec2(px - 8, py - 4), ImVec2(px + ts.x + 8, py + ts.y + 4),
            IM_COL32(12, 12, 12, 200), 4.0f);
        dl->AddRect(ImVec2(px - 8, py - 4), ImVec2(px + ts.x + 8, py + ts.y + 4),
            IM_COL32(45, 45, 45, 120), 4.0f);
        dl->AddText(ImVec2(px, py), IM_COL32(165, 211, 50, 255), buf);
    }

    inline void GlowESP() {
        if (!Settings::esp_glow || !CS2::initialized) return;
        for (int i = 0; i < Game::playerCount; i++) {
            Game::PlayerInfo& p = Game::players[i];
            if (!p.valid || !p.alive) continue;
            bool isEnemy = (p.team != Game::localTeam);
            if (!isEnemy && !Settings::esp_team) continue;

            uintptr_t node = CS2::Read<uintptr_t>(p.pawn + Game::entity::m_pGameSceneNode);
            if (!node) continue;
            float* glowColor = isEnemy ? Settings::esp_glow_color_enemy : Settings::esp_glow_color_team;
            constexpr uintptr_t m_Glow = 0xDD8;
            constexpr uintptr_t m_bGlowing = 0x51;
            constexpr uintptr_t m_iGlowType = 0x30;
            constexpr uintptr_t m_fGlowColor = 0x8;
            constexpr uintptr_t m_glowColorOverride = 0x40;

            uintptr_t glowBase = p.pawn + m_Glow;
            CS2::Write<bool>(glowBase + m_bGlowing, true);
            CS2::Write<int>(glowBase + m_iGlowType, 0);
            Vec3 glowVec = { glowColor[0], glowColor[1], glowColor[2] };
            CS2::Write<Vec3>(glowBase + m_fGlowColor, glowVec);
            uint8_t colorBytes[4] = {
                (uint8_t)(glowColor[0] * 255),
                (uint8_t)(glowColor[1] * 255),
                (uint8_t)(glowColor[2] * 255),
                (uint8_t)(glowColor[3] * 255)
            };
            CS2::Write<uint32_t>(glowBase + m_glowColorOverride, *(uint32_t*)colorBytes);
        }
    }

    inline const char* GetWeaponName(uint16_t defIndex) {
        switch (defIndex) {
            case 1: return "Desert Eagle"; case 2: return "Dual Berettas"; case 3: return "Five-SeveN";
            case 4: return "Glock-18"; case 7: return "AK-47"; case 8: return "AUG";
            case 9: return "AWP"; case 10: return "FAMAS"; case 11: return "G3SG1";
            case 13: return "Galil AR"; case 14: return "M249"; case 16: return "M4A4";
            case 17: return "MAC-10"; case 19: return "P90"; case 23: return "MP5-SD";
            case 24: return "UMP-45"; case 25: return "XM1014"; case 26: return "PP-Bizon";
            case 27: return "MAG-7"; case 28: return "Negev"; case 29: return "Sawed-Off";
            case 30: return "Tec-9"; case 31: return "Zeus x27"; case 32: return "P2000";
            case 33: return "MP7"; case 34: return "MP9"; case 35: return "Nova";
            case 36: return "P250"; case 38: return "SCAR-20"; case 39: return "SG 553";
            case 40: return "SSG 08"; case 42: return "Knife"; case 43: return "Flashbang";
            case 44: return "HE Grenade"; case 45: return "Smoke"; case 46: return "Molotov";
            case 47: return "Decoy"; case 48: return "Incendiary"; case 49: return "C4";
            case 59: return "Knife"; case 60: return "M4A1-S"; case 61: return "USP-S";
            case 63: return "CZ75-Auto"; case 64: return "R8 Revolver";
            default: return "Unknown";
        }
    }

    inline uintptr_t ResolveHandleToPawn(uint32_t handle) {
        if (!handle || handle == 0xFFFFFFFF) return 0;
        uintptr_t entityList = CS2::GetEntityList();
        if (!entityList) return 0;
        uintptr_t pChunk = CS2::Read<uintptr_t>(entityList + 8 * ((handle & 0x7FFF) >> 9) + 0x10);
        if (!pChunk) return 0;
        return CS2::Read<uintptr_t>(pChunk + CS2::ENTRY_SIZE * (handle & 0x1FF));
    }

    inline void ReadWeaponInfo(uintptr_t pawn, int& clip, int& reserve, const char*& weaponName) {
        clip = -1; reserve = -1; weaponName = "";
        uintptr_t weapSvc = CS2::Read<uintptr_t>(pawn + Game::weapon::m_pWeaponServices);
        if (!weapSvc) return;
        uint32_t activeHandle = CS2::Read<uint32_t>(weapSvc + Game::weapon::m_hActiveWeapon);
        uintptr_t weapEnt = ResolveHandleToPawn(activeHandle);
        if (!weapEnt) return;
        clip = CS2::Read<int>(weapEnt + Game::weapon::m_iClip1);
        reserve = CS2::Read<int>(weapEnt + Game::weapon::m_pReserveAmmo);
        uint16_t defIdx = CS2::Read<uint16_t>(weapEnt + Game::weapon::m_iItemDefinitionIndex);
        weaponName = GetWeaponName(defIdx);
    }

    inline void DrawCustomHUD() {
        if (!Settings::custom_hud || !CS2::initialized) return;

        ImDrawList* dl = ImGui::GetBackgroundDrawList();
        ImGuiIO& io = ImGui::GetIO();
        float sw = io.DisplaySize.x;
        float sh = io.DisplaySize.y;

        uintptr_t targetPawn = 0;
        bool isSpectating = false;
        char spectatedName[128] = {};

        if (Game::localAlive) {
            targetPawn = CS2::GetLocalPlayerPawn();
        } else if (Settings::spectator_enemy && Settings::spectator_enemy_index >= 0
                   && Settings::spectator_enemy_index < Settings::spectator_enemy_count) {
            uint32_t handle = Settings::spectator_enemy_list[Settings::spectator_enemy_index];
            targetPawn = ResolveHandleToPawn(handle);
            isSpectating = true;
            uintptr_t entityList = CS2::GetEntityList();
            if (entityList) {
                uintptr_t ctrlListEntry = CS2::Read<uintptr_t>(entityList + 0x10);
                if (ctrlListEntry) {
                    for (int i = 0; i < 64; i++) {
                        uintptr_t ctrl = CS2::Read<uintptr_t>(ctrlListEntry + CS2::ENTRY_SIZE * (i & 0x1FF));
                        if (!ctrl) continue;
                        uint32_t ph = CS2::Read<uint32_t>(ctrl + CS2::controller::m_hPlayerPawn);
                        if (ph == handle) {
                            uintptr_t namePtr = CS2::Read<uintptr_t>(ctrl + CS2::controller::m_sSanitizedPlayerName);
                            if (namePtr) {
                                for (int c = 0; c < 127; c++) {
                                    spectatedName[c] = CS2::Read<char>(namePtr + c);
                                    if (!spectatedName[c]) break;
                                }
                            }
                            break;
                        }
                    }
                }
            }
        }

        if (!targetPawn) return;
        int hp = CS2::Read<int>(targetPawn + Game::entity::m_iHealth);
        int armor = CS2::Read<int>(targetPawn + Game::pawn::m_ArmorValue);

        int clip = -1, reserve = -1;
        const char* weaponName = "";
        ReadWeaponInfo(targetPawn, clip, reserve, weaponName);

        Vec3 vel = CS2::Read<Vec3>(targetPawn + Game::entity::m_vecAbsVelocity);
        float speed = sqrtf(vel.x * vel.x + vel.y * vel.y);
        float panelH = 52.0f;
        float panelY = sh - panelH - 12.0f;
        float barH = 6.0f;
        float rounding = 6.0f;
        ImU32 bgCol = IM_COL32(12, 12, 18, 210);
        ImU32 borderCol = isSpectating ? IM_COL32(255, 60, 60, 60) : IM_COL32(0, 178, 255, 60);
        if (isSpectating && spectatedName[0]) {
            char banner[160];
            sprintf_s(banner, "SPECTATING: %s", spectatedName);
            ImVec2 ts = ImGui::CalcTextSize(banner);
            float bx = (sw - ts.x) * 0.5f;
            float by = sh - panelH - 42.0f;
            dl->AddRectFilled(ImVec2(bx - 12, by - 4), ImVec2(bx + ts.x + 12, by + ts.y + 4),
                IM_COL32(12, 12, 18, 220), 4.0f);
            dl->AddRect(ImVec2(bx - 12, by - 4), ImVec2(bx + ts.x + 12, by + ts.y + 4),
                IM_COL32(255, 60, 60, 100), 4.0f);
            dl->AddText(ImVec2(bx, by), IM_COL32(255, 80, 80, 255), banner);
        }
        float lw = 280.0f;
        float lx = 20.0f;
        dl->AddRectFilled(ImVec2(lx, panelY), ImVec2(lx + lw, panelY + panelH), bgCol, rounding);
        dl->AddRect(ImVec2(lx, panelY), ImVec2(lx + lw, panelY + panelH), borderCol, rounding);

        ImU32 hpCol = (hp > 50) ? IM_COL32(80, 255, 80, 255) :
                      (hp > 25) ? IM_COL32(255, 200, 50, 255) : IM_COL32(255, 60, 60, 255);
        char hpBuf[16]; sprintf_s(hpBuf, "%d", hp);
        dl->AddText(ImVec2(lx + 10, panelY + 6), hpCol, hpBuf);
        dl->AddText(ImVec2(lx + 50, panelY + 6), IM_COL32(180, 180, 190, 200), "HP");

        float hpBarX = lx + 10;
        float hpBarY = panelY + 26;
        float hpBarW = lw - 20;
        float hpFrac = (float)hp / 100.0f; if (hpFrac > 1.0f) hpFrac = 1.0f;
        dl->AddRectFilled(ImVec2(hpBarX, hpBarY), ImVec2(hpBarX + hpBarW, hpBarY + barH), IM_COL32(40, 40, 50, 200), 3.0f);
        if (hpFrac > 0) dl->AddRectFilled(ImVec2(hpBarX, hpBarY), ImVec2(hpBarX + hpBarW * hpFrac, hpBarY + barH), hpCol, 3.0f);

        float arBarY = hpBarY + barH + 4;
        float arFrac = (float)armor / 100.0f; if (arFrac > 1.0f) arFrac = 1.0f;
        dl->AddRectFilled(ImVec2(hpBarX, arBarY), ImVec2(hpBarX + hpBarW, arBarY + barH), IM_COL32(40, 40, 50, 200), 3.0f);
        if (arFrac > 0) dl->AddRectFilled(ImVec2(hpBarX, arBarY), ImVec2(hpBarX + hpBarW * arFrac, arBarY + barH), IM_COL32(0, 140, 255, 255), 3.0f);
        float rw = 240.0f;
        float rx = sw - rw - 20.0f;
        dl->AddRectFilled(ImVec2(rx, panelY), ImVec2(rx + rw, panelY + panelH), bgCol, rounding);
        dl->AddRect(ImVec2(rx, panelY), ImVec2(rx + rw, panelY + panelH), borderCol, rounding);

        dl->AddText(ImVec2(rx + 10, panelY + 6), IM_COL32(200, 200, 210, 255), weaponName);

        if (clip >= 0) {
            char ammoBuf[32];
            if (reserve >= 0)
                sprintf_s(ammoBuf, "%d / %d", clip, reserve);
            else
                sprintf_s(ammoBuf, "%d", clip);
            ImVec2 ammoSize = ImGui::CalcTextSize(ammoBuf);
            dl->AddText(ImVec2(rx + rw - ammoSize.x - 10, panelY + 6), IM_COL32(255, 220, 80, 255), ammoBuf);
        }

        char speedBuf[32]; sprintf_s(speedBuf, "%.0f u/s", speed);
        dl->AddText(ImVec2(rx + 10, panelY + 28), IM_COL32(150, 150, 165, 200), speedBuf);
    }
    namespace Rain {
        struct Drop {
            float x, y;
            float speed;
            float length;
            float alpha;
        };

        constexpr int MAX_DROPS = 1000;
        inline Drop drops[MAX_DROPS];
        inline bool inited = false;
        inline ULONGLONG lastTick = 0;

        inline float Randf(float lo, float hi) {
            return lo + (float)(rand() % 10000) / 10000.0f * (hi - lo);
        }

        inline void InitDrop(Drop& d, float scrW, float scrH, bool top) {
            d.x = Randf(-scrW * 0.1f, scrW * 1.1f);
            d.y = top ? Randf(-scrH * 0.2f, -10.0f) : Randf(-scrH, scrH);
            d.speed = Randf(800.0f, 1600.0f);
            d.length = Randf(15.0f, 40.0f);
            d.alpha = Randf(0.08f, 1.0f);
        }

        inline void Render() {
            if (!Settings::rain_enabled) return;

            ImDrawList* dl = ImGui::GetBackgroundDrawList();
            ImGuiIO& io = ImGui::GetIO();
            float scrW = io.DisplaySize.x;
            float scrH = io.DisplaySize.y;

            ULONGLONG now = GetTickCount64();
            if (!inited || lastTick == 0) {
                srand((unsigned)now);
                int count = Settings::rain_intensity;
                if (count > MAX_DROPS) count = MAX_DROPS;
                for (int i = 0; i < count; i++)
                    InitDrop(drops[i], scrW, scrH, false);
                inited = true;
                lastTick = now;
                return;
            }

            float dt = (float)(now - lastTick) / 1000.0f;
            if (dt > 0.1f) dt = 0.1f;
            lastTick = now;

            int count = Settings::rain_intensity;
            if (count > MAX_DROPS) count = MAX_DROPS;

            float wind = Settings::rain_wind;
            float speedMul = Settings::rain_speed;
            float baseAlpha = Settings::rain_alpha;

            for (int i = 0; i < count; i++) {
                Drop& d = drops[i];
                d.y += d.speed * speedMul * dt;
                d.x += d.speed * speedMul * wind * dt;
                if (d.y > scrH + 50.0f || d.x > scrW * 1.3f || d.x < -scrW * 0.3f) {
                    InitDrop(d, scrW, scrH, true);
                }
                float endX = d.x + d.length * wind;
                float endY = d.y + d.length;

                uint8_t a = (uint8_t)(d.alpha * baseAlpha * 255.0f);
                if (a < 2) continue;
                ImU32 col = IM_COL32(180, 200, 230, a);
                dl->AddLine(ImVec2(d.x, d.y), ImVec2(endX, endY), col, 1.0f);
            }
            for (int i = 0; i < count / 10; i++) {
                Drop& d = drops[i];
                if (d.y > scrH - 80.0f && d.y < scrH) {
                    float splashAlpha = (1.0f - (scrH - d.y) / 80.0f) * baseAlpha * 0.3f;
                    uint8_t sa = (uint8_t)(splashAlpha * 255.0f);
                    if (sa > 2) {
                        ImU32 sc = IM_COL32(180, 200, 230, sa);
                        dl->AddCircle(ImVec2(d.x, scrH - 2), 2.0f, sc, 4, 1.0f);
                    }
                }
            }
        }
    }

    inline void Render() {
        DrawCrosshair();
        DrawWatermark();
        DrawCustomHUD();
        GlowESP();
        Rain::Render();
    }
}
namespace WorldModulation
{
    namespace fog_off {
        constexpr uintptr_t m_fog          = 0x550;
        constexpr uintptr_t enable         = 0x41;
        constexpr uintptr_t colorPrimary   = 0x0C;
        constexpr uintptr_t colorSecondary = 0x10;
        constexpr uintptr_t start          = 0x1C;
        constexpr uintptr_t end_           = 0x20;
        constexpr uintptr_t farz           = 0x24;
        constexpr uintptr_t maxdensity     = 0x28;
        constexpr uintptr_t HDRColorScale  = 0x30;
    }
    namespace tone_off {
        constexpr uintptr_t m_bUseCustomAutoExposureMin = 0x550;
        constexpr uintptr_t m_bUseCustomAutoExposureMax = 0x551;
        constexpr uintptr_t m_flCustomAutoExposureMin   = 0x554;
        constexpr uintptr_t m_flCustomAutoExposureMax   = 0x558;
        constexpr uintptr_t m_flCustomBloomScale        = 0x55C;
        constexpr uintptr_t m_flCustomBloomScaleMin     = 0x560;
    }

    inline uintptr_t fogEnt  = 0;
    inline uintptr_t toneEnt = 0;
    inline ULONGLONG lastScan = 0;
    inline bool hasBackup = false;
    inline uint32_t origFogColor1 = 0, origFogColor2 = 0;
    inline float origFogStart = 0, origFogEnd = 0, origFogDensity = 0;
    inline bool origFogEnable = false;
    inline float origExposureMin = 0, origExposureMax = 0;

    inline const char* ReadDesignerName(uintptr_t ent) {
        uintptr_t identity = CS2::Read<uintptr_t>(ent + 0x10);
        if (!identity) return nullptr;
        uintptr_t nameAddr = CS2::Read<uintptr_t>(identity + 0x20);
        if (!nameAddr || nameAddr < 0x10000) return nullptr;
        __try {
            char first = *(char*)nameAddr;
            if (first >= ' ' && first <= '~') return (const char*)nameAddr;
        } __except(EXCEPTION_EXECUTE_HANDLER) {}
        return nullptr;
    }

    inline void ScanEntities() {
        fogEnt = 0; toneEnt = 0;
        uintptr_t entityList = CS2::GetEntityList();
        if (!entityList) return;

        int highIdx = CS2::GetHighestEntityIndex();
        if (highIdx < 64) highIdx = 1024;
        if (highIdx > 4096) highIdx = 4096;

        for (int i = 0; i <= highIdx; i++) {
            uintptr_t listEntry = CS2::Read<uintptr_t>(entityList + 8 * ((i & 0x7FFF) >> 9) + 0x10);
            if (!listEntry) continue;
            uintptr_t ent = CS2::Read<uintptr_t>(listEntry + CS2::ENTRY_SIZE * (i & 0x1FF));
            if (!ent) continue;

            const char* name = ReadDesignerName(ent);
            if (!name) continue;

            __try {
                if (!fogEnt && strcmp(name, "env_fog_controller") == 0) fogEnt = ent;
                if (!toneEnt && strcmp(name, "env_tonemap_controller") == 0) toneEnt = ent;
            } __except(EXCEPTION_EXECUTE_HANDLER) { continue; }

            if (fogEnt && toneEnt) break;
        }
    }

    inline void BackupOriginals() {
        if (hasBackup) return;
        if (fogEnt) {
            uintptr_t fb = fogEnt + fog_off::m_fog;
            origFogEnable  = CS2::Read<bool>(fb + fog_off::enable);
            origFogColor1  = CS2::Read<uint32_t>(fb + fog_off::colorPrimary);
            origFogColor2  = CS2::Read<uint32_t>(fb + fog_off::colorSecondary);
            origFogStart   = CS2::Read<float>(fb + fog_off::start);
            origFogEnd     = CS2::Read<float>(fb + fog_off::end_);
            origFogDensity = CS2::Read<float>(fb + fog_off::maxdensity);
        }
        if (toneEnt) {
            origExposureMin = CS2::Read<float>(toneEnt + tone_off::m_flCustomAutoExposureMin);
            origExposureMax = CS2::Read<float>(toneEnt + tone_off::m_flCustomAutoExposureMax);
        }
        hasBackup = true;
    }

    inline void RestoreOriginals() {
        if (!hasBackup) return;
        if (fogEnt) {
            uintptr_t fb = fogEnt + fog_off::m_fog;
            CS2::Write<bool>(fb + fog_off::enable, origFogEnable);
            CS2::Write<uint32_t>(fb + fog_off::colorPrimary, origFogColor1);
            CS2::Write<uint32_t>(fb + fog_off::colorSecondary, origFogColor2);
            CS2::Write<float>(fb + fog_off::start, origFogStart);
            CS2::Write<float>(fb + fog_off::end_, origFogEnd);
            CS2::Write<float>(fb + fog_off::maxdensity, origFogDensity);
        }
        if (toneEnt) {
            CS2::Write<bool>(toneEnt + tone_off::m_bUseCustomAutoExposureMin, false);
            CS2::Write<bool>(toneEnt + tone_off::m_bUseCustomAutoExposureMax, false);
            CS2::Write<float>(toneEnt + tone_off::m_flCustomAutoExposureMin, origExposureMin);
            CS2::Write<float>(toneEnt + tone_off::m_flCustomAutoExposureMax, origExposureMax);
        }
        hasBackup = false;
    }

    inline uint32_t MakeColor(float r, float g, float b) {
        return (uint8_t)(r * 255) | ((uint8_t)(g * 255) << 8) |
               ((uint8_t)(b * 255) << 16) | (255u << 24);
    }

    inline void Update() {
        if (!CS2::initialized) return;
        ULONGLONG now = GetTickCount64();
        if (!fogEnt || !toneEnt || (now - lastScan > 3000)) {
            ScanEntities();
            lastScan = now;
        }

        if (!Settings::world_modulation) {
            if (hasBackup) RestoreOriginals();
            return;
        }
        BackupOriginals();

        float* col = Settings::world_color;
        float strength = col[3];
        int mode = Settings::world_mod_mode;
        if (fogEnt) {
            uintptr_t fb = fogEnt + fog_off::m_fog;

            if (mode == 0) {
                CS2::Write<bool>(fb + fog_off::enable, true);
                uint32_t c = MakeColor(col[0], col[1], col[2]);
                CS2::Write<uint32_t>(fb + fog_off::colorPrimary, c);
                CS2::Write<uint32_t>(fb + fog_off::colorSecondary, c);
                CS2::Write<float>(fb + fog_off::start, 200.0f);
                CS2::Write<float>(fb + fog_off::end_, 5000.0f - strength * 3500.0f);
                CS2::Write<float>(fb + fog_off::maxdensity, 0.2f + strength * 0.6f);
            }
            else if (mode == 1) {
                CS2::Write<bool>(fb + fog_off::enable, true);
                uint32_t nightCol = MakeColor(
                    col[0] * 0.15f, col[1] * 0.15f, col[2] * 0.2f);
                CS2::Write<uint32_t>(fb + fog_off::colorPrimary, nightCol);
                CS2::Write<uint32_t>(fb + fog_off::colorSecondary, nightCol);
                CS2::Write<float>(fb + fog_off::start, 10.0f);
                CS2::Write<float>(fb + fog_off::end_, 2000.0f - strength * 1500.0f);
                CS2::Write<float>(fb + fog_off::maxdensity, 0.6f + strength * 0.35f);
            }
            else {
                CS2::Write<bool>(fb + fog_off::enable, true);
                uint32_t c = MakeColor(col[0], col[1], col[2]);
                CS2::Write<uint32_t>(fb + fog_off::colorPrimary, c);
                CS2::Write<uint32_t>(fb + fog_off::colorSecondary, c);
                CS2::Write<float>(fb + fog_off::start, 0.0f);
                CS2::Write<float>(fb + fog_off::end_, 6000.0f - strength * 5000.0f);
                CS2::Write<float>(fb + fog_off::maxdensity, strength);
            }
        }
        if (toneEnt && mode == 1) {
            CS2::Write<bool>(toneEnt + tone_off::m_bUseCustomAutoExposureMin, true);
            CS2::Write<bool>(toneEnt + tone_off::m_bUseCustomAutoExposureMax, true);
            float exposure = 0.05f + (1.0f - strength) * 0.35f;
            CS2::Write<float>(toneEnt + tone_off::m_flCustomAutoExposureMin, exposure);
            CS2::Write<float>(toneEnt + tone_off::m_flCustomAutoExposureMax, exposure);
        }
        else if (toneEnt && hasBackup) {
            CS2::Write<bool>(toneEnt + tone_off::m_bUseCustomAutoExposureMin, false);
            CS2::Write<bool>(toneEnt + tone_off::m_bUseCustomAutoExposureMax, false);
        }
    }
}

namespace SpectatorList
{
    struct SpectatorInfo {
        char name[64];
        uint8_t mode;
    };

    inline SpectatorInfo spectators[64];
    inline int spectatorCount = 0;
    inline ImVec2 pos = ImVec2(-1, -1);
    inline bool pos_init = false;

    inline const char* ObsModeName(uint8_t mode) {
        switch (mode) {
            case 4: return "1st";
            case 5: return "3rd";
            case 6: return "Free";
            default: return "?";
        }
    }

    inline void Update() {
        spectatorCount = 0;
        if (!Settings::spectator_list || !CS2::initialized) return;

        uintptr_t localPawn = CS2::GetLocalPlayerPawn();
        if (!localPawn) return;

        int localHealth = CS2::Read<int>(localPawn + Game::entity::m_iHealth);
        if (localHealth <= 0) return;

        uintptr_t entityList = CS2::GetEntityList();
        if (!entityList) return;

        uintptr_t ctrlListEntry = CS2::Read<uintptr_t>(entityList + 0x10);
        if (!ctrlListEntry) return;

        for (int i = 0; i < 64 && spectatorCount < 64; i++) {
            uintptr_t ctrl = CS2::Read<uintptr_t>(ctrlListEntry + CS2::ENTRY_SIZE * (i & 0x1FF));
            if (!ctrl) continue;
            uintptr_t localCtrl = CS2::GetLocalPlayerController();
            if (ctrl == localCtrl) continue;
            uint32_t pawnHandle = CS2::Read<uint32_t>(ctrl + CS2::controller::m_hPlayerPawn);
            if (!pawnHandle) continue;

            uintptr_t pChunk = CS2::Read<uintptr_t>(entityList + 8 * ((pawnHandle & 0x7FFF) >> 9) + 0x10);
            if (!pChunk) continue;
            uintptr_t pawn = CS2::Read<uintptr_t>(pChunk + CS2::ENTRY_SIZE * (pawnHandle & 0x1FF));
            if (!pawn) continue;
            uintptr_t obsSvc = CS2::Read<uintptr_t>(pawn + Game::pawn::m_pObserverServices);
            if (!obsSvc) continue;

            uint8_t obsMode = CS2::Read<uint8_t>(obsSvc + Game::observer::m_iObserverMode);
            if (obsMode < 4) continue;
            uint32_t targetHandle = CS2::Read<uint32_t>(obsSvc + Game::observer::m_hObserverTarget);
            if (!targetHandle) continue;
            uintptr_t tChunk = CS2::Read<uintptr_t>(entityList + 8 * ((targetHandle & 0x7FFF) >> 9) + 0x10);
            if (!tChunk) continue;
            uintptr_t targetPawn = CS2::Read<uintptr_t>(tChunk + CS2::ENTRY_SIZE * (targetHandle & 0x1FF));
            if (targetPawn != localPawn) continue;
            SpectatorInfo& si = spectators[spectatorCount];
            si.mode = obsMode;
            uintptr_t namePtr = CS2::Read<uintptr_t>(ctrl + CS2::controller::m_sSanitizedPlayerName);
            if (namePtr) {
                __try { strncpy_s(si.name, (const char*)namePtr, 63); }
                __except (EXCEPTION_EXECUTE_HANDLER) { strcpy_s(si.name, "?"); }
            } else {
                strcpy_s(si.name, "?");
            }
            spectatorCount++;
        }
    }

    inline void Render(bool menuOpen) {
        if (!Settings::spectator_list || !CS2::initialized) return;
        if (spectatorCount == 0 && !menuOpen) return;

        ImGuiIO& io = ImGui::GetIO();
        float sw = io.DisplaySize.x;

        if (!pos_init) {
            pos = ImVec2(sw - 220, 80);
            pos_init = true;
        }

        float panelW = 200.0f;
        float headerH = 28.0f;
        float rowH = 22.0f;
        float panelH = headerH + (spectatorCount > 0 ? spectatorCount * rowH + 6 : rowH);

        ImDrawList* dl = ImGui::GetBackgroundDrawList();
        if (menuOpen) {
            ImVec2 mouse = io.MousePos;
            static bool dragging = false;
            static ImVec2 dragOffset;
            ImVec2 p0 = pos;
            ImVec2 p1 = ImVec2(pos.x + panelW, pos.y + headerH);
            bool hovered = (mouse.x >= p0.x && mouse.x <= p1.x && mouse.y >= p0.y && mouse.y <= p1.y);
            if (hovered && ImGui::IsMouseClicked(0)) {
                dragging = true;
                dragOffset = ImVec2(mouse.x - pos.x, mouse.y - pos.y);
            }
            if (!ImGui::IsMouseDown(0)) dragging = false;
            if (dragging) {
                pos.x = mouse.x - dragOffset.x;
                pos.y = mouse.y - dragOffset.y;
            }
        }
        dl->AddRectFilled(ImVec2(pos.x, pos.y), ImVec2(pos.x + panelW, pos.y + panelH),
            IM_COL32(12, 12, 12, 220), 5.0f);
        dl->AddRect(ImVec2(pos.x, pos.y), ImVec2(pos.x + panelW, pos.y + panelH),
            IM_COL32(45, 45, 45, 80), 5.0f);
        dl->AddRectFilled(ImVec2(pos.x, pos.y), ImVec2(pos.x + panelW, pos.y + headerH),
            IM_COL32(165, 211, 50, 40), 5.0f);

        char header[32];
        sprintf_s(header, "\xEE\x80\xA8 Spectators [%d]", spectatorCount);
        ImVec2 hts = ImGui::CalcTextSize(header);
        dl->AddText(ImVec2(pos.x + (panelW - hts.x) * 0.5f, pos.y + (headerH - hts.y) * 0.5f),
            IM_COL32(165, 211, 50, 255), header);
        if (spectatorCount == 0) {
            dl->AddText(ImVec2(pos.x + 10, pos.y + headerH + 2),
                IM_COL32(100, 100, 120, 180), "No spectators");
        } else {
            for (int i = 0; i < spectatorCount; i++) {
                float ry = pos.y + headerH + 3 + i * rowH;
                const char* modeStr = ObsModeName(spectators[i].mode);
                ImU32 modeCol = (spectators[i].mode == 4) ? IM_COL32(255, 80, 80, 200) :
                                (spectators[i].mode == 5) ? IM_COL32(255, 180, 50, 200) :
                                                            IM_COL32(100, 200, 100, 200);
                dl->AddText(ImVec2(pos.x + panelW - 35, ry), modeCol, modeStr);
                dl->AddText(ImVec2(pos.x + 10, ry), IM_COL32(220, 220, 230, 240), spectators[i].name);
            }
        }
    }
}

namespace BombTimer
{
    inline ImVec2 pos = ImVec2(-1, -1);
    inline bool pos_init = false;
    inline bool wasBombActive = false;
    inline DWORD bombStartMs = 0;
    inline float cachedTimerLen = 40.0f;

    inline bool wasDefusing = false;
    inline DWORD defuseStartMs = 0;
    inline float cachedDefuseLen = 10.0f;

    inline void Render(bool menuOpen) {
        if (!Settings::show_bombtimer || !CS2::initialized) return;

        uintptr_t c4 = CS2::GetPlantedC4();
        bool bombActive = false;
        float timeLeft = 0;
        bool defusing = false;
        float defuseTime = 0;
        bool defused = false;
        int site = 0;

        float timerLength = 40.0f;
        float defuseLength = 10.0f;

        if (c4) {
            bool ticking = CS2::Read<bool>(c4 + CS2::bomb::m_bBombTicking);
            defused = CS2::Read<bool>(c4 + CS2::bomb::m_bBombDefused);
            if (ticking && !defused) {
                float tl = CS2::Read<float>(c4 + CS2::bomb::m_flTimerLength);
                if (tl > 0 && tl < 120.0f) { timerLength = tl; cachedTimerLen = tl; }
                else timerLength = cachedTimerLen;

                site = CS2::Read<int>(c4 + CS2::bomb::m_nBombSite);
                if (site < 0 || site > 1) site = 0;
                DWORD now = GetTickCount();
                if (!wasBombActive) {
                    bombStartMs = now;
                    wasBombActive = true;
                }

                float elapsed = (now - bombStartMs) / 1000.0f;
                timeLeft = timerLength - elapsed;
                if (timeLeft < 0) timeLeft = 0;
                bombActive = true;
                bool beingDefused = CS2::Read<bool>(c4 + CS2::bomb::m_bBeingDefused);
                if (beingDefused) {
                    float dl = CS2::Read<float>(c4 + CS2::bomb::m_flDefuseLength);
                    if (dl > 0 && dl < 30.0f) { defuseLength = dl; cachedDefuseLen = dl; }
                    else defuseLength = cachedDefuseLen;

                    if (!wasDefusing) {
                        defuseStartMs = now;
                        wasDefusing = true;
                    }
                    float defElapsed = (now - defuseStartMs) / 1000.0f;
                    defuseTime = defuseLength - defElapsed;
                    if (defuseTime < 0) defuseTime = 0;
                    defusing = true;
                } else {
                    wasDefusing = false;
                }
            } else {
                wasBombActive = false;
                wasDefusing = false;
            }
        } else {
            wasBombActive = false;
            wasDefusing = false;
        }

        if (!bombActive && !menuOpen) return;

        ImGuiIO& io = ImGui::GetIO();
        float panelW = 210.0f;
        float panelH = bombActive ? (defusing ? 120.0f : 95.0f) : 60.0f;

        if (!pos_init) { pos = ImVec2(io.DisplaySize.x - panelW - 15, 200); pos_init = true; }

        auto drawContent = [&](ImDrawList* dl, float px, float py) {
            auto srv = Blur::GetSRV();
            if (srv && io.DisplaySize.x > 0) {
                ImVec2 u0(px / io.DisplaySize.x, py / io.DisplaySize.y);
                ImVec2 u1((px + panelW) / io.DisplaySize.x, (py + panelH) / io.DisplaySize.y);
                dl->AddImageRounded((ImTextureID)srv,
                    ImVec2(px, py), ImVec2(px + panelW, py + panelH),
                    u0, u1, IM_COL32(255, 255, 255, 255), 10.0f);
            }
            dl->AddRectFilled(ImVec2(px, py), ImVec2(px + panelW, py + panelH),
                IM_COL32(12, 12, 12, 185), 10.0f);
            dl->AddRect(ImVec2(px, py), ImVec2(px + panelW, py + panelH),
                IM_COL32(45, 45, 45, 100), 10.0f);

            const char* title = "Bomb Timer";
            ImVec2 tsz = ImGui::CalcTextSize(title);
            dl->AddText(ImVec2(px + (panelW - tsz.x) * 0.5f, py + 5),
                IM_COL32(165, 211, 50, 255), title);
            dl->AddLine(ImVec2(px + 10, py + 28),
                ImVec2(px + panelW - 10, py + 28), IM_COL32(165, 211, 50, 120));

            if (!bombActive) {
                ImVec2 ns = ImGui::CalcTextSize("No bomb planted");
                dl->AddText(ImVec2(px + (panelW - ns.x) * 0.5f, py + 35),
                    IM_COL32(138, 143, 158, 160), "No bomb planted");
                return;
            }
            char siteBuf[16]; sprintf_s(siteBuf, "Site: %s", site == 0 ? "A" : "B");
            dl->AddText(ImVec2(px + 14, py + 34), IM_COL32(180, 185, 195, 240), siteBuf);
            float barY = py + 54;
            float barW = panelW - 28;
            float frac = timeLeft / timerLength; if (frac > 1) frac = 1;
            ImU32 barCol = timeLeft < 10 ? IM_COL32(255, 60, 60, 220) :
                           timeLeft < 20 ? IM_COL32(255, 180, 30, 220) :
                                           IM_COL32(165, 211, 50, 220);
            dl->AddRectFilled(ImVec2(px + 14, barY), ImVec2(px + 14 + barW, barY + 6),
                IM_COL32(30, 35, 45, 200), 3.0f);
            dl->AddRectFilled(ImVec2(px + 14, barY), ImVec2(px + 14 + barW * frac, barY + 6),
                barCol, 3.0f);
            char timeBuf[32]; sprintf_s(timeBuf, "%.1fs", timeLeft);
            ImVec2 tts = ImGui::CalcTextSize(timeBuf);
            dl->AddText(ImVec2(px + panelW - tts.x - 14, py + 34), barCol, timeBuf);
            if (defusing) {
                float dy = py + 70;
                bool canDefuse = defuseTime <= timeLeft;
                ImU32 defCol = canDefuse ? IM_COL32(90, 220, 90, 240) : IM_COL32(255, 60, 60, 240);
                dl->AddText(ImVec2(px + 14, dy), defCol,
                    canDefuse ? "Defusing..." : "Can't defuse!");

                char dbuf[32]; sprintf_s(dbuf, "%.1fs", defuseTime);
                ImVec2 dsz = ImGui::CalcTextSize(dbuf);
                dl->AddText(ImVec2(px + panelW - dsz.x - 14, dy), defCol, dbuf);
                float dfrac = defuseTime / defuseLength; if (dfrac > 1) dfrac = 1;
                float dby = dy + 22;
                dl->AddRectFilled(ImVec2(px + 14, dby), ImVec2(px + 14 + barW, dby + 4),
                    IM_COL32(30, 35, 45, 200), 2.0f);
                dl->AddRectFilled(ImVec2(px + 14, dby), ImVec2(px + 14 + barW * dfrac, dby + 4),
                    defCol, 2.0f);
            }
        };

        if (menuOpen) {
            ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0, 0, 0, 0));
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0, 0, 0, 0));
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
            ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0);
            ImGui::SetNextWindowPos(pos, ImGuiCond_Once);
            ImGui::SetNextWindowSize(ImVec2(panelW, panelH));
            ImGui::Begin("##bt_drag", nullptr,
                ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse |
                ImGuiWindowFlags_NoBackground);
            pos = ImGui::GetWindowPos();
            drawContent(ImGui::GetWindowDrawList(), pos.x, pos.y);
            ImGui::End();
            ImGui::PopStyleVar(2);
            ImGui::PopStyleColor(2);
        } else if (bombActive) {
            drawContent(ImGui::GetBackgroundDrawList(), pos.x, pos.y);
        }
    }
}


