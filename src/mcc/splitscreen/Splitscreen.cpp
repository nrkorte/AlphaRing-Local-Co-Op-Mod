#include "Splitscreen.h"

#include "common.h"

#include "global/Global.h"

#include <offset_mcc.h>

#include "../CGameManager.h"

namespace MCC::Splitscreen {
    DefDetourFunction(__int64, __fastcall, get_index_by_xuid, void* a1, __int64 xuid) {
        auto p_setting = AlphaRing::Global::MCC::Splitscreen();

        if (!p_setting->b_override)
            return ppOriginal_get_index_by_xuid(a1, xuid);

        return CGameManager::get_index(xuid);
    }

    // todo:: let other players have the ability to pause the game

    bool Initialize() {
        bool result;

        // fix: changing team freeze the game
        result = AlphaRing::Hook::Detour({
            {0x38A09C/*0x2D01DC*/, 0x374164/*0x2BD620*/, get_index_by_xuid, (void**)&ppOriginal_get_index_by_xuid},
        });

        assertm(result, "MCC:Splitscreen: failed to hook");

        LoadSettings();

        return true;
    }
}

#include "nlohmann/json.hpp"

#include <fstream>
#include <filesystem>

namespace MCC::Splitscreen {
    using json = nlohmann::json;

    static const char* settings_path = "../../../alpha_ring/splitscreen.json";

    static std::string to_utf8(const wchar_t* str) {
        int size = WideCharToMultiByte(CP_UTF8, 0, str, -1, nullptr, 0, nullptr, nullptr);
        if (size <= 1) return {};
        std::string result(size - 1, '\0');
        WideCharToMultiByte(CP_UTF8, 0, str, -1, result.data(), size, nullptr, nullptr);
        return result;
    }

    static void from_utf8(wchar_t* dest, size_t n, const std::string& str) {
        if (MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, dest, (int)n) == 0)
            dest[0] = L'\0';
        dest[n - 1] = L'\0';
    }

    static std::string to_hex(const void* data, size_t size) {
        static const char digits[] = "0123456789ABCDEF";
        auto bytes = (const unsigned char*)data;
        std::string result(size * 2, '0');
        for (size_t i = 0; i < size; ++i) {
            result[i * 2] = digits[bytes[i] >> 4];
            result[i * 2 + 1] = digits[bytes[i] & 0xF];
        }
        return result;
    }

    // leaves dest untouched unless the hex string is valid and exactly the right size
    static bool from_hex(void* dest, size_t size, const std::string& hex) {
        const auto nibble = [](char c) -> int {
            if (c >= '0' && c <= '9') return c - '0';
            if (c >= 'A' && c <= 'F') return c - 'A' + 10;
            if (c >= 'a' && c <= 'f') return c - 'a' + 10;
            return -1;
        };

        if (hex.size() != size * 2) return false;

        std::vector<unsigned char> bytes(size);
        for (size_t i = 0; i < size; ++i) {
            int hi = nibble(hex[i * 2]), lo = nibble(hex[i * 2 + 1]);
            if (hi < 0 || lo < 0) return false;
            bytes[i] = (unsigned char)((hi << 4) | lo);
        }

        memcpy(dest, bytes.data(), size);
        return true;
    }

    bool LoadSettings() {
        wchar_t path[MAX_PATH];
        auto p_setting = AlphaRing::Global::MCC::Splitscreen();

        AlphaRing::Filesystem::GetDir(settings_path, path);

        std::ifstream file{std::filesystem::path(path)};
        if (!file.is_open()) return false;

        try {
            auto data = json::parse(file);

            p_setting->b_override = data.value("enabled", p_setting->b_override);
            p_setting->b_player0_use_km = data.value("player1_use_km", p_setting->b_player0_use_km);
            p_setting->b_override_profile = data.value("override_profile", p_setting->b_override_profile);
            p_setting->b_use_player0_profile = data.value("use_player1_profile", p_setting->b_use_player0_profile);

            int count = data.value("player_count", p_setting->player_count);
            if (count >= 1 && count <= 4)
                p_setting->player_count = count;

            if (data.contains("players") && data["players"].is_array()) {
                auto& players = data["players"];
                for (int i = 0; i < 4 && i < (int)players.size(); ++i) {
                    auto& player = players[i];
                    auto p_profile = CGameManager::get_profile(i);

                    if (!player.is_object()) continue;

                    if (player.contains("name") && player["name"].is_string())
                        from_utf8(p_profile->name, sizeof(p_profile->name) / sizeof(wchar_t), player["name"].get<std::string>());

                    int controller = player.value("controller", p_profile->controller_index);
                    if (controller >= 0 && controller <= 4)
                        p_profile->controller_index = controller;

                    if (player.contains("profile") && player["profile"].is_string())
                        from_hex(&p_profile->profile, sizeof(CUserProfile), player["profile"].get<std::string>());

                    if (player.contains("mapping") && player["mapping"].is_string())
                        from_hex(&p_profile->mapping, sizeof(CGamepadMapping), player["mapping"].get<std::string>());
                }
            }
        } catch (const std::exception& e) {
            LOG_ERROR("Splitscreen: failed to load settings: {}", e.what());
            return false;
        }

        LOG_INFO("Splitscreen: loaded settings");
        return true;
    }

    bool SaveSettings() {
        wchar_t path[MAX_PATH];
        auto p_setting = AlphaRing::Global::MCC::Splitscreen();

        json data = {
            {"enabled", p_setting->b_override},
            {"player_count", p_setting->player_count},
            {"player1_use_km", p_setting->b_player0_use_km},
            {"override_profile", p_setting->b_override_profile},
            {"use_player1_profile", p_setting->b_use_player0_profile},
            {"players", json::array()},
        };

        for (int i = 0; i < 4; ++i) {
            auto p_profile = CGameManager::get_profile(i);
            data["players"].push_back({
                {"name", to_utf8(p_profile->name)},
                {"controller", p_profile->controller_index},
                {"profile", to_hex(&p_profile->profile, sizeof(CUserProfile))},
                {"mapping", to_hex(&p_profile->mapping, sizeof(CGamepadMapping))},
            });
        }

        AlphaRing::Filesystem::GetDir(settings_path, path);

        std::ofstream file{std::filesystem::path(path), std::ios::trunc};
        if (!file.is_open()) {
            LOG_ERROR("Splitscreen: failed to open settings file for writing");
            return false;
        }

        file << data.dump(4);
        file.close();

        if (file.fail()) {
            LOG_ERROR("Splitscreen: failed to write settings");
            return false;
        }

        LOG_INFO("Splitscreen: saved settings");
        return true;
    }
}

#include "imgui.h"
#include "mcc/mcc.h"

#include <string>

namespace MCC::Splitscreen {
    void RealContext();

    void ImGuiContext() {
        static bool show_splitscreen;

        if (ImGui::BeginMainMenuBar()) {
            ImGui::MenuItem("Splitscreen", nullptr, &show_splitscreen);
            ImGui::EndMainMenuBar();
        }

        if (show_splitscreen) {
            if (ImGui::Begin("Splitscreen", &show_splitscreen, ImGuiWindowFlags_MenuBar))
                RealContext();
            ImGui::End();
        }
    }

    void ProfileContext(int index) {
        char buffer[1024];
        auto p_setting = AlphaRing::Global::MCC::Splitscreen();
        auto p_profile = CGameManager::get_profile(index);
        const char* items[] = {"Controller 1", "Controller 2", "Controller 3", "Controller 4", "NONE"};

        ImGui::PushItemWidth(200);
        String::convert(buffer, p_profile->name, 1024);
        if (ImGui::InputText("Name", buffer, sizeof(buffer)))
            String::convert(p_profile->name, buffer, 1024);
        ImGui::PopItemWidth();

        ImGui::BeginDisabled(!index && p_setting->b_player0_use_km);
        ImGui::PushItemWidth(200);ImGui::Combo("Input", &p_profile->controller_index, items, IM_ARRAYSIZE(items));ImGui::PopItemWidth();
        ImGui::EndDisabled();

        if (ImGui::Button("Load Profile")) {
            __int64 xuid;
            auto p_mng = GameManager();
            auto p_engine = GameEngine();
            if (MCC::IsInGame() && p_mng && (xuid = CGameManager::get_xuid(0))) {
                memcpy(&p_profile->profile, p_mng->ppOriginal.get_player_profile(p_mng, xuid), sizeof(CUserProfile));
                memcpy(&p_profile->mapping, p_mng->ppOriginal.retrive_gamepad_mapping(p_mng, xuid), sizeof(CGamepadMapping));
                if (p_engine)
                    p_engine->load_setting();
            }
        }
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("Use this in game!!!");

        bool is_disabled = (!index && !p_setting->b_override_profile) || (index && p_setting->b_use_player0_profile);

        if (ImGui::CollapsingHeader("Gamepad Mapping")) {
            ImGui::Indent();
            ImGui::BeginDisabled(is_disabled);
            p_profile->mapping.ImGuiContext();
            ImGui::EndDisabled();
            ImGui::Unindent();
        }

        if (ImGui::CollapsingHeader("Profile")) {
            ImGui::Indent();
            ImGui::BeginDisabled(is_disabled);
            p_profile->profile.ImGuiContext();
            ImGui::EndDisabled();
            ImGui::Unindent();
        }
    }

    void RealContext() {
        char buffer[10];
        static const char* save_status = nullptr;
        static double save_status_time = 0;
        auto p_setting = AlphaRing::Global::MCC::Splitscreen();

        if (ImGui::BeginMenuBar()) {
            ImGui::MenuItem(p_setting->b_override ? "Disable" : "Enable", nullptr, &p_setting->b_override);
            if (ImGui::MenuItem("Save")) {
                save_status = SaveSettings() ? "Saved" : "Save failed";
                save_status_time = ImGui::GetTime();
            }
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("Save splitscreen settings, player names, controllers and loaded profiles.\nThey are loaded automatically at startup.");
            if (ImGui::BeginMenu("Options")) {
                ImGui::MenuItem("Use player1's profile", nullptr, &p_setting->b_use_player0_profile);
                if (ImGui::MenuItem("Enable K/M for player1", nullptr, &p_setting->b_player0_use_km))
                    CGameManager::assign_default_controllers(p_setting->b_player0_use_km);
                ImGui::MenuItem("Override profile", nullptr, &p_setting->b_override_profile);
                ImGui::EndMenu();
            }
#pragma region player count
            ImGui::PushItemWidth(200);
            int count = p_setting->player_count;
            if (ImGui::InputInt("Players", &count) && count >= 1 && count <=4) {
                p_setting->player_count = count;
            }
            ImGui::PopItemWidth();
            if (save_status && ImGui::GetTime() - save_status_time < 3.0)
                ImGui::TextUnformatted(save_status);
            ImGui::EndMenuBar();
#pragma endregion
        }

        if (ImGui::BeginTabBar("Players")) {
            for (int i = 0; i < p_setting->player_count; ++i) {
                sprintf(buffer, "Player %d", i + 1);
                if (ImGui::BeginTabItem(buffer)) {
                    ProfileContext(i);
                    ImGui::EndTabItem();
                }
            }
            ImGui::EndTabBar();
        }
    }
}