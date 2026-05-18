#include "Util/ParkourUtility.h"
#include "Util/HavokUtil.hpp"
#include "_References/ModSettings.h"
#include "_References/RuntimeVariables.h"
#include "_References/ParkourType.h"
#include "_References/HardcodedVariables.h"
#include "API/API_Handles.h"
#include "HUD/Scaleform/SkyParkourMenu.hpp"
#include "_References/CustomBlockingVars.h"
#include "_References/Compatibility.h"

bool ParkourUtility::IsParkourActiveFor(RE::Actor *actor) {
    if (actor->IsPlayerRef()) {
        if (RuntimeVariables::IsMenuOpen) return false;
        if (RuntimeVariables::selectedLedgeType == ParkourType::NoLedge) return false;
        if (IsChargenHandsBound(static_cast<RE::PlayerCharacter *>(actor))) return false;
        if (IsBeastForm()) return false;
    }

    if (IsKnockedOut(actor)) return false;
    if (actor->IsAnimationDriven()) return false;
    if (actor->IsStaggering()) return false;
    if (IsInSyncedAnimation(actor)) return false;
    if (IsSitting(actor)) return false;
    if (IsInDrawSheath(actor)) return false;
    if (IsAttacking(actor)) return false;
    if (IsCrouchSliding(actor)) return false;
    if (HasCustomBlock(actor, false)) return false;

    if (!CamLedgeAngleValid()) return false;  // No TDM only
    /* TODO: Find a better way for this */
    // /* Invalid if activate key selected & crosshair prompt available */
    // if (ModSettings::Use_Preset_Parkour_Key && ModSettings::Preset_Parkour_Key == PARKOUR_PRESET_KEYS::kActivate) {
    //     if (IsCrosshairRefActivator()) {
    //         return false;
    //     }
    // }

    return true;
}

bool ParkourUtility::ClimbExtraChecks(RE::NiPoint3 start, const float check_height, RE::NiPoint3 fwdDir) {
    constexpr RE::NiPoint3 upDir{0, 0, 1};

    const float back_length = 40.f * RuntimeVariables::PlayerScale;
    const auto backStart = start - fwdDir * 20.f;

    RayCastResult headRoomRay_BackOffset = HavokUtil::RayCast(backStart, upDir, check_height, COL_LAYER_EXTEND::kClimbObstruction);
    RayCastResult headRoomRay_bwd = HavokUtil::RayCast(backStart, -fwdDir, back_length, COL_LAYER_EXTEND::kClimbObstruction);

    /* DEBUG LINES */
    if (ModSettings::_Debug_Enabled) {
        const auto TH = API_Handles::TrueHUD::Get();
        if (TH) {
            TH->DrawArrow(backStart, backStart + upDir * headRoomRay_BackOffset.distance, 10.f, 0.f,
                          headRoomRay_BackOffset.didHit ? COLOR_HEX_R : COLOR_HEX_G, 1.f);
            TH->DrawArrow(backStart, backStart - fwdDir * headRoomRay_bwd.distance, 10.f, 0.f,
                          headRoomRay_bwd.didHit ? COLOR_HEX_R : COLOR_HEX_G, 1.f);
        }
    }
    /*********************************/

    if (headRoomRay_BackOffset.didHit || headRoomRay_bwd.didHit) {
        return false;
    }
    return true;
}

bool ParkourUtility::SmartClimbCheck(RE::Actor *actor) {
    const auto st = actor->AsActorState();

    if (!ModSettings::Smart_Climb) return true;  // Feature disabled, always allow
    if (!actor->IsMoving()) return true;         // Not inputting move, allow
    if (st->IsSwimming()) return true;           // Swimming, allow

    /* 3.5.0 Smart Climb Rework */
    const auto relativeVel = GetRelativeVelocityToMT(actor);
    if (relativeVel > 0.2f) return false;

    return true;
}

bool ParkourUtility::StepsExtraChecks(RE::Actor *actor, const float ledgePlayerDiff, const RE::NiPoint3 ledgePoint) {
    const auto st = actor->AsActorState();
    if (st->actorState1.movingBack) return false;
    /* 3.5.0 Get a multiplier from normalized fwd velocity. Use it to scale the min ledge height dynamically */

    /* Height Thresholding Logic */
    const float mult = [&] {
        if (actor->IsMoving())
            return GetRelativeVelocityToMT(actor);
        else if (!ModSettings::Smart_Steps)
            return 0.8f;
        return 0.f;
    }();

    constexpr float baseHeight = HardCodedVariables::lowLedgeLimit - HardCodedVariables::climbMinHeight;
    const auto distToAdd = mult * baseHeight;
    const auto calcedTH = baseHeight + distToAdd;

    // DEBUG_PRINT("Ledge Diff {} / Threshold {}", ledgePlayerDiff, calcedTH);
    if (ledgePlayerDiff <= calcedTH) return false;

    const bool closeEnough = [&] {
        auto dist3 = ledgePoint - actor->GetPosition();
        dist3.z = 0;
        const auto hrzDiff = dist3.Length();
        // const auto TH = ModSettings::Smart_Steps ? 70 : 80;
        if (hrzDiff > 70) return false;
        return true;
    }();
    if (!closeEnough) return false;

    return true;
}

bool ParkourUtility::VaultExtraChecks(RE::Actor *actor) {
    if (actor->IsInMidair()) return false;
    if (actor->AsActorState()->actorState1.movingBack) return false;

    if (!ModSettings::Smart_Vault) return true;  // Feature disabled, always allow

    /* 3.2.0 Reverted the sprint only vault feature */
    return actor->IsMoving();  // Feature enabled, allow only when moving
}

bool ParkourUtility::GrabExtraChecks(RE::Actor *actor, const float ledgePlayerDiff, bool &out_grabHighVariant,
                                     const RE::NiPoint3 ledgePoint) {
    if (actor->AsActorState()->actorState1.movingBack) return false;

    // Avoid grabbing ground
    constexpr float dist{35.f};
    constexpr RE::NiPoint3 dir(0, 0, -1);
    RayCastResult downRay = HavokUtil::RayCast(actor->GetPosition(), dir, dist, COL_LAYER_EXTEND::kClimbObstruction, actor);

    if (downRay.didHit) return false;

    const bool closeEnough = [&] {
        auto dist3 = ledgePoint - actor->GetPosition();
        dist3.z = 0;
        const auto hrzDiff = dist3.Length();
        if (hrzDiff > 70) return false;
        return true;
    }();
    if (!closeEnough) return false;

    // Check Start lower point is player feet level + lowest parkour height, which is positive
    if (ledgePlayerDiff > HardCodedVariables::grabMaxHeight * RuntimeVariables::PlayerScale) {
        return false;
    }

    if (ledgePlayerDiff > HardCodedVariables::grabHighVariantThreshold * RuntimeVariables::PlayerScale) {
        out_grabHighVariant = true;
    }

    return true;
}

void ParkourUtility::StopInteractions(RE::Actor &a_actor) {
    a_actor.StopCurrentDialogue();
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

    const auto ctrl = actor->GetCharController();
    return VEC4_TO_VEC3(ctrl->forwardVec * -1);  // * -1 cause it returns the inverse vector pointing backwards?
}

bool ParkourUtility::IsKnockedOut(RE::Actor *actor) {
    return actor->AsActorState()->GetKnockState() != RE::KNOCK_STATE_ENUM::kNormal;
}

bool ParkourUtility::IsSitting(RE::Actor *actor) {
    return actor->AsActorState()->GetSitSleepState() != RE::SIT_SLEEP_STATE::kNormal;
}

bool ParkourUtility::IsCrosshairRefActivator() {
    //auto ref = RE::CrosshairPickData::GetSingleton()->grabPickRef.get();
    const auto ref = RE::CrosshairPickData::GetSingleton()->target.get();
    if (ref) {
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

bool ParkourUtility::IsBeastForm(RE::PlayerCharacter *pl) {
    bool menuLock = RE::MenuControls::GetSingleton()->InBeastForm();
    if (menuLock) return true;

    const auto &runtime = pl->GetPlayerRuntimeData();
    return runtime.preTransformationData;
}

bool ParkourUtility::IsOnMount() {
    return GET_PLAYER->IsOnMount();
}

bool ParkourUtility::IsGamePaused() {
    const auto ui = RE::UI::GetSingleton();
    return ui && ui->GameIsPaused();
}

bool ParkourUtility::IsInSyncedAnimation(RE::Actor *actor) {
    bool out;
    return actor->GetGraphVariableBool("bIsSynced", out) && out;
}

float ParkourUtility::CalculateStaminaReqFromEquipLoad(RE::Actor *actor) {
    if (actor->IsPlayerRef()) {
        const RE::PlayerCharacter *pl = actor->As<RE::PlayerCharacter>();
        if (pl->IsGodMode()) return -1.f;
    }

    const float &equip = actor->GetEquippedWeight();
    //float carry = player->GetTotalCarryWeight();

    return ModSettings::Stamina_Damage + (equip * 0.2f);
}

bool ParkourUtility::ActorHasEnoughStamina(RE::Actor *actor) {
    const auto currentStamina = actor->AsActorValueOwner()->GetActorValue(RE::ActorValue::kStamina);

    if (!ModSettings::Must_Have_Stamina || currentStamina > CalculateStaminaReqFromEquipLoad(actor)) {
        return true;
    }
    return false;
}

bool ParkourUtility::DamageActorStamina(RE::Actor *actor, float amount) {
    if (actor && amount > 0) {
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
bool ParkourUtility::CheckActionRequiresLowEffort(ParkourType ledge) {
    switch (ledge) {
        case ParkourType::High:
        case ParkourType::Highest:
        case ParkourType::Failed:
        case ParkourType::NoLedge:
            return false;

        default:
            return true;
    }
}

bool ParkourUtility::PlayerIsSwimming() {
    const auto player = GET_PLAYER;
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

float ParkourUtility::GetCharForwardVelocity(RE::Actor *act) {
    RE::NiPoint3 vel;
    act->GetLinearVelocity(vel);
    vel.z = 0.0f;

    return vel.Length();
}

float ParkourUtility::GetRelativeVelocityToMT(RE::Actor *actor) {
    const float &vel = GetCharForwardVelocity(actor);
    const float &mt_speed = actor->AsActorState()->DoGetMovementSpeed();

    return vel / (mt_speed <= 0 ? 1 : mt_speed);
}

bool ParkourUtility::HasCustomBlock(RE::Actor *act, bool isSlideList) {
    if (!act) return false;
    namespace vars = CustomBlockingVars;

    for (auto &&i: isSlideList ? vars::SlideList : vars::ParkourList) {
        bool out{false};
        act->GetGraphVariableBool(i, out);

        if (out) return true;
    }
    return false;
}

bool ParkourUtility::CamLedgeAngleValid() {
    if (Compatibility::TrueDirectionalMovement::found) return true;

    auto cam = RE::PlayerCamera::GetSingleton();

    if (!cam || !cam->IsInThirdPerson()) return true;

    auto tpp = static_cast<RE::ThirdPersonState *>(cam->currentState.get());
    if (!tpp) return true;

    auto camRot = tpp->targetYaw;

    auto pl = GET_PLAYER;
    if (!pl) return true;

    auto plRot = pl->data.angle.z;

    constexpr float th{0.4f};
    float diff = camRot - plRot;
    if (diff > th || diff < -th) return false;

    return true;
}