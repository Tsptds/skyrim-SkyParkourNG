#include "ParkourUtility.h"

//class NodeOverride {
//    public:
//        NodeOverride(RE::NiNode *node, float scale)
//            : node(node) {
//            old_scale = node->local.scale;
//            node->local.scale = scale;
//        }
//        ~NodeOverride() {
//            node->local.scale = old_scale;
//        }
//
//    private:
//        RE::NiNode *node;
//        float old_scale;
//};
//static std::vector<std::unique_ptr<NodeOverride>> headOverride;
//static std::vector<std::unique_ptr<NodeOverride>> bodyOverride;

//void ToggleHeadNode(RE::PlayerCharacter *player, bool show) {
//    // Get3D gets false, we need the THIRD PERSON NODE
//    //auto *headNode = player->Get3D(0)->GetObjectByName("NPC Head [Head]")->AsNode();
//    auto *headNode = player->Get3D(0)->GetObjectByName("NPC Head [Head]")->AsNode();
//    auto *bodyNode = player->Get3D(0)->GetObjectByName("NPC Spine [Spn0]")->AsNode();
//
//    //auto cam = RE::PlayerCamera::GetSingleton();
//
//    if (show) {
//        headOverride.clear();
//        bodyOverride.clear();
//    } else {
//        // To hide the head, shrink it. Lol.
//        headOverride.push_back(std::make_unique<NodeOverride>(headNode, 0.001f));
//        bodyOverride.push_back(std::make_unique<NodeOverride>(bodyNode, 0.001f));
//    }
//    RE::NiUpdateData upd{0.0f, RE::NiUpdateData::Flag::kDirty};
//
//    // Push it through the node
//    headNode->UpdateControllers(upd);
//    headNode->UpdateDownwardPass(upd, /* arg2 = */ 0);
//}

bool ParkourUtility::IsParkourActive() {
    if (RuntimeVariables::selectedLedgeType == ParkourType::NoLedge) {
        return false;
    }

    if (RuntimeVariables::IsInRagdollOrGettingUp) {
        return false;
    }

    const auto player = RE::PlayerCharacter::GetSingleton();
    if (IsPlayerInCharGen(player)) {
        //logger::info("PLAYER HANDS BOUND");
        return false;
    }

    if (IsPlayerAlreadyAnimationDriven(player)) {
        return false;
    }

    if (IsPlayerInSyncedAnimation(player)) {
        return false;
    }

    if (IsOnMount()) {
        return false;
    }

    if (IsBeastForm()) {
        return false;
    }

    if (IsPlayerUsingFurniture(player) /*|| !IsActorWeaponSheathed(player)*/) {
        //logger::info("USING FURNITURE");
        return false;
    }

    if (PlayerWantsToDrawSheath()) {
        //logger::info("WILL DRAW/SHEATHE");
        return false;
    }

    // Check if the game is paused
    //auto ui = RE::UI::GetSingleton();
    /*if (ui && ui->GameIsPaused()) {
        return false;
    }*/

    if (RuntimeVariables::IsMenuOpen) {
        return false;
    }

    return true;
}

void ParkourUtility::StopInteractions(RE::Actor &a_actor) {
    a_actor.PauseCurrentDialogue();
    a_actor.InterruptCast(false);
    a_actor.StopInteractingQuick(true);

    //a_actor.reset(RE::Actor::BOOL_FLAGS::kShouldAnimGraphUpdate);

    /*if (const auto charController = a_actor.GetCharController(); charController) {
        charController->flags.set(RE::CHARACTER_FLAGS::kNotPushable);
        charController->flags.set(RE::CHARACTER_FLAGS::kNoCharacterCollisions);

        charController->flags.reset(RE::CHARACTER_FLAGS::kRecordHits);
        charController->flags.reset(RE::CHARACTER_FLAGS::kHitFlags);
    }*/

    //a_actor.EnableAI(false);
    //a_actor.StopMoving(1.0f);
}

bool ParkourUtility::ToggleControlsForParkour(bool enable) {
    auto player = RE::PlayerCharacter::GetSingleton();
    auto playerCamera = RE::PlayerCamera::GetSingleton();
    auto controller = player->GetCharController();

    // Gravity is normally 1, 0 it to prevent falling (BAD SOLUTION, CAN STILL FLING PLAYER ON COLLISIONS)
    controller->gravity = enable;
    /* Reset Fall Damage */
    controller->fallStartHeight = player->GetPositionZ();

    // Toggle common controls
    auto controlMap = RE::ControlMap::GetSingleton();
    //controlMap->ToggleControls(RE::ControlMap::UEFlag::kMainFour, enable);  // Player tab menu
    controlMap->ToggleControls(RE::ControlMap::UEFlag::kActivate, enable);
    controlMap->ToggleControls(RE::ControlMap::UEFlag::kPOVSwitch, enable);
    controlMap->ToggleControls(RE::ControlMap::UEFlag::kWheelZoom, enable);
    controlMap->ToggleControls(RE::ControlMap::UEFlag::kJumping, enable);

    // POV switch is blocked due to kPOVSwitch being disabled, but doesn't cancel ongoing switch (camera sliding into FPS)
    if (playerCamera && !enable && playerCamera->IsInThirdPerson()) {
        RE::ThirdPersonState *thirdPersonState = nullptr;
        thirdPersonState = skyrim_cast<RE::ThirdPersonState *>(playerCamera->currentState.get());
        thirdPersonState->targetZoomOffset = thirdPersonState->savedZoomOffset = thirdPersonState->currentZoomOffset;
        thirdPersonState->stateNotActive = false;  // Manually setting targetZoomOffset sets this true, reset it
    }

    // TDM swim pitch workaround. Player goes into object if presses the sneak key.
    // If disable and swimming, toggle sneak off. If enable, toggle sneak on. Otherwise don't disable sneaking.
    if (Compatibility::TrueDirectionalMovement == true) {
        if (enable || player->AsActorState()->IsSwimming()) {
            // Pitch changes cause incorrect angles
            player->GetCharController()->pitchAngle = 0;
        }
    }
    else {
        // Block camera movement for Vanilla Skyrim, changes direction mid parkour otherwise. Even Starfield ledge grab does this.
        controlMap->ToggleControls(RE::ControlMap::UEFlag::kLooking, enable);
    }

    return true;
}

RE::NiPoint3 ParkourUtility::GetPlayerDirFlat(RE::Actor *player) {
    // Calculate player forward direction (normalized)
    const float playerYaw = player->data.angle.z;  // Player's yaw

    RE::NiPoint3 playerDirFlat{std::sin(playerYaw), std::cos(playerYaw), 0};
    const float dirMagnitude = std::hypot(playerDirFlat.x, playerDirFlat.y);
    playerDirFlat.x /= dirMagnitude;
    playerDirFlat.y /= dirMagnitude;

    return playerDirFlat;
}

void ParkourUtility::UpdateLastObjectHitType(RE::COL_LAYER obj) {
    RuntimeVariables::lastHitObject = obj;
}

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
        UpdateLastObjectHitType(static_cast<RE::COL_LAYER>(layerIndex));

        if (layerIndex == 0) {
            return -1.0f;  // Invalid layer hit
        }

        // Optionally log the layer hit
        // if (logLayer) logger::info("\nLayer hit: {}", layerIndex);

        // Check for useful collision layers
        switch (RuntimeVariables::lastHitObject) {
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

bool ParkourUtility::IsPlayerAlreadyAnimationDriven(RE::PlayerCharacter *player) {
    bool out;
    return player->GetGraphVariableBool("bAnimationDriven", out) && out;
}

bool ParkourUtility::IsPlayerUsingFurniture(RE::PlayerCharacter *player) {
    auto ref = player->GetOccupiedFurniture();
    if (ref) {
        return true;
    }
    return false;
}

bool ParkourUtility::IsPlayerInCharGen(RE::PlayerCharacter *player) {
    // Check if player has chargen flag hands bound
    const auto &gs = player->GetGameStatsData();
    if (gs.byCharGenFlag.any(RE::PlayerCharacter::ByCharGenFlag::kHandsBound)) {
        //logger::info(">> Chargen: {}", gs.byCharGenFlag.underlying());
        return true;
    }
    return false;
}

bool ParkourUtility::IsBeastForm() {
    return RE::MenuControls::GetSingleton()->InBeastForm();
}

bool ParkourUtility::IsOnMount() {
    return RE::PlayerCharacter::GetSingleton()->IsOnMount();
}

bool ParkourUtility::IsPlayerInSyncedAnimation(RE::PlayerCharacter *player) {
    bool out;
    return player->GetGraphVariableBool("bIsSynced", out) && out;
}

float ParkourUtility::CalculateParkourStamina() {
    const auto player = RE::PlayerCharacter::GetSingleton();
    float equip = player->GetEquippedWeight();
    //float carry = player->GetTotalCarryWeight();

    return ModSettings::Stamina_Damage + (equip * 0.2f);
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
bool ParkourUtility::CheckIsVaultActionFromType(int32_t ledge) {
    return ledge == ParkourType::Vault || ledge == ParkourType::Grab || ledge == ParkourType::StepHigh || ledge == ParkourType::StepLow ||
           ledge == ParkourType::Low || ledge == ParkourType::Medium;
}

bool ParkourUtility::PlayerIsGroundedOrSliding() {
    const auto player = RE::PlayerCharacter::GetSingleton();
    const auto charController = player->GetCharController();

    // logger::info("Flag {}", charController->flags.underlying());
    // Check if the player is in the air (jumping flag)
    if (player && charController && /*!charController->flags.any(RE::CHARACTER_FLAGS::kJumping) &&
        charController->flags.all(RE::CHARACTER_FLAGS::kCanJump) &&*/
        charController->surfaceInfo.supportedState != RE::hkpSurfaceInfo::SupportedState::kUnsupported) {
        return true;
    }
    return false;
}

bool ParkourUtility::PlayerIsMidairAndNotSliding() {
    const auto player = RE::PlayerCharacter::GetSingleton();
    const auto charController = player->GetCharController();

    if (player && charController && charController->surfaceInfo.supportedState == RE::hkpSurfaceInfo::SupportedState::kUnsupported) {
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

bool ParkourUtility::PlayerWantsToDrawSheath() {
    const auto player = RE::PlayerCharacter::GetSingleton();
    return player->AsActorState()->GetWeaponState() == RE::WEAPON_STATE::kWantToDraw ||
           player->AsActorState()->GetWeaponState() == RE::WEAPON_STATE::kWantToSheathe;
}

bool ParkourUtility::PlayerIsOnStairs() {
    const auto player = RE::PlayerCharacter::GetSingleton();
    if (!player) {
        return false;  // Early exit if the player is null
    }

    const auto charController = player->GetCharController();
    return charController && charController->flags.any(RE::CHARACTER_FLAGS::kOnStairs) /*&&
            PluginReferences::lastHitObject == RE::COL_LAYER::kStairHelper*/
        ;
}

float ParkourUtility::magnitudeXY(float x, float y) {
    return sqrt(x * x + y * y);
}

//void ParkourUtility::MoveMarkerToLedge(RE::TESObjectREFR *ledgeMarker, RE::NiPoint3 ledgePoint, RE::NiPoint3 backwardAdjustment,
//                                       float zAdjust) {
//    // Position ledge marker with adjustments
//    ledgeMarker->SetPosition({ledgePoint.x - backwardAdjustment.x, ledgePoint.y - backwardAdjustment.y, ledgePoint.z + zAdjust});
//}

//void ParkourUtility::RotateLedgeMarker(RE::TESObjectREFR *ledgeMarker, RE::NiPoint3 playerDirFlat) {
//    ledgeMarker->data.angle = RE::NiPoint3(0, 0, atan2(playerDirFlat.x, playerDirFlat.y));
//}