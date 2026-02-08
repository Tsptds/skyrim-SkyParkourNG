#include "CrouchSliding.h"
#include "Listeners/ButtonListener.h"
#include "_References/ModSettings.h"
#include "_References/RuntimeVariables.h"
#include "_References/RuntimeMethods.h"
#include "Util/ParkourUtility.h"

#include "API/API_Handles.h"

namespace CrouchSliding {
    bool TrySprintSlide() {
        const auto &pl = GET_PLAYER;
        bool out_isRoll{false};
        
        if (IsSlideActiveFor(pl, out_isRoll)) {
            pl->SetGraphVariableBool(SPPF_SLIDE_IS_ROLL, out_isRoll);
            return pl->NotifyAnimationGraph(SPPF_NOTIFY_SLIDE);
        }

        return false;
    }

    bool IsSlideActiveFor(RE::Actor *actor, bool &out_isRoll) {
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

        const auto &state = actor->AsActorState();
        const auto &ctrl = actor->GetCharController();

        bool sneaking = state->actorState1.sneaking;
        bool sprinting = state->IsSprinting();
        bool midair = actor->IsInMidair();
        float fallTime = ctrl->fallTime;
        // float fellDist = ctrl->fallStartHeight - actor->GetPositionZ();

        if (midair && fallTime >= 0.5f) {
            const auto downRay = [actor] {
                constexpr RE::NiPoint3 downDir{0, 0, -1};
                constexpr float dist = 200.f;
                RE::NiPoint3 startPos{actor->GetPosition()};

                const auto &ray = ParkourUtility::RayCast(startPos, downDir, dist, COL_LAYER_EXTEND::kCrouchSlideDistCheck, actor);

                /* DEBUG LINES */
                if (ModSettings::_Debug_Draw_Lines) {
                    const auto &TH = API_Handles::TrueHUD::Get();
                    if (TH) {
                        TH->DrawArrow(startPos, startPos + downDir * ray.distance, 10.f, 2.f, ray.didHit ? 0x00FF00FF : 0xFF0000FF, 1.f);
                    }
                }
                /***********************************/
                return ray;
            }();

            if (downRay.didHit) {
                out_isRoll = true;
                return true;
            }
            return false;
        }
        else {
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
                LOG("Slide: < ON >");
            }
        }
        else {
            if (Buttons::SlideListener::GetSingleton()->SinkRegistered) {
                Buttons::SlideListener::Unregister();
                LOG("Slide: < Off >");
            }

            RuntimeMethods::ResetSlide();
        }
    }
}  // namespace CrouchSliding