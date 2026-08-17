#include "Listeners/ButtonListener.h"
#include "CrouchSliding.h"
#include "ModSettings/ModSettings.hpp"
#include "_References/RuntimeVariables.h"
#include "_References/Compatibility.h"

void Buttons::SlideListener::CrouchSlide(RE::ButtonEvent *buttonEvent)
{
    if (buttonEvent->IsDown())
    {
        CrouchSliding::TrySprintSlide(false);
    }
    /* Sneak on slide begin */
    else if (!ModSettings::Advanced_Slide_Sneak && buttonEvent->IsHeld())
    {
        CrouchSliding::TrySprintSlide(true);
        RE::PlayerCharacter *act = GET_PLAYER;

        if (act->NotifyAnimationGraph(SPPF_SLIDE_SNEAK))
        {
            bool isTDM = Compatibility::TrueDirectionalMovement::found;
            if (!(isTDM && API_Handles::TDM::IsLockedOn()))
            {
                act->AsActorState()->actorState1.sneaking = true;
                act->SetGraphVariableInt("iIsInSneak", 1);
            }
        }
    }
    /* Sneak Advanced */
    else if (ModSettings::Advanced_Slide_Sneak && buttonEvent->HeldDuration() > 0.2f)
    {
        if (RuntimeVariables::SlideOngoing && RuntimeVariables::RecoveryFramesActive)
        {
            RE::PlayerCharacter *act = GET_PLAYER;
            if (!act->IsInMidair())
            {
                bool isRoll;
                if (act->GetGraphVariableBool(SPPF_SLIDE_IS_ROLL, isRoll) && !isRoll)
                {
                    if (act->NotifyAnimationGraph(SPPF_SLIDE_SNEAK_ADV))
                    {
                        act->AsActorState()->actorState1.sneaking = true;
                        act->SetGraphVariableInt("iIsInSneak", 1);
                    }
                }
            }
        }
        else
        {
            CrouchSliding::TrySprintSlide(true);
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