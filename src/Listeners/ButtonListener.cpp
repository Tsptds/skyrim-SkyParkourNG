#include "Listeners/ButtonListener.h"
#include "Parkouring.h"
#include "_References/ModSettings.h"
#include "_References/RuntimeVariables.h"

std::unordered_map<uint32_t, uint32_t> ButtonStates::xinputToCKMap = {
    // Mouse
    {2, 258},  // Mouse middle
    {3, 259},  // M4
    {4, 260},  // M5
    // Not natively supported
    //{5, 261},  // M6
    //{6, 262},  // M7
    //{7, 263},  // ? tf is 8th button

    // These have no button up events, input gets stuck on wheel up down
    //{8, 264},  // Wheel Up
    //{9, 265},  // Wheel Down

    // Gamepad
    //{0x0001, 266},  // DPAD_UP
    //{0x0002, 267},  // DPAD_DOWN
    //{0x0004, 268},  // DPAD_LEFT
    //{0x0008, 269},  // DPAD_RIGHT
    //{0x0010, 270},  // START
    //{0x0020, 271},  // BACK
    //{0x0040, 272},  // LEFT_THUMB
    //{0x0080, 273},  // RIGHT_THUMB
    //{0x0100, 274},  // LEFT_SHOULDER
    //{0x0200, 275},  // RIGHT_SHOULDER
    //{0x1000, 276},  // A
    //{0x2000, 277},  // B
    //{0x4000, 278},  // X
    //{0x8000, 279}   // Y
};

uint32_t ButtonStates::MapToCKIfPossible(uint32_t dxcode) {
    auto it = xinputToCKMap.find(dxcode);
    if (it != xinputToCKMap.end()) {
        //LOG("Alt. CK input found, mapping {}", it->second);
        return it->second;
    }
    return dxcode;  // Return default value if key not found
}
void ButtonStates::Parkour(RE::ButtonEvent *buttonEvent) {
    // Delay Threshold Passed
    if (buttonEvent->IsDown() || buttonEvent->IsHeld()) {
        if (ModSettings::Parkour_Delay <= buttonEvent->heldDownSecs) {
            Parkouring::TryActivateParkour();
        }
    }
}

void ButtonStates::CrouchSlide(RE::ButtonEvent *buttonEvent) {
    if (!ModSettings::Crouch_Slide_Enabled) return;
    if (RuntimeVariables::ParkourInProgress) return;

    if (buttonEvent->IsDown()) {
        const auto &pl = GET_PLAYER;
        const auto &state = pl->AsActorState();
        if (state->actorState1.sneaking) return;

        if (state->IsSprinting()) {
            pl->NotifyAnimationGraph(SPPF_NOTIFY_SLIDE);
        }
    }
}

void ButtonEventListener::Register() {
    auto inputManager = RE::BSInputDeviceManager::GetSingleton();
    if (inputManager) {
        inputManager->AddEventSink(ButtonEventListener::GetSingleton());
        ButtonEventListener::GetSingleton()->SinkRegistered = true;
        //LOG("Buttons - Listening");
    }
}
void ButtonEventListener::Unregister() {
    auto inputManager = RE::BSInputDeviceManager::GetSingleton();
    if (inputManager) {
        inputManager->RemoveEventSink(ButtonEventListener::GetSingleton());
        ButtonEventListener::GetSingleton()->SinkRegistered = false;
        //LOG("Buttons - Not Listening");
    }
}

RE::BSEventNotifyControl ButtonEventListener::ProcessEvent(RE::InputEvent *const *a_event, RE::BSTEventSource<RE::InputEvent *> *) {
    if (!a_event) return RE::BSEventNotifyControl::kContinue;

    const auto &UE = RE::UserEvents::GetSingleton();

    for (auto event = *a_event; event; event = event->next) {
        if (const auto &buttonEvent = event->AsButtonEvent()) {
            const auto &userEventName = event->QUserEvent();
            // LOG("{}", userEventName.c_str());

            if (userEventName == UE->sneak) {
                ButtonStates::CrouchSlide(buttonEvent);
            }

            else if (ModSettings::Use_Preset_Parkour_Key) {
                //LOG("PresetParkourKey {}\n ButtonEvent ID {}", ModSettings::PresetParkourKey, buttonId);
                //LOG("JumpMap {}\n SprintMap {}\nActivateMap {}", jumpMapping,sprintMapping,activateMapping);

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
                    ButtonStates::Parkour(buttonEvent);
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
                    dxScanCode = ButtonStates::xinputToCKMap[dxScanCode];
                }

                if (dxScanCode == ModSettings::Custom_Parkour_Key) {
                    if (RuntimeVariables::ParkourInProgress) {
                        continue;
                    }

                    ButtonStates::Parkour(buttonEvent);
                }
            }
        }
    }
    // DON'T SKIP ANY INPUTS, THIS GOES AFTER FOR LOOP. OTHERWISE BREAKS OTHER INPUTS
    return RE::BSEventNotifyControl::kContinue;
}