#pragma once

#include "_References/BehaviorGraph.h"
#include "_References/ExclusionLists.h"
#include "_References/HavokRayShapeCast.h"
#include "_References/Fmt.h"

/* Macro func */
#define PRINT_LAYER(x) (RE::CollisionLayerToString(x))

namespace SkyParkour
{

    const enum ParkourKeyOptions { kJump = 0, kSprint, kActivate };
    const enum AutoParkourOptions { kDisabled = 0, kNonCombatOnly, kAlways };

    static void LogCharacterFlags()
    {
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

    static RE::NiPoint3 Vec4_To_Vec3(RE::hkVector4 vec)
    {
        return NiPoint3(vec.quad.m128_f32[0], vec.quad.m128_f32[1], vec.quad.m128_f32[2]);
    }

    static hkVector4 Vec3_To_Vec4(NiPoint3 vec)
    {
        return hkVector4(vec.x, vec.y, vec.z, 0);
    }
}  // namespace SkyParkour

/* Log macros */
using TRACE = SKSE::log::trace;
using INFO = SKSE::log::info;
using WARN = SKSE::log::warn;
using ERROR = SKSE::log::error;
using CRITICAL = SKSE::log::critical;

/* Task Queue Macro */
#define _TASK_Q SKSE::GetTaskInterface()->AddTask

/* Generic Stuff */
#define GET_PLAYER RE::PlayerCharacter::GetSingleton()

#define VEC3_TO_VEC4 SkyParkour::Vec3_To_Vec4               // Only assign the properties
#define VEC4_TO_VEC3 SkyParkour::Vec4_To_Vec3               // Only assign the properties
#define nipoint_to_hkvector(a) VEC3_TO_VEC4(a / 69.99125f)  // Scale from ni to havok
#define hkvector_to_nipoint(a) VEC4_TO_VEC3(a * 69.99125f)  // Scale from havok to ni

#define RayCastResult SkyParkour::RayCastResult
#define ShapeCastResult SkyParkour::ShapeCastResult

#define PARKOUR_PRESET_KEYS SkyParkour::ParkourKeyOptions
#define AUTO_PARKOUR_OPTIONS SkyParkour::AutoParkourOptions