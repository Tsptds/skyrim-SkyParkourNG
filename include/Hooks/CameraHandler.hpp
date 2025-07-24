#include "References.h"
#pragma once
namespace Hooks {

    class CameraHandler {
        public:
            static bool InstallCamStateHooks();

        private:
            struct TPP {
                    struct Install {
                            static bool CanProcess();
                            static bool Update();
                    };

                    struct Callback {
                            static bool CanProcess(RE::ThirdPersonState *a_this, RE::InputEvent *a_event);
                            static void Update(RE::ThirdPersonState *a_this, RE::BSTSmartPointer<RE::TESCameraState> &a_nextState);
                    };

                    struct OG {
                            static inline REL::Relocation<decltype(Callback::CanProcess)> _CanProcess;
                            static inline REL::Relocation<decltype(Callback::Update)> _Update;
                    };
            };

            struct FPP {
                    struct Install {
                            static bool CanProcess();
                            static bool Update();
                    };

                    struct Callback {
                            static bool CanProcess(RE::FirstPersonState *a_this, RE::InputEvent *a_event);
                            static void Update(RE::FirstPersonState *a_this, RE::BSTSmartPointer<RE::TESCameraState> &a_nextState);
                    };

                    struct OG {
                            static inline REL::Relocation<decltype(Callback::CanProcess)> _CanProcess;
                            static inline REL::Relocation<decltype(Callback::Update)> _Update;
                    };
            };
    };
}  // namespace Hooks

// Install

bool Hooks::CameraHandler::InstallCamStateHooks() {
    bool res = false;

    res &= TPP::Install::CanProcess();
    res &= TPP::Install::Update();

    res &= FPP::Install::CanProcess();
    res &= FPP::Install::Update();

    return res;
}

/* Third person */
bool Hooks::CameraHandler::TPP::Install::CanProcess() {
    /* VTABLE 0 ->TesCameraState /  1 ->PlayerInputHandler */

    REL::Relocation<uintptr_t> vtblInput{RE::VTABLE_ThirdPersonState[1]};
    OG::_CanProcess = vtblInput.write_vfunc(0x1, &Callback::CanProcess);

    if (!OG::_CanProcess.address()) {
        logger::critical("TPP CanProcess Hook Not Installed");
        return false;
    }
    return true;
}
bool Hooks::CameraHandler::TPP::Install::Update() {
    /* VTABLE 0 ->TesCameraState /  1 ->PlayerInputHandler */

    REL::Relocation<uintptr_t> vtblInput{RE::VTABLE_ThirdPersonState[0]};
    OG::_Update = vtblInput.write_vfunc(0x3, &Callback::Update);

    if (!OG::_Update.address()) {
        logger::critical("TPP Update Hook Not Installed");
        return false;
    }
    return true;
}
/* ------------------------------------------------------------- */

/* First person */
bool Hooks::CameraHandler::FPP::Install::CanProcess() {
    /* VTABLE 0 ->TesCameraState /  1 ->PlayerInputHandler */

    REL::Relocation<uintptr_t> vtblPlayer{RE::VTABLE_FirstPersonState[1]};
    OG::_CanProcess = vtblPlayer.write_vfunc(0x1, &Callback::CanProcess);

    if (!OG::_CanProcess.address()) {
        logger::critical("FPP State Hook Not Installed");
        return false;
    }
    return true;
}
bool Hooks::CameraHandler::FPP::Install::Update() {
    /* VTABLE 0 ->TesCameraState /  1 ->PlayerInputHandler */

    REL::Relocation<uintptr_t> vtblInput{RE::VTABLE_FirstPersonState[0]};
    OG::_Update = vtblInput.write_vfunc(0x3, &Callback::Update);

    if (!OG::_Update.address()) {
        logger::critical("FPP Update Hook Not Installed");
        return false;
    }
    return true;
}
/* ------------------------------------------------------------- */

// Callbacks

/* Third person */
bool Hooks::CameraHandler::TPP::Callback::CanProcess(RE::ThirdPersonState *a_this, RE::InputEvent *a_event) {
    if (ModSettings::ModEnabled) {
        if (RuntimeVariables::ParkourInProgress) {
            return false;
        }
    }

    return OG::_CanProcess(a_this, a_event);
}
void Hooks::CameraHandler::TPP::Callback::Update(RE::ThirdPersonState *a_this, RE::BSTSmartPointer<RE::TESCameraState> &a_nextState) {
    if (RuntimeVariables::ParkourInProgress) {
        /* Prevent Havok from pulling the player towards ground */
        auto ctrl = RE::PlayerCharacter::GetSingleton()->GetCharController();
        ctrl->surfaceInfo.supportedState = RE::hkpSurfaceInfo::SupportedState::kUnsupported;

        /* TDM swim pitch angle thing */
        if (Compatibility::TrueDirectionalMovement) {
            float pitch = ctrl->pitchAngle;
            if (pitch > 0.4f) {
                ctrl->pitchAngle = 0.4f;
            }
            else if (pitch < -0.4f) {
                ctrl->pitchAngle = -0.4f;
            }
        }
    }

    OG::_Update(a_this, a_nextState);
}
/* ------------------------------------------------------------- */

/* First person */
bool Hooks::CameraHandler::FPP::Callback::CanProcess(RE::FirstPersonState *a_this, RE::InputEvent *a_event) {
    if (ModSettings::ModEnabled) {
        if (RuntimeVariables::ParkourInProgress) {
            return false;
        }
    }

    return OG::_CanProcess(a_this, a_event);
}
void Hooks::CameraHandler::FPP::Callback::Update(RE::FirstPersonState *a_this, RE::BSTSmartPointer<RE::TESCameraState> &a_nextState) {
    if (RuntimeVariables::ParkourInProgress) {
        /* Clamp Player looking angle to prevent weird visuals */
        auto player = RE::PlayerCharacter::GetSingleton();
        const auto vertAngle = player->data.angle.x;
        if (vertAngle > 0.9f) {
            player->data.angle.x = 0.9f;
        }
        else if (vertAngle < -0.9f) {
            player->data.angle.x = -0.9f;
        }
    }

    OG::_Update(a_this, a_nextState);
}
/* ------------------------------------------------------------- */