#include "Parkouring.h"

using namespace ParkourUtility;

int Parkouring::LedgeCheck(RE::NiPoint3 &ledgePoint, RE::NiPoint3 checkDir, float minLedgeHeight, float maxLedgeHeight) {
    const auto player = RE::PlayerCharacter::GetSingleton();
    const auto playerPos = player->GetPosition();

    // Constants adjusted for player scale
    const float startZOffset = 100 * RuntimeVariables::PlayerScale;
    const float playerHeight = 120 * RuntimeVariables::PlayerScale;
    const float minUpCheck = 100 * RuntimeVariables::PlayerScale;
    const float maxUpCheck = (maxLedgeHeight - startZOffset) + 20 * RuntimeVariables::PlayerScale;
    const float fwdCheckStep = 8 * RuntimeVariables::PlayerScale;
    const int fwdCheckIterations = 10;   // 15
    const float minLedgeFlatness = 0.5;  //0.5
    const float ledgeHypotenuse = 1.0;   // 0.75 - larger is more relaxed, lesser is more strict. Don't set 0

    RE::hkVector4 normalOut(0, 0, 0, 0);

    // Upward raycast to check for headroom
    RE::NiPoint3 upRayStart = playerPos + RE::NiPoint3(0, 0, startZOffset);
    RE::NiPoint3 upRayDir(0, 0, 1);

    float upRayDist = RayCast(upRayStart, upRayDir, maxUpCheck, normalOut, RE::COL_LAYER::kLOS);
    if (upRayDist < minUpCheck) {
        return ParkourType::NoLedge;
    }

    // Forward raycast initialization
    RE::NiPoint3 fwdRayStart = upRayStart + upRayDir * (upRayDist - 10);
    RE::NiPoint3 downRayDir(0, 0, -1);

    bool foundLedge = false;
    float normalZ = 0;

    // Incremental forward raycast to find a ledge
    for (int i = 0; i < fwdCheckIterations; i++) {
        float fwdRayDist = RayCast(fwdRayStart, checkDir, fwdCheckStep * i, normalOut, RE::COL_LAYER::kLOS);
        if (fwdRayDist < fwdCheckStep * i) {
            continue;
        }

        // Downward raycast to detect ledge point
        RE::NiPoint3 downRayStart = fwdRayStart + checkDir * fwdRayDist;
        float downRayDist = RayCast(downRayStart, downRayDir, startZOffset + maxUpCheck, normalOut, RE::COL_LAYER::kLOS);

        ledgePoint = downRayStart + downRayDir * downRayDist;
        normalZ = normalOut.quad.m128_f32[2];

        // Validate ledge based on height and flatness
        if (ledgePoint.z < playerPos.z + minLedgeHeight || ledgePoint.z > playerPos.z + maxLedgeHeight || downRayDist < 10 ||
            normalZ < minLedgeFlatness) {
            continue;
        }

        // Backward ray to check for obstructions behind the vaultable surface
        RE::NiPoint3 backwardRayStart = fwdRayStart + checkDir * (fwdRayDist - 2) + RE::NiPoint3(0, 0, 5);
        float backwardRayDist = RayCast(backwardRayStart, checkDir, 40.0f, normalOut, RE::COL_LAYER::kLOS);

        if (backwardRayDist > 0 && backwardRayDist < 20.0f) {
            continue;  // Obstruction behind the vaultable surface
        }

        foundLedge = true;
        break;
    }

    if (!foundLedge) {
        return ParkourType::NoLedge;
    }

    // Ensure there is sufficient headroom for the player to stand
    float headroomBuffer = 10 * RuntimeVariables::PlayerScale;
    RE::NiPoint3 headroomRayStart = ledgePoint + upRayDir * headroomBuffer;
    float headroomRayDist = RayCast(headroomRayStart, upRayDir, playerHeight - headroomBuffer, normalOut, RE::COL_LAYER::kLOS);

    if (headroomRayDist < playerHeight - headroomBuffer) {
        return ParkourType::NoLedge;
    }

    float ledgePlayerDiff = ledgePoint.z - playerPos.z;

    if (PlayerIsGroundedOrSliding() || PlayerIsSwimming()) {
        if (ledgePlayerDiff >= HardCodedVariables::highestLedgeLimit * RuntimeVariables::PlayerScale) {
            if (ShouldReplaceMarkerWithFailed()) {
                return ParkourType::Failed;
            }
            return ParkourType::Highest;  // Highest ledge

        } else if (ledgePlayerDiff >= HardCodedVariables::highLedgeLimit * RuntimeVariables::PlayerScale) {
            if (ShouldReplaceMarkerWithFailed()) {
                return ParkourType::Failed;
            }
            return ParkourType::High;  // High ledge

        } else if (ledgePlayerDiff >= HardCodedVariables::medLedgeLimit * RuntimeVariables::PlayerScale) {
            if (ShouldReplaceMarkerWithFailed()) {
                return ParkourType::Failed;
            }
            return ParkourType::Medium;  // Medium ledge

        } else if (ledgePlayerDiff >= HardCodedVariables::highStepLimit * RuntimeVariables::PlayerScale) {
            if (PlayerIsSwimming()) {
                return ParkourType::Grab;  // Grab ledge out of water, don't step out
            }

            // Additional horizontal and vertical checks for low ledge
            double horizontalDistance = sqrt(pow(ledgePoint.x - playerPos.x, 2) + pow(ledgePoint.y - playerPos.y, 2));
            double verticalDistance = abs(ledgePlayerDiff);

            if (horizontalDistance < verticalDistance * ledgeHypotenuse) {
                return ParkourType::StepHigh;  // Low ledge
            }

        } else {
            if (PlayerIsSwimming()) {
                return ParkourType::Grab;  // Grab ledge out of water, don't step out
            }

            // Additional horizontal and vertical checks for low ledge
            double horizontalDistance = sqrt(pow(ledgePoint.x - playerPos.x, 2) + pow(ledgePoint.y - playerPos.y, 2));
            double verticalDistance = abs(ledgePlayerDiff);

            if (!PlayerIsOnStairs() && horizontalDistance < verticalDistance * ledgeHypotenuse) {
                return ParkourType::StepLow;  // Quick Step
            }
        }

    } else if (PlayerIsMidairAndNotSliding() && ledgePlayerDiff > -35 && ledgePlayerDiff <= 100 * RuntimeVariables::PlayerScale) {
        if (!PlayerIsOnStairs() && player->AsActorState()->GetWeaponState() == RE::WEAPON_STATE::kSheathed &&
            player->GetCharController()->fallTime > 0.4f) {
            return ParkourType::Grab;
        }
    }
    return ParkourType::NoLedge;
}
int Parkouring::VaultCheck(RE::NiPoint3 &ledgePoint, RE::NiPoint3 checkDir, float vaultLength, float maxElevationIncrease,
                           float minVaultHeight, float maxVaultHeight) {
    const auto player = RE::PlayerCharacter::GetSingleton();

    if (!PlayerIsGroundedOrSliding()) {
        return ParkourType::NoLedge;
    }

    const auto playerPos = player->GetPosition();

    RE::hkVector4 normalOut(0, 0, 0, 0);

    float headHeight = 120 * RuntimeVariables::PlayerScale;

    // Forward raycast to check for a vaultable surface
    RE::NiPoint3 fwdRayStart = playerPos + RE::NiPoint3(0, 0, headHeight);
    float fwdRayDist = RayCast(fwdRayStart, checkDir, vaultLength, normalOut, RE::COL_LAYER::kLOS);

    if (RuntimeVariables::lastHitObject == RE::COL_LAYER::kTerrain || fwdRayDist < vaultLength) {
        return ParkourType::NoLedge;  // Not vaultable if terrain or insufficient distance
    }

    // Backward ray to check for obstructions behind the vaultable surface
    RE::NiPoint3 backwardRayStart = fwdRayStart + checkDir * (fwdRayDist - 2) + RE::NiPoint3(0, 0, 5);
    float backwardRayDist = RayCast(backwardRayStart, checkDir, 50.0f, normalOut, RE::COL_LAYER::kLOS);

    if (backwardRayDist > 0 && backwardRayDist < 50.0f) {
        return ParkourType::NoLedge;  // Obstruction behind the vaultable surface
    }

    // Downward raycast initialization
    int downIterations = /*static_cast<int>(std::floor(vaultLength / 5.0f))*/ 20;
    RE::NiPoint3 downRayDir(0, 0, -1);

    bool foundVaulter = false;
    float foundVaultHeight = -10000.0f;
    bool foundLanding = false;
    float foundLandingHeight = 10000.0f;

    // Incremental downward raycasts
    for (int i = 0; i < downIterations; i++) {
        float iDist = static_cast<float>(i) * 5.0f;
        RE::NiPoint3 downRayStart = playerPos + checkDir * iDist;
        downRayStart.z = fwdRayStart.z;

        float downRayDist = RayCast(downRayStart, downRayDir, headHeight + 100.0f, normalOut, RE::COL_LAYER::kLOS);
        float hitHeight = (fwdRayStart.z - downRayDist) - playerPos.z;

        // Check hit height for vaultable surfaces
        if (hitHeight > maxVaultHeight) {
            return ParkourType::NoLedge;  // Too high to vault
        } else if (hitHeight > minVaultHeight && hitHeight < maxVaultHeight) {
            if (hitHeight >= foundVaultHeight) {
                foundVaultHeight = hitHeight;
                foundLanding = false;
            }
            ledgePoint = downRayStart + downRayDir * downRayDist;
            foundVaulter = true;
        } else if (foundVaulter && hitHeight < minVaultHeight) {
            foundLandingHeight = std::min(hitHeight, foundLandingHeight);
            foundLanding = true;
        }
    }

    // Final validation for vault
    if (foundVaulter && foundLanding && foundLandingHeight < maxElevationIncrease) {
        ledgePoint.z = playerPos.z + foundVaultHeight;
        if (!PlayerIsOnStairs()) {
            return ParkourType::Vault;  // Vault successful
        }
    }

    return ParkourType::NoLedge;  // Vault failed
}

int Parkouring::GetLedgePoint(float backwardOffset = 55.0f) {
    using namespace GameReferences;
    using namespace ModSettings;

    if (!indicatorRef_Blue || !indicatorRef_Red) {
        return ParkourType::NoLedge;
    }

    const auto player = RE::PlayerCharacter::GetSingleton();
    //const auto playerPos = player->GetPosition();
    const bool isMoving = player->IsMoving();

    RE::NiPoint3 playerDirFlat = GetPlayerDirFlat(player);

    // Perform ledge or vault checks
    int selectedLedgeType = ParkourType::NoLedge;
    RE::NiPoint3 ledgePoint;

    if (isMoving || !ModSettings::Smart_Parkour_Enabled) {
        selectedLedgeType = VaultCheck(ledgePoint, playerDirFlat, 85, 70 * RuntimeVariables::PlayerScale,
                                       HardCodedVariables::vaultMinHeight * RuntimeVariables::PlayerScale,
                                       HardCodedVariables::vaultMaxHeight * RuntimeVariables::PlayerScale);
    }

    if (selectedLedgeType == ParkourType::NoLedge) {
        selectedLedgeType = LedgeCheck(ledgePoint, playerDirFlat, HardCodedVariables::climbMinHeight * RuntimeVariables::PlayerScale,
                                       HardCodedVariables::climbMaxHeight * RuntimeVariables::PlayerScale);
    }
    if (selectedLedgeType == ParkourType::NoLedge || PlayerVsObjectAngle(ledgePoint) > 80) {
        return ParkourType::NoLedge;
    }

    // Don't ever parkour into water, last check before saying this ledge is valid
    float waterLevel;
    player->GetParentCell()->GetWaterHeight(player->GetPosition(), waterLevel);  //Relative to player

    if (ledgePoint.z < waterLevel - 15) {
        return ParkourType::NoLedge;
    }

    // Choose indicator depending on stamina
    currentIndicatorRef = indicatorRef_Blue;
    if (Enable_Stamina_Consumption && PlayerHasEnoughStamina() == false && CheckIsVaultActionFromType(selectedLedgeType) == false) {
        currentIndicatorRef = indicatorRef_Red;
        indicatorRef_Blue->Disable();
    } else {
        indicatorRef_Red->Disable();
    }

    // Move indicator to the correct position
    if (currentIndicatorRef->GetParentCell() != player->GetParentCell()) {
        currentIndicatorRef->MoveTo(player->AsReference());
    }

    // RE::NiPoint3 cameraDirFlat = GetCameraDirFlat();

    RE::NiPoint3 backwardAdjustment = playerDirFlat * backwardOffset * RuntimeVariables::PlayerScale;
    currentIndicatorRef->data.location = ledgePoint + RE::NiPoint3(0, 0, 10);  // Offset upwards slightly, 5 -> 10
    currentIndicatorRef->Update3DPosition(true);
    currentIndicatorRef->data.angle = RE::NiPoint3(0, 0, atan2(playerDirFlat.x, playerDirFlat.y));

    RuntimeVariables::backwardAdjustment = backwardAdjustment;
    RuntimeVariables::ledgePoint = ledgePoint;
    RuntimeVariables::playerDirFlat = playerDirFlat;

    return selectedLedgeType;
}
void Parkouring::AdjustPlayerPosition() {
    const auto player = RE::PlayerCharacter::GetSingleton();

    // Select appropriate ledge marker and adjustments
    // switch takes constants so can't pass these from references.h, here's the lookup table

    float zAdjust = 0;
    float z = 0;
    //const int Highest = 7;
    //const int High = 6;
    //const int Medium = 5;
    //const int StepHigh = 4;
    //const int StepLow = 3;
    //const int Vault = 2;
    //const int Grab = 1;
    //const int Failed = 0;
    //const int NoLedge = -1;

    switch (RuntimeVariables::selectedLedgeType) {
        case 7:  // Highest Ledge
            z = HardCodedVariables::highestLedgeElevation - 3;
            zAdjust = -z * RuntimeVariables::PlayerScale;
            break;

        case 6:  // High ledge
            z = HardCodedVariables::highLedgeElevation - 3;
            zAdjust = -z * RuntimeVariables::PlayerScale;
            break;

        case 5:  // Medium ledge
            z = HardCodedVariables::medLedgeElevation - 3;
            zAdjust = -z * RuntimeVariables::PlayerScale;
            break;

        case 4:  // Step High (Former Low Ledge)
            z = HardCodedVariables::stepHighElevation - 3;
            zAdjust = -z * RuntimeVariables::PlayerScale;
            RuntimeVariables::backwardAdjustment =
                RuntimeVariables::playerDirFlat * 20 * RuntimeVariables::PlayerScale;  // Override backward offset
            break;

        case 3:  // Step Low
            z = HardCodedVariables::stepLowElevation - 3;
            zAdjust = -z * RuntimeVariables::PlayerScale;
            RuntimeVariables::backwardAdjustment =
                RuntimeVariables::playerDirFlat * 20 * RuntimeVariables::PlayerScale;  // Override backward offset
            break;

        case 2:  // Vault
            z = HardCodedVariables::vaultElevation - 3;
            zAdjust = -z * RuntimeVariables::PlayerScale;
            break;

        case 1:  // Grab (Midair or Out of Water)
            z = HardCodedVariables::grabElevation - 3;
            zAdjust = -z * RuntimeVariables::PlayerScale;
            RuntimeVariables::backwardAdjustment =
                RuntimeVariables::playerDirFlat * 40 * RuntimeVariables::PlayerScale;  // Override backward offset
            break;

        case 0:  // Failed (Low Stamina Animation)
            return;

        default:
            return;
    }

    // Check if the player will go underwater after position adjustment, and decrease ledge player diff.
    float waterLevel;
    RE::NiPoint3 playerPos = player->GetPosition();
    player->GetParentCell()->GetWaterHeight(playerPos, waterLevel);
    auto playerWaterDiff = playerPos.z - waterLevel;
    auto adjustThreshold = -50.0f * RuntimeVariables::PlayerScale;

    if (playerWaterDiff < adjustThreshold && !player->AsActorState()->IsSwimming() && player->IsInWater()) {
        zAdjust += (abs(playerWaterDiff) - 60) * RuntimeVariables::PlayerScale;
        //logger::info("ledgeZ: {} threshold {} zadjust:{} diff{}", RuntimeVariables::ledgePoint.z - playerPos.z, adjustThreshold, zAdjust,playerWaterDiff);
    }

    player->SetPosition(
        RE::NiPoint3{RuntimeVariables::ledgePoint.x - RuntimeVariables::backwardAdjustment.x,
                     RuntimeVariables::ledgePoint.y - RuntimeVariables::backwardAdjustment.y, RuntimeVariables::ledgePoint.z + zAdjust},
        true);
}

void Parkouring::UpdateParkourPoint() {
    if (!ModSettings::ModEnabled || RuntimeVariables::ParkourEndQueued) {
        if (GameReferences::currentIndicatorRef)
            GameReferences::currentIndicatorRef->Disable();
        RuntimeVariables::selectedLedgeType = -1;
        return;
    }

    if (!AnimEventListener::Register()) {
        return;
    }

    const auto player = RE::PlayerCharacter::GetSingleton();
    //logger::info("Ongoing {}", PluginReferences::ParkourOngoing);

    RuntimeVariables::PlayerScale = ScaleUtility::GetScale();
    RuntimeVariables::selectedLedgeType = GetLedgePoint();

    if (ModSettings::PresetParkourKey == ModSettings::ParkourKeyOptions::kJump && ModSettings::parkourDelay == 0) {
        if (RuntimeVariables::selectedLedgeType == -1) {
            RE::ControlMap::GetSingleton()->ToggleControls(RE::ControlMap::UEFlag::kJumping, true);
            //logger::info("jump enabled");
        } else if (RuntimeVariables::ParkourEndQueued == false) {
            RE::ControlMap::GetSingleton()->ToggleControls(RE::ControlMap::UEFlag::kJumping, false);
            //logger::info("jump disabled");
        }
    }

    /* ===================================== */

    player->SetGraphVariableInt("SkyParkourLedge", RuntimeVariables::selectedLedgeType);

    /* ===================================== */

    // If player is not grounded or is in water, reset jump key and return early
    if (!IsParkourActive() /*&& PluginReferences::ParkourOngoing == false*/) {
        //ToggleJumpingInternal(true);  // Ensure jump key is re-enabled to prevent being stuck
        //ToggleControlsForParkour(true);
        if (GameReferences::currentIndicatorRef)
            GameReferences::currentIndicatorRef->Disable();
        //return;
    } else {
        if (GameReferences::currentIndicatorRef)
            GameReferences::currentIndicatorRef->Enable(false);  // Don't reset inventory
    }
}
bool Parkouring::TryActivateParkour() {
    using namespace GameReferences;
    using namespace ModSettings;
    const auto player = RE::PlayerCharacter::GetSingleton();

    if (!IsParkourActive()) {
        return false;
    }
    if (RuntimeVariables::ParkourEndQueued) {
        return false;
    }

    const bool isMoving = player->IsMoving();
    // const bool isSprinting = player->IsSprinting();

    if (Smart_Parkour_Enabled && isMoving) {
        if (RuntimeVariables::selectedLedgeType == ParkourType::High || RuntimeVariables::selectedLedgeType == ParkourType::Highest ||
            RuntimeVariables::selectedLedgeType == ParkourType::Failed) {
            return false;
        }
    }

    RuntimeVariables::ParkourEndQueued = true;

    ToggleControlsForParkour(false);
    AdjustPlayerPosition();

    SKSE::GetTaskInterface()->AddTask([]() { ParkourReadyRun(); });

    return true;
}
void Parkouring::ParkourReadyRun() {
    const auto player = RE::PlayerCharacter::GetSingleton();

    bool isVault = CheckIsVaultActionFromType(RuntimeVariables::selectedLedgeType);

    if (RuntimeVariables::selectedLedgeType == ParkourType::Grab && !PlayerIsSwimming()) {
        player->NotifyAnimationGraph("JumpStandingStart");
    }

    //if (PlayerIsGrounded()) {
    //player->NotifyAnimationGraph("JumpLandEnd");
    //} else {
    //player->NotifyAnimationGraph("JumpStandingStart");
    //}

    if (player->AsActorState()->IsSwimming()) {
        player->NotifyAnimationGraph("SwimStop");
    }

    player->NotifyAnimationGraph("IdleLeverPushStart");

    // Reliably detect if the player is actually playing the animation (LOST COUNTLESS SLEEPLESS NIGHTS TO THIS,
    // WORTH IT WOOOOO)
    bool success = player->IsAnimationDriven();
    // logger::info("Animation Driven: {}", success);

    if (success) {
        if (ModSettings::Enable_Stamina_Consumption) {
            float cost = ParkourUtility::CalculateParkourStamina();

            if (isVault) {
                // logger::info("cost{}", cost / 2);
                DamageActorStamina(player, cost / 2);

            } else if (PlayerHasEnoughStamina()) {
                // logger::info("cost{}", cost);
                DamageActorStamina(player, cost);
            } else {
                RE::HUDMenu::FlashMeter(RE::ActorValue::kStamina);
                player->UpdateRegenDelay(RE::ActorValue::kStamina, 2.0f);
            }
        }
    } else {
        ToggleControlsForParkour(true);
        RuntimeVariables::ParkourEndQueued = false;
        return;
    }
}

void Parkouring::SetParkourOnOff(bool turnOn) {
    if (turnOn) {
        ButtonEventListener::Unregister();
        AnimEventListener::Unregister();

        ParkourUtility::ToggleControlsForParkour(true);
        RuntimeVariables::ParkourEndQueued = false;

        GameReferences::currentIndicatorRef->Disable();

    } else {
        ButtonEventListener::Register();
        AnimEventListener::Register();
    }
}
void Parkouring::ToggleModOnOff(bool turnOn) {}