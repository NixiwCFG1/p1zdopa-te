#include "includes.h"
#include "cs2.h"
#include "game.h"
#include "features.h"
#include <cstring>
#include <algorithm>

namespace
{
    constexpr float NORMALIZE_ANGLE_MULT = 0.0027777778f; // 1/360

    float normalize_angle(float a)
    {
        return a - std::floorf(a * NORMALIZE_ANGLE_MULT + 0.5f) * 360.0f;
    }

    float quantize_angle(float a)
    {
        return std::floorf(normalize_angle(a) * 2.0f) * 0.5f;
    }

    // SHA1 implementation
    class sha1
    {
    public:
        void reset()
        {
            m_state[0] = 0x67452301;
            m_state[1] = 0xEFCDAB89;
            m_state[2] = 0x98BADCFE;
            m_state[3] = 0x10325476;
            m_state[4] = 0xC3D2E1F0;
            m_count = 0;
        }

        void update(const void* data, std::size_t len)
        {
            const auto* bytes = static_cast<const std::uint8_t*>(data);
            auto index = static_cast<std::size_t>(m_count & 63);
            m_count += len;

            std::size_t i = 0;

            if (index)
            {
                auto part_len = 64 - index;
                if (len >= part_len)
                {
                    std::memcpy(m_buffer + index, bytes, part_len);
                    transform(m_buffer);
                    i = part_len;
                }
                else
                {
                    std::memcpy(m_buffer + index, bytes, len);
                    return;
                }
            }

            for (; i + 64 <= len; i += 64)
                transform(bytes + i);

            if (i < len)
                std::memcpy(m_buffer, bytes + i, len - i);
        }

        void final()
        {
            std::uint8_t padding[64]{};
            padding[0] = 0x80;

            auto index = static_cast<std::size_t>(m_count & 63);
            auto pad_len = (index < 56) ? (56 - index) : (120 - index);
            auto bit_count = m_count * 8;

            update(padding, pad_len);

            std::uint8_t bits[8]{};
            for (int i = 0; i < 8; ++i)
                bits[7 - i] = static_cast<std::uint8_t>(bit_count >> (i * 8));

            update(bits, 8);

            for (int i = 0; i < 5; ++i)
            {
                m_digest[i * 4 + 0] = static_cast<std::uint8_t>(m_state[i] >> 24);
                m_digest[i * 4 + 1] = static_cast<std::uint8_t>(m_state[i] >> 16);
                m_digest[i * 4 + 2] = static_cast<std::uint8_t>(m_state[i] >> 8);
                m_digest[i * 4 + 3] = static_cast<std::uint8_t>(m_state[i]);
            }
        }

        std::uint32_t get_first_uint32() const
        {
            std::uint32_t result;
            std::memcpy(&result, m_digest, sizeof(result));
            return result;
        }

    private:
        static std::uint32_t rotl(std::uint32_t v, int n)
        {
            return (v << n) | (v >> (32 - n));
        }

        void transform(const std::uint8_t* block)
        {
            std::uint32_t w[80]{};

            for (int i = 0; i < 16; ++i)
            {
                w[i] = static_cast<std::uint32_t>(block[i * 4]) << 24 |
                    static_cast<std::uint32_t>(block[i * 4 + 1]) << 16 |
                    static_cast<std::uint32_t>(block[i * 4 + 2]) << 8 |
                    static_cast<std::uint32_t>(block[i * 4 + 3]);
            }

            for (int i = 16; i < 80; ++i)
                w[i] = rotl(w[i - 3] ^ w[i - 8] ^ w[i - 14] ^ w[i - 16], 1);

            auto a = m_state[0];
            auto b = m_state[1];
            auto c = m_state[2];
            auto d = m_state[3];
            auto e = m_state[4];

            for (int i = 0; i < 80; ++i)
            {
                std::uint32_t f, k;

                if (i < 20)
                {
                    f = (b & c) | ((~b) & d);
                    k = 0x5A827999;
                }
                else if (i < 40)
                {
                    f = b ^ c ^ d;
                    k = 0x6ED9EBA1;
                }
                else if (i < 60)
                {
                    f = (b & c) | (b & d) | (c & d);
                    k = 0x8F1BBCDC;
                }
                else
                {
                    f = b ^ c ^ d;
                    k = 0xCA62C1D6;
                }

                auto temp = rotl(a, 5) + f + e + k + w[i];
                e = d;
                d = c;
                c = rotl(b, 30);
                b = a;
                a = temp;
            }

            m_state[0] += a;
            m_state[1] += b;
            m_state[2] += c;
            m_state[3] += d;
            m_state[4] += e;
        }

        std::uint32_t m_state[5]{};
        std::uint64_t m_count{ 0 };
        std::uint8_t m_buffer[64]{};
        std::uint8_t m_digest[20]{};
    };

    // Valve RNG implementation
    class valve_rng
    {
    public:
        void seed(int s)
        {
            m_state = -std::abs(s);
            m_index = 0;
            m_seeded = false;
        }

        int generate()
        {
            if (!m_seeded)
            {
                auto v = -m_state;
                if (v < 1)
                    v = 1;

                for (int j = 39; j >= 0; --j)
                {
                    v = lcg(v);
                    if (j < 32)
                        m_table[j] = v;
                }

                m_state = v;
                m_index = m_table[0];
                m_seeded = true;
            }

            m_state = lcg(m_state);
            const auto idx = m_index / 0x4000000;
            m_index = m_table[idx];
            m_table[idx] = m_state;
            return m_index;
        }

        float random_float(float min = 0.0f, float max = 1.0f)
        {
            const auto raw = generate();
            const auto norm = std::fminf(0.99999988f, static_cast<float>(raw) * 4.6566129e-10f);
            return min + norm * (max - min);
        }

    private:
        static int lcg(int state)
        {
            const auto k = state / 127773;
            auto result = 16807 * (state - k * 127773) - 2836 * k;
            if (result < 0)
                result += 2147483647;
            return result;
        }

        int m_state{ 0 };
        int m_index{ 0 };
        int m_table[32]{};
        bool m_seeded{ false };
    };

    // Spread functions
    static std::uint32_t premium_get_spread_seed_rebuild(const Vec3& angles, int tick)
    {
        struct
        {
            float pitch;
            float yaw;
            int player_render_tick;
        } buffer{};

        buffer.pitch = quantize_angle(angles.x);
        buffer.yaw = quantize_angle(angles.y);
        buffer.player_render_tick = tick;

        sha1 hash;
        hash.reset();
        hash.update(&buffer, 12);
        hash.final();

        return hash.get_first_uint32();
    }

    struct Vector2D
    {
        float x, y;
    };

    static Vector2D premium_calculate_spread_rebuild(int seed, float inaccuracy, float spread, float recoil_index, int item_def_idx, int num_bullets)
    {
        constexpr std::uint16_t revolver_id = 64;
        constexpr std::uint16_t negev_id = 28;
        constexpr auto two_pi = 2.0f * 3.14159265358979323846f;

        valve_rng rng;
        rng.seed(seed);

        auto inac_r = rng.random_float(0.0f, 1.0f);
        auto inac_a = rng.random_float(0.0f, two_pi);

        if (item_def_idx == revolver_id && num_bullets == 1)
        {
            inac_r = 1.0f - (inac_r * inac_r);
        }
        else if (item_def_idx == negev_id && recoil_index < 3.0f)
        {
            auto v = inac_r;
            auto c = 3;
            do { --c; v *= v; } while (static_cast<float>(c) > recoil_index);
            inac_r = 1.0f - v;
        }

        inac_r *= inaccuracy;

        auto spr_r = rng.random_float(0.0f, 1.0f);
        auto spr_a = rng.random_float(0.0f, two_pi);

        if (item_def_idx == revolver_id && num_bullets == 1)
        {
            spr_r = 1.0f - (spr_r * spr_r);
        }
        else if (item_def_idx == negev_id && recoil_index < 3.0f)
        {
            auto v = spr_r;
            auto c = 3;
            do { --c; v *= v; } while (static_cast<float>(c) > recoil_index);
            spr_r = 1.0f - v;
        }

        spr_r *= spread;

        return
        {
            std::cosf(spr_a) * spr_r + std::cosf(inac_a) * inac_r,
            std::sinf(spr_a) * spr_r + std::sinf(inac_a) * inac_r
        };
    }

    static void AngleVectors(const Vec3& angles, Vec3* forward, Vec3* right, Vec3* up)
    {
        float sp, sy, sr, cp, cy, cr;

        sy = std::sinf(angles.y * (3.14159265f / 180.0f));
        cy = std::cosf(angles.y * (3.14159265f / 180.0f));
        sp = std::sinf(angles.x * (3.14159265f / 180.0f));
        cp = std::cosf(angles.x * (3.14159265f / 180.0f));
        sr = std::sinf(angles.z * (3.14159265f / 180.0f));
        cr = std::cosf(angles.z * (3.14159265f / 180.0f));

        if (forward)
        {
            forward->x = cp * cy;
            forward->y = cp * sy;
            forward->z = -sp;
        }

        if (right)
        {
            right->x = -1.0f * sr * sp * cy + -1.0f * cr * -sy;
            right->y = -1.0f * sr * sp * sy + -1.0f * cr * cy;
            right->z = -1.0f * sr * cp;
        }

        if (up)
        {
            up->x = cr * sp * cy + -sr * -sy;
            up->y = cr * sp * sy + -sr * cy;
            up->z = cr * cp;
        }
    }

    static bool ray_hits_capsule(
        const Vec3& ray_origin,
        const Vec3& ray_dir,
        const Vec3& capsule_start,
        const Vec3& capsule_end,
        float radius)
    {
        Vec3 capsule_vec = capsule_end - capsule_start;
        float capsule_length = capsule_vec.Length();

        if (capsule_length < 0.001f)
        {
            Vec3 to_center = capsule_start - ray_origin;
            const float projection = to_center.DotProduct(ray_dir);

            if (projection < 0.0f)
            {
                return false;
            }

            Vec3 closest = ray_origin + (ray_dir * projection);
            Vec3 diff = closest - capsule_start;
            return diff.LengthSqr() <= radius * radius;
        }

        Vec3 capsule_dir = capsule_vec / capsule_length;
        Vec3 w = ray_origin - capsule_start;

        const float a = ray_dir.DotProduct(ray_dir);
        const float b = ray_dir.DotProduct(capsule_dir);
        const float c = capsule_dir.DotProduct(capsule_dir);
        const float d = ray_dir.DotProduct(w);
        const float e = capsule_dir.DotProduct(w);

        const float denom = a * c - b * b;

        float s, t;

        if (std::abs(denom) < 0.0001f)
        {
            s = 0.0f;
            t = (b > c) ? (d / b) : (e / c);
        }
        else
        {
            s = (b * e - c * d) / denom;
            t = (a * e - b * d) / denom;
        }

        if (t < 0.0f) t = 0.0f;
        if (t > capsule_length) t = capsule_length;

        if (s < 0.0f)
        {
            return false;
        }

        Vec3 point_on_capsule = capsule_start + (capsule_dir * t);
        Vec3 point_on_ray = ray_origin + (ray_dir * s);

        Vec3 diff = point_on_ray - point_on_capsule;
        return diff.LengthSqr() <= radius * radius;
    }
}

namespace Spread
{
    bool TriggerSpreadProSpread()
    {
        if (!CS2::initialized) return false;

        uintptr_t localPawn = CS2::GetLocalPlayerPawn();
        if (!localPawn) return false;

        uintptr_t localController = CS2::GetLocalPlayerController();
        if (!localController) return false;

        int tick = CS2::Read<int>(localController + CS2::controller::m_iPawnHealth);
        Vec3 viewAngles = Game::localViewAngles;

        uint32_t seed = premium_get_spread_seed_rebuild(viewAngles, tick);

        // Get weapon data
        uintptr_t weaponServices = CS2::Read<uintptr_t>(localPawn + Game::pawn::m_pWeaponServices);
        if (!weaponServices) return false;

        uint32_t weaponHandle = CS2::Read<uint32_t>(weaponServices + Game::weapon::m_hActiveWeapon);
        if (!weaponHandle) return false;

        uintptr_t entityList = CS2::GetEntityList();
        if (!entityList) return false;

        uintptr_t weaponChunk = CS2::Read<uintptr_t>(entityList + 8 * ((weaponHandle & 0x7FFF) >> 9) + 16);
        if (!weaponChunk) return false;

        uintptr_t weapon = CS2::Read<uintptr_t>(weaponChunk + CS2::ENTRY_SIZE * (weaponHandle & 0x1FF));
        if (!weapon) return false;

        // Weapon data - placeholders (need proper offsets)
        float flInaccuracy = 0.01f;
        float flSpread = 0.01f;
        float flRecoilIndex = 0.0f;
        int nWeaponID = CS2::Read<int>(weapon + Game::weapon::m_iItemDefinitionIndex);
        int nNumBullets = 1;

        Vector2D spread_offset = premium_calculate_spread_rebuild(
            seed + 1, flInaccuracy, flSpread, flRecoilIndex, nWeaponID, nNumBullets
        );

        Vec3 vForward, vRight, vUp;
        AngleVectors(viewAngles, &vForward, &vRight, &vUp);

        Vec3 vDirection = vForward + (vRight * spread_offset.x) + (vUp * spread_offset.y);

        Vec3 vecStart = Game::localPos;
        vecStart.z += 64.0f;

        // Check all players
        for (int i = 0; i < Game::playerCount; i++)
        {
            Game::PlayerInfo& p = Game::players[i];
            if (!p.valid || !p.alive) continue;
            if (!Settings::trigger_team && p.team == Game::localTeam) continue;

            Vec3 vecTargetPos = p.headPos;
            float flDistance = Game::Distance(vecStart, vecTargetPos);

            if (flDistance > 0.0f && flDistance <= 8000.0f)
            {
                // Check bone hits
                int bones[] = { Game::BONE_HEAD, Game::BONE_NECK, Game::BONE_CHEST, Game::BONE_PELVIS };
                for (int bone : bones)
                {
                    Vec3 bonePos = Game::GetBonePosition(p.pawn, bone);
                    if (bonePos.x == 0 && bonePos.y == 0 && bonePos.z == 0) continue;

                    Vec3 toBone = bonePos - vecStart;
                    float toBoneLen = toBone.Length();
                    if (toBoneLen > 0.0f)
                    {
                        Vec3 rayDir = vDirection;
                        Vec3 toBoneNorm = toBone / toBoneLen;

                        float dot = rayDir.DotProduct(toBoneNorm);
                        if (dot > 0.95f)
                        {
                            return true;
                        }
                    }
                }
            }
        }

        return false;
    }
}
