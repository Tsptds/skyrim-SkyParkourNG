#include "ButtonListener.h"
#include "InputHandler.h"

int32_t ButtonStates::DXCODE = 0;

std::unordered_map<int32_t, int32_t> ButtonStates::xinputToCKMap = {
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

int32_t ButtonStates::MapToCKIfPossible(int32_t dxcode) {
    auto it = xinputToCKMap.find(dxcode);
    if (it != xinputToCKMap.end()) {
        //logger::info("Alt. CK input found, mapping {}", it->second);
        return it->second;
    }
    return dxcode;  // Return default value if key not found
}
void ButtonStates::RegisterActivation(RE::InputEvent* event) {
    const auto buttonEvent = event->AsButtonEvent();

    // Delay Threshold Passed
    if (buttonEvent->IsDown() || buttonEvent->IsHeld()) {
        if (ModSettings::parkourDelay <= buttonEvent->heldDownSecs) {
            Parkouring::TryActivateParkour();
        }
    }
}

void ButtonEventListener::Register() {
    auto inputManager = RE::BSInputDeviceManager::GetSingleton();
    if (inputManager) {
        inputManager->AddEventSink(ButtonEventListener::GetSingleton());
        logger::info("Buttons - Listening");
    }
}
void ButtonEventListener::Unregister() {
    auto inputManager = RE::BSInputDeviceManager::GetSingleton();
    if (inputManager) {
        inputManager->RemoveEventSink(ButtonEventListener::GetSingleton());
        logger::info("Buttons - Not Listening");
    }
}

RE::BSEventNotifyControl ButtonEventListener::ProcessEvent(RE::InputEvent* const* a_event, RE::BSTEventSource<RE::InputEvent*>*) {
    if (!a_event)
        return RE::BSEventNotifyControl::kContinue;

    // Update this here,
    std::jthread([]() { SKSE::GetTaskInterface()->AddTask([] { Parkouring::UpdateParkourPoint(); }); }).detach();

    for (auto event = *a_event; event; event = event->next) {
        if (const auto buttonEvent = event->AsButtonEvent()) {
            auto dxScanCode = static_cast<int32_t>(buttonEvent->GetIDCode());  // DX Scan Code
            // logger::info("DX code : {}, Input Type: {}", dxScanCode, buttonEvent->GetDevice());

            // Convert Xinput codes to creation kit versions
            if (buttonEvent->GetDevice() == RE::INPUT_DEVICE::kGamepad) {
                dxScanCode = SKSE::InputMap::GamepadMaskToKeycode(dxScanCode);

            } else if (buttonEvent->GetDevice() == RE::INPUT_DEVICE::kMouse) {
                dxScanCode = ButtonStates::xinputToCKMap[dxScanCode];
            }

            if (ModSettings::UsePresetParkourKey) {
                auto userEventName = event->QUserEvent();
                //event->AsCharEvent()->
                //logger::info("PresetParkourKey {}\n ButtonEvent ID {}", ModSettings::PresetParkourKey, buttonId);
                //logger::info("JumpMap {}\n SprintMap {}\nActivateMap {}", jumpMapping,sprintMapping,activateMapping);

                if (ModSettings::PresetParkourKey == ModSettings::ParkourKeyOptions::kJump &&
                    userEventName == RE::UserEvents::GetSingleton()->jump) {
                    if (RuntimeVariables::ParkourEndQueued) {
                        continue;
                    }

                    ButtonStates::RegisterActivation(event);

                } else if (ModSettings::PresetParkourKey == ModSettings::ParkourKeyOptions::kSprint &&
                           userEventName == RE::UserEvents::GetSingleton()->sprint) {
                    if (RuntimeVariables::ParkourEndQueued) {
                        continue;
                    }

                    ButtonStates::RegisterActivation(event);

                } else if (ModSettings::PresetParkourKey == ModSettings::ParkourKeyOptions::kActivate &&
                           userEventName == RE::UserEvents::GetSingleton()->activate) {
                    if (RuntimeVariables::ParkourEndQueued) {
                        continue;
                    }

                    ButtonStates::RegisterActivation(event);
                }

            } else {
                if (dxScanCode == ButtonStates::DXCODE) {
                    if (RuntimeVariables::ParkourEndQueued) {
                        continue;
                    }

                    ButtonStates::RegisterActivation(event);
                }
            }
        }
    }
    // DON'T SKIP ANY INPUTS, THIS GOES AFTER FOR LOOP. OTHERWISE BREAKS OTHER INPUTS
    return RE::BSEventNotifyControl::kContinue;
}