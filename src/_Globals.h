#pragma once

#include "Util/ThreadPool.hpp"
#include "_References/BehaviorGraph.h"

#ifdef _DEBUG
#include "_Logging.h"
#endif

/* Macro func */
#define PRINT_LAYER(x) (RE::CollisionLayerToString(x))

namespace SkyParkourUtil {
    using namespace RE;

    /* Mark ledge point layers that are considered invalid for climbing */
    static const std::unordered_set<COL_LAYER> ClimbLayerExclusionList{
        COL_LAYER::kNonCollidable, COL_LAYER::kCharController, /*COL_LAYER::kAnimStatic,*/ COL_LAYER::kWeapon,
        COL_LAYER::kProjectile,    COL_LAYER::kTransparent,    COL_LAYER::kClutter,
        COL_LAYER::kBiped};

    /* Head Level Check Layers. If hit, consider vault has obstruction behind */
    static const std::unordered_set<COL_LAYER> VaultForwardRayList{
        COL_LAYER::kStatic, COL_LAYER::kTerrain,    COL_LAYER::kGround,      COL_LAYER::kProps,      COL_LAYER::kDoorDetection,
        COL_LAYER::kTrees,  COL_LAYER::kAnimStatic, COL_LAYER::kDebrisLarge, COL_LAYER::kTransparent};

    /* Ledge Point Layers. If hit, consider vault point invalid. */
    static const std::unordered_set<COL_LAYER> VaultDownRayList{COL_LAYER::kWeapon,  COL_LAYER::kProjectile, COL_LAYER::kCharController,
                                                                COL_LAYER::kClutter, COL_LAYER::kBiped,      COL_LAYER::kDeadBip};

    static ThreadPool threads;
    static hkVector4 zeroVector{0, 0, 0, 0};

    struct RayCastResult {
            float distance = -1.0f;
            COL_LAYER layer = COL_LAYER::kUnidentified;
            hkVector4 normalOut = hkVector4(0, 0, 0, 0);
            bool didHit = false;

            // Do a null check before using this
            TESObjectREFR *hitObjectRef = nullptr;

            RayCastResult() = default;

            RayCastResult(float d, COL_LAYER l, const hkVector4 &n, bool h, TESObjectREFR *r)
                : distance(d), layer(l), normalOut(n), didHit(h), hitObjectRef(r) {}
    };

    enum class COL_LAYER_EXTEND {
        kClimbLedge = static_cast<uint32_t>(COL_LAYER::kLOS),
        kClimbObstruction = static_cast<uint32_t>(COL_LAYER::kCustomPick1),
        kVaultDown = static_cast<uint32_t>(COL_LAYER::kCustomPick1),
        kVaultForward = static_cast<uint32_t>(COL_LAYER::kTransparent),
        kVaultPostLedgeObstruction = static_cast<uint32_t>(COL_LAYER::kLOS),
        kCrouchSlideDistCheck = static_cast<uint32_t>(COL_LAYER::kTransparent),
    };

    const enum ParkourKeyOptions { kJump = 0, kSprint, kActivate };

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

    static NiPoint3 Vec4_To_Vec3(hkVector4 vec) {
        return NiPoint3(vec.quad.m128_f32[0], vec.quad.m128_f32[1], vec.quad.m128_f32[2]);
    }

    static hkVector4 Vec3_To_Vec4(NiPoint3 vec) {
        return hkVector4(vec.x, vec.y, vec.z, 0);
    }
}  // namespace SkyParkourUtil

/* Log macros */
#define LOG(x, ...) logger::info(x, __VA_ARGS__)
#define WARN(x, ...) logger::warn(x, __VA_ARGS__)
#define ERROR(x, ...) logger::error(x, __VA_ARGS__)
#define CRITICAL(x, ...) logger::error(x, __VA_ARGS__)

/* Task Queue & Thread Pool Macro */
#define _THREAD_POOL SkyParkourUtil::threads
#define _TASK_Q SKSE::GetTaskInterface()->AddTask

/* Generic Stuff */
#define GET_PLAYER RE::PlayerCharacter::GetSingleton()
#define ZERO_VECTOR SkyParkourUtil::zeroVector

#define VEC3_TO_VEC4 SkyParkourUtil::Vec3_To_Vec4
#define VEC4_TO_VEC3 SkyParkourUtil::Vec4_To_Vec3

#define RayCastResult SkyParkourUtil::RayCastResult
#define COL_LAYER_EXTEND SkyParkourUtil::COL_LAYER_EXTEND
#define PARKOUR_PRESET_KEYS SkyParkourUtil::ParkourKeyOptions
#define LOG_PLAYER_CONTROLLER_FLAGS SkyParkourUtil::LogCharacterFlags()

#define LAYERS_CLIMB_EXCLUDE SkyParkourUtil::ClimbLayerExclusionList
#define LAYERS_VAULT_DOWN_RAY SkyParkourUtil::VaultDownRayList
#define LAYERS_VAULT_FORWARD_RAY SkyParkourUtil::VaultForwardRayList
