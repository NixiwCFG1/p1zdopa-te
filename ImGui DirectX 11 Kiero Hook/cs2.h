#pragma once
#include <Windows.h>
#include <cstdint>
#include <d3d11.h>
#include <string>

namespace CS2
{
    namespace offsets {
        constexpr uintptr_t dwEntityList = 0x24D0DC0;
        constexpr uintptr_t dwLocalPlayerController = 0x230A4F0;
        constexpr uintptr_t dwLocalPlayerPawn = 0x2056700;
        constexpr uintptr_t dwViewRender = 0x232FCD0;
        constexpr uintptr_t dwViewMatrix = 0x2330AE0;
        constexpr uintptr_t dwGlobalVars = 0x204B5D8;
        constexpr uintptr_t dwGameEntitySystem = 0x24D0DC0;
        constexpr uintptr_t dwGameEntitySystem_highestEntityIndex = 0x2090;
        constexpr uintptr_t dwGlowManager = 0x2327D40;
        constexpr uintptr_t dwViewAngles = 0x2340288;
        constexpr uintptr_t dwPlantedC4 = 0x2338A68;
        constexpr uintptr_t dwGameRules = 0x232AF48;
        constexpr uintptr_t dwCSGOInput = 0x233FC00;
    }
    namespace csgo_input {
        constexpr uintptr_t m_nCamCommand = 0x58;
    }
    constexpr size_t ENTRY_SIZE = 0x70;

    inline uintptr_t clientBase = 0;
    inline uintptr_t engine2Base = 0;
    inline bool initialized = false;

    template<typename T>
    inline T Read(uintptr_t addr) {
        __try { return *(T*)addr; }
        __except (EXCEPTION_EXECUTE_HANDLER) { return T{}; }
    }

    template<typename T>
    inline void Write(uintptr_t addr, T val) {
        __try { *(T*)addr = val; }
        __except (EXCEPTION_EXECUTE_HANDLER) {}
    }

    inline bool Init() {
        clientBase = (uintptr_t)GetModuleHandleA("client.dll");
        engine2Base = (uintptr_t)GetModuleHandleA("engine2.dll");
        initialized = (clientBase != 0);
        return initialized;
    }

    inline uintptr_t GetEntityList() {
        if (!clientBase) return 0;
        return Read<uintptr_t>(clientBase + offsets::dwEntityList);
    }

    inline uintptr_t GetEntityByIndex(int index) {
        uintptr_t entityList = GetEntityList();
        if (!entityList) return 0;
        uintptr_t listEntry = Read<uintptr_t>(entityList + 8 * ((index & 0x7FFF) >> 9) + 16);
        if (!listEntry) return 0;
        return Read<uintptr_t>(listEntry + ENTRY_SIZE * (index & 0x1FF));
    }

    inline uintptr_t GetLocalPlayerController() {
        if (!clientBase) return 0;
        return Read<uintptr_t>(clientBase + offsets::dwLocalPlayerController);
    }

    inline uintptr_t GetLocalPlayerPawn() {
        if (!clientBase) return 0;
        return Read<uintptr_t>(clientBase + offsets::dwLocalPlayerPawn);
    }

    inline int GetHighestEntityIndex() {
        uintptr_t ges = GetEntityList();
        if (!ges) return 0;
        return Read<int>(clientBase + offsets::dwGameEntitySystem_highestEntityIndex);
    }
    namespace bomb {
        constexpr uintptr_t m_bBombTicking = 0x1160;
        constexpr uintptr_t m_nBombSite = 0x1164;
        constexpr uintptr_t m_flC4Blow = 0x1190;
        constexpr uintptr_t m_flTimerLength = 0x1198;
        constexpr uintptr_t m_bBeingDefused = 0x119C;
        constexpr uintptr_t m_flDefuseLength = 0x11AC;
        constexpr uintptr_t m_flDefuseCountDown = 0x11B0;
        constexpr uintptr_t m_bBombDefused = 0x11B4;
        constexpr uintptr_t m_hBombDefuser = 0x11B8;
    }
    namespace gamerules {
        constexpr uintptr_t m_bBombPlanted = 0x1078;
        constexpr uintptr_t m_bFreezePeriod = 0x40;
        constexpr uintptr_t m_bWarmupPeriod = 0x41;
    }

    inline uintptr_t GetGameRules() {
        if (!clientBase) return 0;
        return Read<uintptr_t>(clientBase + offsets::dwGameRules);
    }

    inline bool IsBombPlanted() {
        uintptr_t gr = GetGameRules();
        if (!gr) return false;
        return Read<bool>(gr + gamerules::m_bBombPlanted);
    }

    inline uintptr_t GetPlantedC4() {
        if (!clientBase) return 0;
        if (!IsBombPlanted()) return 0;
        uintptr_t ptr = Read<uintptr_t>(clientBase + offsets::dwPlantedC4);
        if (!ptr) return 0;
        uintptr_t ent = Read<uintptr_t>(ptr);
        if (!ent || ent < 0x10000) return 0;
        return ent;
    }
    namespace controller {
        constexpr uintptr_t m_sSanitizedPlayerName = 0x860;
        constexpr uintptr_t m_szClan = 0x858;
        constexpr uintptr_t m_iPawnHealth = 0x918;
        constexpr uintptr_t m_hPlayerPawn = 0x90C;
        constexpr uintptr_t m_iPing = 0x828;
        constexpr uintptr_t m_iCompetitiveRanking = 0x880;
        constexpr uintptr_t m_iScore = 0x934;
        constexpr uintptr_t m_bPawnIsAlive = 0x914;
    }
    inline const char* GetLocalName() {
        if (!initialized || !clientBase) return nullptr;
        uintptr_t ctrl = GetLocalPlayerController();
        if (!ctrl) return nullptr;
        uintptr_t nameStr = Read<uintptr_t>(ctrl + controller::m_sSanitizedPlayerName);
        if (!nameStr) return nullptr;
        __try {
            const char* name = (const char*)nameStr;
            if (name[0] == '\0') return nullptr;
            return name;
        }
        __except (EXCEPTION_EXECUTE_HANDLER) { return nullptr; }
    }
    inline ID3D11ShaderResourceView* avatarSRV = nullptr;
    inline bool avatarLoaded = false;
    inline int avatarRetries = 0;
    inline ID3D11Device* g_device = nullptr;

    inline void LoadSteamAvatar(ID3D11Device* device) {
        if (avatarLoaded || avatarRetries > 15) return;
        avatarRetries++;
        g_device = device;

        HMODULE hSteam = GetModuleHandleA("steam_api64.dll");
        if (!hSteam) return;

        typedef void* (__cdecl* fn_SteamUser)();
        typedef void* (__cdecl* fn_SteamFriends)();
        typedef void* (__cdecl* fn_SteamUtils)();

        auto pSteamUser = (fn_SteamUser)GetProcAddress(hSteam, "SteamUser");
        auto pSteamFriends = (fn_SteamFriends)GetProcAddress(hSteam, "SteamFriends");
        auto pSteamUtils = (fn_SteamUtils)GetProcAddress(hSteam, "SteamUtils");
        if (!pSteamUser || !pSteamFriends || !pSteamUtils) return;

        void* user = pSteamUser();
        void* friends = pSteamFriends();
        void* utils = pSteamUtils();
        if (!user || !friends || !utils) return;

        __try {
            typedef uint64_t(__thiscall* fn_GetSteamID)(void*);
            uint64_t steamID = ((fn_GetSteamID)(*(void***)user)[2])(user);
            typedef int(__thiscall* fn_GetSmallFriendAvatar)(void*, uint64_t);
            int avatarHandle = ((fn_GetSmallFriendAvatar)(*(void***)friends)[34])(friends, steamID);
            if (avatarHandle <= 0) return;
            uint32_t w = 0, h = 0;
            typedef bool(__thiscall* fn_GetImageSize)(void*, int, uint32_t*, uint32_t*);
            if (!((fn_GetImageSize)(*(void***)utils)[5])(utils, avatarHandle, &w, &h)) return;
            if (w == 0 || h == 0) return;
            int dataSize = w * h * 4;
            uint8_t* data = new uint8_t[dataSize];
            typedef bool(__thiscall* fn_GetImageRGBA)(void*, int, uint8_t*, int);
            bool ok = ((fn_GetImageRGBA)(*(void***)utils)[6])(utils, avatarHandle, data, dataSize);
            if (!ok) { delete[] data; return; }
            D3D11_TEXTURE2D_DESC td = {};
            td.Width = w; td.Height = h;
            td.MipLevels = 1; td.ArraySize = 1;
            td.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
            td.SampleDesc.Count = 1;
            td.Usage = D3D11_USAGE_DEFAULT;
            td.BindFlags = D3D11_BIND_SHADER_RESOURCE;

            D3D11_SUBRESOURCE_DATA sd = {};
            sd.pSysMem = data; sd.SysMemPitch = w * 4;

            ID3D11Texture2D* tex = nullptr;
            device->CreateTexture2D(&td, &sd, &tex);
            delete[] data;
            if (!tex) return;

            device->CreateShaderResourceView(tex, nullptr, &avatarSRV);
            tex->Release();
            if (avatarSRV) avatarLoaded = true;
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {}
    }
    inline void TryLoadAvatar() {
        if (avatarLoaded || !g_device || avatarRetries > 15) return;
        static int frameCounter = 0;
        if (++frameCounter % 120 == 0) {
            LoadSteamAvatar(g_device);
        }
    }
    inline char loaderName[64] = {};
    inline bool loaderProfileLoaded = false;
    inline bool loaderProfileChecked = false;

    inline bool LoadProfileFromLoader() {
        if (loaderProfileLoaded) return true;
        if (!g_device) return false;

        char appdata[MAX_PATH];
        if (!GetEnvironmentVariableA("APPDATA", appdata, MAX_PATH)) return false;

        std::string filePath = std::string(appdata) + "\\pizdopasta\\profile.dat";
        HANDLE hFile = CreateFileA(filePath.c_str(), GENERIC_READ, FILE_SHARE_READ,
            NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hFile == INVALID_HANDLE_VALUE) return false;

        DWORD fileSize = GetFileSize(hFile, NULL);
        if (fileSize < 80) { CloseHandle(hFile); return false; }

        uint8_t* data = new uint8_t[fileSize];
        DWORD bytesRead = 0;
        ReadFile(hFile, data, fileSize, &bytesRead, NULL);
        CloseHandle(hFile);

        if (bytesRead < 80) { delete[] data; return false; }
        uint32_t magic = *(uint32_t*)data;
        if (magic != 0x46504B43) { delete[] data; return false; }
        memcpy(loaderName, data + 8, 64);
        loaderName[63] = '\0';
        uint32_t w = *(uint32_t*)(data + 72);
        uint32_t h = *(uint32_t*)(data + 76);

        if (w > 0 && h > 0 && w <= 512 && h <= 512 && fileSize >= 80 + w * h * 4) {
            D3D11_TEXTURE2D_DESC td = {};
            td.Width = w; td.Height = h;
            td.MipLevels = 1; td.ArraySize = 1;
            td.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
            td.SampleDesc.Count = 1;
            td.Usage = D3D11_USAGE_DEFAULT;
            td.BindFlags = D3D11_BIND_SHADER_RESOURCE;

            D3D11_SUBRESOURCE_DATA sd = {};
            sd.pSysMem = data + 80;
            sd.SysMemPitch = w * 4;

            ID3D11Texture2D* tex = nullptr;
            g_device->CreateTexture2D(&td, &sd, &tex);
            if (tex) {
                if (avatarSRV) { avatarSRV->Release(); avatarSRV = nullptr; }
                g_device->CreateShaderResourceView(tex, nullptr, &avatarSRV);
                tex->Release();
                if (avatarSRV) avatarLoaded = true;
            }
        }

        delete[] data;
        loaderProfileLoaded = true;
        return true;
    }
    inline const char* GetDisplayName() {
        if (loaderProfileLoaded && loaderName[0] != '\0')
            return loaderName;
        const char* csName = GetLocalName();
        if (csName) return csName;
        return "Player";
    }
    inline void UpdateProfile() {
        if (!loaderProfileChecked && g_device) {
            loaderProfileChecked = true;
            LoadProfileFromLoader();
        }
        if (!loaderProfileLoaded) {
            TryLoadAvatar();
        }
    }
}
