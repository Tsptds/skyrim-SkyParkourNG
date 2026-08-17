#include "Listeners/ButtonListener.h"
#include "Parkouring.h"
#include "Util/ParkourUtility.h"
#include "_References/ParkourType.h"
#include "ModSettings/ModSettings.hpp"
#include "_References/RuntimeVariables.h"
#include "HUD/Scaleform/SkyParkourMenu.hpp"

void Buttons::ParkourListener::Parkour(RE::ButtonEvent *buttonEvent)
{
    // Delay Threshold Passed
    if (buttonEvent->IsDown() || buttonEvent->IsHeld())
    {
        if (ModSettings::Parkour_Delay <= buttonEvent->HeldDuration())
        {
            Parkouring::TryActivateParkour();
        }
    }
}

void Buttons::ParkourListener::Register()
{
    auto inputManager = RE::BSInputDeviceManager::GetSingleton();
    if (inputManager)
    {
        inputManager->AddEventSink(Buttons::ParkourListener::GetSingleton());
        Buttons::ParkourListener::GetSingleton()->SinkRegistered = true;
        //INFO("Buttons - Listening");
    }
}
void Buttons::ParkourListener::Unregister()
{
    auto inputManager = RE::BSInputDeviceManager::GetSingleton();
    if (inputManager)
    {
        inputManager->RemoveEventSink(Buttons::ParkourListener::GetSingleton());
        Buttons::ParkourListener::GetSingleton()->SinkRegistered = false;
        //INFO("Buttons - Not Listening");
    }
}

RE::BSEventNotifyControl Buttons::ParkourListener::ProcessEvent(RE::InputEvent *const *a_event, RE::BSTEventSource<RE::InputEvent *> *)
{
    if (!a_event) return RE::BSEventNotifyControl::kContinue;

    for (auto event = *a_event; event; event = event->next)
    {
        if (const auto buttonEvent = event->AsButtonEvent())
        {
            const auto userEventName = event->QUserEvent();
            const auto UE = RE::UserEvents::GetSingleton();
            // INFO("{}", userEventName.c_str());

            if (ModSettings::Use_Preset_Parkour_Key)
            {
                //INFO("PresetParkourKey {}\n ButtonEvent ID {}", ModSettings::PresetParkourKey, buttonId);
                //INFO("JumpMap {}\n SprintMap {}\nActivateMap {}", jumpMapping,sprintMapping,activateMapping);
                if (!UE) continue;
                RE::BSFixedString expectedEvent;

                using pk = SkyParkour::PresetKeys;
                switch (ModSettings::Preset_Parkour_Key)
                {
                    case pk::kJump:
                        expectedEvent = UE->jump;
                        break;
                    case pk::kSprint:
                        expectedEvent = UE->sprint;
                        break;
                    case pk::kActivate:
                        expectedEvent = UE->activate;
                        break;
                    default:
                        break;
                }

                if (userEventName == expectedEvent)
                {
                    if (RuntimeVariables::ParkourInProgress)
                    {
                        continue;
                    }
                    Buttons::ParkourListener::Parkour(buttonEvent);
                }
            }
            else
            {
                auto dxScanCode = buttonEvent->GetIDCode();  // DX Scan Code
                // INFO("DX code : {}, Input Type: {}", dxScanCode, buttonEvent->GetDevice());

                // Convert Xinput codes to creation kit versions
                if (buttonEvent->GetDevice() == RE::INPUT_DEVICE::kGamepad)
                {
                    dxScanCode = SKSE::InputMap::GamepadMaskToKeycode(dxScanCode);
                }
                else if (buttonEvent->GetDevice() == RE::INPUT_DEVICE::kMouse)
                {
                    dxScanCode = Buttons::MapToCKIfPossible(dxScanCode);
                }

                if (dxScanCode == ModSettings::Custom_Parkour_Key)
                {
                    if (RuntimeVariables::ParkourInProgress)
                    {
                        continue;
                    }

                    Buttons::ParkourListener::Parkour(buttonEvent);
                }
            }

            [&] -> void {
                const auto pl = GET_PLAYER;
                using opt = SkyParkour::AutoParkourOptions;
                namespace pu = ParkourUtility;

                if (ModSettings::Auto_Parkour == opt::kNonCombatOnly && !pu::IsActorWeaponOut(pl) ||
                    ModSettings::Auto_Parkour == opt::kAlways)
                {
                    switch (RuntimeVariables::selectedLedgeType)
                    {
                        case ParkourType::StepHigh:
                        case ParkourType::StepLow:
                            if (userEventName == UE->forward || userEventName == UE->back || userEventName == UE->strafeLeft ||
                                userEventName == UE->strafeRight)
                            {
                                if (buttonEvent->HeldDuration() >= 0.4f)
                                {
                                    Parkouring::TryActivateParkour();
                                }
                            }
                    }
                }
            }();
        }
    }
    // DON'T SKIP ANY INPUTS, THIS GOES AFTER FOR LOOP. OTHERWISE BREAKS OTHER INPUTS
    return RE::BSEventNotifyControl::kContinue;
}