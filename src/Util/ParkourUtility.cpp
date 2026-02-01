#include "Util/ParkourUtility.h"
#include "_References/ModSettings.h"
#include "_References/RuntimeVariables.h"
#include "_References/ParkourType.h"
#include "_References/HardcodedVariables.h"
#include "API/API_Handles.h"

bool ParkourUtility::IsParkourActiveFor(RE::Actor *actor) {
    if (actor->IsPlayerRef()) {
        if (RuntimeVariables::IsMenuOpen) return false;

        if (RuntimeVariables::selectedLedgeType == ParkourType::NoLedge) return false;

        if (IsChargenHandsBound(static_cast<RE::PlayerCharacter *>(actor))) return false;
    }

    if (IsKnockedOut(actor)) return false;

    if (actor->IsAnimationDriven()) return false;

    if (actor->IsStaggering()) return false;

    if (IsInSyncedAnimation(actor)) return false;

    if (IsBeastForm()) return false;

    if (IsSitting(actor)) return false;

    if (IsInDrawSheath(actor)) return false;

    if (IsAttacking(actor)) return false;

    if (IsCrouchSliding(actor)) return false;

    /* TODO: Find a better way for this */
    // /* Invalid if activate key selected & crosshair prompt available */
    // if (ModSettings::Use_Preset_Parkour_Key && ModSettings::Preset_Parkour_Key == PARKOUR_PRESET_KEYS::kActivate) {
    //     if (IsCrosshairRefActivator()) {
    //         return false;
    //     }
    // }

    return true;
}

bool ParkourUtility::ClimbExtraChecks(RE::NiPoint3 start, const float check_height) {
    constexpr RE::NiPoint3 upDir{0, 0, 1};
    const auto &fwdDir = RuntimeVariables::playerDirFlat;

    const float back_length = 40.f * RuntimeVariables::PlayerScale;
    const auto backStart = start - fwdDir * 20.f;

    RayCastResult headRoomRay_BackOffset = RayCast(backStart, upDir, check_height, COL_LAYER_EXTEND::kClimbObstruction);
    RayCastResult headRoomRay_bwd = RayCast(backStart, -fwdDir, back_length, COL_LAYER_EXTEND::kClimbObstruction);

    /* DEBUG LINES */
    if (ModSettings::_Debug_Draw_Lines) {
        const auto &TH = API_Handles::TrueHUD::Get();
        if (TH) {
            TH->DrawArrow(backStart, backStart + upDir * headRoomRay_BackOffset.distance, 10.f, 0.f,
                          headRoomRay_BackOffset.didHit ? 0xFF0000FF : 0x00FF00FF, 1.f);
            TH->DrawArrow(backStart, backStart - fwdDir * headRoomRay_bwd.distance, 10.f, 0.f,
                          headRoomRay_bwd.didHit ? 0xFF0000FF : 0x00FF00FF, 1.f);
        }
    }
    /*********************************/

    if (headRoomRay_BackOffset.didHit || headRoomRay_bwd.didHit) {
        return false;
    }
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

    if (speed < -1.0f) { /* Standing still has precision errors */
        return false;
    }

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

    if (!ModSettings::Smart_Steps) return true;  // Feature disabled, always allow

    return isMoving;  // Feature enabled, only allow if moving
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
            // if (z < 0.65f) {
            //     return false;
            // }
            /* Update 3.3.0 - Don't step onto ground at all*/
            return false;
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

    /* 3.2.0 Reverted the sprint only vault feature */
    return actor->IsMoving();  // Feature enabled, allow only when moving
}

bool ParkourUtility::GrabExtraChecks(const float ledgePlayerDiff, const RayCastResult ray, bool &isGrabFromBelow) {
    // Avoid grabbing ground
    if (ray.layer == RE::COL_LAYER::kGround) {
        return false;
    }

    // Check Start lower point is player feet level + lowest parkour height, which is positive

    if (ledgePlayerDiff > HardCodedVariables::grabMaxHeight * RuntimeVariables::PlayerScale) {
        return false;
    }

    if (ledgePlayerDiff > HardCodedVariables::grabHighVariantThreshold * RuntimeVariables::PlayerScale) {
        isGrabFromBelow = true;
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

RE::NiPoint3 ParkourUtility::GetActorDirFlat(RE::Actor *actor) {
    // Calculate player forward direction (normalized)
    // const float actorYaw = actor->data.angle.z;  // Player's yaw

    // RE::NiPoint3 actorDirFlat{std::sin(actorYaw), std::cos(actorYaw), 0};
    // const float dirMagnitude = std::hypot(actorDirFlat.x, actorDirFlat.y);
    // if (dirMagnitude == 0) {
    //     actorDirFlat.x = actorDirFlat.y = 0;
    // }
    // else {
    //     actorDirFlat.x /= dirMagnitude;
    //     actorDirFlat.y /= dirMagnitude;
    // }

    // return actorDirFlat;
    const auto &ctrl = actor->GetCharController();
    if (!ctrl) return RuntimeVariables::playerDirFlat;

    return VEC4_TO_VEC3(ctrl->forwardVec * -1);  // * -1 cause it returns the inverse vector pointing backwards?
}

RayCastResult ParkourUtility::RayCast(RE::NiPoint3 rayStart, RE::NiPoint3 rayDir, float maxDist, COL_LAYER_EXTEND layerMask,
                                      RE::Actor *actor) {
    RayCastResult result{};
    result.distance = maxDist;

    if (!actor) {
        return result;
    }
    const auto &cell = actor->GetParentCell();
    if (!cell) {
        return result;
    }
    const auto &bhkWorld = cell->GetbhkWorld();
    if (!bhkWorld) {
        return result;
    }

    RE::bhkPickData pickData;
    const auto &havokWorldScale = RE::bhkWorld::GetWorldScale();

    // Set ray start and end points (scaled to Havok world)
    pickData.rayInput.from = rayStart * havokWorldScale;
    pickData.rayInput.to = (rayStart + rayDir * maxDist) * havokWorldScale;

    // Set the collision filter info to exclude the player
    /* hkpCollidable.h, lower 4 bits: CollidesWith, higher 4 bits: BelongsTo */

    //static_cast<uint32_t>(COL_LAYER::kAnimStatic) & ~static_cast<uint32_t>(COL_LAYER::kDoorDetection)

    RE::CFilter cFilter;
    actor->GetCollisionFilterInfo(cFilter);
    cFilter.SetCollisionLayer(static_cast<RE::COL_LAYER>(layerMask));
    pickData.rayInput.filterInfo = cFilter;
    // static_cast<RE::CFilter>(cFilter.filter | static_cast<uint32_t>(layerMask));

    // Perform the raycast
    if (bhkWorld->PickObject(pickData) && pickData.rayOutput.HasHit()) {
        result.didHit = true;
        result.distance = maxDist * pickData.rayOutput.hitFraction;
        result.normalOut = pickData.rayOutput.normal;

        result.layer = pickData.rayOutput.rootCollidable->GetCollisionLayer();

        result.hitObjectRef = RE::TESHavokUtilities::FindCollidableRef(*pickData.rayOutput.rootCollidable);
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

bool ParkourUtility::ActorHasEnoughStamina(RE::Actor *actor) {
    const auto &currentStamina = actor->AsActorValueOwner()->GetActorValue(RE::ActorValue::kStamina);

    if (!ModSettings::Must_Have_Stamina || currentStamina > CalculateParkourStamina(actor) /* && ModSettings::Is_Stamina_Required */) {
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

bool ParkourUtility::ShouldClimbActionFail(RE::Actor *actor) {
    // If stamina options are on, check if player has enough stamina. If not, play failed anim. If stamina is on but
    // isn't required or player is swimming, just deal stamina damage. Only for high & higher climbing, would get annoying otherwise.
    if (ModSettings::Enable_Stamina_Consumption && !actor->AsActorState()->IsSwimming()) {
        if (ActorHasEnoughStamina(actor) == false) {
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

bool ParkourUtility::IsActorWeaponOut(RE::Actor *actor) {
    return actor->AsActorState()->GetWeaponState() == RE::WEAPON_STATE::kDrawn;
}

bool ParkourUtility::IsInDrawSheath(RE::Actor *actor) {
    bool equipping;
    bool unequipping;

    /* return player->AsActorState()->GetWeaponState() != RE::WEAPON_STATE::kDrawn &&
           player->AsActorState()->GetWeaponState() != RE::WEAPON_STATE::kSheathed;*/
    actor->GetGraphVariableBool("IsEquipping", equipping);
    actor->GetGraphVariableBool("IsUnequipping", unequipping);

    return equipping || unequipping;
}

bool ParkourUtility::IsAttacking(RE::Actor *actor) {
    return actor->AsActorState()->actorState1.meleeAttackState != RE::ATTACK_STATE_ENUM::kNone;
}

bool ParkourUtility::IsCrouchSliding(RE::Actor *actor) {
    bool sliding;
    return actor->GetGraphVariableBool(SPPF_SLIDE_ONGOING, sliding) && sliding;
}