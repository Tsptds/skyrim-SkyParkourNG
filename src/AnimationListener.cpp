#include "AnimationListener.h"
#include "References.h"


bool AnimEventListener::RegisterAnimationEventListener() {

    if (auto player = RE::PlayerCharacter::GetSingleton()) {

        RE::BSTSmartPointer<RE::BSAnimationGraphManager> animGraphManager;

        if (player->GetAnimationGraphManager(animGraphManager)) {
            
            static AnimEventListener listener;

            if (player->AddAnimationGraphEventSink(&listener)) {
                logger::info("Registered Animation Event Listener");
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