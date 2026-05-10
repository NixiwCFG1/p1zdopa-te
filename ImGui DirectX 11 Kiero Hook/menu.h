#pragma once
#include <cmath>
#include <stdio.h>
#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"
#include "cs2.h"
#include "features.h"

namespace Menu
{
    inline bool show_menu = true;
    static int active_tab = 0;
    inline void HotkeyButton(const char* settingName, bool* settingPtr)
    {
        int bindIdx = Hotkeys::FindBind(settingPtr);
        if (bindIdx < 0) return;

        auto& bind = Hotkeys::binds[bindIdx];
        const char* keyName = Hotkeys::VkName(bind.vk);

        ImGui::SameLine();
        char btnLabel[32];
        if (Hotkeys::rebind_idx == bindIdx) {
            sprintf_s(btnLabel, "[...]");
        } else if (keyName) {
            sprintf_s(btnLabel, "[%s]", keyName);
        } else {
            sprintf_s(btnLabel, "[+]");
        }

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
        ImGui::PushStyleColor(ImGuiCol_Text, ImColor(100, 100, 100).Value);
        if (ImGui::SmallButton(btnLabel)) {
            if (bind.vk != 0) bind.vk = 0;
            else Hotkeys::StartRebind(settingPtr);
        }
        ImGui::PopStyleColor(2);
    }
    inline void BeginGroupbox(const char* name, ImVec2 size)
    {
        ImGui::BeginChild(name, size, true, ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoScrollbar);
        
        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        ImVec2 p = ImGui::GetCursorScreenPos();
        ImVec2 text_size = ImGui::CalcTextSize(name);
        draw_list->AddRectFilled(ImVec2(p.x + 10, p.y - 18), ImVec2(p.x + 15 + text_size.x, p.y - 2), ImColor(12, 12, 12));
        draw_list->AddText(ImVec2(p.x + 12, p.y - 18), ImColor(150, 150, 150), name);

        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 5);
    }

    inline void EndGroupbox()
    {
        ImGui::EndChild();
    }

    inline void Render()
    {
        if (!show_menu) return;
        static bool binds_init = false;
        if (!binds_init) {
            Hotkeys::RegisterBind("Aimbot", &Settings::aim_enabled, 0, 2, Hotkeys::CATEGORY_COMBAT);
            Hotkeys::RegisterBind("Triggerbot", &Settings::trigger_enabled, 0, 2, Hotkeys::CATEGORY_COMBAT);
            Hotkeys::RegisterBind("ESP", &Settings::esp_enabled, 0, 1, Hotkeys::CATEGORY_VISUALS);
            Hotkeys::RegisterBind("Glow", &Settings::esp_glow, 0, 1, Hotkeys::CATEGORY_VISUALS);
            Hotkeys::RegisterBind("Bunny Hop", &Settings::bhop_enabled, 0, 2, Hotkeys::CATEGORY_MISC);
            Hotkeys::RegisterBind("Auto Strafe", &Settings::autostrafe, 0, 2, Hotkeys::CATEGORY_MISC);
            binds_init = true;
        }

        Hotkeys::ProcessRebind();
        CS2::UpdateProfile();
        ImGuiStyle& style = ImGui::GetStyle();
        style.WindowPadding = ImVec2(0, 0);
        style.WindowRounding = 0.0f;
        style.FrameRounding = 0.0f;
        style.ChildRounding = 0.0f;
        style.ItemSpacing = ImVec2(8, 12);

        style.Colors[ImGuiCol_WindowBg] = ImColor(12, 12, 12);
        style.Colors[ImGuiCol_Border] = ImColor(45, 45, 45);
        style.Colors[ImGuiCol_FrameBg] = ImColor(25, 25, 25);
        style.Colors[ImGuiCol_CheckMark] = ImColor(165, 211, 50);
        style.Colors[ImGuiCol_SliderGrab] = ImColor(165, 211, 50);
        style.Colors[ImGuiCol_SliderGrabActive] = ImColor(185, 231, 70);

        ImGui::SetNextWindowSize(ImVec2(600, 450), ImGuiCond_Once);
        ImGui::Begin("GamesenseMenu", &show_menu, ImGuiWindowFlags_NoDecoration);
        {
            ImDrawList* draw_list = ImGui::GetWindowDrawList();
            ImVec2 p = ImGui::GetWindowPos();
            draw_list->AddRectFilled(p, ImVec2(p.x + 600, p.y + 2), ImColor(165, 211, 50));
            draw_list->AddLine(ImVec2(p.x + 100, p.y + 2), ImVec2(p.x + 100, p.y + 450), ImColor(45, 45, 45));
            ImGui::SetCursorPos(ImVec2(0, 20));
            ImGui::BeginChild("##tabs", ImVec2(100, 430), false, ImGuiWindowFlags_NoBackground);
            {
                const char* tab_names[] = { "legit", "visuals", "misc", "prikol" };
                for (int i = 0; i < 4; i++) {
                    bool selected = (active_tab == i);

                    ImGui::SetCursorPosX(15);
                    if (selected) ImGui::PushStyleColor(ImGuiCol_Text, ImColor(165, 211, 50).Value);
                    else ImGui::PushStyleColor(ImGuiCol_Text, ImColor(150, 150, 150).Value);
                    if (selected) {
                        ImVec2 cur = ImGui::GetCursorScreenPos();
                        draw_list->AddRect(ImVec2(cur.x - 5, cur.y - 2), ImVec2(cur.x + 75, cur.y + 32), ImColor(165, 211, 50, 100));
                    }

                    if (ImGui::Selectable(tab_names[i], false, 0, ImVec2(75, 30))) {
                        active_tab = i;
                    }

                    ImGui::PopStyleColor();
                    ImGui::Spacing();
                }
            }
            ImGui::EndChild();
            ImGui::SetCursorPos(ImVec2(115, 25));
            ImGui::BeginChild("##content_area", ImVec2(475, 415), false, ImGuiWindowFlags_NoBackground);
            {
                if (active_tab == 0) {
                    ImGui::BeginGroup();
                    {
                        BeginGroupbox("Aimbot", ImVec2(225, 210));
                        ImGui::Checkbox("Enabled", &Settings::aim_enabled);
                        HotkeyButton("Aimbot", &Settings::aim_enabled);
                        
                        ImGui::Text("Smooth");
                        ImGui::PushItemWidth(-1);
                        ImGui::SliderFloat("##smooth", &Settings::aim_smooth, 1.0f, 20.0f, "%.1f");
                        ImGui::PopItemWidth();

                        ImGui::Text("Maximum FOV");
                        ImGui::PushItemWidth(-1);
                        ImGui::SliderFloat("##fov", &Settings::aim_fov, 1.0f, 30.0f, "%.1f");
                        ImGui::PopItemWidth();
                        EndGroupbox();
                    }
                    ImGui::EndGroup();

                    ImGui::SameLine(0, 15);

                    ImGui::BeginGroup();
                    {
                        BeginGroupbox("Triggerbot", ImVec2(225, 130));
                        ImGui::Checkbox("Enabled ", &Settings::trigger_enabled);
                        HotkeyButton("Triggerbot", &Settings::trigger_enabled);

                        ImGui::Text("Delay (ms)");
                        ImGui::PushItemWidth(-1);
                        ImGui::SliderInt("##delay", &Settings::trigger_delay, 0, 200, "%d");
                        ImGui::PopItemWidth();
                        EndGroupbox();
                    }
                    ImGui::EndGroup();
                }
                else if (active_tab == 1) {
                    BeginGroupbox("ESP Main", ImVec2(225, 250));
                    ImGui::Checkbox("Enable ESP", &Settings::esp_enabled);
                    ImGui::Checkbox("Box", &Settings::esp_box);
                    ImGui::Checkbox("Health", &Settings::esp_health);
                    ImGui::Checkbox("Names", &Settings::esp_name);
                    ImGui::Checkbox("Skeleton", &Settings::esp_skeleton);
                    ImGui::Checkbox("Snaplines", &Settings::esp_snaplines);
                    EndGroupbox();

                    ImGui::SameLine(0, 15);

                    BeginGroupbox("Glow", ImVec2(225, 200));
                    ImGui::Checkbox("Enable Glow", &Settings::esp_glow);
                    ImGui::Text("Enemy Color");
                    ImGui::ColorEdit4("##glow_enemy", Settings::esp_glow_color_enemy);
                    ImGui::Text("Team Color");
                    ImGui::ColorEdit4("##glow_team", Settings::esp_glow_color_team);
                    EndGroupbox();

                    BeginGroupbox("World", ImVec2(465, 100));
                    ImGui::Checkbox("No Flash", &Settings::noflash_enabled);
                    ImGui::Checkbox("Radar Hack", &Settings::radar_hack);
                    ImGui::Checkbox("Crosshair", &Settings::crosshair_enabled);
                    EndGroupbox();
                }
                else if (active_tab == 2) {
                    BeginGroupbox("Movement", ImVec2(225, 130));
                    ImGui::Checkbox("Bunny Hop", &Settings::bhop_enabled);
                    ImGui::Checkbox("Auto Strafe", &Settings::autostrafe);
                    EndGroupbox();

                    ImGui::SameLine(0, 15);

                    BeginGroupbox("View", ImVec2(225, 130));
                    ImGui::Checkbox("FOV Changer", &Settings::fov_changer);
                    ImGui::Text("FOV Value");
                    ImGui::PushItemWidth(-1);
                    ImGui::SliderInt("##fov_value", &Settings::fov_value, 60, 150, "%d");
                    ImGui::PopItemWidth();
                    EndGroupbox();

                }
                else if (active_tab == 3) {
                    BeginGroupbox("Warning", ImVec2(465, 85));
                    ImGui::Checkbox("warning", &Settings::warning_enabled);
                    EndGroupbox();

                    BeginGroupbox("Trashtalk", ImVec2(465, 85));
                    ImGui::Checkbox("Enabled", &Settings::trashtalk_enabled);
                    EndGroupbox();

                    BeginGroupbox("Bitcoin Price", ImVec2(465, 100));
                    ImGui::Text("BTC/USD: $%.2f", Settings::bitcoin_price);
                    ImGui::Text("Last updated: %s", Settings::bitcoin_last_update.c_str());
                    EndGroupbox();

                    BeginGroupbox("Nigeria Weather", ImVec2(465, 100));
                    ImGui::Text("Temperature: %.1f°C", Settings::nigeria_temp);
                    ImGui::Text("Condition: %s", Settings::nigeria_condition.c_str());
                    ImGui::Text("Last updated: %s", Settings::nigeria_last_update.c_str());
                    EndGroupbox();

                }
            }
            ImGui::EndChild();
        }
        ImGui::End();
    }
}
