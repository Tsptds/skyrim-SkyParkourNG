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

#include "HUD/Scaleform/SkyParkourMenu.hpp"

static std::mutex g_ParkourActivateLock;

using namespace ParkourUtility;

ParkourType Parkouring::GetLedgePoint(RayCastResult &out_LedgeRay) {
    using namespace ModSettings;

    const auto player = GET_PLAYER;
    const auto actorDirFlat = GetActorDirFlat(player);

    // Perform ledge or vault checks
    ParkourType selectedLedgeType = ParkourType::NoLedge;

    RE::NiPoint3 ledgePoint;
    constexpr int vaultLength = 100;
    constexpr int maxElevationIncrease = 70;
    bool out_isForwardBlocked{false};

    selectedLedgeType = VaultCheck(ledgePoint, actorDirFlat, vaultLength, maxElevationIncrease * RuntimeVariables::PlayerScale,
                                   HardCodedVariables::vaultMinHeight * RuntimeVariables::PlayerScale,
                                   HardCodedVariables::vaultMaxHeight * RuntimeVariables::PlayerScale, out_LedgeRay, out_isForwardBlocked);

    if (selectedLedgeType == ParkourType::NoLedge) {
        selectedLedgeType = ClimbCheck(ledgePoint, actorDirFlat, HardCodedVariables::climbMinHeight * RuntimeVariables::PlayerScale,
                                       HardCodedVariables::climbMaxHeight * RuntimeVariables::PlayerScale, out_LedgeRay);
    }

    if (selectedLedgeType == ParkourType::NoLedge) return ParkourType::NoLedge;
    if (out_isForwardBlocked) {
        if (selectedLedgeType == ParkourType::StepHigh || selectedLedgeType == ParkourType::StepLow) return ParkourType::NoLedge;
    }

    // Don't ever parkour into water, last check before saying this ledge is valid
    float waterLevel{-200000.0f};
    auto parentCell = player->GetParentCell();
    if (parentCell) {
        parentCell->GetWaterHeight(player->GetPosition(), waterLevel);
    }

    constexpr int validWaterDepth = 10;
    if (ledgePoint.z < waterLevel - validWaterDepth) return ParkourType::NoLedge;

    RuntimeVariables::ledgePoint = ledgePoint;

    return selectedLedgeType;
}

ParkourType Parkouring::ClimbCheck(RE::NiPoint3 &ledgePoint, RE::NiPoint3 actorDirFlat, float minLedgeHeight, float maxLedgeHeight,
                                   RayCastResult &out_LedgeRay) {
    const auto player = GET_PLAYER;
    const auto playerPos = player->GetPosition();

    constexpr RE::NiPoint3 upDir(0, 0, 1);
    constexpr RE::NiPoint3 downDir(0, 0, -1);

    // Constants adjusted for player scale
    const float playerHeight = 120 * RuntimeVariables::PlayerScale;
    const float fwdCheckStep = 5;                                               // 8
    const float dynamicIter = player->GetCharController()->speedPct * 15 + 15;  // Dynamically scale check dist to speed
    const int fwdCheckIterations = static_cast<int>(dynamicIter);               // 15
    const float minLedgeFlatness = 0.5f;                                        // 0.5

    // Raycast above player, is there enough room
    RE::NiPoint3 climbHeadRoomRayStart = playerPos + RE::NiPoint3(0, 0, playerHeight);
    RayCastResult upRay = HavokUtil::RayCast(climbHeadRoomRayStart, upDir, playerHeight, COL_LAYER_EXTEND::kClimbObstruction);

    if (upRay.distance < playerHeight) {
        /* DEBUG LINES */
        if (ModSettings::_Debug_Enabled) {
            const auto TH = API_Handles::TrueHUD::Get();
            if (TH) {
                TH->DrawArrow(climbHeadRoomRayStart, climbHeadRoomRayStart + upDir * upRay.distance, 10.f, 0.f, COLOR_HEX_R, 1.f);
            }
        }
        /***********************************/
        return ParkourType::NoLedge;
    }

    // Forward raycast initialization
    RE::NiPoint3 fwdRayStart = playerPos;
    fwdRayStart.z += maxLedgeHeight;

    RayCastResult ledgeRay{};
    bool foundLedge = false;
    float normalZ = 0;
    float lastZ = minLedgeHeight; /* v3.5.0 Prevent grabbing a ledge through an obstruction */

    // Incremental forward raycast to find a ledge
    for (int i = 0; i < fwdCheckIterations; i++) {
        const auto curFwdProbeLen = fwdCheckStep * i;

        RayCastResult fwdRay = HavokUtil::RayCast(fwdRayStart, actorDirFlat, curFwdProbeLen, COL_LAYER_EXTEND::kClimbObstruction);

        if (ModSettings::_Debug_Enabled) {
            const auto TH = API_Handles::TrueHUD::Get();
            if (TH) TH->DrawArrow(fwdRayStart, fwdRayStart + actorDirFlat * fwdRay.distance, 10.f, 0.f, COLOR_HEX_B, 1.f);
        }

        /* If probe hits smth, skip  */
        if (fwdRay.distance < curFwdProbeLen) continue;

        // Downward raycast to detect ledge point
        RE::NiPoint3 ledgeRayStart = fwdRayStart + actorDirFlat * fwdRay.distance;
        const float ledgeRayMaxCheck = maxLedgeHeight - minLedgeHeight;
        ledgeRay = HavokUtil::RayCast(ledgeRayStart, downDir, ledgeRayMaxCheck, COL_LAYER_EXTEND::kClimbLedge);

        /* There is no one solution fits all collision layer mask for raycast, so v3.5.0 iterates over all hits for climb and filters their layers*/
        for (auto &&hit: ledgeRay.hits) {
            if (!LAYERS_CLIMB_EXCLUDE.contains(hit.rootCollidable->GetCollisionLayer())) {
                const auto hitRef = RE::TESHavokUtilities::FindCollidableRef(*hit.rootCollidable);
                if (hitRef) { /* can be null */
                    ledgeRay.distance = hit.hitFraction * ledgeRayMaxCheck;
                    ledgeRay.normalOut = hit.normal;
                    ledgeRay.layer = hit.rootCollidable->GetCollisionLayer();
                    ledgeRay.hitObjectRef = hitRef;
                    break;
                }
            }
        }

        /* DEBUG LINES */
        if (ModSettings::_Debug_Enabled) {
            const auto TH = API_Handles::TrueHUD::Get();
            if (TH) {
                /* Draw min/max heights set in config */
                const auto limitDrawStartMin = playerPos + RE::NiPoint3(0, 0, minLedgeHeight);
                const auto limitDrawStartMax = playerPos + RE::NiPoint3(0, 0, maxLedgeHeight);
                const auto offset = actorDirFlat * fwdCheckStep * static_cast<float>(fwdCheckIterations);

                TH->DrawLine(limitDrawStartMin, limitDrawStartMin + offset, 0.f, COLOR_HEX_Y, 2.f);
                TH->DrawLine(limitDrawStartMax, limitDrawStartMax + offset, 0.f, COLOR_HEX_Y, 2.f);

                /* Show all downward hits */
                float prevhitDist = 0;
                for (auto &&hit: ledgeRay.hits) {
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
        if (ledgePoint.z - playerPos.z < lastZ) continue;
        if (ledgeRay.distance < 10) continue;
        if (normalZ < minLedgeFlatness) continue;
        if (ledgePoint.z < playerPos.z + minLedgeHeight) continue;
        if (ledgePoint.z > playerPos.z + maxLedgeHeight) continue;

        lastZ = ledgePoint.z - playerPos.z;

        // if (ModSettings::_Debug_Enabled) DEBUG_PRINT("Layer {} / Form {}", ledgeRay.layer, ledgeRay.GetHitObjectFormType_Safe());

        // Check for obstructions behind the Ledge point
        const bool obsFound = [ledgePoint, actorDirFlat] {
            constexpr float obsBackOffset = 15.f;
            const float ray1MaxDist = obsBackOffset + 15.f;
            const float minSpaceRequired = obsBackOffset + 3.f;

            /* 2 rays. if 1 distance is too low, not valid. if 1 passes, cast another ray with the ledge normal obtained from 1.
            this prevents extending the distance by looking slightly sideways */

            const RE::NiPoint3 obsCheckStart = RE::NiPoint3(ledgePoint.x, ledgePoint.y, ledgePoint.z + 5.f) - actorDirFlat * obsBackOffset;
            RayCastResult obsRay1 = HavokUtil::RayCast(obsCheckStart, actorDirFlat, ray1MaxDist, COL_LAYER_EXTEND::kClimbObstruction);
            auto normalizedDir = -VEC4_TO_VEC3(obsRay1.normalOut);
            normalizedDir.z = 0;
            RayCastResult obsRay2 = HavokUtil::RayCast(obsCheckStart, normalizedDir, minSpaceRequired, COL_LAYER_EXTEND::kClimbObstruction);

            /* DEBUG LINES */
            if (ModSettings::_Debug_Enabled) {
                const auto TH = API_Handles::TrueHUD::Get();
                if (TH) {
                    TH->DrawArrow(obsCheckStart, obsCheckStart + actorDirFlat * obsRay1.distance, 10.f, 0.f,
                                  obsRay1.distance < minSpaceRequired ? COLOR_HEX_R : COLOR_HEX_G, 1.f);
                    TH->DrawArrow(obsCheckStart, obsCheckStart + normalizedDir * obsRay2.distance, 10.f, 0.f,
                                  obsRay2.didHit ? COLOR_HEX_R : COLOR_HEX_G, 1.f);
                }
            }
            /*********************************/

            if (obsRay2.didHit || obsRay1.distance < minSpaceRequired) {
                return true;  // Obstruction behind the ledge point
            }

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

    const RE::NiPoint3 headRoomRayStart = RE::NiPoint3(playerPos.x, playerPos.y, ledgePoint.z - 5);  // On player at ledge height
    const RE::NiPoint3 sideDir = actorDirFlat.Cross(upDir);

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
    RayCastResult headRoomRay_L = HavokUtil::RayCast(leftStart, dir_L, playerHeight, COL_LAYER_EXTEND::kClimbObstruction);
    RayCastResult headRoomRay_R = HavokUtil::RayCast(rightStart, dir_R, playerHeight, COL_LAYER_EXTEND::kClimbObstruction);

    /* DEBUG LINES */
    if (ModSettings::_Debug_Enabled) {
        const auto TH = API_Handles::TrueHUD::Get();
        if (TH) {
            TH->DrawArrow(leftStart, leftStart + dir_L * headRoomRay_L.distance, 10.f, 0.f,
                          headRoomRay_L.didHit ? COLOR_HEX_R : COLOR_HEX_G, 1.f);
            // TH->DrawArrow(midStart, midStart + dir_M * headRoomRay_M.distance, 10.f, 0.f, headRoomRay_M.didHit ? COLOR_HEX_R : COLOR_HEX_G,
            //               1.f);
            TH->DrawArrow(rightStart, rightStart + dir_R * headRoomRay_R.distance, 10.f, 0.f,
                          headRoomRay_R.didHit ? COLOR_HEX_R : COLOR_HEX_G, 1.f);
        }
    }
    /*********************************/

    if (headRoomRay_L.didHit || /*headRoomRay_M.didHit ||*/ headRoomRay_R.didHit) {
        return ParkourType::NoLedge;
    }

    return Parkouring::ChooseClimbHeight(player, playerHeight, ledgePoint, playerPos, actorDirFlat);
}
ParkourType Parkouring::ChooseClimbHeight(RE::Actor *player, const float playerHeight, RE::NiPoint3 &ledgePoint,
                                          const RE::NiPoint3 &playerPos, RE::NiPoint3 actorDirFlat) {
    const float ledgePlayerDiff = ledgePoint.z - playerPos.z;
    // const bool isMidair = player->IsInMidair();
    using cf = RE::CHARACTER_FLAGS;
    const bool isGrounded = player->GetCharController()->flags.any(cf::kSupport, cf::kHasPotentialSupportManifold);

    if (isGrounded || PlayerIsSwimming()) {
        if (ledgePlayerDiff >= HardCodedVariables::highestLedgeLimit * RuntimeVariables::PlayerScale) {
            // Highest ledge
            const RE::NiPoint3 headRoomRayStart{playerPos.x, playerPos.y, ledgePoint.z};

            if (!ClimbExtraChecks(headRoomRayStart, playerHeight, actorDirFlat)) return ParkourType::NoLedge;
            if (!SmartClimbCheck(player)) return ParkourType::NoLedge;
            if (ShouldClimbActionFail(player)) return ParkourType::Failed;

            return ParkourType::Highest;
        }
        else if (ledgePlayerDiff >= HardCodedVariables::highLedgeLimit * RuntimeVariables::PlayerScale) {
            // High ledge
            const RE::NiPoint3 headRoomRayStart{playerPos.x, playerPos.y, ledgePoint.z};

            if (!ClimbExtraChecks(headRoomRayStart, playerHeight, actorDirFlat)) return ParkourType::NoLedge;
            if (!SmartClimbCheck(player)) return ParkourType::NoLedge;
            if (ShouldClimbActionFail(player)) return ParkourType::Failed;

            return ParkourType::High;
        }
        else if (ledgePlayerDiff >= HardCodedVariables::medLedgeLimit * RuntimeVariables::PlayerScale) {
            // Medium ledge
            const RE::NiPoint3 headRoomRayStart{playerPos.x, playerPos.y, ledgePoint.z};

            if (!ClimbExtraChecks(headRoomRayStart, playerHeight, actorDirFlat)) return ParkourType::NoLedge;

            return ParkourType::Medium;
        }
        else if (ledgePlayerDiff >= HardCodedVariables::lowLedgeLimit * RuntimeVariables::PlayerScale) {
            // Low ledge
            if (PlayerIsSwimming()) return ParkourType::Grab;  // Grab ledge out of water

            return ParkourType::Low;
        }
        else if (ledgePlayerDiff >= HardCodedVariables::highStepLimit * RuntimeVariables::PlayerScale) {
            // High Step
            if (PlayerIsSwimming()) return ParkourType::Grab;  // Grab ledge out of water

            if (StepsExtraChecks(player, ledgePlayerDiff, ledgePoint)) return ParkourType::StepHigh;
        }
        else {
            // Low Step
            if (PlayerIsSwimming()) return ParkourType::Grab;  // Grab ledge out of water

            if (StepsExtraChecks(player, ledgePlayerDiff, ledgePoint)) return ParkourType::StepLow;
        }
    }
    else if (!isGrounded) {
        // We are midair, check for grab
        bool out_grabHighVariant{false};
        if (!GrabExtraChecks(player, ledgePlayerDiff, out_grabHighVariant, ledgePoint)) return ParkourType::NoLedge;

        player->SetGraphVariableBool(SPPF_Grab_Variant, out_grabHighVariant);
        return ParkourType::Grab;
    }
    return ParkourType::NoLedge;
}

ParkourType Parkouring::VaultCheck(RE::NiPoint3 &ledgePoint, RE::NiPoint3 actorDirFlat, float vaultLength, float maxElevationIncrease,
                                   float minVaultHeight, float maxVaultHeight, RayCastResult &out_LedgeRay, bool &out_isForwardBlocked) {
    const auto player = GET_PLAYER;

    if (!VaultExtraChecks(player)) return ParkourType::NoLedge;

    const auto playerPos = player->GetPosition();
    const float playerHeight = 120 * RuntimeVariables::PlayerScale;
    const RE::NiPoint3 &playerDirFlat = GetActorDirFlat(player);
    const RE::NiPoint3 upRayDir{0, 0, 1};
    const RE::NiPoint3 sideRayDirR = playerDirFlat.Cross(upRayDir);
    const RE::NiPoint3 sideRayDirL = -playerDirFlat.Cross(upRayDir);
    const RE::NiPoint3 plHeadPos = playerPos + RE::NiPoint3(0, 0, playerHeight);

    out_isForwardBlocked =
        [&] { /* Forward raycast to check if there is an obstruction at head level in vaultLength */
              const RE::NiPoint3 fwdRayStart = plHeadPos;
              const float &minSpaceRequired = vaultLength;

              // Side directions

              // Offset start positions for L R rays
              constexpr float fwdOffsetRays = 15.f;
              const RE::NiPoint3 leftRayStart = fwdRayStart + (sideRayDirL * fwdOffsetRays);
              const RE::NiPoint3 rightRayStart = fwdRayStart + (sideRayDirR * fwdOffsetRays);

              RayCastResult fwdRay = HavokUtil::RayCast(fwdRayStart, actorDirFlat, minSpaceRequired, COL_LAYER_EXTEND::kVaultForward);
              RayCastResult leftRay = HavokUtil::RayCast(leftRayStart, actorDirFlat, minSpaceRequired, COL_LAYER_EXTEND::kVaultForward);
              RayCastResult rightRay = HavokUtil::RayCast(rightRayStart, actorDirFlat, minSpaceRequired, COL_LAYER_EXTEND::kVaultForward);

              /* DEBUG LINES */
              if (ModSettings::_Debug_Enabled) {
                  const auto TH = API_Handles::TrueHUD::Get();
                  if (TH) {
                      auto draw = [&](const RE::NiPoint3 &start, const RayCastResult &ray) {
                          TH->DrawArrow(start, start + actorDirFlat * ray.distance, 10.f, 0.f, ray.didHit ? COLOR_HEX_R : COLOR_HEX_G, 1.f);
                      };
                      draw(fwdRayStart, fwdRay);
                      draw(leftRayStart, leftRay);
                      draw(rightRayStart, rightRay);
                  }
              }
              /*********************************************/

              if (fwdRay.didHit) return true;
              if (leftRay.didHit) return true;
              if (rightRay.didHit) return true;
              return false;
        }();
    if (out_isForwardBlocked) return ParkourType::NoLedge;

    /* Move forward by this steps, and RayCast downwards. If a valid layer is found, mark it. */
    constexpr int downIterations = 20;
    RE::NiPoint3 downRayDir(0, 0, -1);
    RayCastResult downRay;

    bool foundVaulter = false;
    float foundVaultHeight = -10000.0f;
    bool foundLanding = false;
    float foundLandingHeight = 10000.0f;
    float vaultableGap = playerHeight + 100.0f * RuntimeVariables::PlayerScale;

    RE::NiPoint3 downRayStart;
    // Incremental downward raycasts
    for (int i = 0; i < downIterations; i++) {
        float iDist = static_cast<float>(i) * 5.0f;
        downRayStart = playerPos + actorDirFlat * iDist;
        downRayStart.z = plHeadPos.z;

        downRay = HavokUtil::RayCast(downRayStart, downRayDir, vaultableGap, COL_LAYER_EXTEND::kVaultDown);

        // If vault point is invalid layer or form type, ignore
        if (LAYERS_VAULT_DOWN_RAY.contains(downRay.layer)) continue;
        if (FORMS_VAULT_EXCLUDE.contains(downRay.GetHitObjectFormType_Safe())) continue;

        const float hitHeight = (plHeadPos.z - downRay.distance) - playerPos.z;

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

        const float landingDistance = plHeadPos.z - (playerPos.z + foundLandingHeight);
        const RE::NiPoint3 landingPoint = downRayStart + downRayDir * landingDistance;
        /* DEBUG LINES */
        if (ModSettings::_Debug_Enabled) {
            const auto TH = API_Handles::TrueHUD::Get();
            if (TH) {
                TH->DrawArrow(downRayStart, landingPoint, 10.f, 0.f, COLOR_HEX_B, 1.f);
                // TH->DrawSphere(landingPoint, 2.f, 2, 1, COLOR_HEX_R);
            }
        }
        /*********************************************/
        out_LedgeRay = downRay;
        /* Check if ledge has obstruction above */
        const float halfPlayerHeight = playerHeight * 0.5f;
        const auto upRayStart = ledgePoint + RE::NiPoint3(0, 0, 5);
        const RayCastResult upRay =
            HavokUtil::RayCast(upRayStart, upRayDir, halfPlayerHeight, COL_LAYER_EXTEND::kVaultPostLedgeObstruction);
        /* DEBUG LINES */
        if (ModSettings::_Debug_Enabled) {
            const auto TH = API_Handles::TrueHUD::Get();
            if (TH) {
                TH->DrawArrow(upRayStart, upRayStart + upRayDir * upRay.distance, 10.f, 0.f, upRay.didHit ? COLOR_HEX_R : COLOR_HEX_G, 1.f);
            }
        }
        /*********************************************/
        if (upRay.didHit) return ParkourType::NoLedge;

        /* Check if landing spot has obstruction forward */
        const RE::NiPoint3 landObsCheckStart = landingPoint + RE::NiPoint3(0, 0, HardCodedVariables::climbMinHeight);
        const RayCastResult landingObsRay =
            HavokUtil::RayCast(landObsCheckStart, actorDirFlat, vaultLength, COL_LAYER_EXTEND::kVaultForward);
        /* DEBUG LINES */
        if (ModSettings::_Debug_Enabled) {
            const auto TH = API_Handles::TrueHUD::Get();
            if (TH) {
                RE::NiPoint3 landObsCheckEnd = landObsCheckStart + landingObsRay.distance * actorDirFlat;
                TH->DrawArrow(landObsCheckStart, landObsCheckEnd, 10.f, 0.f, landingObsRay.didHit ? COLOR_HEX_R : COLOR_HEX_G, 1.f);
            }
        }
        /*********************************************/
        if (landingObsRay.didHit) return ParkourType::NoLedge;

        // Check if the structure is horizontally tiny by casting sideways rays
        const float sideMaxCheckOffset = 15.f;

        const auto sideRayStart = ledgePoint + RE::NiPoint3(0, 0, 10);
        const float sideMaxCheck = sideMaxCheckOffset * RuntimeVariables::PlayerScale;
        const RayCastResult sideRayR =
            HavokUtil::RayCast(sideRayStart, sideRayDirR, sideMaxCheck, COL_LAYER_EXTEND::kVaultPostLedgeObstruction);
        const RayCastResult sideRayL =
            HavokUtil::RayCast(sideRayStart, sideRayDirL, sideMaxCheck, COL_LAYER_EXTEND::kVaultPostLedgeObstruction);

        /* DEBUG LINES */
        if (ModSettings::_Debug_Enabled) {
            const auto TH = API_Handles::TrueHUD::Get();
            if (TH) {
                TH->DrawArrow(sideRayStart, sideRayStart + sideRayDirL * sideRayL.distance, 10.f, 0.f,
                              sideRayL.didHit ? COLOR_HEX_R : COLOR_HEX_G, 1.f);
            }
            if (TH) {
                TH->DrawArrow(sideRayStart, sideRayStart + sideRayDirR * sideRayR.distance, 10.f, 0.f,
                              sideRayR.didHit ? COLOR_HEX_R : COLOR_HEX_G, 1.f);
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
    constexpr auto charFlag = RE::CHARACTER_FLAGS::kNoSim;
    const auto ctrl = actor->GetCharController();

    if (isStop) {
        ctrl->flags.reset(charFlag);
        actor->SetGraphVariableInt(SPPF_Ledge, std::to_underlying(ParkourType::NoLedge));

        // The other graph doesn't see the current graph, interrupt on stop to notify all
        // DO NOT SEND SPPF_STOP OR IT WILL RECURSE INFINITELY, STACK OVERFLOW AND CRASH
        actor->NotifyAnimationGraph(SPPF_INTERRUPT);

        using JA = Compatibility::JumpingAttack;
        if (JA::found) {
            if (ParkourUtility::IsActorWeaponOut(actor)) {
                actor->NotifyAnimationGraph(JA::event);
            }
        }

        if (Compatibility::TrueDirectionalMovement::found) {
            API_Handles::TDM::ObtainYawControl(false);
        }

        /* Prevent actor flinging away if char ctrl state is kInAir by clamping velocity */
        if (ctrl->context.currentState != RE::hkpCharacterStateType::kOnGround) {
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

        if (actor->IsPlayerRef()) {
            RuntimeVariables::RecoveryFramesActive = false;
            RuntimeVariables::ParkourInProgress = false;
        }
    }
    else /* if isStart */ {
        ParkourUtility::StopInteractions(*actor);

        // Disable simulation, fixes char controller taking over on hit & enables vertical root motion
        ctrl->flags.set(charFlag);

        if (Compatibility::TrueDirectionalMovement::found) {
            API_Handles::TDM::ObtainYawControl(true);
        }
    }

    if (actor->IsPlayerRef()) {
        const auto ctrlMap = RE::ControlMap::GetSingleton();
        if (ctrlMap) {
            ctrlMap->ToggleControls(RE::ControlMap::UEFlag::kJumping, isStop, true);
            ctrlMap->ToggleControls(RE::ControlMap::UEFlag::kMainFour, isStop,
                                    true);  // Player tab menu & equip. Gets stuck if player uses TFC.
        }
    }
}

bool Parkouring::CalculateStartingPosition(RE::Actor *actor, ParkourType ledgeType, RE::NiPoint3 &out) {
    float zAdjust{0};
    float z{0};
    float backOffset{55.0f};
    RE::NiPoint3 backwardAdjustment;

    switch (ledgeType) {
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

    zAdjust = -z * RuntimeVariables::PlayerScale;
    backwardAdjustment = RuntimeVariables::PlayerScale * GetActorDirFlat(actor) * backOffset;

    out = RE::NiPoint3{RuntimeVariables::ledgePoint.x - backwardAdjustment.x, RuntimeVariables::ledgePoint.y - backwardAdjustment.y,
                       ledgeType == ParkourType::Failed ? actor->GetPositionZ() : RuntimeVariables::ledgePoint.z + zAdjust};

    return true;
}

void Parkouring::InvalidateVars() {
    RuntimeVariables::selectedLedgeType = ParkourType::NoLedge;
    using sppf = Scaleform::SkyParkourMenu;
    auto menu = sppf::GetSingleton();
    if (menu && menu->IsOpen()) menu->SetActiveIndicatorType(sppf::IndicatorType::kInvisible);
}

void Parkouring::UpdateIndicatorMenu() {
    if (!ModSettings::Use_Indicators) return;

    using sppf = Scaleform::SkyParkourMenu;
    const auto menu = sppf::GetSingleton();
    if (!menu || !menu->IsOpen()) return;

    sppf::IndicatorType indic;
    const auto ledge = RuntimeVariables::selectedLedgeType;

    switch (ledge) {
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

void Parkouring::UpdateParkourPoint() {
    RayCastResult ledgeRay;
    if (ModSettings::_Debug_Enabled) {
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

    if (RuntimeVariables::ParkourInProgress) {
        InvalidateVars();
        return;
    }

    if (!ModSettings::_Debug_Enabled) {
        RuntimeVariables::selectedLedgeType = GetLedgePoint(ledgeRay);
    }

    const auto player = GET_PLAYER;
    RuntimeVariables::IsParkourActive = IsParkourActiveFor(player);
    // RuntimeVariables::PlayerScale = ScaleUtility::GetScale();
    RuntimeVariables::PlayerScale = ScaleUtility::GetScale(player);
    // RuntimeVariables::playerDirFlat = GetActorDirFlat(player);

    [&] -> void {
        using opt = AUTO_PARKOUR_OPTIONS;
        namespace pu = ParkourUtility;
        if (ModSettings::Auto_Parkour == opt::kNonCombatOnly && !pu::IsActorWeaponOut(GET_PLAYER) ||
            ModSettings::Auto_Parkour == opt::kAlways) {
            switch (RuntimeVariables::selectedLedgeType) {
                case ParkourType::StepHigh:
                case ParkourType::StepLow:
                case ParkourType::Vault:
                    if (RE::PlayerControls::GetSingleton()->data.autoMove) {
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

bool Parkouring::TryActivateParkour() {
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
    //LOG(">> Fall time: {}", fallTime);

    if (LedgeTypeToProcess == ParkourType::Grab) {
        if (considerGrounded && !isSwimming) return false;  // Grab animation is also used for replacing steps when swimming
    }

    if (!HavokUtil::ValidateBehaviorPatch(player)) return false;

    RuntimeVariables::ParkourInProgress = true;

    ParkourReadyRun(LedgeTypeToProcess);

    return true;
}
void Parkouring::ParkourReadyRun(ParkourType ledgeType) {
    const auto player = GET_PLAYER;
    // const auto &ctrl = player->GetCharController();

    player->SetGraphVariableInt(SPPF_Ledge, std::to_underlying(ledgeType));

    RE::NiPoint3 startPos;
    if (Parkouring::CalculateStartingPosition(player, ledgeType, startPos)) {
        RuntimeVariables::startPos = startPos;
    }
    else {
        RuntimeVariables::startPos = player->GetPosition();
    }

    if (ledgeType == ParkourType::StepHigh || ledgeType == ParkourType::StepLow)
        player->SetGraphVariableBool(SPPF_Lower_Body_Only, IsActorWeaponOut(player));

    // RE::hkVector4 hkPos = nipoint_to_hkvector(startPos);
    // ctrl->SetPositionImpl(hkPos, true, false);

    player->NotifyAnimationGraph(SPPF_NOTIFY);
}
void Parkouring::PostParkourStaminaDamage(RE::Actor *actor, bool isLowEffort, bool isSwimming) {
    if (ModSettings::Enable_Stamina_Consumption) {
        float cost = CalculateStaminaReqFromEquipLoad(actor);
        if (cost < 0) return;

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
        if (!Buttons::ParkourListener::GetSingleton()->SinkRegistered) {
            Buttons::ParkourListener::Register();
            LOG("Parkour: < ON >");
        }
    }
    else {
        if (Buttons::ParkourListener::GetSingleton()->SinkRegistered) {
            Buttons::ParkourListener::Unregister();
            LOG("Parkour: < Off >");
        }

        RuntimeMethods::ResetParkour();
    }
}