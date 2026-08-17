#include "Parkouring.h"
#include "Util/ParkourUtility.h"
#include "Util/ScaleUtility.h"
#include "Util/HavokUtil.hpp"
#include "Listeners/ButtonListener.h"

#include "ModSettings/ModSettings.hpp"
#include "_References/Compatibility.h"
#include "_References/ParkourType.h"
#include "_References/RuntimeVariables.h"
#include "_References/HardcodedVariables.h"
#include "_References/RuntimeMethods.h"

#include "API/API_Handles.h"

#include "HUD/Scaleform/SkyParkourMenu.hpp"

static std::mutex g_ParkourActivateLock;

using namespace ParkourUtility;

ParkourType Parkouring::GetLedgePoint(RayCastResult &out_LedgeRay)
{
    using namespace ModSettings;
    using pt = ParkourType;
    namespace hv = HardCodedVariables;
    namespace rt = RuntimeVariables;

    const auto player = GET_PLAYER;
    const auto facingDir = GetActorDirFlat(player);
    const auto scale = RuntimeVariables::PlayerScale = ScaleUtility::GetScale(player);

    // Perform ledge or vault checks
    pt selectedLedgeType = pt::NoLedge;

    RE::NiPoint3 ledgePoint;

    selectedLedgeType = VaultCheck(ledgePoint, facingDir, hv::vaultMinHeight * scale, hv::vaultMaxHeight * scale, out_LedgeRay);

    if (selectedLedgeType == pt::NoLedge)
    {
        selectedLedgeType = ClimbCheck(ledgePoint, facingDir, hv::climbMinHeight * scale, hv::climbMaxHeight * scale, out_LedgeRay);
    }

    if (selectedLedgeType == pt::NoLedge) return pt::NoLedge;

    // Don't ever parkour into water, last check before saying this ledge is valid
    float waterLevel{-200000.0f};
    auto parentCell = player->GetParentCell();
    if (parentCell)
    {
        parentCell->GetWaterHeight(player->GetPosition(), waterLevel);
    }

    constexpr int validWaterDepth = 10;
    if (ledgePoint.z < waterLevel - validWaterDepth) return pt::NoLedge;

    rt::ledgePoint = ledgePoint;

    return selectedLedgeType;
}

ParkourType Parkouring::ClimbCheck(RE::NiPoint3 &ledgePoint, RE::NiPoint3 facingDir, float minLedgeHeight, float maxLedgeHeight,
                                   RayCastResult &out_LedgeRay)
{
    const auto player = GET_PLAYER;
    const auto playerPos = player->GetPosition();

    constexpr RE::NiPoint3 upDir(0, 0, 1);
    constexpr RE::NiPoint3 downDir(0, 0, -1);

    // Constants adjusted for player scale
    const float playerHeight = 120.f * RuntimeVariables::PlayerScale;
    const float fwdCheckStep = 5;                                               // 8
    const float dynamicIter = player->GetCharController()->speedPct * 15 + 15;  // Dynamically scale check dist to speed
    const int fwdCheckIterations = static_cast<int>(dynamicIter);               // 15
    const float minLedgeFlatness = 0.3f;                                        // 0.5

    // Pick the ceiling, and make that the cast start to prevent cancelling lower level climbing in lower ceiling places
    const RE::NiPoint3 ceilPickStart = playerPos + RE::NiPoint3(0, 0, playerHeight);
    const float ceilPickRad = 5.f;

    ShapeCastResult ceilPick =
        HavokUtil::ShapeCast(ceilPickStart, upDir, maxLedgeHeight - playerHeight, ceilPickRad, COL_LAYER_EXTEND::kClimbObstruction);

    const float ledgeRayMaxCheck =
        maxLedgeHeight - minLedgeHeight - (ceilPick.didHit ? maxLedgeHeight - playerHeight - ceilPick.distance : 0);

    // Forward raycast initialization
    RE::NiPoint3 fwdRayStart = playerPos;
    fwdRayStart.z = ceilPick.didHit ? ceilPick.hitPos.z : (playerPos.z + maxLedgeHeight);

    RayCastResult ledgeRay{};
    bool foundLedge = false;
    float normalZ = 0;

    // Incremental forward raycast to find a ledge
    for (int i = 0; i < fwdCheckIterations; i++)
    {
        const auto curFwdProbeLen = fwdCheckStep * i;

        RayCastResult fwdRay = HavokUtil::RayCast(fwdRayStart, facingDir, curFwdProbeLen, COL_LAYER_EXTEND::kClimbObstruction);

        [[unlikely]] if (ModSettings::_Debug_Enabled) { fwdRay.Debug_Visualize(COLOR_HEX_B); }

        /* If probe hits smth, skip  */
        if (fwdRay.distance < curFwdProbeLen) continue;

        // Downward raycast to detect ledge point
        RE::NiPoint3 ledgeRayStart = fwdRayStart + facingDir * fwdRay.distance;

        ledgeRay = HavokUtil::RayCast(ledgeRayStart, downDir, ledgeRayMaxCheck, COL_LAYER_EXTEND::kClimbLedge);

        /* There is no one solution fits all collision layer mask for raycast, so v3.5.0 iterates over all hits for climb and filters their layers*/
        for (auto &&hit: ledgeRay.hits)
        {
            if (!LAYERS_CLIMB_EXCLUDE.contains(hit.rootCollidable->GetCollisionLayer()))
            {
                const auto hitRef = RE::TESHavokUtilities::FindCollidableRef(*hit.rootCollidable);
                if (hitRef)
                { /* can be null */
                    ledgeRay.distance = hit.hitFraction * ledgeRayMaxCheck;
                    ledgeRay.normalOut = hit.normal;
                    ledgeRay.layer = hit.rootCollidable->GetCollisionLayer();
                    ledgeRay.hitObjectRef = hitRef;
                    break;
                }
            }
        }

        /* DEBUG LINES */
        [[unlikely]] if (ModSettings::_Debug_Enabled)
        {
            const auto TH = API_Handles::TrueHUD::Get();
            if (TH)
            {
                /* Draw min/max heights set in config */
                const auto limitDrawStartMin = playerPos + RE::NiPoint3(0, 0, minLedgeHeight);
                const auto limitDrawStartMax = ceilPick.didHit ? ceilPick.hitPos : playerPos + RE::NiPoint3(0, 0, maxLedgeHeight);
                const auto offset = facingDir * fwdCheckStep * static_cast<float>(fwdCheckIterations);

                TH->DrawLine(limitDrawStartMin, limitDrawStartMin + offset, 0.f, COLOR_HEX_Y, 2.f);
                TH->DrawLine(limitDrawStartMax, limitDrawStartMax + offset, 0.f, COLOR_HEX_Y, 2.f);

                /* Show all downward hits */
                float prevhitDist = 0;
                for (auto &&hit: ledgeRay.hits)
                {
                    TH->DrawArrow(ledgeRayStart + RE::NiPoint3(0, 0, -prevhitDist),
                                  ledgeRayStart + downDir * hit.hitFraction * ledgeRayMaxCheck, 10.f, 0.f, COLOR_HEX_B, 1.f);

                    prevhitDist = hit.hitFraction * ledgeRayMaxCheck;
                }
            }
            // DEBUG_PRINT("Hit count: {}", ledgeRay.hits.size());
        } /*********************************/

        ledgePoint = ledgeRayStart + downDir * ledgeRay.distance;
        normalZ = ledgeRay.normalOut.quad.m128_f32[2];

        // Validate ledge
        if (LAYERS_CLIMB_EXCLUDE.contains(ledgeRay.layer)) continue;
        if (normalZ < minLedgeFlatness) continue;
        if (ledgePoint.z < playerPos.z + minLedgeHeight) continue;
        if (ledgePoint.z > playerPos.z + maxLedgeHeight) continue;

        // Check for obstructions behind the Ledge point
        const bool obsFound = [ledgePoint, facingDir] {
            constexpr float obsBackOffset = 15.f;
            const float ray1MaxDist = obsBackOffset + 15.f;
            const float minSpaceRequired = obsBackOffset + 3.f;

            /* 2 rays. if 1 distance is too low, not valid. if 1 passes, cast another ray with the ledge normal obtained from 1.
            this prevents extending the distance by looking slightly sideways */

            const RE::NiPoint3 obsCheckStart = RE::NiPoint3(ledgePoint.x, ledgePoint.y, ledgePoint.z + 5.f) - facingDir * obsBackOffset;

            RayCastResult obsRay1 = HavokUtil::RayCast(obsCheckStart, facingDir, ray1MaxDist, COL_LAYER_EXTEND::kClimbObstruction);

            auto normalizedDir = -VEC4_TO_VEC3(obsRay1.normalOut);
            normalizedDir.z = 0;

            RayCastResult obsRay2 = HavokUtil::RayCast(obsCheckStart, normalizedDir, minSpaceRequired, COL_LAYER_EXTEND::kClimbObstruction);

            [[unlikely]] if (ModSettings::_Debug_Enabled)
            {
                obsRay1.Debug_Visualize(obsRay1.distance < minSpaceRequired ? COLOR_HEX_R : COLOR_HEX_G);
                obsRay2.Debug_Visualize();
            }

            if (obsRay2.didHit || obsRay1.distance < minSpaceRequired) return true;  // Obstruction behind the ledge point

            return false;
        }();

        if (obsFound) continue;

        foundLedge = true;
        break;
    }

    if (!foundLedge) return ParkourType::NoLedge;

    out_LedgeRay = ledgeRay;

    // DON'T CLIMB ON DOORS FFS
    if (FORMS_CLIMB_EXCLUDE.contains(ledgeRay.GetHitObjectFormType_Safe())) return ParkourType::NoLedge;

    const float headRoomRayRadius = 15.f;
    const RE::NiPoint3 headRoomRayStart = ledgePoint + RE::NiPoint3(0, 0, headRoomRayRadius);

    ShapeCastResult headRoomCast = HavokUtil::ShapeCast(headRoomRayStart, upDir, playerHeight - headRoomRayRadius, headRoomRayRadius,
                                                        COL_LAYER_EXTEND::kClimbObstruction);
    [[unlikely]] if (ModSettings::_Debug_Enabled) { headRoomCast.Debug_Visualize(); }

    if (headRoomCast.didHit) return ParkourType::NoLedge;

    // Check if the ledge point is inbetween tiny gaps, where player can't pass through
    const float upCastRadius = 10.f;
    const float upCastDist = ledgePoint.z - playerPos.z - playerHeight;

    RE::NiPoint3 climbHeadRoomRayStart = RE::NiPoint3(playerPos.x, playerPos.y, playerPos.z + playerHeight) - upCastRadius * facingDir;

    ShapeCastResult upCast =
        HavokUtil::ShapeCast(climbHeadRoomRayStart, upDir, upCastDist, upCastRadius, COL_LAYER_EXTEND::kClimbObstruction);

    [[unlikely]] if (ModSettings::_Debug_Enabled) { upCast.Debug_Visualize(); }
    if (upCast.didHit) return ParkourType::NoLedge;

    // Check if the actor can 'see' the ledge, there's no objects between

    constexpr float visionCastRad = 5.f;
    constexpr float visionOffset = 5.f + visionCastRad;
    const RE::NiPoint3 plPosLedgeHeightOffsetByRad = RE::NiPoint3(playerPos.x, playerPos.y, ledgePoint.z + visionOffset);
    const RE::NiPoint3 ledgePosOffsetByRad = ledgePoint + RE::NiPoint3(0, 0, visionOffset);
    const float visionCastDist = plPosLedgeHeightOffsetByRad.GetDistance(ledgePosOffsetByRad);

    ShapeCastResult visionCast =
        HavokUtil::ShapeCast(plPosLedgeHeightOffsetByRad, facingDir, visionCastDist, visionCastRad, COL_LAYER_EXTEND::kClimbObstruction);

    [[unlikely]] if (ModSettings::_Debug_Enabled) { visionCast.Debug_Visualize(); }

    if (visionCast.didHit) return ParkourType::NoLedge;

    return Parkouring::ChooseClimbHeight(player, ledgePoint, playerPos);
}
ParkourType Parkouring::ChooseClimbHeight(RE::Actor *player, RE::NiPoint3 &ledgePoint, const RE::NiPoint3 &playerPos)
{
    const float scale = RuntimeVariables::PlayerScale;
    const float ledgePlayerDiff = (ledgePoint.z - playerPos.z) / (scale <= 0 ? 1 : scale);

    // const bool isMidair = player->IsInMidair();
    using cf = RE::CHARACTER_FLAGS;
    const bool isGrounded = player->GetCharController()->flags.any(cf::kSupport, cf::kHasPotentialSupportManifold);

    if (isGrounded || PlayerIsSwimming())
    {
        if (ledgePlayerDiff >= HardCodedVariables::highestLedgeLimit)
        {
            // Highest ledge

            if (!SmartClimbCheck(player)) return ParkourType::NoLedge;
            if (ShouldClimbActionFail(player)) return ParkourType::Failed;

            return ParkourType::Highest;
        }
        else if (ledgePlayerDiff >= HardCodedVariables::highLedgeLimit)
        {
            // High ledge

            if (!SmartClimbCheck(player)) return ParkourType::NoLedge;
            if (ShouldClimbActionFail(player)) return ParkourType::Failed;

            return ParkourType::High;
        }
        else if (ledgePlayerDiff >= HardCodedVariables::medLedgeLimit)
        {
            // Medium ledge

            return ParkourType::Medium;
        }
        else if (ledgePlayerDiff >= HardCodedVariables::lowLedgeLimit)
        {
            // Low ledge
            if (PlayerIsSwimming()) return ParkourType::Grab;  // Grab ledge out of water

            return ParkourType::Low;
        }
        else if (ledgePlayerDiff >= HardCodedVariables::highStepLimit)
        {
            // High Step
            if (PlayerIsSwimming()) return ParkourType::Grab;  // Grab ledge out of water

            if (StepsExtraChecks(player, ledgePlayerDiff, ledgePoint)) return ParkourType::StepHigh;
        }
        else
        {
            // Low Step
            if (PlayerIsSwimming()) return ParkourType::Grab;  // Grab ledge out of water

            if (StepsExtraChecks(player, ledgePlayerDiff, ledgePoint)) return ParkourType::StepLow;
        }
    }
    else if (!isGrounded)
    {
        // We are midair, check for grab
        bool out_grabHighVariant{false};
        if (!GrabExtraChecks(player, ledgePlayerDiff, out_grabHighVariant, ledgePoint)) return ParkourType::NoLedge;

        player->SetGraphVariableBool(SPPF_Grab_Variant, out_grabHighVariant);
        return ParkourType::Grab;
    }
    return ParkourType::NoLedge;
}

ParkourType Parkouring::VaultCheck(RE::NiPoint3 &ledgePoint, RE::NiPoint3 fwdDir, float minVaultHeight, float maxVaultHeight,
                                   RayCastResult &out_LedgeRay)
{
    const auto player = GET_PLAYER;

    if (!VaultExtraChecks(player)) return ParkourType::NoLedge;

    constexpr RE::NiPoint3 upDir{0, 0, 1};
    constexpr RE::NiPoint3 downDir(0, 0, -1);

    const auto playerPos = player->GetPosition();
    const float playerHeight = 120.f * RuntimeVariables::PlayerScale;
    const RE::NiPoint3 plHeadPos = playerPos + RE::NiPoint3(0, 0, playerHeight);

    /* Move forward by this steps, and RayCast downwards. If a valid layer is found, mark it. */
    const float dynamicIter = player->GetCharController()->speedPct * 5 + 15;  // Dynamically scale check dist to speed
    const int downIterations = static_cast<int>(dynamicIter);                  // 15

    // Step by step, go fwd and cast down. Find a vaulter and validate
    // RayCastResult downRay;
    ShapeCastResult downCast;

    bool foundVaulter = false;
    float foundVaultHeight = -10000.0f;
    const float lookForVaultDist = playerHeight;

    RE::NiPoint3 downRayStart;
    // Incremental downward raycasts
    for (int i = 0; i < downIterations; i++)
    {
        float iDist = static_cast<float>(i) * 5.0f;
        downRayStart = playerPos + fwdDir * iDist;
        downRayStart.z = plHeadPos.z;
        constexpr float rad = 5.f;

        // downRay = HavokUtil::RayCast(downRayStart, downDir, lookForVaultDist, COL_LAYER_EXTEND::kVaultDown);
        downCast = HavokUtil::ShapeCast(downRayStart, downDir, lookForVaultDist, rad, COL_LAYER_EXTEND::kVaultDown);

        // If vault point is invalid layer or form type, ignore
        if (LAYERS_VAULT_DOWN_RAY.contains(downCast.layer)) continue;
        if (FORMS_VAULT_EXCLUDE.contains(downCast.GetHitObjectFormType_Safe())) continue;

        const float ledgeHeight = (plHeadPos.z - downCast.distance) - playerPos.z;

        // Check hit height for vaultable surfaces
        if (ledgeHeight > maxVaultHeight)
        {
            return ParkourType::NoLedge;  // Too high to vault
        }
        else if (ledgeHeight > minVaultHeight)
        {
            if (ledgeHeight >= foundVaultHeight)
            {
                foundVaultHeight = ledgeHeight;
            }
            ledgePoint = downRayStart + downDir * downCast.distance;
            foundVaulter = true;
            break;
        }
    }

    if (!foundVaulter) return ParkourType::NoLedge;

    ledgePoint.z = playerPos.z + foundVaultHeight;

    [[unlikely]] if (ModSettings::_Debug_Enabled) { downCast.Debug_Visualize(COLOR_HEX_B); }

    // out_LedgeRay = downRay;
    out_LedgeRay = SkyParkour::ShapeToRayCast(downCast);

    /* Check if ledge has obstruction above */
    const float halfPlayerHeight = playerHeight * 0.5f;
    const auto upCastStart = ledgePoint + RE::NiPoint3(0, 0, 5);
    const float upCastRadius = 10.f;

    ShapeCastResult upCast =
        HavokUtil::ShapeCast(upCastStart, upDir, halfPlayerHeight, upCastRadius, COL_LAYER_EXTEND::kVaultPostLedgeObstruction);

    [[unlikely]] if (ModSettings::_Debug_Enabled) { upCast.Debug_Visualize(); }

    if (upCast.didHit) return ParkourType::NoLedge;

    /* Check if landing spot has obstruction forward */
    const float fwdObsRadius = 10.f;
    const float fwdObsOffset = 45.f;
    const RE::NiPoint3 downObsCastStart = RE::NiPoint3(ledgePoint.x, ledgePoint.y, plHeadPos.z) + fwdDir * (fwdObsOffset + fwdObsRadius);

    ShapeCastResult downObsCast = HavokUtil::ShapeCast(downObsCastStart, downDir, playerHeight - minVaultHeight, fwdObsRadius,
                                                       COL_LAYER_EXTEND::kVaultPostLedgeObstruction);

    const bool postVaultHasGap = [downObsCast, downCast, playerPos]() {
        if (!downObsCast.didHit) return true;
        if (downCast.distance >= downObsCast.distance + 10.f) return false;
        if (downObsCast.hitPos.z >= downCast.hitPos.z - 10.f) return false;
        // const auto downCastNormals = downObsCast.normalOut.quad.m128_f32;

        // const float x = downCastNormals[0];
        // const float y = downCastNormals[1];
        // const float z = downCastNormals[2];

        // return z > 0.9f && std::abs(x) < 0.1f && std::abs(y) < 0.1f;
        return true;
    }();

    [[unlikely]] if (ModSettings::_Debug_Enabled) { downObsCast.Debug_Visualize(!postVaultHasGap ? COLOR_HEX_R : COLOR_HEX_G); }
    if (!postVaultHasGap) return ParkourType::NoLedge;

    constexpr float visionCastRad = 5.f;
    constexpr float visionOffset = 5.f + visionCastRad;
    const RE::NiPoint3 plPosLedgeHeightOffsetByRad = RE::NiPoint3(playerPos.x, playerPos.y, ledgePoint.z + visionOffset);
    const RE::NiPoint3 visionCastEnd = RE::NiPoint3(downObsCastStart.x, downObsCastStart.y, plPosLedgeHeightOffsetByRad.z);
    const float visionCastDist = plPosLedgeHeightOffsetByRad.GetDistance(visionCastEnd);

    ShapeCastResult visionCast =
        HavokUtil::ShapeCast(plPosLedgeHeightOffsetByRad, fwdDir, visionCastDist, visionCastRad, COL_LAYER_EXTEND::kClimbObstruction);

    [[unlikely]] if (ModSettings::_Debug_Enabled) { visionCast.Debug_Visualize(); }

    if (visionCast.didHit) return ParkourType::NoLedge;

    return ParkourType::Vault;
}

void Parkouring::OnStartStop(bool isStop, RE::Actor *actor)
{
    // IS_START true / IS_STOP false
    constexpr auto charFlag = RE::CHARACTER_FLAGS::kNoSim;
    const auto ctrl = actor->GetCharController();

    if (isStop)
    {
        ctrl->flags.reset(charFlag);
        actor->SetGraphVariableInt(SPPF_Ledge, std::to_underlying(ParkourType::NoLedge));

        // The other graph doesn't see the current graph, interrupt on stop to notify all
        // DO NOT SEND SPPF_STOP OR IT WILL RECURSE INFINITELY, STACK OVERFLOW AND CRASH
        actor->NotifyAnimationGraph(SPPF_INTERRUPT);

        using JA = Compatibility::JumpingAttack;
        if (JA::found)
        {
            if (ParkourUtility::IsActorWeaponOut(actor))
            {
                actor->NotifyAnimationGraph(JA::event);
            }
        }

        if (Compatibility::TrueDirectionalMovement::found)
        {
            API_Handles::TDM::ReleaseYaw();
        }

        /* Prevent actor flinging away if char ctrl state is kInAir by clamping velocity */
        if (ctrl->context.currentState != RE::hkpCharacterStateType::kOnGround)
        {
            [&ctrl] {
                RE::hkVector4 out;
                ctrl->GetLinearVelocityImpl(out);
                out.quad.m128_f32[2] = 0;  // 0 the vert component (z)
                out = out / (out.Length3() <= 0 ? 1 : out.Length3());

                const auto fwdVec3 = VEC4_TO_VEC3(ctrl->forwardVec * -1);
                out = VEC3_TO_VEC4(fwdVec3);

                ctrl->SetLinearVelocityImpl(out);
            }();
        }

        if (actor->IsPlayerRef())
        {
            RuntimeVariables::RecoveryFramesActive = false;
            RuntimeVariables::ParkourInProgress = false;
        }
    }
    else /* if isStart */
    {
        ParkourUtility::StopInteractions(*actor);

        // Disable simulation, fixes char controller taking over on hit & enables vertical root motion
        ctrl->flags.set(charFlag);

        if (Compatibility::TrueDirectionalMovement::found)
        {
            API_Handles::TDM::ObtainYaw(true);
        }
    }

    if (actor->IsPlayerRef())
    {
        const auto ctrlMap = RE::ControlMap::GetSingleton();
        if (ctrlMap)
        {
            ctrlMap->ToggleControls(RE::ControlMap::UEFlag::kJumping, isStop, true);
            ctrlMap->ToggleControls(RE::ControlMap::UEFlag::kMainFour, isStop,
                                    true);  // Player tab menu & equip. Gets stuck if player uses TFC.
        }
    }
}

bool Parkouring::CalculateStartingPosition(RE::Actor *actor, ParkourType ledgeType, RE::NiPoint3 &out)
{
    float zAdjust{0};
    float z{0};
    float backOffset{55.0f};
    RE::NiPoint3 backwardAdjustment;

    switch (ledgeType)
    {
        case ParkourType::Highest:
            z = HardCodedVariables::highestLedgeElevation - 5;
            backOffset = 62.f;
            break;

        case ParkourType::High:
            z = HardCodedVariables::highLedgeElevation - 5;
            break;

        case ParkourType::Medium:
            z = HardCodedVariables::medLedgeElevation - 5;
            break;

        case ParkourType::Low:
            z = HardCodedVariables::lowLedgeElevation - 5;
            break;

        case ParkourType::StepHigh:
            z = HardCodedVariables::stepHighElevation - 5;
            backOffset = 15;  // Override backward offset
            break;

        case ParkourType::StepLow:
            z = HardCodedVariables::stepLowElevation - 5;
            backOffset = 15;  // Override backward offset
            break;

        case ParkourType::Vault:
            z = HardCodedVariables::vaultElevation - 5;
            break;

        case ParkourType::Grab:
            bool grabHighVariant;
            actor->GetGraphVariableBool(SPPF_Grab_Variant, grabHighVariant);
            z = (grabHighVariant ? HardCodedVariables::grabHighElevation : HardCodedVariables::grabElevation) - 5;
            backOffset = 45;  // Override backward offset
            break;

        case ParkourType::Failed:  // Low Stamina Animation
            break;

        default:
            ERROR(" >> START POSITION NOT SET, INVALID LEDGE TYPE {} <<", ledgeType);
            return false;
    }

    const auto scale = RuntimeVariables::PlayerScale;
    zAdjust = -z * scale;
    backwardAdjustment = scale * GetActorDirFlat(actor) * backOffset;

    out = RE::NiPoint3{RuntimeVariables::ledgePoint.x - backwardAdjustment.x, RuntimeVariables::ledgePoint.y - backwardAdjustment.y,
                       ledgeType == ParkourType::Failed ? actor->GetPositionZ() : RuntimeVariables::ledgePoint.z + zAdjust};

    return true;
}

void Parkouring::InvalidateVars()
{
    RuntimeVariables::selectedLedgeType = ParkourType::NoLedge;
    using sppf = Scaleform::SkyParkourMenu;
    auto menu = sppf::GetSingleton();
    if (menu && menu->IsOpen()) menu->SetActiveIndicatorType(sppf::IndicatorType::kInvisible);
}

void Parkouring::UpdateIndicatorMenu()
{
    if (!ModSettings::Use_Indicators) return;

    using sppf = Scaleform::SkyParkourMenu;
    const auto menu = sppf::GetSingleton();
    if (!menu || !menu->IsOpen()) return;

    sppf::IndicatorType indic;
    const auto ledge = RuntimeVariables::selectedLedgeType;

    switch (ledge)
    {
        case ParkourType::Failed:
            indic = sppf::IndicatorType::kOutOfStamina;
            break;

        case ParkourType::Vault:
            indic = sppf::IndicatorType::kVault;
            break;

        case ParkourType::StepHigh:
        case ParkourType::StepLow:
            indic = sppf::IndicatorType::kStep;
            break;

        case ParkourType::Grab:
        case ParkourType::High:
        case ParkourType::Highest:
        case ParkourType::Medium:
        case ParkourType::Low:
            indic = sppf::IndicatorType::kClimb;
            break;

        case ParkourType::NoLedge:
        default:
            indic = sppf::IndicatorType::kInvisible;
    }
    menu->SetActiveIndicatorType(indic);
}

void Parkouring::UpdateParkourPoint()
{
    RayCastResult ledgeRay;

    [[unlikely]] if (ModSettings::_Debug_Enabled)
    {
        RuntimeVariables::selectedLedgeType = GetLedgePoint(ledgeRay);
        const auto pl = GET_PLAYER;
        const auto cl = pl->GetCharController();
        const auto sf = cl->surfaceInfo;
        const auto as = pl->AsActorState();
        DEBUG_PRINT(
            "MS: {:.2f} / PCT: {:.2f} / Vel: {:.2f}\n"
            "Sprint: {} / Walk: {} / Run: {} / Sneak:{}\n"
            "Ledge-> Layer: {} / Form: {} / Dist: {:.2f} / zNorm: {:.2f}\n"
            "Surface-> Gnd Sup: {} / zNorm: {:.2f}",
            as->DoGetMovementSpeed(), pl->GetCharController()->speedPct, ParkourUtility::GetCharForwardVelocity(pl), (as->IsSprinting()),
            as->IsWalking(), as->IsRunning(), as->IsSneaking(), ledgeRay.layer, ledgeRay.GetHitObjectFormType_Safe(), ledgeRay.distance,
            ledgeRay.normalOut.quad.m128_f32[2], sf.supportedState.get(), sf.surfaceNormal.quad.m128_f32[2]);
    }

    if (RuntimeVariables::ParkourInProgress)
    {
        InvalidateVars();
        return;
    }

    if (!ModSettings::_Debug_Enabled)
    {
        RuntimeVariables::selectedLedgeType = GetLedgePoint(ledgeRay);
    }

    const auto player = GET_PLAYER;
    RuntimeVariables::IsParkourActive = IsParkourActiveFor(player);
    // RuntimeVariables::PlayerScale = ScaleUtility::GetScale(player);

    [&] -> void {
        using opt = SkyParkour::AutoParkourOptions;
        namespace pu = ParkourUtility;
        if (ModSettings::Auto_Parkour == opt::kNonCombatOnly && !pu::IsActorWeaponOut(GET_PLAYER) ||
            ModSettings::Auto_Parkour == opt::kAlways)
        {
            switch (RuntimeVariables::selectedLedgeType)
            {
                case ParkourType::StepHigh:
                case ParkourType::StepLow:
                case ParkourType::Vault:
                    if (RE::PlayerControls::GetSingleton()->data.autoMove)
                    {
                        Parkouring::TryActivateParkour();
                    }
                    break;
                case ParkourType::Grab:
                    if (player->GetCharController()->fallTime > 0.5f) Parkouring::TryActivateParkour();
            }
        }
    }();

    UpdateIndicatorMenu();
}

bool Parkouring::TryActivateParkour()
{
    std::unique_lock<std::mutex> lock(g_ParkourActivateLock, std::try_to_lock);
    if (!lock.owns_lock()) return false;

    const auto player = GET_PLAYER;
    const auto LedgeTypeToProcess = RuntimeVariables::selectedLedgeType;

    if (LedgeTypeToProcess == ParkourType::NoLedge) return false;

    bool Ongoing;
    if (player->GetGraphVariableBool(SPPF_ONGOING, Ongoing) && Ongoing) return false;

    if (!RuntimeVariables::IsParkourActive || RuntimeVariables::IsMenuOpen) return false;

    const bool isSwimming = PlayerIsSwimming();
    const auto fallTime = player->GetCharController()->fallTime;
    const bool considerGrounded = fallTime < 0.17f;  // Delay grabbing immediately after jumping
    //INFO(">> Fall time: {}", fallTime);

    if (LedgeTypeToProcess == ParkourType::Grab)
    {
        if (considerGrounded && !isSwimming) return false;  // Grab animation is also used for replacing steps when swimming
    }

    if (!HavokUtil::ValidateBehaviorPatch(player)) return false;

    RuntimeVariables::ParkourInProgress = true;

    ParkourReadyRun(LedgeTypeToProcess);

    return true;
}
void Parkouring::ParkourReadyRun(ParkourType ledgeType)
{
    const auto player = GET_PLAYER;
    // const auto &ctrl = player->GetCharController();

    player->SetGraphVariableInt(SPPF_Ledge, std::to_underlying(ledgeType));

    RE::NiPoint3 startPos;
    if (Parkouring::CalculateStartingPosition(player, ledgeType, startPos))
    {
        RuntimeVariables::startPos = startPos;
    }
    else
    {
        RuntimeVariables::startPos = player->GetPosition();
    }

    if (ledgeType == ParkourType::StepHigh || ledgeType == ParkourType::StepLow)
        player->SetGraphVariableBool(SPPF_Lower_Body_Only, IsActorWeaponOut(player));

    // RE::hkVector4 hkPos = nipoint_to_hkvector(startPos);
    // ctrl->SetPositionImpl(hkPos, true, false);

    player->NotifyAnimationGraph(SPPF_NOTIFY);
}
void Parkouring::PostParkourStaminaDamage(RE::Actor *actor, bool isLowEffort, bool isSwimming)
{
    if (ModSettings::Enable_Stamina_Consumption)
    {
        float cost = CalculateStaminaReqFromEquipLoad(actor);
        if (cost < 0) return;

        /* If swimming, fail animation won't play. So no need to flash the bar. Just consume half the stamina cost like low effort. */
        if (isLowEffort || isSwimming)
        {
            // INFO("cost{}", cost / 2);
            DamageActorStamina(actor, cost / 2);
        }
        else if (ActorHasEnoughStamina(actor))
        {
            // INFO("cost{}", cost);
            DamageActorStamina(actor, cost);
        }
        else
        {
            RE::HUDMenu::FlashMeter(RE::ActorValue::kStamina);
        }
        actor->UpdateRegenDelay(RE::ActorValue::kStamina, 2.0f);
    }
}

void Parkouring::SetParkourOnOff(bool turnOn)
{
    if (turnOn)
    {
        if (!Buttons::ParkourListener::GetSingleton()->SinkRegistered)
        {
            Buttons::ParkourListener::Register();
            INFO("Parkour: < ON >");
        }
    }
    else
    {
        if (Buttons::ParkourListener::GetSingleton()->SinkRegistered)
        {
            Buttons::ParkourListener::Unregister();
            INFO("Parkour: < Off >");
        }

        RuntimeMethods::ResetParkour();
    }
}