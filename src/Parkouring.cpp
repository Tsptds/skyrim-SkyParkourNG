#include "Parkouring.h"
#include "Util/ParkourUtility.h"
#include "Listeners/ButtonListener.h"
#include "Listeners/MenuListener.h"
#include "Util/ScaleUtility.h"
#include "Util/HavokUtil.hpp"

#include "_References/ModSettings.h"
#include "_References/Compatibility.h"
#include "_References/ParkourType.h"
#include "_References/RuntimeVariables.h"
#include "_References/HardcodedVariables.h"
#include "_References/RuntimeMethods.h"

#include "API/API_Handles.h"
#include "API/TrueHUDAPI.h"

#include "HUD/Scaleform/SkyParkourMenu.hpp"

static std::mutex g_ParkourActivateLock;
static std::mutex g_UpdateLock;

using namespace ParkourUtility;

int Parkouring::GetLedgePoint() {
    using namespace ModSettings;

    const auto &player = GET_PLAYER;

    const auto &playerDirFlat = RuntimeVariables::playerDirFlat;

    // Perform ledge or vault checks
    int selectedLedgeType = ParkourType::NoLedge;
    RE::NiPoint3 ledgePoint;

    constexpr int vaultLength = 100;
    constexpr int maxElevationIncrease = 80;

    selectedLedgeType = VaultCheck(ledgePoint, playerDirFlat, vaultLength, maxElevationIncrease * RuntimeVariables::PlayerScale,
                                   HardCodedVariables::vaultMinHeight * RuntimeVariables::PlayerScale,
                                   HardCodedVariables::vaultMaxHeight * RuntimeVariables::PlayerScale);

    if (selectedLedgeType == ParkourType::NoLedge) {
        selectedLedgeType = ClimbCheck(ledgePoint, playerDirFlat, HardCodedVariables::climbMinHeight * RuntimeVariables::PlayerScale,
                                       HardCodedVariables::climbMaxHeight * RuntimeVariables::PlayerScale);
    }
    if (selectedLedgeType == ParkourType::NoLedge) {
        return ParkourType::NoLedge;
    }

    // Don't ever parkour into water, last check before saying this ledge is valid
    float waterLevel;
    player->GetParentCell()->GetWaterHeight(player->GetPosition(), waterLevel);  //Relative to player

    constexpr int validWaterDepth = 10;

    if (ledgePoint.z < waterLevel - validWaterDepth) {
        return ParkourType::NoLedge;
    }

    RuntimeVariables::ledgePoint = ledgePoint;

    return selectedLedgeType;
}

int Parkouring::ClimbCheck(RE::NiPoint3 &ledgePoint, RE::NiPoint3 checkDir, float minLedgeHeight, float maxLedgeHeight) {
    const auto &player = GET_PLAYER;
    const auto &playerPos = player->GetPosition();
    constexpr RE::NiPoint3 upDir(0, 0, 1);
    constexpr RE::NiPoint3 downDir(0, 0, -1);

    // Constants adjusted for player scale
    const float startZOffset = 100 * RuntimeVariables::PlayerScale;
    const float playerHeight = 120 * RuntimeVariables::PlayerScale;
    const float minUpCheck = 100 * RuntimeVariables::PlayerScale;
    const float maxUpCheck = (maxLedgeHeight - startZOffset) + 20 * RuntimeVariables::PlayerScale;
    const float fwdCheckStep = 5 * RuntimeVariables::PlayerScale;  // 8
    const int fwdCheckIterations = 15;                             // 15
    const float minLedgeFlatness = 0.5;                            //0.5

    // Raycast above player, is there enough room
    RE::NiPoint3 upRayStart = playerPos + RE::NiPoint3(0, 0, playerHeight);  // Fixed using startZOffset instead of playerHeight
    RayCastResult upRay = RayCast(upRayStart, upDir, maxUpCheck, COL_LAYER_EXTEND::kClimbObstruction);

    if (upRay.distance < minUpCheck) {
        /* DEBUG LINES */
        if (ModSettings::_Debug_Draw_Lines) {
            const auto &TH = API_Handles::TrueHUD::Get();
            if (TH) {
                TH->DrawArrow(upRayStart, upRayStart + upDir * upRay.distance, 10.f, 0.f, 0xFFF000FF, 1.f);
            }
        }
        /***********************************/
        return ParkourType::NoLedge;
    }

    // Forward raycast initialization
    RE::NiPoint3 fwdRayStart = upRayStart + upDir * (upRay.distance - 10);

    RayCastResult ledgeRay;
    bool foundLedge = false;
    float normalZ = 0;

    // Incremental forward raycast to find a ledge
    for (int i = 0; i < fwdCheckIterations; i++) {
        RayCastResult fwdRay = RayCast(fwdRayStart, checkDir, fwdCheckStep * i, COL_LAYER_EXTEND::kClimbObstruction);

#ifdef LOG_CLIMB
        LOG("Ledge FWD: {}", PRINT_LAYER(fwdRay.layer));
#endif

        if (fwdRay.distance < fwdCheckStep * i) {
            continue;
        }

        // Downward raycast to detect ledge point
        RE::NiPoint3 ledgeRayStart = fwdRayStart + checkDir * fwdRay.distance;
        ledgeRay = RayCast(ledgeRayStart, downDir, startZOffset + maxUpCheck, COL_LAYER_EXTEND::kClimbLedge);

#ifdef LOG_CLIMB
        LOG("Ledge Down: {}", PRINT_LAYER(ledgeRay.layer));
#endif

        if (LAYERS_CLIMB_EXCLUDE.contains(ledgeRay.layer)) {
            continue;
        }

        ledgePoint = ledgeRayStart + downDir * ledgeRay.distance;
        normalZ = ledgeRay.normalOut.quad.m128_f32[2];

        // Validate ledge based on height and flatness
        if (ledgeRay.distance < 10) continue;
        if (normalZ < minLedgeFlatness) continue;
        if (ledgePoint.z < playerPos.z + minLedgeHeight) continue;
        if (ledgePoint.z > playerPos.z + maxLedgeHeight) continue;

        // Check for obstructions behind the Ledge point
        const bool obsFound = [ledgePoint, checkDir] {
            constexpr float obsBackOffset = 15.f;
            const float ray1MaxDist = obsBackOffset + 15.f;
            const float minSpaceRequired = (obsBackOffset + 3.f) * RuntimeVariables::PlayerScale;

            const RE::NiPoint3 obsCheckStart = RE::NiPoint3(ledgePoint.x, ledgePoint.y, ledgePoint.z + 5.f) - checkDir * obsBackOffset;

            RayCastResult obsRay1 = RayCast(obsCheckStart, checkDir, ray1MaxDist, COL_LAYER_EXTEND::kClimbObstruction);

            auto normalizedDir = -VEC4_TO_VEC3(obsRay1.normalOut);
            normalizedDir.z = 0;
            RayCastResult obsRay2 = RayCast(obsCheckStart, normalizedDir, minSpaceRequired, COL_LAYER_EXTEND::kClimbObstruction);

            /* DEBUG LINES */
            if (ModSettings::_Debug_Draw_Lines) {
                const auto &TH = API_Handles::TrueHUD::Get();
                if (TH) {
                    TH->DrawArrow(obsCheckStart, obsCheckStart + checkDir * obsRay1.distance, 10.f, 0.f,
                                  obsRay1.distance < minSpaceRequired ? 0xFF0000FF : 0x00FF00FF, 1.f);
                    TH->DrawArrow(obsCheckStart, obsCheckStart + normalizedDir * obsRay2.distance, 10.f, 0.f,
                                  obsRay2.didHit ? 0xFF0000FF : 0x00FF00FF, 1.f);
                }
            }
            /*********************************/

            if (obsRay2.didHit || obsRay1.distance < minSpaceRequired) {
                return true;  // Obstruction behind the ledge point
            }

            return false;
        }();

        if (obsFound) {
            continue;
        }

        foundLedge = true;
        break;
    }

    if (!foundLedge) {
        return ParkourType::NoLedge;
    }

    {
        // DON'T CLIMB ON DOORS FFS
        using ft = RE::FormType;
        const auto &ref = ledgeRay.hitObjectRef;
        if (ref) {
            // LOG("Climb point ref is: {}", RE::FormTypeToString(ref->GetObjectReference()->GetFormType()));
            if (ref->GetObjectReference()->GetFormType() == ft::Door) return ParkourType::NoLedge;
        }
    }

    const RE::NiPoint3 headRoomRayStart = RE::NiPoint3(playerPos.x, playerPos.y, ledgePoint.z - 5);  // On player at ledge height
    const RE::NiPoint3 sideDir = checkDir.Cross(upDir);

    // side rays
    // const float check_height = 121.85f * RuntimeVariables::PlayerScale;  // 120 vert, 80 degrees
    // constexpr float horz_angle = 0.1736f;                                // cos80
    // constexpr float vert_angle = 0.9848f;                                // sin80

    // const RE::NiPoint3 dir_L = (-sideDir * horz_angle + upDir * vert_angle);
    // const RE::NiPoint3 dir_R = (sideDir * horz_angle + upDir * vert_angle);
    // const auto midStart = ledgePoint + RE::NiPoint3(0, 0, 5);
    const auto leftStart = headRoomRayStart - sideDir * 15.f;
    const auto rightStart = headRoomRayStart + sideDir * 15.f;

    // const RE::NiPoint3 dir_M = upDir;
    const RE::NiPoint3 dir_L = upDir;
    const RE::NiPoint3 dir_R = upDir;

    // RayCastResult headRoomRay_M = RayCast(midStart, dir_M, playerHeight, COL_LAYER_EXTEND::kClimbObstruction);
    RayCastResult headRoomRay_L = RayCast(leftStart, dir_L, playerHeight, COL_LAYER_EXTEND::kClimbObstruction);
    RayCastResult headRoomRay_R = RayCast(rightStart, dir_R, playerHeight, COL_LAYER_EXTEND::kClimbObstruction);

    /* DEBUG LINES */
    if (ModSettings::_Debug_Draw_Lines) {
        const auto &TH = API_Handles::TrueHUD::Get();
        if (TH) {
            TH->DrawArrow(leftStart, leftStart + dir_L * headRoomRay_L.distance, 10.f, 0.f, headRoomRay_L.didHit ? 0xFF0000FF : 0x00FF00FF,
                          1.f);
            // TH->DrawArrow(midStart, midStart + dir_M * headRoomRay_M.distance, 10.f, 0.f, headRoomRay_M.didHit ? 0xFF0000FF : 0x00FF00FF,
            //               1.f);
            TH->DrawArrow(rightStart, rightStart + dir_R * headRoomRay_R.distance, 10.f, 0.f,
                          headRoomRay_R.didHit ? 0xFF0000FF : 0x00FF00FF, 1.f);
        }
    }
    /*********************************/

    if (headRoomRay_L.didHit || /*headRoomRay_M.didHit ||*/ headRoomRay_R.didHit) {
        return ParkourType::NoLedge;
    }

    return Parkouring::ChooseClimbHeight(player, playerHeight, ledgePoint, playerPos, ledgeRay);
}
int Parkouring::ChooseClimbHeight(RE::Actor *player, const float playerHeight, RE::NiPoint3 &ledgePoint, const RE::NiPoint3 &playerPos,
                                  RayCastResult &ledgeRay) {
    const float ledgePlayerDiff = ledgePoint.z - playerPos.z;
    if (IsSupportGroundedOrSliding(player) || PlayerIsSwimming()) {
        if (ledgePlayerDiff >= HardCodedVariables::highestLedgeLimit * RuntimeVariables::PlayerScale) {
            // Highest ledge
            const RE::NiPoint3 headRoomRayStart{playerPos.x, playerPos.y, ledgePoint.z};
            if (!ClimbExtraChecks(headRoomRayStart, playerHeight)) {
                return ParkourType::NoLedge;
            }

            if (ShouldClimbActionFail(player)) {
                return ParkourType::Failed;
            }
            return ParkourType::Highest;
        }
        else if (ledgePlayerDiff >= HardCodedVariables::highLedgeLimit * RuntimeVariables::PlayerScale) {
            // High ledge
            const RE::NiPoint3 headRoomRayStart{playerPos.x, playerPos.y, ledgePoint.z};
            if (!ClimbExtraChecks(headRoomRayStart, playerHeight)) {
                return ParkourType::NoLedge;
            }

            if (ShouldClimbActionFail(player)) {
                return ParkourType::Failed;
            }
            return ParkourType::High;
        }
        else if (ledgePlayerDiff >= HardCodedVariables::medLedgeLimit * RuntimeVariables::PlayerScale) {
            // Medium ledge
            const RE::NiPoint3 headRoomRayStart{playerPos.x, playerPos.y, ledgePoint.z};
            if (!ClimbExtraChecks(headRoomRayStart, playerHeight)) {
                return ParkourType::NoLedge;
            }

            return ParkourType::Medium;
        }
        else if (ledgePlayerDiff >= HardCodedVariables::lowLedgeLimit * RuntimeVariables::PlayerScale) {
            // Low ledge
            if (PlayerIsSwimming()) {
                return ParkourType::Grab;  // Grab ledge out of water
            }

            return ParkourType::Low;
        }
        else if (ledgePlayerDiff >= HardCodedVariables::highStepLimit * RuntimeVariables::PlayerScale) {
            // High Step
            if (PlayerIsSwimming()) {
                player->SetGraphVariableBool(SPPF_Grab_Variant, false);
                return ParkourType::Grab;  // Grab ledge out of water
            }

            if (StepsExtraChecks(player, ledgeRay)) {
                return ParkourType::StepHigh;
            }
        }
        else {
            // Low Step
            if (PlayerIsSwimming()) {
                player->SetGraphVariableBool(SPPF_Grab_Variant, false);
                return ParkourType::Grab;  // Grab ledge out of water
            }

            if (StepsExtraChecks(player, ledgeRay)) {
                return ParkourType::StepLow;
            }
        }
    }
    else if (IsSupportUnsupported(player)) {
        // We are midair, check for grab
        bool grabHighVariant = false;
        if (GrabExtraChecks(ledgePlayerDiff, ledgeRay, grabHighVariant)) {
            player->SetGraphVariableBool(SPPF_Grab_Variant, grabHighVariant);
            return ParkourType::Grab;
        }
    }
    return ParkourType::NoLedge;
}

int Parkouring::VaultCheck(RE::NiPoint3 &ledgePoint, RE::NiPoint3 checkDir, float vaultLength, float maxElevationIncrease,
                           float minVaultHeight, float maxVaultHeight) {
    const auto &player = GET_PLAYER;

    if (!IsSupportGrounded(player)) {
        return ParkourType::NoLedge;
    }

    if (!VaultExtraChecks(player)) {
        return ParkourType::NoLedge;
    }

    const auto &playerPos = player->GetPosition();
    const float playerHeight = 120 * RuntimeVariables::PlayerScale;

    /* Forward raycast to check if there is an obstruction at head level in vaultLength */

    const RE::NiPoint3 fwdRayStart = playerPos + RE::NiPoint3(0, 0, playerHeight);
    const float minSpaceRequired = 2 * vaultLength * RuntimeVariables::PlayerScale;

    RayCastResult fwdRay = RayCast(fwdRayStart, checkDir, minSpaceRequired, COL_LAYER_EXTEND::kVaultForward);
#ifdef LOG_VAULT
    LOG("Vault FWD: {}", PRINT_LAYER(fwdRay.layer));
#endif

    if (fwdRay.didHit && fwdRay.distance < minSpaceRequired && LAYERS_VAULT_FORWARD_RAY.contains(fwdRay.layer)) {
        return ParkourType::NoLedge;  // Obstruction behind the vaultable surface
    }

    /* Move forward by this steps, and RayCast downwards. If a valid layer is found, mark it. */
    constexpr int downIterations = 20;
    RE::NiPoint3 downRayDir(0, 0, -1);
    RayCastResult downRay;

    bool foundVaulter = false;
    float foundVaultHeight = -10000.0f;
    bool foundLanding = false;
    float foundLandingHeight = 10000.0f;
    float vaultableGap = playerHeight + 100.0f * RuntimeVariables::PlayerScale;

    // Incremental downward raycasts
    for (int i = 0; i < downIterations; i++) {
        float iDist = static_cast<float>(i) * 5.0f;
        RE::NiPoint3 downRayStart = playerPos + checkDir * iDist;
        downRayStart.z = fwdRayStart.z;

        downRay = RayCast(downRayStart, downRayDir, vaultableGap, COL_LAYER_EXTEND::kVaultDown);

        // If vault point is invalid layer, ignore
        if (LAYERS_VAULT_DOWN_RAY.contains(downRay.layer)) {
            continue;
        }

        {
            // DON'T CLIMB ON DOORS FFS
            using ft = RE::FormType;
            const auto &ref = downRay.hitObjectRef;
            if (ref) {
                // LOG("Vault point ref is: {}", RE::FormTypeToString(ref->GetObjectReference()->GetFormType()));
                if (ref->GetObjectReference()->GetFormType() == ft::Door) continue;
            }
        }

        const float hitHeight = (fwdRayStart.z - downRay.distance) - playerPos.z;

        // Check hit height for vaultable surfaces
        if (hitHeight > maxVaultHeight) {
            return ParkourType::NoLedge;  // Too high to vault
        }
        else if (hitHeight > minVaultHeight && hitHeight < maxVaultHeight) {
            if (hitHeight >= foundVaultHeight) {
                foundVaultHeight = hitHeight;
                foundLanding = false;
            }
            ledgePoint = downRayStart + downRayDir * downRay.distance;
            foundVaulter = true;
#ifdef LOG_VAULT
            LOG("Vault Down: {}", PRINT_LAYER(downRay.layer));
#endif
        }
        else if (foundVaulter && hitHeight < minVaultHeight) {
            foundLandingHeight = std::min(hitHeight, foundLandingHeight);
            foundLanding = true;
            break;
        }
    }

    // Vaulter & Landing exist
    if (foundVaulter && foundLanding && foundLandingHeight < maxElevationIncrease) {
        ledgePoint.z = playerPos.z + foundVaultHeight;

        /* Check if there's enough room */
        // Check if the structure is like a railing by casting an upwards ray on the valid ledge
        const RE::NiPoint3 upRayDir(0, 0, 1);
        const float halfPlayerHeight = playerHeight * 0.5f;
        const auto &upRayStart = ledgePoint + RE::NiPoint3(0, 0, 5);
        const RayCastResult upRay = RayCast(upRayStart, upRayDir, halfPlayerHeight, COL_LAYER_EXTEND::kVaultPostLedgeObstruction);

        /* DEBUG LINES */
        if (ModSettings::_Debug_Draw_Lines) {
            const auto &TH = API_Handles::TrueHUD::Get();
            if (TH) {
                TH->DrawArrow(upRayStart, upRayStart + upRayDir * upRay.distance, 10.f, 0.f, upRay.didHit ? 0xFF0000FF : 0x00FF00FF, 1.f);
            }
        }
        /**************************************/

        // Check if the structure is horizontally tiny by casting a sideways rays
        const RE::NiPoint3 sideRayDirR = RuntimeVariables::playerDirFlat.Cross(upRayDir);
        const RE::NiPoint3 sideRayDirL = -RuntimeVariables::playerDirFlat.Cross(upRayDir);

        float sideMaxCheckOffset;
        if (upRay.didHit) {
            if (upRay.distance < 55.f * RuntimeVariables::PlayerScale) return ParkourType::NoLedge;

            sideMaxCheckOffset = 30.f;
        }
        else {
            sideMaxCheckOffset = 15.f;
        }

        const auto &sideRayStart = upRayStart + RE::NiPoint3(0, 0, 5);
        const float sideMaxCheck = sideMaxCheckOffset * RuntimeVariables::PlayerScale;
        const RayCastResult sideRayR = RayCast(sideRayStart, sideRayDirR, sideMaxCheck, COL_LAYER_EXTEND::kVaultPostLedgeObstruction);
        const RayCastResult sideRayL = RayCast(sideRayStart, sideRayDirL, sideMaxCheck, COL_LAYER_EXTEND::kVaultPostLedgeObstruction);

        /* DEBUG LINES */
        if (ModSettings::_Debug_Draw_Lines) {
            const auto &TH = API_Handles::TrueHUD::Get();
            if (TH) {
                TH->DrawArrow(sideRayStart, sideRayStart + sideRayDirL * sideRayL.distance, 10.f, 0.f,
                              sideRayL.didHit ? 0xFF0000FF : 0x00FF00FF, 1.f);
            }
            if (TH) {
                TH->DrawArrow(sideRayStart, sideRayStart + sideRayDirR * sideRayR.distance, 10.f, 0.f,
                              sideRayR.didHit ? 0xFF0000FF : 0x00FF00FF, 1.f);
            }
        }
        /*********************************************/

        if (sideRayL.didHit || sideRayR.didHit) {
            return ParkourType::NoLedge;
        }

        return ParkourType::Vault;
    }

    return ParkourType::NoLedge;  // Vault failed
}

void Parkouring::OnStartStop(bool isStop, RE::Actor *actor) {
    // IS_START true / IS_STOP false

    const auto &ctrl = actor->GetCharController();

    if (isStop) {
        ctrl->flags.reset(RE::CHARACTER_FLAGS::kNoSim);
        actor->SetGraphVariableInt(SPPF_Ledge, ParkourType::NoLedge);

        // The other graph doesn't see the current graph, interrupt on stop to notify all
        // DO NOT SEND SPPF_STOP OR IT WILL RECURSE INFINITELY, STACK OVERFLOW AND CRASH
        actor->NotifyAnimationGraph(SPPF_INTERRUPT);

        using JA = Compatibility::JumpingAttack;
        if (JA::found) {
            if (ParkourUtility::IsActorWeaponOut(actor)) {
                actor->NotifyAnimationGraph(JA::event);
            }
        }

        /* Prevent actor flinging away if char ctrl state is kInAir, grounded preserves horizontal velocity but thresholds vertical */
        if (ctrl->context.currentState != RE::hkpCharacterStateType::kOnGround) ctrl->SetLinearVelocityImpl(ZERO_VECTOR);

        if (actor->IsPlayerRef()) {
            RuntimeVariables::RecoveryFramesActive = false;
            RuntimeVariables::ParkourInProgress = false;
        }
    }
    else /* if isStart */ {
        ParkourUtility::StopInteractions(*actor);

        // Disable simulation, fixes char controller taking over on hit
        ctrl->flags.set(RE::CHARACTER_FLAGS::kNoSim);
        ctrl->context.currentState = RE::hkpCharacterStateType::kOnGround;
    }

    if (actor->IsPlayerRef()) {
        const auto &ctrlMap = RE::ControlMap::GetSingleton();
        ctrlMap->ToggleControls(RE::ControlMap::UEFlag::kJumping, isStop);
        ctrlMap->ToggleControls(RE::ControlMap::UEFlag::kMainFour, isStop);  // Player tab menu & equip. Gets stuck if player uses TFC.
    }
}

void Parkouring::InterpolateRefToPosition(const RE::Actor *movingRef, RE::NiPoint3 to, float seconds) {
    auto vm = RE::BSScript::Internal::VirtualMachine::GetSingleton();
    if (!vm) {
        return;
    }

    /* Calculate speed from cur pos to target dist / time. BUT Read annotations relative to start position. */
    auto curPos = movingRef->GetPosition();
    RE::NiPoint3 relativeTranslatedToWorld = to;

    const auto diff = relativeTranslatedToWorld - curPos;

    float mult;
    movingRef->GetGraphVariableFloat(SPPF_SPEEDMULT, mult);
    if (mult <= 0.0f) {
        mult = 1.0f;
    }

    auto speed = seconds <= 0 ? 5000 : diff.Length() / seconds;  // Snap to pos if 0 or negative seconds
    speed *= mult;

    const auto time_mult_clamped = *g_gameTimeMult <= 0 ? 1 : *g_gameTimeMult;
    speed *= (1 / time_mult_clamped);  // This invalidates the SGTM factor of TranslateTo

    // Wrap movingRef in a Papyrus handle
    auto policy = vm->GetObjectHandlePolicy();
    RE::VMHandle handle = policy->GetHandleForObject(movingRef->GetFormType(), movingRef);
    if (handle == policy->EmptyHandle()) {
        return;
    }

    // Lookup the Papyrus-bound "ObjectReference" instance
    RE::BSFixedString scriptName = "ObjectReference";
    RE::BSFixedString functionName =
        "TranslateTo";  // For SplineTranslateTo, add std::move(float) between rz and speed. Does an overshoot, and pullback. Sometimes too strong.

    RE::BSTSmartPointer<RE::BSScript::Object> object;
    if (!vm->FindBoundObject(handle, scriptName.c_str(), object)) {
        return;
    }

    float px = relativeTranslatedToWorld.x;
    float py = relativeTranslatedToWorld.y;
    float pz = relativeTranslatedToWorld.z;
    float rx = movingRef->data.angle.x;
    float ry = movingRef->data.angle.y;
    float rz = movingRef->data.angle.z;
    float maxRotSpeed = 0.0f;

    // Build the IFunctionArguments with those locals:
    auto args = RE::MakeFunctionArguments(std::move(px),  // afX
                                          std::move(py),  // afY
                                          std::move(pz),  // afZ
                                          std::move(rx),  // afRX
                                          std::move(ry),  // afRY
                                          std::move(rz),  // afRZ
                                          //std::move(100.0f),
                                          std::move(speed), std::move(maxRotSpeed));

    StopInterpolatingRef(movingRef);

    // Call the Papyrus method
    RE::BSTSmartPointer<RE::BSScript::IStackCallbackFunctor> result;
    vm->DispatchMethodCall1(object,        // the Papyrus ObjectReference instance
                            functionName,  // "TranslateTo"
                            args,          // packed arguments
                            result);
}
void Parkouring::StopInterpolatingRef(const RE::Actor *actor) {
    auto movingRef = actor;
    auto vm = RE::BSScript::Internal::VirtualMachine::GetSingleton();
    if (!vm) {
        return;
    }
    auto policy = vm->GetObjectHandlePolicy();
    RE::VMHandle handle = policy->GetHandleForObject(movingRef->GetFormType(), movingRef);
    if (handle == policy->EmptyHandle()) {
        return;
    }

    RE::BSFixedString scriptName = "ObjectReference";
    RE::BSFixedString functionName = "StopTranslation";

    RE::BSTSmartPointer<RE::BSScript::Object> object;
    if (!vm->FindBoundObject(handle, scriptName.c_str(), object)) {
        return;
    }

    auto args = RE::MakeFunctionArguments();

    RE::BSTSmartPointer<RE::BSScript::IStackCallbackFunctor> result;
    vm->DispatchMethodCall1(object,  // the Papyrus ObjectReference instance
                            functionName,
                            args,  // packed arguments
                            result);
}

void Parkouring::CalculateStartingPosition(const RE::Actor *actor, int ledgeType, RE::NiPoint3 &out) {
    float zAdjust = 0;
    float z = 0;
    float backOffset = 55.0f;
    RE::NiPoint3 backwardAdjustment;

    switch (ledgeType) {
        case 8:  // Highest Ledge
            z = HardCodedVariables::highestLedgeElevation - 5;
            backOffset = 62.f;
            break;

        case 7:  // High ledge
            z = HardCodedVariables::highLedgeElevation - 5;
            break;

        case 6:  // Medium ledge
            z = HardCodedVariables::medLedgeElevation - 5;
            break;

        case 5:  // Low ledge
            z = HardCodedVariables::lowLedgeElevation - 5;
            break;

        case 4:  // Step High
            z = HardCodedVariables::stepHighElevation - 5;
            backOffset = 15;  // Override backward offset
            break;

        case 3:  // Step Low
            z = HardCodedVariables::stepLowElevation - 5;
            backOffset = 15;  // Override backward offset
            break;

        case 2:  // Vault
            z = HardCodedVariables::vaultElevation - 5;
            break;

        case 1:  // Grab (Midair or Out of Water)
            bool grabHighVariant;
            actor->GetGraphVariableBool(SPPF_Grab_Variant, grabHighVariant);
            z = (grabHighVariant ? HardCodedVariables::grabHighElevation : HardCodedVariables::grabElevation) - 5;
            backOffset = 45;  // Override backward offset
            break;

        case 0:  // Failed (Low Stamina Animation)
            break;
        default:
            ERROR(" >> START POSITION NOT SET, INVALID LEDGE TYPE {} <<", ledgeType);
            return;
    }

    zAdjust = -z * RuntimeVariables::PlayerScale;
    backwardAdjustment = RuntimeVariables::PlayerScale * RuntimeVariables::playerDirFlat * backOffset;

    out = RE::NiPoint3{RuntimeVariables::ledgePoint.x - backwardAdjustment.x, RuntimeVariables::ledgePoint.y - backwardAdjustment.y,
                       ledgeType == ParkourType::Failed ? actor->GetPositionZ() : RuntimeVariables::ledgePoint.z + zAdjust};
}

void Parkouring::InvalidateVars() {
    RuntimeVariables::selectedLedgeType = -1;

    using sppf = Scaleform::SkyParkourMenu;
    const auto &ui = RE::UI::GetSingleton();
    if (!ui) return;

    const auto &menu = ui->GetMenu<sppf>(sppf::MENU_NAME);
    if (!menu) return;

    menu->SetActiveIndicatorType(sppf::IndicatorType::kInvisible);
}

void Parkouring::UpdateIndicatorMenu() {
    if (!ModSettings::Use_Indicators) {
        return;
    }

    using sppf = Scaleform::SkyParkourMenu;
    const auto &ui = RE::UI::GetSingleton();
    if (!ui) return;

    const auto &menu = ui->GetMenu<sppf>(sppf::MENU_NAME);
    if (!menu || !menu->IsOpen()) return;

    sppf::IndicatorType indic;
    const auto ledge = RuntimeVariables::selectedLedgeType;

    switch (ledge) {
        case ParkourType::NoLedge:
            indic = sppf::IndicatorType::kInvisible;
            break;
        case ParkourType::Failed:
            indic = sppf::IndicatorType::kOutOfStamina;
            break;
        case ParkourType::Vault:
            indic = sppf::IndicatorType::kVault;
            break;
        default:
            indic = sppf::IndicatorType::kClimb;
    }
    menu->SetActiveIndicatorType(indic);
}

void Parkouring::UpdateParkourPoint() {
    if (RuntimeVariables::ParkourInProgress) {
        InvalidateVars();
        return;
    }

    RuntimeVariables::selectedLedgeType = GetLedgePoint();

    _THREAD_POOL.enqueue([]() {
        std::unique_lock<std::mutex> lock(g_UpdateLock, std::try_to_lock);
        if (!lock.owns_lock()) {
            return;
        }

        const auto &player = GET_PLAYER;
        RuntimeVariables::IsParkourActive = IsParkourActiveFor(player);
        RuntimeVariables::PlayerScale = ScaleUtility::GetScale();
        RuntimeVariables::playerDirFlat = GetActorDirFlat(player);
    });

    UpdateIndicatorMenu();
}

bool Parkouring::TryActivateParkour() {
    std::unique_lock<std::mutex> lock(g_ParkourActivateLock, std::try_to_lock);
    if (!lock.owns_lock()) {
        return false;
    }

    const auto &player = GET_PLAYER;
    const auto &LedgeTypeToProcess = RuntimeVariables::selectedLedgeType;

    if (LedgeTypeToProcess == ParkourType::NoLedge) {
        return false;
    }

    bool Ongoing;
    if (player->GetGraphVariableBool(SPPF_ONGOING, Ongoing) && Ongoing) {
        return false;
    }

    float turningDelta;
    player->GetGraphVariableFloat("TurnDelta", turningDelta);
    if (turningDelta > 50.0f) {
        return false;
    }

    if (!RuntimeVariables::IsParkourActive || RuntimeVariables::IsMenuOpen) {
        return false;
    }

    const bool isMoving = player->IsMoving();
    const bool lowEffort = CheckActionRequiresLowEffort(LedgeTypeToProcess);
    const bool isSwimming = PlayerIsSwimming();
    // const bool isSprinting = player->IsSprinting();

    const auto &fallTime = player->GetCharController()->fallTime;
    const bool avoidOnGroundParkour = fallTime > 0.0f;
    const bool avoidMidairParkour = fallTime < 0.17f;  // Timeout activation immediately after jumping
    //LOG(">> Fall time: {}", fallTime);

    if (LedgeTypeToProcess != ParkourType::Grab) {
        if (avoidOnGroundParkour) {
            return false;
        }
    }
    else {
        if (avoidMidairParkour && !isSwimming) {
            return false;
        }
    }

    /* Cancel if moving, but allow movement during swimming */
    if (ModSettings::Smart_Climb && isMoving && !isSwimming) {
        if (!lowEffort) {
            return false;
        }
    }

    if (!HavokUtil::ValidateBehaviorPatch(player)) return false;

    RuntimeVariables::ParkourInProgress = true;

    /* Also pass swimming state for stamina calculation logic */
    ParkourReadyRun(LedgeTypeToProcess);

    return true;
}
void Parkouring::ParkourReadyRun(int32_t ledgeType) {
    const auto &player = GET_PLAYER;
    //auto dist = player->GetPosition().GetDistance(RuntimeVariables::ledgePoint);
    //LOG("Dist: {}", dist);

    player->SetGraphVariableInt(SPPF_Ledge, ledgeType);

    RE::NiPoint3 startPos;
    Parkouring::CalculateStartingPosition(player, ledgeType, startPos);

    const float timeOfAdjust = ledgeType == ParkourType::Grab ? 0.1f : 0.15f; /* Grab's gotta be more precise */
    InterpolateRefToPosition(player, startPos, timeOfAdjust);

    _THREAD_POOL.enqueue([player, ledgeType, startPos] {
        auto startTime = std::chrono::high_resolution_clock::now();
        long long elapsedMS;
        do {
            elapsedMS =
                std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - startTime).count();

        } while (elapsedMS < 100 && (player->GetPosition().GetDistance(startPos) >= 1.0f));

        _TASK_Q([player, ledgeType] {
            if (IsActorWeaponOut(player) && (ledgeType == ParkourType::StepHigh || ledgeType == ParkourType::StepLow)) {
                player->SetGraphVariableBool(SPPF_Lower_Body_Only, true);
            }
            else {
                player->SetGraphVariableBool(SPPF_Lower_Body_Only, false);
            }

            player->NotifyAnimationGraph(SPPF_NOTIFY);

            StopInterpolatingRef(player);
        });
    });
}
void Parkouring::PostParkourStaminaDamage(RE::Actor *actor, bool isLowEffort, bool isSwimming) {
    if (ModSettings::Enable_Stamina_Consumption) {
        if (actor->IsPlayerRef()) {
            const RE::PlayerCharacter *pl = actor->As<RE::PlayerCharacter>();
            if (pl->IsGodMode()) {
                return;
            }
        }

        float cost = ParkourUtility::CalculateParkourStamina(actor);

        /* If swimming, fail animation won't play. So no need to flash the bar. Just consume half the stamina cost like low effort. */
        if (isLowEffort || isSwimming) {
            // LOG("cost{}", cost / 2);
            DamageActorStamina(actor, cost / 2);
        }
        else if (ActorHasEnoughStamina(actor)) {
            // LOG("cost{}", cost);
            DamageActorStamina(actor, cost);
        }
        else {
            RE::HUDMenu::FlashMeter(RE::ActorValue::kStamina);
        }
        actor->UpdateRegenDelay(RE::ActorValue::kStamina, 2.0f);
    }
}

void Parkouring::SetParkourOnOff(bool turnOn) {
    if (turnOn) {
        if (!ButtonEventListener::GetSingleton()->SinkRegistered) {
            ButtonEventListener::Register();
            LOG("Processing: < ON >");
        }
    }
    else {
        if (ButtonEventListener::GetSingleton()->SinkRegistered) {
            ButtonEventListener::Unregister();
            LOG("Processing: < Off >");
        }

        RuntimeMethods::ResetRuntimeVariables();
    }
}