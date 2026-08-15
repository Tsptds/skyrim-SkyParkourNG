#pragma once

namespace SkyParkour
{
    using namespace RE;
    using cl = COL_LAYER;
    using ft = FormType;

    /* Mark ledge point layers that are considered invalid for climbing */
    static const std::unordered_set<cl> ClimbLayerExclusionList{cl::kNonCollidable, cl::kCharController, /*cl::kAnimStatic,*/ cl::kWeapon,
                                                                cl::kProjectile,    cl::kTransparent,    cl::kClutter,
                                                                cl::kBiped,         cl::kActorZone,      cl::kDebrisLarge};

    /* Head Level Check Layers. If hit, consider vault has obstruction behind */
    static const std::unordered_set<cl> VaultForwardRayList{cl::kStatic,     cl::kTerrain,       cl::kGround,
                                                            cl::kProps,      cl::kDoorDetection, cl::kTrees,
                                                            cl::kAnimStatic, cl::kDebrisLarge,   cl::kTransparent};

    /* Ledge Point Layers. If hit, consider vault point invalid. */
    static const std::unordered_set<cl> VaultDownRayList{cl::kWeapon,  cl::kProjectile, cl::kCharController,
                                                         cl::kClutter, cl::kBiped,      cl::kDeadBip};

    /* Invalid Form Types */
    static const std::unordered_set<ft> ExcludeFormsClimb{
        ft::NPC,
    };
    static const std::unordered_set<ft> ExcludeFormsVault{
        ft::Activator,
        ft::NPC,
    };

    enum class COL_LAYER_EXTEND {
        kClimbLedge = static_cast<uint32_t>(cl::kTransparent),
        kClimbObstruction = static_cast<uint32_t>(cl::kTransparent),
        kVaultDown = static_cast<uint32_t>(cl::kTransparent),
        kVaultForward = static_cast<uint32_t>(cl::kTransparent),
        kVaultPostLedgeObstruction = static_cast<uint32_t>(cl::kTransparent),
        kCrouchSlideDistCheck = static_cast<uint32_t>(cl::kTransparent),
    };
}  // namespace SkyParkour

#define LAYERS_CLIMB_EXCLUDE SkyParkour::ClimbLayerExclusionList
#define LAYERS_VAULT_DOWN_RAY SkyParkour::VaultDownRayList
#define LAYERS_VAULT_FORWARD_RAY SkyParkour::VaultForwardRayList

#define FORMS_CLIMB_EXCLUDE SkyParkour::ExcludeFormsClimb
#define FORMS_VAULT_EXCLUDE SkyParkour::ExcludeFormsVault

#define COL_LAYER_EXTEND SkyParkour::COL_LAYER_EXTEND