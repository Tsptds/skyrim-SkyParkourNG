#include "Parkouring.h"

using namespace ParkourUtility;

RE::NiPoint3 GetPlayerDirFlat(RE::Actor* player) {
    // Calculate player forward direction (normalized)
    const float playerYaw = player->data.angle.z;  // Player's yaw

    RE::NiPoint3 playerDirFlat{std::sin(playerYaw), std::cos(playerYaw), 0};
    const float dirMagnitude = std::hypot(playerDirFlat.x, playerDirFlat.y);
    playerDirFlat.x /= dirMagnitude;
    playerDirFlat.y /= dirMagnitude;

    return playerDirFlat;
}
int GetLedgePoint(float backwardOffset = 55.0f) {
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
                                        HardCodedVariables::vaultMinHeight * RuntimeVariables::PlayerScale, HardCodedVariables::vaultMaxHeight * RuntimeVariables::PlayerScale);
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
    player->GetParentCell()->GetWaterHeight(player->GetPosition(), waterLevel); //Relative to player

    if (ledgePoint.z < waterLevel -15) {
        return ParkourType::NoLedge;
    }

    // Choose indicator depending on stamina
    currentIndicatorRef = indicatorRef_Blue;
    if (Enable_Stamina_Consumption && PlayerHasEnoughStamina() == false &&
        CheckIsVaultActionFromType(selectedLedgeType) == false) {
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
void AdjustPlayerPosition() {
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
                RuntimeVariables::playerDirFlat * 20 * RuntimeVariables::PlayerScale;   // Override backward offset
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
                RuntimeVariables::playerDirFlat * 40 * RuntimeVariables::PlayerScale;   // Override backward offset
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
        zAdjust += (abs(playerWaterDiff) -60)* RuntimeVariables::PlayerScale;
    //logger::info("ledgeZ: {} threshold {} zadjust:{} diff{}", RuntimeVariables::ledgePoint.z - playerPos.z, adjustThreshold, zAdjust,playerWaterDiff);
    }

    player->SetPosition(RE::NiPoint3{
        RuntimeVariables::ledgePoint.x - RuntimeVariables::backwardAdjustment.x , 
        RuntimeVariables::ledgePoint.y - RuntimeVariables::backwardAdjustment.y , 
        RuntimeVariables::ledgePoint.z + zAdjust} ,
        true
    );
}

void Parkouring::UpdateParkourPoint() {

    if (!ModSettings::ModEnabled || RuntimeVariables::ParkourEndQueued) {
        if (GameReferences::currentIndicatorRef) GameReferences::currentIndicatorRef->Disable();
        RuntimeVariables::selectedLedgeType = -1;
        return;
    }
        
    if (!AnimEventListener::RegisterAnimationEventListener()) {
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
        if (RuntimeVariables::selectedLedgeType == ParkourType::High ||
            RuntimeVariables::selectedLedgeType == ParkourType::Highest ||
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

    if (RuntimeVariables::selectedLedgeType == ParkourType::Grab &&
        !PlayerIsSwimming()) {
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