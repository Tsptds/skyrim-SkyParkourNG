#pragma once

#include "_References/BehaviorGraph.h"
#include "_References/ExclusionLists.h"
#include "_References/Fmt.h"

/* Macro func */
#define PRINT_LAYER(x) (RE::CollisionLayerToString(x))

namespace SkyParkour {

    static RE::hkVector4 zeroVector{0, 0, 0, 0};

    struct RayCastResult {
            float distance = -1.0f;
            COL_LAYER layer = COL_LAYER::kUnidentified;
            hkVector4 normalOut = hkVector4(0, 0, 0, 0);
            bool didHit = false;

            // Do a null check before using this
            TESObjectREFR *hitObjectRef = nullptr;
            hkInplaceArray<hkpWorldRayCastOutput, 8> hits;

            RE::FormType GetHitObjectFormType_Safe() const {
                if (!hitObjectRef) return RE::FormType::None;
                return hitObjectRef->GetObjectReference()->GetFormType();
            }

            RayCastResult() = default;

            RayCastResult(float d, COL_LAYER l, const hkVector4 &n, bool h, TESObjectREFR *r, hkInplaceArray<hkpWorldRayCastOutput, 8> arr)
                : distance(d), layer(l), normalOut(n), didHit(h), hitObjectRef(r), hits(arr) {}
    };

    const enum ParkourKeyOptions { kJump = 0, kSprint, kActivate };
    const enum AutoParkourOptions { kDisabled = 0, kNonCombatOnly, kAlways };

    static void LogCharacterFlags() {
        if (auto *controller = PlayerCharacter::GetSingleton()->GetCharController()) {
            auto flags = controller->flags;

            struct FlagInfo {
                    const char *name;
                    std::uint32_t mask;
            };
            static constexpr FlagInfo table[] = {{"kQuadruped", 1 << 0},
                                                 {"kNoGravityOnGround", 1 << 1},
                                                 {"kTryStep", 1 << 2},
                                                 {"kNoFriction", 1 << 3},
                                                 {"kAllowJumpNoContact", 1 << 4},
                                                 {"kStuckQuad", 1 << 5},
                                                 {"kAnimAngleMod", 1 << 6},
                                                 {"kHitDamage", 1 << 7},
                                                 {"kSupport", 1 << 8},
                                                 {"kHasPotentialSupportManifold", 1 << 9},
                                                 {"kCanJump", 1 << 10},
                                                 {"kChaseBip", 1 << 11},
                                                 {"kFollowRagdoll", 1 << 12},
                                                 {"kJumping", 1 << 13},
                                                 {"kNotPushable", 1 << 14},
                                                 {"kFloatLand", 1 << 15},
                                                 {"kCheckSupport", 1 << 16},
                                                 {"kNoSim", 1 << 17},
                                                 {"kFarAway", 1 << 18},
                                                 {"kOnStilts", 1 << 19},
                                                 {"kQuickSimulate", 1 << 20},
                                                 {"kRecordHits", 1 << 21},
                                                 {"kComputeTiltPreIntegrate", 1 << 22},
                                                 {"kShouldersUnderWater", 1 << 23},
                                                 {"kOnStairs", 1 << 24},
                                                 {"kCanPitch", 1 << 25},
                                                 {"kCanRoll", 1 << 26},
                                                 {"kNoCharacterCollisions", 1 << 27},
                                                 {"kNotPushablePermanent", 1 << 28},
                                                 {"kPossiblePathObstacle", 1 << 29},
                                                 {"kShapeRequiresZRot", 1 << 30},
                                                 {"kSwimAtWaterSurface", 1u << 31}};

            for (auto &f: table) {
                if (flags.any(static_cast<CHARACTER_FLAGS>(f.mask))) {
                    logger::info(" - {}", f.name);
                }
            }
        }
        else {
            logger::info("No character controller on player.");
        }
    }

    static RE::NiPoint3 Vec4_To_Vec3(RE::hkVector4 vec) {
        return NiPoint3(vec.quad.m128_f32[0], vec.quad.m128_f32[1], vec.quad.m128_f32[2]);
    }

    static hkVector4 Vec3_To_Vec4(NiPoint3 vec) {
        return hkVector4(vec.x, vec.y, vec.z, 0);
    }
}  // namespace SkyParkour

/* Log macros */
#define LOG(x, ...) logger::info(x __VA_OPT__(, ) __VA_ARGS__)
#define WARN(x, ...) logger::warn(x __VA_OPT__(, ) __VA_ARGS__)
#define ERROR(x, ...) logger::error(x __VA_OPT__(, ) __VA_ARGS__)
#define CRITICAL(x, ...) logger::critical(x __VA_OPT__(, ) __VA_ARGS__)

/* Task Queue Macro */
#define _TASK_Q SKSE::GetTaskInterface()->AddTask

/* Generic Stuff */
#define GET_PLAYER RE::PlayerCharacter::GetSingleton()
#define ZERO_VECTOR SkyParkour::zeroVector

#define VEC3_TO_VEC4 SkyParkour::Vec3_To_Vec4
#define VEC4_TO_VEC3 SkyParkour::Vec4_To_Vec3
#define nipoint_to_hkvector(a) VEC3_TO_VEC4(a / 69.99125f)
#define hkvector_to_nipoint(a) VEC4_TO_VEC3(a * 69.99125f)

#define RayCastResult SkyParkour::RayCastResult

#define PARKOUR_PRESET_KEYS SkyParkour::ParkourKeyOptions
#define AUTO_PARKOUR_OPTIONS SkyParkour::AutoParkourOptions