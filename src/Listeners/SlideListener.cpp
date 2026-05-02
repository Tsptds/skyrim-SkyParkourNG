#include "Listeners/ButtonListener.h"
#include "CrouchSliding.h"
#include "_References/ModSettings.h"
#include "_References/RuntimeVariables.h"

void Buttons::SlideListener::CrouchSlide(RE::ButtonEvent *buttonEvent) {
    if (buttonEvent->IsDown()) {
        CrouchSliding::TrySprintSlide(false);
    }
    else if (buttonEvent->IsHeld()) {
        CrouchSliding::TrySprintSlide(true);

        if (ModSettings::ExpSlideTackle) {
            if (RuntimeVariables::SlideOngoing) CrouchSliding::TryKnockCollidedActor(GET_PLAYER);
        }
    }
}

void Buttons::SlideListener::Register() {
    auto inputManager = RE::BSInputDeviceManager::GetSingleton();
    if (inputManager) {
        inputManager->AddEventSink(Buttons::SlideListener::GetSingleton());
        Buttons::SlideListener::GetSingleton()->SinkRegistered = true;
        //LOG("Buttons - Listening");
    }
}
void Buttons::SlideListener::Unregister() {
    auto inputManager = RE::BSInputDeviceManager::GetSingleton();
    if (inputManager) {
        inputManager->RemoveEventSink(Buttons::SlideListener::GetSingleton());
        Buttons::SlideListener::GetSingleton()->SinkRegistered = false;
        //LOG("Buttons - Not Listening");
    }
}

RE::BSEventNotifyControl Buttons::SlideListener::ProcessEvent(RE::InputEvent *const *a_event, RE::BSTEventSource<RE::InputEvent *> *) {
    if (!a_event) return RE::BSEventNotifyControl::kContinue;

    for (auto event = *a_event; event; event = event->next) {
        if (const auto &buttonEvent = event->AsButtonEvent()) {
            const auto &userEventName = event->QUserEvent();

            const auto &UE = RE::UserEvents::GetSingleton();

            if (userEventName == UE->sneak) {
                Buttons::SlideListener::CrouchSlide(buttonEvent);
            }
        }
    }

    return RE::BSEventNotifyControl::kContinue;
}