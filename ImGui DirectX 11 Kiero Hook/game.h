#pragma once
#include "cs2.h"
#include <cmath>

struct Vec3 { float x, y, z; };
struct Vec2 { float x, y; };

namespace Game
{
    namespace entity {
        constexpr uintptr_t m_iHealth = 0x34C;
        constexpr uintptr_t m_iTeamNum = 0x3EB;
        constexpr uintptr_t m_lifeState = 0x354;
        constexpr uintptr_t m_pGameSceneNode = 0x330;
        constexpr uintptr_t m_fFlags = 0x3F8;
        constexpr uintptr_t m_vecAbsVelocity = 0x3FC;
    }
    namespace scene {
        constexpr uintptr_t m_vecAbsOrigin = 0xC8;
        constexpr uintptr_t m_bDormant = 0x103;
    }
    namespace pawn {
        constexpr uintptr_t m_pAimPunchServices = 0x1490;
        constexpr uintptr_t m_iShotsFired = 0x1C64;
        constexpr uintptr_t m_entitySpottedState = 0x11E0;
        constexpr uintptr_t m_bSpottedByMask = 0xC;
        constexpr uintptr_t m_bSpotted = 0x8;
        constexpr uintptr_t m_ArmorValue = 0x1C7C;
        constexpr uintptr_t m_flFlashMaxAlpha = 0x13FC;
        constexpr uintptr_t m_bIsScoped = 0x1C50;
        constexpr uintptr_t m_iIDEntIndex = 0x344C;
        constexpr uintptr_t m_pObserverServices = 0x11F8;
        constexpr uintptr_t m_pCameraServices = 0x1218;
        constexpr uintptr_t m_pMovementServices = 0x1220;
    }
    namespace weapon {
        constexpr uintptr_t m_pWeaponServices = 0x11E0;
        constexpr uintptr_t m_hActiveWeapon = 0x60;
        constexpr uintptr_t m_iClip1 = 0x16D8;
        constexpr uintptr_t m_iClip2 = 0x16DC;
        constexpr uintptr_t m_pReserveAmmo = 0x16E0;
        constexpr uintptr_t m_iItemDefinitionIndex = 0x1BA;
    }
    namespace observer {
        constexpr uintptr_t m_iObserverMode = 0x48;
        constexpr uintptr_t m_hObserverTarget = 0x4C;
    }
    namespace movement {
        constexpr uintptr_t m_flStamina = 0x69C;
        constexpr uintptr_t m_flFallVelocity = 0x25C;
    }
    namespace ctrl {
        constexpr uintptr_t m_iDesiredFOV = 0x78C;
    }
    namespace camera {
        constexpr uintptr_t m_vecCsViewPunchAngle = 0x48;
        constexpr uintptr_t m_iFOV = 0x290;
        constexpr uintptr_t m_iFOVStart = 0x294;
    }
    namespace buttons {
        constexpr uintptr_t dwForceJump = 0x204FEA0;
        constexpr uintptr_t dwForceAttack = 0x204F990;
        constexpr uintptr_t dwForceForward = 0x204FBD0;
        constexpr uintptr_t dwForceBack = 0x204FC60;
        constexpr uintptr_t dwForceLeft = 0x204FC60;
        constexpr uintptr_t dwForceRight = 0x204FD80;
    }
    namespace identity {
        constexpr uintptr_t m_pEntity = 0x10;
        constexpr uintptr_t m_designerName = 0x20;
    }
    namespace bones {
        constexpr uintptr_t m_modelState = 0x150;
        constexpr uintptr_t m_boneArray = 0x80;
    }
    enum BoneIndex : int {
        BONE_HEAD = 7,
        BONE_NECK = 6,
        BONE_CHEST = 4,
        BONE_PELVIS = 1,
        BONE_LEFT_SHOULDER = 13,
        BONE_RIGHT_SHOULDER = 39,
        BONE_LEFT_HAND = 15,
        BONE_RIGHT_HAND = 41,
        BONE_LEFT_KNEE = 73,
        BONE_RIGHT_KNEE = 82,
        BONE_LEFT_FOOT = 74,
        BONE_RIGHT_FOOT = 83,
    };

    struct BoneMatrix {
        float m[4][3];
    };

    struct PlayerInfo {
        bool valid;
        bool alive;
        int health;
        int team;
        int armor;
        bool helmet;
        Vec3 origin;
        Vec3 headPos;
        Vec3 bonePositions[64];
        int boneCount;
        char name[64];
        char clanTag[32];
        bool isFriend;
        uintptr_t controller;
        uintptr_t pawn;
    };

    inline float viewMatrix[16] = {};
    inline PlayerInfo players[64] = {};
    inline int playerCount = 0;
    inline int localTeam = 0;
    inline Vec3 localPos = {};
    inline Vec3 localViewAngles = {};
    inline Vec3 localAimPunch = {};
    inline bool localAlive = false;
    inline int previousHealth[64] = {};

    inline Vec3 GetEntityOrigin(uintptr_t pawn) {
        uintptr_t node = CS2::Read<uintptr_t>(pawn + entity::m_pGameSceneNode);
        if (!node) return {};
        return CS2::Read<Vec3>(node + scene::m_vecAbsOrigin);
    }

    inline Vec3 GetBonePos(uintptr_t pawn, int boneIndex) {
        uintptr_t node = CS2::Read<uintptr_t>(pawn + entity::m_pGameSceneNode);
        if (!node) return {};
        uintptr_t boneData = CS2::Read<uintptr_t>(node + bones::m_modelState + bones::m_boneArray);
        if (!boneData) return {};
        BoneMatrix bm = CS2::Read<BoneMatrix>(boneData + boneIndex * sizeof(BoneMatrix));
        return { bm.m[0][0] * 0 + bm.m[0][1] * 0 + bm.m[0][2] * 0 + bm.m[3][0],
                 bm.m[0][0] * 0 + bm.m[0][1] * 0 + bm.m[0][2] * 0 + bm.m[3][1],
                 bm.m[0][0] * 0 + bm.m[0][1] * 0 + bm.m[0][2] * 0 + bm.m[3][2] };
    }
    inline Vec3 GetBonePosition(uintptr_t pawn, int boneIndex) {
        uintptr_t node = CS2::Read<uintptr_t>(pawn + entity::m_pGameSceneNode);
        if (!node) return {};
        uintptr_t boneData = CS2::Read<uintptr_t>(node + bones::m_modelState + bones::m_boneArray);
        if (!boneData) return {};
        return CS2::Read<Vec3>(boneData + boneIndex * 32);
    }

    inline bool WorldToScreen(const Vec3& world, Vec2& screen, float screenW, float screenH) {
        float w = viewMatrix[12] * world.x + viewMatrix[13] * world.y + viewMatrix[14] * world.z + viewMatrix[15];
        if (w < 0.001f) return false;

        float invW = 1.0f / w;
        float x = viewMatrix[0] * world.x + viewMatrix[1] * world.y + viewMatrix[2] * world.z + viewMatrix[3];
        float y = viewMatrix[4] * world.x + viewMatrix[5] * world.y + viewMatrix[6] * world.z + viewMatrix[7];

        screen.x = (screenW * 0.5f) * (1.0f + x * invW);
        screen.y = (screenH * 0.5f) * (1.0f - y * invW);
        return true;
    }

    inline bool WorldToScreenDepth(const Vec3& world, Vec2& screen, float& depth, float screenW, float screenH) {
        float w = viewMatrix[12] * world.x + viewMatrix[13] * world.y + viewMatrix[14] * world.z + viewMatrix[15];
        if (w < 0.001f) return false;

        float invW = 1.0f / w;
        float x = viewMatrix[0] * world.x + viewMatrix[1] * world.y + viewMatrix[2] * world.z + viewMatrix[3];
        float y = viewMatrix[4] * world.x + viewMatrix[5] * world.y + viewMatrix[6] * world.z + viewMatrix[7];
        float z = viewMatrix[8] * world.x + viewMatrix[9] * world.y + viewMatrix[10] * world.z + viewMatrix[11];

        screen.x = (screenW * 0.5f) * (1.0f + x * invW);
        screen.y = (screenH * 0.5f) * (1.0f - y * invW);
        depth = z * invW;
        return true;
    }

    inline void UpdateViewMatrix() {
        if (!CS2::clientBase) return;
        memcpy(viewMatrix, (void*)(CS2::clientBase + CS2::offsets::dwViewMatrix), sizeof(viewMatrix));
    }

    inline void UpdateLocalPlayer() {
        uintptr_t localPawn = CS2::GetLocalPlayerPawn();
        if (localPawn) {
            localTeam = (int)CS2::Read<uint8_t>(localPawn + entity::m_iTeamNum);
            int localHealth = CS2::Read<int>(localPawn + entity::m_iHealth);
            localAlive = (localHealth > 0);
            localPos = GetEntityOrigin(localPawn);

            if (CS2::clientBase) {
                localViewAngles = CS2::Read<Vec3>(CS2::clientBase + CS2::offsets::dwViewAngles);
                uintptr_t punchSvc = CS2::Read<uintptr_t>(localPawn + pawn::m_pAimPunchServices);
                if (punchSvc) {
                    localAimPunch = CS2::Read<Vec3>(punchSvc + camera::m_vecCsViewPunchAngle);
                } else {
                    localAimPunch = {};
                }
            }
        } else {
            localAlive = false;
            uintptr_t localCtrl = CS2::GetLocalPlayerController();
            if (localCtrl) {
                uint32_t pawnHandle = CS2::Read<uint32_t>(localCtrl + CS2::controller::m_hPlayerPawn);
                if (pawnHandle) {
                    uintptr_t entityList = CS2::GetEntityList();
                    if (entityList) {
                        uintptr_t pChunk = CS2::Read<uintptr_t>(entityList + 8 * ((pawnHandle & 0x7FFF) >> 9) + 0x10);
                        if (pChunk) {
                            uintptr_t pawnEnt = CS2::Read<uintptr_t>(pChunk + CS2::ENTRY_SIZE * (pawnHandle & 0x1FF));
                            if (pawnEnt) {
                                localTeam = (int)CS2::Read<uint8_t>(pawnEnt + entity::m_iTeamNum);
                            }
                        }
                    }
                }
            }
        }
    }

    inline void UpdatePlayers() {
        playerCount = 0;
        if (!CS2::initialized) return;

        uintptr_t entityList = CS2::GetEntityList();
        if (!entityList) return;

        uintptr_t localPawn = CS2::GetLocalPlayerPawn();
        uintptr_t listEntry = CS2::Read<uintptr_t>(entityList + 0x10);
        if (!listEntry) return;

        for (int i = 0; i < 64; i++) {
            uintptr_t ctrl = CS2::Read<uintptr_t>(listEntry + CS2::ENTRY_SIZE * (i & 0x1FF));
            if (!ctrl) continue;
            uint32_t pawnHandle = CS2::Read<uint32_t>(ctrl + CS2::controller::m_hPlayerPawn);
            if (!pawnHandle) continue;

            uintptr_t pChunk = CS2::Read<uintptr_t>(entityList + 8 * ((pawnHandle & 0x7FFF) >> 9) + 0x10);
            if (!pChunk) continue;

            uintptr_t pawnEnt = CS2::Read<uintptr_t>(pChunk + CS2::ENTRY_SIZE * (pawnHandle & 0x1FF));
            if (!pawnEnt || pawnEnt == localPawn) continue;
            uint8_t lifeState = CS2::Read<uint8_t>(pawnEnt + entity::m_lifeState);
            if (lifeState != 0) continue;
            uintptr_t node = CS2::Read<uintptr_t>(pawnEnt + entity::m_pGameSceneNode);
            if (!node) continue;
            bool dormant = CS2::Read<bool>(node + scene::m_bDormant);
            if (dormant) continue;

            int health = CS2::Read<int>(pawnEnt + entity::m_iHealth);
            if (health <= 0 || health > 100) continue;

            int playerIndex = i;
            previousHealth[playerIndex] = health;

            PlayerInfo& p = players[playerCount];
            p.valid = true;
            p.alive = true;
            p.health = health;
            p.team = (int)CS2::Read<uint8_t>(pawnEnt + entity::m_iTeamNum);
            p.armor = CS2::Read<int>(pawnEnt + pawn::m_ArmorValue);
            p.helmet = CS2::Read<bool>(ctrl + 0x919);
            p.origin = CS2::Read<Vec3>(node + scene::m_vecAbsOrigin);
            p.headPos = GetBonePosition(pawnEnt, BONE_HEAD);
            p.controller = ctrl;
            p.pawn = pawnEnt;
            p.boneCount = 7;
            p.bonePositions[0] = GetBonePosition(pawnEnt, BONE_HEAD);
            p.bonePositions[1] = GetBonePosition(pawnEnt, BONE_NECK);
            p.bonePositions[2] = GetBonePosition(pawnEnt, BONE_CHEST);
            p.bonePositions[3] = GetBonePosition(pawnEnt, BONE_PELVIS);
            p.bonePositions[4] = GetBonePosition(pawnEnt, BONE_LEFT_SHOULDER);
            p.bonePositions[5] = GetBonePosition(pawnEnt, BONE_RIGHT_SHOULDER);
            p.bonePositions[6] = GetBonePosition(pawnEnt, BONE_LEFT_FOOT);
            uintptr_t namePtr = CS2::Read<uintptr_t>(ctrl + CS2::controller::m_sSanitizedPlayerName);
            if (namePtr) {
                __try { strncpy_s(p.name, (const char*)namePtr, 63); }
                __except (EXCEPTION_EXECUTE_HANDLER) { strcpy_s(p.name, "?"); }
            } else {
                strcpy_s(p.name, "?");
            }
            p.clanTag[0] = '\0';
            p.isFriend = false;
            uintptr_t clanPtr = CS2::Read<uintptr_t>(ctrl + CS2::controller::m_szClan);
            if (clanPtr && clanPtr > 0x10000) {
                __try { strncpy_s(p.clanTag, (const char*)clanPtr, 31); }
                __except (EXCEPTION_EXECUTE_HANDLER) { p.clanTag[0] = '\0'; }
            }

            playerCount++;
            if (playerCount >= 64) break;
        }
    }

    inline float Distance(Vec3 a, Vec3 b) {
        float dx = a.x - b.x, dy = a.y - b.y, dz = a.z - b.z;
        return sqrtf(dx * dx + dy * dy + dz * dz);
    }

    inline Vec3 CalcAngle(Vec3 src, Vec3 dst) {
        Vec3 delta = { dst.x - src.x, dst.y - src.y, dst.z - src.z };
        float hyp = sqrtf(delta.x * delta.x + delta.y * delta.y);
        Vec3 angle;
        angle.x = -atan2f(delta.z, hyp) * (180.0f / 3.14159265f);
        angle.y = atan2f(delta.y, delta.x) * (180.0f / 3.14159265f);
        angle.z = 0.0f;
        return angle;
    }

    inline float GetFov(Vec3 viewAngle, Vec3 aimAngle) {
        float dx = aimAngle.x - viewAngle.x;
        float dy = aimAngle.y - viewAngle.y;
        while (dy > 180.0f) dy -= 360.0f;
        while (dy < -180.0f) dy += 360.0f;
        return sqrtf(dx * dx + dy * dy);
    }

    inline void Update() {
        UpdateViewMatrix();
        UpdateLocalPlayer();
        UpdatePlayers();
    }
}
