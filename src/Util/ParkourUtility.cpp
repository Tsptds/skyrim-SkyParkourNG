﻿#include "Util/ParkourUtility.h"
#include "_References/ModSettings.h"
#include "_References/RuntimeVariables.h"
#include "_References/ParkourType.h"
#include "_References/HardcodedVariables.h"

bool ParkourUtility::IsParkourActive() {
    if (RuntimeVariables::selectedLedgeType == ParkourType::NoLedge) {
        return false;
    }

    const auto player = GET_PLAYER;
    if (IsChargenHandsBound(player)) {
        //LOG("PLAYER HANDS BOUND");
        return false;
    }

    if (IsKnockedOut(player)) {
        return false;
    }

    if (player->IsAnimationDriven()) {
        return false;
    }

    if (player->IsStaggering()) {
        return false;
    }

    if (IsInSyncedAnimation(player)) {
        return false;
    }

    if (IsBeastForm()) {
        return false;
    }

    if (IsSitting(player)) {
        return false;
    }

    if (IsInDrawSheath(player)) {
        return false;
    }

    if (RuntimeVariables::IsMenuOpen) {
        return false;
    }

    /* TODO: Find a better way for this */
    // /* Invalid if activate key selected & crosshair prompt available */
    // if (ModSettings::Use_Preset_Parkour_Key && ModSettings::Preset_Parkour_Key == PARKOUR_PRESET_KEYS::kActivate) {
    //     if (IsCrosshairRefActivator()) {
    //         return false;
    //     }
    // }

    return true;
}

bool ParkourUtility::StepsExtraChecks(RE::Actor *player, const RayCastResult ray) {
    /* Velocity threshold */
    RE::hkVector4 vel;
    auto ctrl = player->GetCharController();
    ctrl->GetLinearVelocityImpl(vel);
    auto dir = RuntimeVariables::playerDirFlat;

    auto speed = vel.quad.m128_f32[0] * dir.x + vel.quad.m128_f32[1] * dir.y;

#ifdef LOG_STEPS_VELOCITY
    LOG("{}", speed);
#endif
    /* If player isn't actually moving forward steps are not valid */
    const auto &notStuck = speed > 1;
    if (notStuck) {
        return false;
    }

    const auto &isMoving = player->IsMoving();

    /* If player has just started moving, block premature steps */
    float graphSpeed;
    player->GetGraphVariableFloat("Speed", graphSpeed);

    if (isMoving && graphSpeed < 150) {
        return false;
    }

    bool normalsValid = IsStepNormalValid(ray, isMoving);

    if (!normalsValid) {
        return false;
    }

    if (!ModSettings::Smart_Steps)
        return true;  // Feature disabled, always allow

    return player->IsMoving();  // Feature enabled, only allow if moving
}

bool ParkourUtility::IsStepNormalValid(const RayCastResult ray, bool isMoving) {
    // Actor velocity low, check ledge normals
    const auto &normals = ray.normalOut.quad.m128_f32;

#ifdef LOG_STEPS
    LOG("{}\nStep Normals: {} {} {}", PRINT_LAYER(ray.layer), normals[0], normals[1], normals[2]);
#endif

    // 0, 1, 2 ->x, y, z
    const auto &z = normals[2];
    switch (ray.layer) {
        case RE::COL_LAYER::kTerrain:
            // default normal check 0.5 in ClimbCheck
            break;
        case RE::COL_LAYER::kGround:
            if (z < 0.65f) {
                return false;
            }
            break;
        default:
            // Still inputting move ? normalZ = 0.5 : normalZ = 0.9
            if (!isMoving) {
                if (z < 0.9f) {
                    return false;
                }
            }
    }
    return true;
}

bool ParkourUtility::VaultExtraChecks(RE::Actor *actor) {
    if (!ModSettings::Smart_Vault) {
        return true;  // Feature disabled, always allow
    }

    const auto &state = actor->AsActorState();

    // Sprint state is locked behind a perk, handle it differently
    if (state->IsSneaking()) {
        return actor->IsMoving();  // Allow only if moving
    }

    return state->IsSprinting();  // Feature enabled & not sneaking, only allow if sprinting.
}

bool ParkourUtility::GrabExtraChecks(const float ledgePlayerDiff, const RayCastResult ray) {
    // Avoid grabbing ground
    if (ray.layer == RE::COL_LAYER::kGround) {
        return false;
    }

    // Height checks
    constexpr auto &maxDistToledgeBelow = HardCodedVariables::grabPlayerAboveLedgeMaxDiff;
    constexpr auto &maxDistToledgeAbove = HardCodedVariables::grabPlayerBelowLedgeMaxDiff;

    /* Ledge above is relative to player height, not ledge below. */
    if (ledgePlayerDiff <= maxDistToledgeBelow || ledgePlayerDiff > maxDistToledgeAbove * RuntimeVariables::PlayerScale) {
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

RE::NiPoint3 ParkourUtility::GetPlayerDirFlat(RE::Actor *player) {
    // Calculate player forward direction (normalized)
    const float playerYaw = player->data.angle.z;  // Player's yaw

    RE::NiPoint3 playerDirFlat{std::sin(playerYaw), std::cos(playerYaw), 0};
    const float dirMagnitude = std::hypot(playerDirFlat.x, playerDirFlat.y);
    if (dirMagnitude == 0) {
        playerDirFlat.x = playerDirFlat.y = 0;
    }
    else {
        playerDirFlat.x /= dirMagnitude;
        playerDirFlat.y /= dirMagnitude;
    }

    return playerDirFlat;
}

RayCastResult ParkourUtility::RayCast(RE::NiPoint3 rayStart, RE::NiPoint3 rayDir, float maxDist, COL_LAYER_EXTEND layerMask) {
    const auto player = GET_PLAYER;

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

    RE::CFilter cFilter;
    player->GetCollisionFilterInfo(cFilter);
    cFilter.SetCollisionLayer(static_cast<RE::COL_LAYER>(layerMask));
    pickData.rayInput.filterInfo = cFilter;
    // static_cast<RE::CFilter>(cFilter.filter | static_cast<uint32_t>(layerMask));

    // Perform the raycast
    if (bhkWorld->PickObject(pickData) && pickData.rayOutput.HasHit()) {
        result.didHit = true;
        result.distance = maxDist * pickData.rayOutput.hitFraction;
        result.normalOut = pickData.rayOutput.normal;

        const RE::COL_LAYER layer = static_cast<RE::COL_LAYER>(pickData.rayOutput.rootCollidable->GetCollisionLayer());

        result.layer = layer;
    }

    return result;
}

bool ParkourUtility::IsKnockedOut(RE::Actor *actor) {
    return actor->AsActorState()->GetKnockState() != RE::KNOCK_STATE_ENUM::kNormal;
}

bool ParkourUtility::IsSitting(RE::Actor *actor) {
    return actor->AsActorState()->GetSitSleepState() != RE::SIT_SLEEP_STATE::kNormal;
}

bool ParkourUtility::IsCrosshairRefActivator() {
    //auto ref = RE::CrosshairPickData::GetSingleton()->grabPickRef.get();
    const auto &ref = RE::CrosshairPickData::GetSingleton()->target.get();
    if (ref) {
#ifdef LOG_CROSSHAIR
        auto layer = ref->Get3D()->GetCollisionLayer();
        LOG("Layer: {}", PRINT_LAYER(layer));
#endif

        /* Something activatable in crosshair */
        if (ref->GetFormFlags() & RE::TESObjectREFR::RecordFlags::kHarvested) {
            //LOG("Harvested");
            /* Activator is harvested, don't consider it valid */
            return false;
        }

        return true;
    }
    return false;
}

bool ParkourUtility::IsChargenHandsBound(RE::PlayerCharacter *player) {
    // Check if player has chargen flag hands bound
    const auto &gs = player->GetGameStatsData();
    if (gs.byCharGenFlag.any(RE::PlayerCharacter::ByCharGenFlag::kShowControlsDisabledMessage)) {
        //LOG(">> Chargen: {}", gs.byCharGenFlag.underlying());
        return true;
    }
    return false;
}

bool ParkourUtility::IsBeastForm() {
    return RE::MenuControls::GetSingleton()->InBeastForm();
}

bool ParkourUtility::IsOnMount() {
    return GET_PLAYER->IsOnMount();
}

bool ParkourUtility::IsGamePaused() {
    const auto &ui = RE::UI::GetSingleton();
    return ui && ui->GameIsPaused();
}

bool ParkourUtility::IsInSyncedAnimation(RE::Actor *actor) {
    bool out;
    return actor->GetGraphVariableBool("bIsSynced", out) && out;
}

float ParkourUtility::CalculateParkourStamina(RE::Actor *actor) {
    const float &equip = actor->GetEquippedWeight();
    //float carry = player->GetTotalCarryWeight();

    return ModSettings::Stamina_Damage + (equip * 0.2f);
}

bool ParkourUtility::PlayerHasEnoughStamina() {
    const auto &player = GET_PLAYER;
    const auto &currentStamina = player->AsActorValueOwner()->GetActorValue(RE::ActorValue::kStamina);

    if (!ModSettings::Must_Have_Stamina || currentStamina > CalculateParkourStamina(player) /* && ModSettings::Is_Stamina_Required */) {
        return true;
    }
    return false;
}

bool ParkourUtility::DamageActorStamina(RE::Actor *actor, float amount) {
    if (actor) {
        actor->AsActorValueOwner()->DamageActorValue(RE::ActorValue::kStamina, amount);
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

bool ParkourUtility::IsSupportGroundedOrSliding(RE::Actor *actor) {
    const auto &charController = actor->GetCharController();

    // LOG("Flag {}", charController->flags.underlying());
    // Check if the player is in the air (jumping flag)
    if (actor && charController && /*!charController->flags.any(RE::CHARACTER_FLAGS::kJumping) &&
        charController->flags.all(RE::CHARACTER_FLAGS::kCanJump) &&*/
        charController->surfaceInfo.supportedState != RE::hkpSurfaceInfo::SupportedState::kUnsupported) {
        return true;
    }
    return false;
}

bool ParkourUtility::IsSupportUnsupported(RE::Actor *actor) {
    const auto &charController = actor->GetCharController();

    if (actor && charController && charController->surfaceInfo.supportedState == RE::hkpSurfaceInfo::SupportedState::kUnsupported) {
        return true;
    }
    return false;
}

bool ParkourUtility::IsSupportSliding(RE::Actor *actor) {
    const auto &charController = actor->GetCharController();

    if (actor && charController && charController->surfaceInfo.supportedState == RE::hkpSurfaceInfo::SupportedState::kSliding) {
        return true;
    }
    return false;
}

bool ParkourUtility::IsSupportGrounded(RE::Actor *actor) {
    const auto &charController = actor->GetCharController();

    if (actor && charController && charController->surfaceInfo.supportedState == RE::hkpSurfaceInfo::SupportedState::kSupported) {
        return true;
    }
    return false;
}

bool ParkourUtility::PlayerIsSwimming() {
    const auto &player = GET_PLAYER;
    return player->AsActorState()->IsSwimming();

    // IDK why swim at surface works this way, but it does.
    //return player->boolBits.any(RE::Actor::BOOL_BITS::kSwimming) ||
    //        player->boolBits.any(RE::Actor::BOOL_BITS::kInWater) /*&& !player->GetCharController()->flags.any(RE::CHARACTER_FLAGS::kSwimAtWaterSurface))*/;
}

/*bool IsActorWeaponSheathed(RE::Actor *actor) {
    return actor->GetWeaponState() == RE::WEAPON_STATE::kSheathed;
}*/

bool ParkourUtility::IsInDrawSheath(RE::Actor *actor) {
    bool equipping;
    bool unequipping;

    /* return player->AsActorState()->GetWeaponState() != RE::WEAPON_STATE::kDrawn &&
           player->AsActorState()->GetWeaponState() != RE::WEAPON_STATE::kSheathed;*/
    actor->GetGraphVariableBool("IsEquipping", equipping);
    actor->GetGraphVariableBool("IsUnequipping", unequipping);

    return equipping || unequipping;
}