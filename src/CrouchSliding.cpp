#include "CrouchSliding.h"
#include "Listeners/ButtonListener.h"
#include "_References/ModSettings.h"
#include "_References/RuntimeVariables.h"
#include "_References/RuntimeMethods.h"
#include "Util/ParkourUtility.h"
#include "Util/HavokUtil.hpp"

#include "API/API_Handles.h"
#include "_References/Compatibility.h"

namespace CrouchSliding {

    // Seems to fail if the initiator is player
    // int32_t StartCombat(RE::TESObjectREFR *a_initiator, RE::TESObjectREFR *a_target) {
    void StartCombat(RE::TESObjectREFR *a_initiator, RE::TESObjectREFR *a_target) {
        using func_t = void (*)(RE::TaskQueueInterface *, RE::TESObjectREFR *, RE::TESObjectREFR *);
        REL::Relocation<func_t> func{RELOCATION_ID(35984, 36959)};

        const auto taskPool = RE::TaskQueueInterface::GetSingleton();
        return func(taskPool, a_initiator, a_target);
    }

    bool TrySprintSlide(bool isHoldingKey) {
        const auto pl = GET_PLAYER;
        bool out_isRoll{false};

        if (IsSlideActiveFor(pl, out_isRoll, isHoldingKey)) {
            pl->SetGraphVariableBool(SPPF_SLIDE_IS_ROLL, out_isRoll);
            bool res = pl->NotifyAnimationGraph(SPPF_NOTIFY_SLIDE);
            if (res) {
                RuntimeVariables::SlideOngoing = true;
                const auto ctrlMap = RE::ControlMap::GetSingleton();
                if (ctrlMap) ctrlMap->ToggleControls(RE::ControlMap::UEFlag::kMainFour, false, true);
            }
            return res;
        }

        return false;
    }

    bool IsSlideActiveFor(RE::Actor *actor, bool &out_isRoll, bool isHoldingKey) {
        if (actor->IsPlayerRef()) {
            if (!ModSettings::Crouch_Slide_Enabled) return false;
            if (RuntimeVariables::ParkourInProgress) return false;
            if (RuntimeVariables::IsMenuOpen) return false;

            if (ParkourUtility::IsChargenHandsBound(static_cast<RE::PlayerCharacter *>(actor))) return false;
        }

        if (ParkourUtility::IsKnockedOut(actor)) return false;
        if (actor->IsAnimationDriven()) return false;
        if (actor->IsStaggering()) return false;
        if (ParkourUtility::IsInSyncedAnimation(actor)) return false;
        if (ParkourUtility::IsBeastForm()) return false;
        if (ParkourUtility::IsSitting(actor)) return false;
        if (ParkourUtility::IsInDrawSheath(actor)) return false;
        if (ParkourUtility::IsAttacking(actor)) return false;
        if (ParkourUtility::IsCrouchSliding(actor)) return false;

        const auto state = actor->AsActorState();
        if (state && state->IsSwimming()) return false;

        const auto ctrl = actor->GetCharController();

        bool sneaking = state->actorState1.sneaking;
        bool sprinting = state->IsSprinting();
        bool midair = actor->IsInMidair();
        float fallTime = ctrl->fallTime;
        // float fellDist = ctrl->fallStartHeight - actor->GetPositionZ();

        if (midair && fallTime >= 0.5f) {
            const auto downRay = [actor] {
                constexpr RE::NiPoint3 downDir{0, 0, -1};
                constexpr float dist = 100.f;
                RE::NiPoint3 startPos{actor->GetPosition()};

                const auto ray = HavokUtil::RayCast(startPos, downDir, dist, COL_LAYER_EXTEND::kCrouchSlideDistCheck, actor);

                /* DEBUG LINES */
                if (ModSettings::_Debug_Enabled) {
                    const auto TH = API_Handles::TrueHUD::Get();
                    if (TH) {
                        TH->DrawArrow(startPos, startPos + downDir * ray.distance, 10.f, 2.f, ray.didHit ? COLOR_HEX_G : COLOR_HEX_R, 1.f);
                    }
                }
                /***********************************/
                return ray;
            }();

            if (downRay.didHit) {
                /* Stamina logic for landing roll, which reduces the fall damage by decreasing the jump start height by 100 units */
                if (ModSettings::Enable_Stamina_Consumption && !ParkourUtility::ActorHasEnoughStamina(actor)) {
                    RE::HUDMenu::FlashMeter(RE::ActorValue::kStamina);
                    actor->UpdateRegenDelay(RE::ActorValue::kStamina, 2.0f);

                    return false;
                }

                out_isRoll = true;
                return true;
            }
            return false;
        }
        else {
            if (isHoldingKey) return false;
            if (fallTime > 0.2f) return false;
            if (!sprinting) return false;
            if (sneaking) return false;
        }

        return true;
    }

    void SetSlideOnOff(bool turnOn) {
        if (turnOn) {
            if (!Buttons::SlideListener::GetSingleton()->SinkRegistered) {
                Buttons::SlideListener::Register();
                LOG("Slide & Roll: < ON >");
            }
        }
        else {
            if (Buttons::SlideListener::GetSingleton()->SinkRegistered) {
                Buttons::SlideListener::Unregister();
                LOG("Slide & Roll: < Off >");
            }

            RuntimeMethods::ResetSlide();
        }
    }

    void OnStartStop(bool isStop, RE::Actor *actor, bool isRoll) {
        if (isStop) {
            if (RuntimeVariables::SlideOngoing) RuntimeVariables::SlideOngoing = false;
            // actor->GetCharController()->flags.reset(RE::CHARACTER_FLAGS::kNoFriction);
            /* Other POV bugging out shit again, figures why bethesda stopped running both graphs FO4 and onwards */
            actor->NotifyAnimationGraph(SPPF_SLIDE_STOP);

            if (Compatibility::TrueDirectionalMovement::found) {
                API_Handles::TDM::LockYaw(false);
            }

            if (!isRoll) {
                if (actor->IsPlayerRef()) {
                    const auto AS = actor->AsActorState();
                    if (AS && AS->IsSneaking()) {
                        GET_PLAYER->GetPlayerRuntimeData().playerFlags.isSprinting = false;
                        AS->actorState1.sprinting = false;
                        actor->NotifyAnimationGraph("SprintStop");
                    }
                    else {
                        if (!Compatibility::ClassicSprintingRedone::found)
                            GET_PLAYER->GetPlayerRuntimeData().playerFlags.isSprinting = true;
                    }
                }
            }
        }
        else {
            // actor->GetCharController()->flags.set(RE::CHARACTER_FLAGS::kNoFriction);
            if (!actor->IsInMidair()) {
                bool isTDM = Compatibility::TrueDirectionalMovement::found;
                if (!(isTDM && API_Handles::TDM::IsLockedOn())) {
                    actor->SetGraphVariableInt("iIsInSneak", true);
                    actor->AsActorState()->actorState1.sneaking = true;
                }
            }

            if (Compatibility::TrueDirectionalMovement::found) {
                API_Handles::TDM::LockYaw(true);
            }
        }

        if (actor->IsPlayerRef()) {
            const auto ctrlMap = RE::ControlMap::GetSingleton();
            if (ctrlMap)
                ctrlMap->ToggleControls(RE::ControlMap::UEFlag::kMainFour, isStop,
                                        true);  // Player tab menu & equip. Gets stuck if player uses TFC.
        }
    }

    bool TryKnockCollidedActor(RE::Actor *pl) {
        const auto bumped = pl->GetCharController()->bumpedCharCollisionObject;
        if (!bumped) return false;

        const auto ref = RE::TESHavokUtilities::FindCollidableRef(bumped.get()->collidable);
        if (!ref) return false;

        if (!ref->IsActor()) return false;
        const auto act = ref->As<RE::Actor>();
        pl->GetActorRuntimeData().currentProcess->KnockExplosion(act, pl->GetPosition(), 5.f);
        RE::PlaySound("PHYBodyMediumDirtH");

        // act->AsActorValueOwner()->DamageActorValue(RE::ActorValue::kHealth, 5.f);
        act->DoDamage(5.f, pl, true);
        StartCombat(act, pl);

        return true;
    }
}  // namespace CrouchSliding