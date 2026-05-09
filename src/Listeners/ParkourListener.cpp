#include "Listeners/ButtonListener.h"
#include "Parkouring.h"
#include "Util/ParkourUtility.h"
#include "_References/ParkourType.h"
#include "_References/ModSettings.h"
#include "_References/RuntimeVariables.h"
#include "HUD/Scaleform/SkyParkourMenu.hpp"

void Buttons::ParkourListener::Parkour(RE::ButtonEvent *buttonEvent) {
    // Delay Threshold Passed
    if (buttonEvent->IsDown() || buttonEvent->IsHeld()) {
        if (ModSettings::Parkour_Delay <= buttonEvent->heldDownSecs) {
            Parkouring::TryActivateParkour();
        }
    }
}

void Buttons::ParkourListener::Register() {
    auto inputManager = RE::BSInputDeviceManager::GetSingleton();
    if (inputManager) {
        inputManager->AddEventSink(Buttons::ParkourListener::GetSingleton());
        Buttons::ParkourListener::GetSingleton()->SinkRegistered = true;
        //LOG("Buttons - Listening");
    }
}
void Buttons::ParkourListener::Unregister() {
    auto inputManager = RE::BSInputDeviceManager::GetSingleton();
    if (inputManager) {
        inputManager->RemoveEventSink(Buttons::ParkourListener::GetSingleton());
        Buttons::ParkourListener::GetSingleton()->SinkRegistered = false;
        //LOG("Buttons - Not Listening");
    }
}

RE::BSEventNotifyControl Buttons::ParkourListener::ProcessEvent(RE::InputEvent *const *a_event, RE::BSTEventSource<RE::InputEvent *> *) {
    if (!a_event) return RE::BSEventNotifyControl::kContinue;

    for (auto event = *a_event; event; event = event->next) {
        if (const auto buttonEvent = event->AsButtonEvent()) {
            const auto userEventName = event->QUserEvent();
            const auto UE = RE::UserEvents::GetSingleton();
            // LOG("{}", userEventName.c_str());

            if (ModSettings::Use_Preset_Parkour_Key) {
                //LOG("PresetParkourKey {}\n ButtonEvent ID {}", ModSettings::PresetParkourKey, buttonId);
                //LOG("JumpMap {}\n SprintMap {}\nActivateMap {}", jumpMapping,sprintMapping,activateMapping);
                if (!UE) continue;
                RE::BSFixedString expectedEvent;

                switch (ModSettings::Preset_Parkour_Key) {
                    case PARKOUR_PRESET_KEYS::kJump:
                        expectedEvent = UE->jump;
                        break;
                    case PARKOUR_PRESET_KEYS::kSprint:
                        expectedEvent = UE->sprint;
                        break;
                    case PARKOUR_PRESET_KEYS::kActivate:
                        expectedEvent = UE->activate;
                        break;
                    default:
                        break;
                }

                if (userEventName == expectedEvent) {
                    if (RuntimeVariables::ParkourInProgress) {
                        continue;
                    }
                    Buttons::ParkourListener::Parkour(buttonEvent);
                }
            }
            else {
                auto dxScanCode = buttonEvent->GetIDCode();  // DX Scan Code
                // LOG("DX code : {}, Input Type: {}", dxScanCode, buttonEvent->GetDevice());

                // Convert Xinput codes to creation kit versions
                if (buttonEvent->GetDevice() == RE::INPUT_DEVICE::kGamepad) {
                    dxScanCode = SKSE::InputMap::GamepadMaskToKeycode(dxScanCode);
                }
                else if (buttonEvent->GetDevice() == RE::INPUT_DEVICE::kMouse) {
                    dxScanCode = Buttons::xinputToCKMap[dxScanCode];
                }

                if (dxScanCode == ModSettings::Custom_Parkour_Key) {
                    if (RuntimeVariables::ParkourInProgress) {
                        continue;
                    }

                    Buttons::ParkourListener::Parkour(buttonEvent);
                }
            }

            [&] -> void {
                const auto pl = GET_PLAYER;
                using opt = AUTO_PARKOUR_OPTIONS;
                namespace pu = ParkourUtility;

                if (ModSettings::Auto_Parkour == opt::kNonCombatOnly && !pu::IsActorWeaponOut(pl) ||
                    ModSettings::Auto_Parkour == opt::kAlways) {
                    switch (RuntimeVariables::selectedLedgeType) {
                        case ParkourType::StepHigh:
                        case ParkourType::StepLow:
                            if (userEventName == UE->forward || userEventName == UE->back || userEventName == UE->strafeLeft ||
                                userEventName == UE->strafeRight) {
                                if (buttonEvent->HeldDuration() >= 0.4f) {
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