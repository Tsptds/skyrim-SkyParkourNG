#pragma once
namespace Hooks {

    class CameraHandler {
        public:
            static bool InstallCamStateHook();

        private:
            static bool InstallTPP();
            static bool InstallFPP();

            // Our hook callbacks
            static bool CanProcess_TPP(RE::ThirdPersonState *a_this, RE::InputEvent *a_event);
            static bool CanProcess_FPP(RE::FirstPersonState *a_this, RE::InputEvent *a_event);

            // Originals
            static inline REL::Relocation<decltype(CanProcess_TPP)> _CanProcess_TPP;
            static inline REL::Relocation<decltype(CanProcess_FPP)> _CanProcess_FPP;
    };
}  // namespace Hooks

/* Install */
bool Hooks::CameraHandler::InstallCamStateHook() {
    bool res = InstallTPP();
    res &= InstallFPP();

    return res;
}

bool Hooks::CameraHandler::InstallTPP() {
    /* VTABLE 0 ->TesCameraState /  1 ->PlayerInputHandler */

    REL::Relocation<uintptr_t> vtblPlayer{RE::VTABLE_ThirdPersonState[1]};
    _CanProcess_TPP = vtblPlayer.write_vfunc(0x1, &CanProcess_TPP);

    if (!_CanProcess_TPP.address()) {
        logger::critical("TPP State Hook Not Installed");
        return false;
    }
    return true;
}
bool Hooks::CameraHandler::InstallFPP() {
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
            /* Prevent Havok from pulling the player towards ground */
            RE::PlayerCharacter::GetSingleton()->GetCharController()->surfaceInfo.supportedState =
                RE::hkpSurfaceInfo::SupportedState::kUnsupported;
            return false;
        }
    }

    return _CanProcess_TPP(a_this, a_event);
}
bool Hooks::CameraHandler::CanProcess_FPP(RE::FirstPersonState *a_this, RE::InputEvent *a_event) {
    if (ModSettings::ModEnabled) {
        if (RuntimeVariables::ParkourInProgress) {
            return false;
        }
    }

    return _CanProcess_FPP(a_this, a_event);
}