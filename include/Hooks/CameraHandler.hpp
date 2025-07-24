#include "References.h"
#pragma once
namespace Hooks {

    class CameraHandler {
        public:
            static bool InstallCamStateHooks();

        private:
            static bool Install_CanProcess_TPP();
            static bool Install_Update_TPP();
            static bool Install_CanProcess_FPP();

            // Our hook callbacks
            static bool CanProcess_TPP(RE::ThirdPersonState *a_this, RE::InputEvent *a_event);
            static void Update_TPP(RE::ThirdPersonState *a_this, RE::BSTSmartPointer<RE::TESCameraState> &a_nextState);

            static bool CanProcess_FPP(RE::FirstPersonState *a_this, RE::InputEvent *a_event);

            // Originals
            static inline REL::Relocation<decltype(CanProcess_TPP)> _CanProcess_TPP;
            static inline REL::Relocation<decltype(Update_TPP)> _Update_TPP;

            static inline REL::Relocation<decltype(CanProcess_FPP)> _CanProcess_FPP;
    };
}  // namespace Hooks

/* Install */
bool Hooks::CameraHandler::InstallCamStateHooks() {
    bool res = Install_CanProcess_TPP();
    res &= Install_Update_TPP();
    res &= Install_CanProcess_FPP();

    return res;
}

bool Hooks::CameraHandler::Install_CanProcess_TPP() {
    /* VTABLE 0 ->TesCameraState /  1 ->PlayerInputHandler */

    REL::Relocation<uintptr_t> vtblInput{RE::VTABLE_ThirdPersonState[1]};
    _CanProcess_TPP = vtblInput.write_vfunc(0x1, &CanProcess_TPP);

    if (!_CanProcess_TPP.address()) {
        logger::critical("TPP CanProcess Hook Not Installed");
        return false;
    }
    return true;
}
bool Hooks::CameraHandler::Install_Update_TPP() {
    /* VTABLE 0 ->TesCameraState /  1 ->PlayerInputHandler */

    REL::Relocation<uintptr_t> vtblInput{RE::VTABLE_ThirdPersonState[0]};
    _Update_TPP = vtblInput.write_vfunc(0x3, &Update_TPP);

    if (!_Update_TPP.address()) {
        logger::critical("TPP Update Hook Not Installed");
        return false;
    }
    return true;
}

bool Hooks::CameraHandler::Install_CanProcess_FPP() {
    /* VTABLE 0 ->TesCameraState /  1 ->PlayerInputHandler */

    REL::Relocation<uintptr_t> vtblPlayer{RE::VTABLE_FirstPersonState[1]};
    _CanProcess_FPP = vtblPlayer.write_vfunc(0x1, &CanProcess_FPP);

    if (!_CanProcess_FPP.address()) {
        logger::critical("FPP State Hook Not Installed");
        return false;
    }
    return true;
}

/* Hooks */
bool Hooks::CameraHandler::CanProcess_TPP(RE::ThirdPersonState *a_this, RE::InputEvent *a_event) {
    if (ModSettings::ModEnabled) {
        if (RuntimeVariables::ParkourInProgress) {
            return false;
        }
    }

    return _CanProcess_TPP(a_this, a_event);
}
void Hooks::CameraHandler::Update_TPP(RE::ThirdPersonState *a_this, RE::BSTSmartPointer<RE::TESCameraState> &a_nextState) {
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

    _Update_TPP(a_this, a_nextState);
}

bool Hooks::CameraHandler::CanProcess_FPP(RE::FirstPersonState *a_this, RE::InputEvent *a_event) {
    if (ModSettings::ModEnabled) {
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
            return false;
        }
    }

    return _CanProcess_FPP(a_this, a_event);
}