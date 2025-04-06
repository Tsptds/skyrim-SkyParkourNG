#include "AnimationListener.h"
#include "References.h"

bool AnimEventListener::Register() {
    if (auto player = RE::PlayerCharacter::GetSingleton()) {
        RE::BSTSmartPointer<RE::BSAnimationGraphManager> animGraphManager;

        if (player->GetAnimationGraphManager(animGraphManager)) {
            auto listener = AnimEventListener::GetSingleton();

            if (player->AddAnimationGraphEventSink(listener)) {
                logger::info("AnimEvent - Listening");
                return true;

            } else {
                //logger::info("Anim Event Already Has Sink");
                return true;
            }
        } else {
            //logger::info("Player Anim Graph Isn't Available");
            return false;
        }
    }
    return false;
}

bool AnimEventListener::Unregister() {
    if (auto player = RE::PlayerCharacter::GetSingleton()) {
        auto listener = AnimEventListener::GetSingleton();

        player->RemoveAnimationGraphEventSink(listener);
        logger::info("AnimEvent - Not Listening");

        return true;
    }
    return false;
}

RE::BSEventNotifyControl AnimEventListener::ProcessEvent(const RE::BSAnimationGraphEvent* a_event,
                                                         RE::BSTEventSource<RE::BSAnimationGraphEvent>*) {
    if (!a_event) {
        return RE::BSEventNotifyControl::kContinue;
    }

    // logger::info("Event: {}", a_event->tag);
    if (RuntimeVariables::ParkourEndQueued) {
        if (RE::PlayerCharacter::GetSingleton()->IsInRagdollState()) {
            ParkourUtility::ToggleControlsForParkour(true);
            RuntimeVariables::ParkourEndQueued = false;
        }
        // Reenable controls
        else if (a_event->tag == "idleChairGetUp") {
            // Swap the leg for step animation
            RuntimeMethods::SwapLegs();

            ParkourUtility::ToggleControlsForParkour(true);
            Parkouring::UpdateParkourPoint();
            RuntimeVariables::ParkourEndQueued = false;
        }
    }
    return RE::BSEventNotifyControl::kContinue;
}