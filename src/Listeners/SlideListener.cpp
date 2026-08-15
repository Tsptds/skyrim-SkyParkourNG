#include "Listeners/ButtonListener.h"
#include "CrouchSliding.h"
#include "_References/ModSettings.h"
#include "_References/RuntimeVariables.h"

void Buttons::SlideListener::CrouchSlide(RE::ButtonEvent *buttonEvent)
{
    if (buttonEvent->IsDown())
    {
        CrouchSliding::TrySprintSlide(false);
    }
    else if (buttonEvent->HeldDuration() > 0.2f)
    {
        if (RuntimeVariables::SlideOngoing)
        {
            RE::PlayerCharacter *act = GET_PLAYER;
            if (auto st = act->AsActorState(); !st->IsSneaking() && !act->IsInMidair())
            {
                bool isRoll;
                if (act->GetGraphVariableBool(SPPF_SLIDE_IS_ROLL, isRoll) && !isRoll)
                {
                    if (act->NotifyAnimationGraph("SneakStart")) act->AsActorState()->actorState1.sneaking = true;
                }
            }
        }
        else
        {
            CrouchSliding::TrySprintSlide(true);
        }

        if (ModSettings::ExpSlideTackle)
        {
            if (RuntimeVariables::SlideOngoing) CrouchSliding::TryKnockCollidedActor(GET_PLAYER);
        }
    }
}

void Buttons::SlideListener::Register()
{
    auto inputManager = RE::BSInputDeviceManager::GetSingleton();
    if (inputManager)
    {
        inputManager->AddEventSink(Buttons::SlideListener::GetSingleton());
        Buttons::SlideListener::GetSingleton()->SinkRegistered = true;
        //INFO("Buttons - Listening");
    }
}
void Buttons::SlideListener::Unregister()
{
    auto inputManager = RE::BSInputDeviceManager::GetSingleton();
    if (inputManager)
    {
        inputManager->RemoveEventSink(Buttons::SlideListener::GetSingleton());
        Buttons::SlideListener::GetSingleton()->SinkRegistered = false;
        //INFO("Buttons - Not Listening");
    }
}

RE::BSEventNotifyControl Buttons::SlideListener::ProcessEvent(RE::InputEvent *const *a_event, RE::BSTEventSource<RE::InputEvent *> *)
{
    if (!a_event) return RE::BSEventNotifyControl::kContinue;

    for (auto event = *a_event; event; event = event->next)
    {
        if (const auto buttonEvent = event->AsButtonEvent())
        {
            const auto userEventName = event->QUserEvent();

            const auto UE = RE::UserEvents::GetSingleton();
            if (!UE) continue;

            if (userEventName == UE->sneak)
            {
                Buttons::SlideListener::CrouchSlide(buttonEvent);
            }
        }
    }

    return RE::BSEventNotifyControl::kContinue;
}