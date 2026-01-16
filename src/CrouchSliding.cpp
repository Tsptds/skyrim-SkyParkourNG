#include "CrouchSliding.h"
#include "_References/ModSettings.h"
#include "_References/RuntimeVariables.h"
#include "Util/ParkourUtility.h"

#include "API/API_Handles.h"

namespace CrouchSliding {
    bool TrySprintSlide() {
        const auto &pl = GET_PLAYER;

        if (IsSlideActiveFor(pl)) {
            return pl->NotifyAnimationGraph(SPPF_NOTIFY_SLIDE);
        }

        return false;
    }

    bool IsSlideActiveFor(RE::Actor *actor) {
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
        float fellDist = ctrl->fallStartHeight - actor->GetPositionZ();

        if (midair && fellDist > 200.0f) {
            const auto downRay = [actor] {
                constexpr RE::NiPoint3 downDir{0, 0, -1};
                constexpr float dist = 150.f;
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

            if (downRay.didHit) return true;

            return false;
        }
        else {
            if (fallTime > 0.2f) return false;
            if (!sprinting) return false;
            if (sneaking) return false;
        }

        return true;
    }
}  // namespace CrouchSliding