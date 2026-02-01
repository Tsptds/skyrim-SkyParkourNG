#include "Listeners/ButtonListener.h"
#include "Parkouring.h"
#include "_References/ModSettings.h"
#include "_References/RuntimeVariables.h"

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
        if (const auto &buttonEvent = event->AsButtonEvent()) {
            const auto &userEventName = event->QUserEvent();
            // LOG("{}", userEventName.c_str());

            if (ModSettings::Use_Preset_Parkour_Key) {
                //LOG("PresetParkourKey {}\n ButtonEvent ID {}", ModSettings::PresetParkourKey, buttonId);
                //LOG("JumpMap {}\n SprintMap {}\nActivateMap {}", jumpMapping,sprintMapping,activateMapping);

                RE::BSFixedString expectedEvent;
                const auto &UE = RE::UserEvents::GetSingleton();

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
        }
    }
    // DON'T SKIP ANY INPUTS, THIS GOES AFTER FOR LOOP. OTHERWISE BREAKS OTHER INPUTS
    return RE::BSEventNotifyControl::kContinue;
}