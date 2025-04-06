#include "ParkourUtility.h"


bool ParkourUtility::ToggleControlsForParkour(bool enable) {
    auto player = RE::PlayerCharacter::GetSingleton();
    auto playerCamera = RE::PlayerCamera::GetSingleton();
    if (!player || !playerCamera) return false;

    auto controlMap = RE::ControlMap::GetSingleton();
    controlMap->ToggleControls(RE::ControlMap::UEFlag::kPOVSwitch, enable);
    controlMap->ToggleControls(RE::ControlMap::UEFlag::kJumping, enable);
    controlMap->ToggleControls(RE::ControlMap::UEFlag::kFighting, enable);
    controlMap->ToggleControls(RE::ControlMap::UEFlag::kInvalid, enable);
    controlMap->ToggleControls(RE::ControlMap::UEFlag::kMainFour, enable);
    //controlMap->ToggleControls(RE::ControlMap::UEFlag::kMenu, enable);
    controlMap->ToggleControls(RE::ControlMap::UEFlag::kActivate, enable);
    controlMap->ToggleControls(RE::ControlMap::UEFlag::kWheelZoom, enable);

    /*if (!enable && player->actorState1.sneaking) {
        controlMap->enabledControls.reset(RE::ControlMap::UEFlag::kSneaking);
    }*/


    if (!enable && playerCamera && playerCamera->IsInThirdPerson()) {
        // Stop POV switching if it is already happening in 3rd person, then enable cam state so mouse wheel works
        // after parkour ends
        RE::ThirdPersonState *thirdPersonState = nullptr;
        thirdPersonState = skyrim_cast<RE::ThirdPersonState *>(playerCamera->currentState.get());
        thirdPersonState->targetZoomOffset = thirdPersonState->currentZoomOffset;
        thirdPersonState->stateNotActive = false;

    } else if (!enable && playerCamera && playerCamera->IsInFirstPerson()) {
        RuntimeVariables::wasFirstPerson = true;
        playerCamera->ForceThirdPerson();

        RE::ThirdPersonState *thirdPersonState = nullptr;
        thirdPersonState = skyrim_cast<RE::ThirdPersonState *>(playerCamera->currentState.get());
        thirdPersonState->targetZoomOffset = thirdPersonState->currentZoomOffset = 0.3f;
        thirdPersonState->stateNotActive = false;
            
            
    } else if (enable && RuntimeVariables::wasFirstPerson) {
            
        // Match the third person camera angle to first person, so it feels better like vanilla
        RE::ThirdPersonState *thirdPersonState = nullptr;
        thirdPersonState = skyrim_cast<RE::ThirdPersonState *>(playerCamera->currentState.get());

        player->data.angle.z = thirdPersonState->currentYaw;
            
        RuntimeVariables::wasFirstPerson = false;
        playerCamera->ForceFirstPerson();
    }


    if (enable && player->AsActorState()->actorState1.sneaking) {
        player->NotifyAnimationGraph("SneakStart");
        //controlMap->enabledControls.set(RE::ControlMap::UEFlag::kSneaking);
    }
    if (enable && player->AsActorState()->actorState2.weaponState != RE::WEAPON_STATE::kSheathed) {
        player->AsActorState()->actorState2.weaponState = RE::WEAPON_STATE::kWantToDraw;
    }

    return true;
}

float ParkourUtility::PlayerVsObjectAngle(const RE::NiPoint3 &objPoint) {
    const auto player = RE::PlayerCharacter::GetSingleton();
    if (!player) {
        return 0.0f;  // Return a safe default if the player singleton is null
    }

    // Get the vector from the player's head to the object
    RE::NiPoint3 playerToObject = objPoint - player->GetPosition();
    playerToObject.z -= 120.0f;  // Adjust for head level

    // Normalize the vector
    const float distance = playerToObject.Length();
    if (distance == 0.0f) {
        return 0.0f;  // Avoid division by zero
    }
    playerToObject /= distance;

    // Get the player's forward direction in the XY plane
    const float playerYaw = player->data.angle.z;
    RE::NiPoint3 playerForwardDir{std::sin(playerYaw), std::cos(playerYaw), 0.0f};

    // Dot product between player's forward direction and the object direction
    const float dot = playerToObject.x * playerForwardDir.x + playerToObject.y * playerForwardDir.y;

    // Clamp the dot product to avoid domain errors in acos
    const float clampedDot = std::clamp(dot, -1.0f, 1.0f);

    // Calculate the angle in degrees
    return std::acos(clampedDot) * 57.2958f;  // radToDeg constant
}

void ParkourUtility::LastObjectHitType(RE::COL_LAYER obj) { RuntimeVariables::lastHitObject = obj; }

float ParkourUtility::RayCast(RE::NiPoint3 rayStart, RE::NiPoint3 rayDir, float maxDist, RE::hkVector4 &normalOut,
                RE::COL_LAYER layerMask) {
    const auto player = RE::PlayerCharacter::GetSingleton();
    if (!player) {
        normalOut = RE::hkVector4(0.0f, 0.0f, 0.0f, 0.0f);
        return maxDist;  // Return maxDist if player is null
    }
    const auto cell = player->GetParentCell();
    if (!cell) {
        normalOut = RE::hkVector4(0.0f, 0.0f, 0.0f, 0.0f);
        return maxDist;  // Return maxDist if cell is unavailable
    }
    const auto bhkWorld = cell->GetbhkWorld();
    if (!bhkWorld) {
        normalOut = RE::hkVector4(0.0f, 0.0f, 0.0f, 0.0f);
        return maxDist;  // Return maxDist if Havok world is unavailable
    }

    RE::bhkPickData pickData;
    const auto havokWorldScale = RE::bhkWorld::GetWorldScale();

    // Set ray start and end points (scaled to Havok world)
    pickData.rayInput.from = rayStart * havokWorldScale;
    pickData.rayInput.to = (rayStart + rayDir * maxDist) * havokWorldScale;

    // Set the collision filter info to exclude the player
    uint32_t collisionFilterInfo = 0;
    player->GetCollisionFilterInfo(collisionFilterInfo);
    pickData.rayInput.filterInfo = (collisionFilterInfo & 0xFFFF0000) | static_cast<uint32_t>(layerMask);

    // Perform the raycast
    if (bhkWorld->PickObject(pickData) && pickData.rayOutput.HasHit()) {
        normalOut = pickData.rayOutput.normal;

        const uint32_t layerIndex = pickData.rayOutput.rootCollidable->broadPhaseHandle.collisionFilterInfo & 0x7F;

        if (layerIndex == 0) {
            return -1.0f;  // Invalid layer hit
        }

        // Optionally log the layer hit
        // if (logLayer) logger::info("\nLayer hit: {}", layerIndex);

        // Check for useful collision layers
        switch (static_cast<RE::COL_LAYER>(layerIndex)) {
            case RE::COL_LAYER::kStatic:
            case RE::COL_LAYER::kCollisionBox:
            case RE::COL_LAYER::kTerrain:
            case RE::COL_LAYER::kGround:
            case RE::COL_LAYER::kProps:
            case RE::COL_LAYER::kDoorDetection:
            case RE::COL_LAYER::kTrees:
            case RE::COL_LAYER::kClutterLarge:
            case RE::COL_LAYER::kAnimStatic:
            case RE::COL_LAYER::kDebrisLarge:
                // Update last hit object type
                LastObjectHitType(static_cast<RE::COL_LAYER>(layerIndex));
                return maxDist * pickData.rayOutput.hitFraction;

            default:
                return -1.0f;  // Ignore unwanted layers
        }
    }

    // No hit
    normalOut = RE::hkVector4(0.0f, 0.0f, 0.0f, 0.0f);
    // if (logLayer) logger::info("Nothing hit");

    return maxDist;
}

bool ParkourUtility::IsActorUsingFurniture(RE::Actor *actor) {
    auto ref = actor->GetOccupiedFurniture();
    if (ref) {
        return true;
    }
    return false;
}

float ParkourUtility::CalculateParkourStamina() {
    const auto player = RE::PlayerCharacter::GetSingleton();
    float equip = player->GetEquippedWeight();
    //float carry = player->GetTotalCarryWeight();

    float finalCalc = ModSettings::Stamina_Damage + (equip * 0.2f);

    return finalCalc;
}

bool ParkourUtility::PlayerHasEnoughStamina() {
    const auto player = RE::PlayerCharacter::GetSingleton();
    const auto currentStamina = player->AsActorValueOwner()->GetActorValue(RE::ActorValue::kStamina);

    if (!ModSettings::Is_Stamina_Required || currentStamina > CalculateParkourStamina() /* && ModSettings::Is_Stamina_Required */) {
        return true;
    }
    return false;
}

bool ParkourUtility::DamageActorStamina(RE::Actor *actor, float amount) {
    if (actor) {
        actor->AsActorValueOwner()->RestoreActorValue(RE::ACTOR_VALUE_MODIFIERS::kDamage, RE::ActorValue::kStamina, -amount);
        return true;
    }
    return false;
}

bool ParkourUtility::ShouldReplaceMarkerWithFailed() {
    // If stamina options are on, check if player has enough stamina. If not, play failed anim. If stamina is on but
    // isn't required, just deal stamina damage. Only for med and high climbing, would get annoying fast.
    if (ModSettings::Enable_Stamina_Consumption && !PlayerIsSwimming()) {
        if (PlayerHasEnoughStamina() == false) {
            return true;
        }
    }
    return false;
}

// Return true if action is vaulting, and not a climbing, low grab is also considered vault
bool ParkourUtility::CheckIsVaultActionFromType(int32_t selectedLedgeType) {
    return selectedLedgeType == ParkourType::Vault || selectedLedgeType == ParkourType::StepHigh || selectedLedgeType == ParkourType::Grab || selectedLedgeType == ParkourType::StepLow;
}

bool ParkourUtility::PlayerIsGroundedOrSliding() {
    const auto player = RE::PlayerCharacter::GetSingleton();
    const auto charController = player->GetCharController();

    // logger::info("Flag {}", charController->flags.underlying());
    // Check if the player is in the air (jumping flag)
    if (player && charController && /*!charController->flags.any(RE::CHARACTER_FLAGS::kJumping) &&
        charController->flags.all(RE::CHARACTER_FLAGS::kCanJump) &&*/ charController->surfaceInfo.supportedState != RE::hkpSurfaceInfo::SupportedState::kUnsupported){
        return true;
    }
    return false;
}

bool ParkourUtility::PlayerIsMidairAndNotSliding() {
    const auto player = RE::PlayerCharacter::GetSingleton();
    const auto charController = player->GetCharController();

    if (player && charController &&
            charController->surfaceInfo.supportedState == RE::hkpSurfaceInfo::SupportedState::kUnsupported) {
        return true;
    }
    return false;
}

bool ParkourUtility::PlayerIsSwimming() {
    const auto player = RE::PlayerCharacter::GetSingleton();
    if (!player) {
        return false;
    }
    return player->AsActorState()->IsSwimming();
        
    // IDK why swim at surface works this way, but it does.
    //return player->boolBits.any(RE::Actor::BOOL_BITS::kSwimming) ||
    //        player->boolBits.any(RE::Actor::BOOL_BITS::kInWater) /*&& !player->GetCharController()->flags.any(RE::CHARACTER_FLAGS::kSwimAtWaterSurface))*/;
}

/*bool IsActorWeaponSheathed(RE::Actor *actor) {
    return actor->GetWeaponState() == RE::WEAPON_STATE::kSheathed;
}*/

bool ParkourUtility::PlayerWantsToDrawSheath() 
{
    const auto player = RE::PlayerCharacter::GetSingleton();

    return player->AsActorState()->GetWeaponState() == RE::WEAPON_STATE::kWantToDraw ||
           player->AsActorState()->GetWeaponState() == RE::WEAPON_STATE::kWantToSheathe;
}

bool ParkourUtility::IsParkourActive() {
    auto player = RE::PlayerCharacter::GetSingleton();

    auto ui = RE::UI::GetSingleton();
    if (!player || !ui) return false;

    if (IsActorUsingFurniture(player) /*|| !IsActorWeaponSheathed(player)*/) {
        return false;
    }
        
    if (PlayerWantsToDrawSheath()) {
        return false;
    }

    if (RuntimeVariables::selectedLedgeType == ParkourType::NoLedge) {
        return false;
    }

    // Check if the game is paused
    if (ui->GameIsPaused()) {
        return false;
    }

    // Check if player has chargen flags (hands bound, saving disabled etc)     6 hands bound, 3 vamp lord transform
    const auto &gs = player->GetGameStatsData();  // RE::PlayerCharacter::GetGameStatsData
    /*logger::info(" chargen flag: {}", charGenFlag.underlying());*/
    if (gs.byCharGenFlag != RE::PlayerCharacter::ByCharGenFlag::kNone) {
        return false;
    }

    // Check if the player has transformed into a beast race
    const auto playerPreTransformData = player->GetPlayerRuntimeData().preTransformationData;
    if (playerPreTransformData) {
        /* logger::info("player race {}", playerPreTransformData->storedRace->GetFormEditorID());*/
        return false;
    }

    // List of disqualifying menu names
    const std::string_view excludedMenus[] = {RE::BarterMenu::MENU_NAME,       RE::ConsoleNativeUIMenu::MENU_NAME,
                                                RE::ContainerMenu::MENU_NAME,    RE::CraftingMenu::MENU_NAME,
                                                RE::CreationClubMenu::MENU_NAME, RE::DialogueMenu::MENU_NAME,
                                                RE::FavoritesMenu::MENU_NAME,    RE::GiftMenu::MENU_NAME,
                                                RE::InventoryMenu::MENU_NAME,    RE::JournalMenu::MENU_NAME,
                                                RE::LevelUpMenu::MENU_NAME,      RE::LockpickingMenu::MENU_NAME,
                                                RE::MagicMenu::MENU_NAME,        RE::MapMenu::MENU_NAME,
                                                RE::MessageBoxMenu::MENU_NAME,   RE::MistMenu::MENU_NAME,
                                                RE::RaceSexMenu::MENU_NAME,      RE::SleepWaitMenu::MENU_NAME,
                                                RE::StatsMenu::MENU_NAME,        RE::TrainingMenu::MENU_NAME,
                                                RE::TweenMenu::MENU_NAME};

    // Check if any of the excluded menus are open
    for (const std::string_view menuName : excludedMenus) {
        if (ui->IsMenuOpen(menuName)) {
            return false;
        }
    }

    return true;
}

bool ParkourUtility::PlayerIsOnStairs() {
    const auto player = RE::PlayerCharacter::GetSingleton();
    if (!player) {
        return false;  // Early exit if the player is null
    }

    const auto charController = player->GetCharController();
    return charController && charController->flags.any(RE::CHARACTER_FLAGS::kOnStairs) /*&&
            PluginReferences::lastHitObject == RE::COL_LAYER::kStairHelper*/;
}

float ParkourUtility::magnitudeXY(float x, float y) { return sqrt(x * x + y * y); }

void ParkourUtility::MoveMarkerToLedge(RE::TESObjectREFR *ledgeMarker, RE::NiPoint3 ledgePoint, RE::NiPoint3 backwardAdjustment,
                        float zAdjust) {
    // Position ledge marker with adjustments
    ledgeMarker->SetPosition(
        {ledgePoint.x - backwardAdjustment.x, ledgePoint.y - backwardAdjustment.y, ledgePoint.z + zAdjust});
}

void ParkourUtility::RotateLedgeMarker(RE::TESObjectREFR *ledgeMarker, RE::NiPoint3 playerDirFlat) {
    ledgeMarker->data.angle = RE::NiPoint3(0, 0, atan2(playerDirFlat.x, playerDirFlat.y));
}

int ParkourUtility::LedgeCheck(RE::NiPoint3 &ledgePoint, RE::NiPoint3 checkDir, float minLedgeHeight, float maxLedgeHeight) {
    const auto player = RE::PlayerCharacter::GetSingleton();
    const auto playerPos = player->GetPosition();

    // Constants adjusted for player scale
    const float startZOffset = 100 * RuntimeVariables::PlayerScale;
    const float playerHeight = 120 * RuntimeVariables::PlayerScale;
    const float minUpCheck = 100 * RuntimeVariables::PlayerScale;
    const float maxUpCheck = (maxLedgeHeight - startZOffset) + 20 * RuntimeVariables::PlayerScale;
    const float fwdCheckStep = 8 * RuntimeVariables::PlayerScale;
    const int fwdCheckIterations = 10;  // 15
    const float minLedgeFlatness = 0.5; //0.5
    const float ledgeHypotenuse = 1.0;  // 0.75 - larger is more relaxed, lesser is more strict. Don't set 0

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
        float downRayDist =
            RayCast(downRayStart, downRayDir, startZOffset + maxUpCheck, normalOut, RE::COL_LAYER::kLOS);

        ledgePoint = downRayStart + downRayDir * downRayDist;
        normalZ = normalOut.quad.m128_f32[2];

        // Validate ledge based on height and flatness
        if (ledgePoint.z < playerPos.z + minLedgeHeight || ledgePoint.z > playerPos.z + maxLedgeHeight ||
            downRayDist < 10 || normalZ < minLedgeFlatness) {
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
    float headroomRayDist =
        RayCast(headroomRayStart, upRayDir, playerHeight - headroomBuffer, normalOut, RE::COL_LAYER::kLOS);

    if (headroomRayDist < playerHeight - headroomBuffer) {
        return ParkourType::NoLedge;
    }

    float ledgePlayerDiff = ledgePoint.z - playerPos.z;

    if (PlayerIsGroundedOrSliding() || PlayerIsSwimming()) {
        if (ledgePlayerDiff >= HardCodedVariables::highestLedgeLimit * RuntimeVariables::PlayerScale) {
            if (ShouldReplaceMarkerWithFailed()) {
                return ParkourType::Failed;
            }
            return ParkourType::Highest;    // Highest ledge

        } else if (ledgePlayerDiff >= HardCodedVariables::highLedgeLimit * RuntimeVariables::PlayerScale) {
            if (ShouldReplaceMarkerWithFailed()) {
                return ParkourType::Failed;
            }
            return ParkourType::High;       // High ledge

        } else if (ledgePlayerDiff >= HardCodedVariables::medLedgeLimit * RuntimeVariables::PlayerScale) {
            if (ShouldReplaceMarkerWithFailed()) {
                return ParkourType::Failed;
            }
            return ParkourType::Medium;     // Medium ledge

        } else if(ledgePlayerDiff >= HardCodedVariables::highStepLimit * RuntimeVariables::PlayerScale){
                
            if (PlayerIsSwimming()) {
                return ParkourType::Grab;  // Grab ledge out of water, don't step out
            }
                
            // Additional horizontal and vertical checks for low ledge
            double horizontalDistance =
                sqrt(pow(ledgePoint.x - playerPos.x, 2) + pow(ledgePoint.y - playerPos.y, 2));
            double verticalDistance = abs(ledgePlayerDiff);

            if (horizontalDistance < verticalDistance * ledgeHypotenuse) {
                return ParkourType::StepHigh;  // Low ledge
            }

        } else {

            if (PlayerIsSwimming()) {
                return ParkourType::Grab;    // Grab ledge out of water, don't step out
            }

            // Additional horizontal and vertical checks for low ledge
            double horizontalDistance =
                sqrt(pow(ledgePoint.x - playerPos.x, 2) + pow(ledgePoint.y - playerPos.y, 2));
            double verticalDistance = abs(ledgePlayerDiff);

            if (!PlayerIsOnStairs() && horizontalDistance < verticalDistance * ledgeHypotenuse) {
                return ParkourType::StepLow;  // Quick Step
            }
        }

    } else if (PlayerIsMidairAndNotSliding() && ledgePlayerDiff > -35 &&
               ledgePlayerDiff <= 100 * RuntimeVariables::PlayerScale) {

        if (!PlayerIsOnStairs() && player->AsActorState()->GetWeaponState() == RE::WEAPON_STATE::kSheathed &&
            player->GetCharController()->fallTime > 0.4f) {
            return ParkourType::Grab;
        }
    }
    return ParkourType::NoLedge;
}

int ParkourUtility::VaultCheck(RE::NiPoint3 &ledgePoint, RE::NiPoint3 checkDir, float vaultLength, float maxElevationIncrease,
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

    if (RuntimeVariables::lastHitObject == RE::COL_LAYER::kTerrain || 
        fwdRayDist < vaultLength) {

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