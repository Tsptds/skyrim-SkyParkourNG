#include "CrouchSliding.h"
#include "_References/ModSettings.h"
#include "_References/RuntimeVariables.h"
#include "Util/ParkourUtility.h"

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
        const auto &state = actor->AsActorState();

        if (state->actorState1.sneaking) return false;
        if (!state->IsSprinting()) return false;

        if (actor->IsInMidair()) return false;
        if (ParkourUtility::IsKnockedOut(actor)) return false;
        if (actor->IsAnimationDriven()) return false;
        if (actor->IsStaggering()) return false;
        if (ParkourUtility::IsInSyncedAnimation(actor)) return false;
        if (ParkourUtility::IsBeastForm()) return false;
        if (ParkourUtility::IsSitting(actor)) return false;
        if (ParkourUtility::IsInDrawSheath(actor)) return false;
        if (ParkourUtility::IsAttacking(actor)) return false;
        if (ParkourUtility::IsCrouchSliding(actor)) return false;

        return true;
    }
}  // namespace CrouchSliding