#include "Util/ParkourUtility.h"
#include "References.h"
#include "Util/ScaleUtility.h"

bool ParkourUtility::IsParkourActive() {
    if (RuntimeVariables::selectedLedgeType == ParkourType::NoLedge) {
        return false;
    }

    const auto player = RE::PlayerCharacter::GetSingleton();
    if (IsChargenHandsBound(player)) {
        //logger::info("PLAYER HANDS BOUND");
        return false;
    }

    if (IsKnockedOut(player)) {
        return false;
    }

    if (IsPlayerAlreadyAnimationDriven(player)) {
        return false;
    }

    if (IsPlayerInSyncedAnimation(player)) {
        return false;
    }

    if (IsBeastForm()) {
        return false;
    }

    if (IsSitting(player)) {
        return false;
    }

    if (PlayerWantsToDrawSheath()) {
        //logger::info("WILL DRAW/SHEATHE");
        return false;
    }

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
    a_actor.StopMoving(0.0f);
}

bool ParkourUtility::ToggleControls(bool enable) {
    auto player = RE::PlayerCharacter::GetSingleton();
    auto controller = player->GetCharController();
    /* Reset Fall Damage */
    controller->fallStartHeight = player->GetPositionZ();
    /* Set gravity on off */
    controller->gravity = enable;

    //controller->context.currentState = enable ? RE::hkpCharacterStateType::kInAir : RE::hkpCharacterStateType::kClimbing; /* Crashes modded setups, not needed */
    //controller->wantState = enable ? RE::hkpCharacterStateType::kInAir : RE::hkpCharacterStateType::kClimbing;

    // Toggle common controls
    //auto ctrlMap = RE::ControlMap::GetSingleton();
    //ctrlMap->ToggleControls(RE::ControlMap::UEFlag::kMainFour, enable);  // Player tab menu

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

RayCastResult ParkourUtility::RayCast(RE::NiPoint3 rayStart, RE::NiPoint3 rayDir, float maxDist, COL_LAYER_EXTEND layerMask) {
    const auto player = RE::PlayerCharacter::GetSingleton();

    RayCastResult result{};
    result.distance = maxDist;

    if (!player) {
        return result;
    }
    const auto cell = player->GetParentCell();
    if (!cell) {
        return result;
    }
    const auto bhkWorld = cell->GetbhkWorld();
    if (!bhkWorld) {
        return result;
    }

    RE::bhkPickData pickData;
    const auto havokWorldScale = RE::bhkWorld::GetWorldScale();

    // Set ray start and end points (scaled to Havok world)
    pickData.rayInput.from = rayStart * havokWorldScale;
    pickData.rayInput.to = (rayStart + rayDir * maxDist) * havokWorldScale;

    // Set the collision filter info to exclude the player
    /* hkpCollidable.h, lower 4 bits: CollidesWith, higher 4 bits: BelongsTo */

    //static_cast<uint32_t>(COL_LAYER::kAnimStatic) & ~static_cast<uint32_t>(COL_LAYER::kDoorDetection)

    /* TODO: Bitwise or layers, keep them as uint32_t as a custom mask for the layers valid */
    uint32_t collisionFilterInfo = 0;
    player->GetCollisionFilterInfo(collisionFilterInfo);
    pickData.rayInput.filterInfo = (collisionFilterInfo & 0xFFFF0000) | static_cast<uint32_t>(layerMask);

    // Perform the raycast
    if (bhkWorld->PickObject(pickData) && pickData.rayOutput.HasHit()) {
        result.didHit = true;
        result.distance = maxDist * pickData.rayOutput.hitFraction;
        result.normalOut = pickData.rayOutput.normal;

        const RE::COL_LAYER layer =
            static_cast<RE::COL_LAYER>(pickData.rayOutput.rootCollidable->broadPhaseHandle.collisionFilterInfo & 0x7F);

        result.layer = layer;
    }

    return result;
}

bool ParkourUtility::IsKnockedOut(RE::Actor *actor) {
    return actor->AsActorState()->GetKnockState() != RE::KNOCK_STATE_ENUM::kNormal;
}

bool ParkourUtility::IsPlayerAlreadyAnimationDriven(RE::PlayerCharacter *player) {
    bool out;
    return player->GetGraphVariableBool("bAnimationDriven", out) && out;
}

bool ParkourUtility::IsSitting(RE::PlayerCharacter *player) {
    return player->AsActorState()->GetSitSleepState() != RE::SIT_SLEEP_STATE::kNormal;
}

bool ParkourUtility::IsChargenHandsBound(RE::PlayerCharacter *player) {
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

bool ParkourUtility::IsGamePaused() {
    auto ui = RE::UI::GetSingleton();
    return ui && ui->GameIsPaused();
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

// Return true if action should consume half the stamina cost
bool ParkourUtility::CheckActionRequiresLowEffort(int32_t ledge) {
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
    return player->AsActorState()->GetWeaponState() != RE::WEAPON_STATE::kDrawn &&
           player->AsActorState()->GetWeaponState() != RE::WEAPON_STATE::kSheathed;
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