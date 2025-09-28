#pragma once
#include "_References/ModSettings.h"
#include "_References/RuntimeVariables.h"
#include "_References/Compatibility.h"

namespace Hooks {

    class CameraHandler {
        public:
            static bool InstallCamStateHooks();

        private:
            struct TPP {
                    inline static float TDM_Pitch_Clamp = 0.1f;

                    struct Install {
                            /* TesCameraState */
                            static bool Begin();
                            static bool End();
                            static bool Update();

                            /* PlayerInputHandler */
                            static bool CanProcess();
                    };

                    struct Callback {
                            static void Begin(RE::ThirdPersonState *a_this);
                            static void End(RE::ThirdPersonState *a_this);
                            static void Update(RE::ThirdPersonState *a_this, RE::BSTSmartPointer<RE::TESCameraState> &a_nextState);
                            static bool CanProcess(RE::ThirdPersonState *a_this, RE::InputEvent *a_event);
                    };

                    struct OG {
                            static inline REL::Relocation<decltype(Callback::Begin)> _Begin;
                            static inline REL::Relocation<decltype(Callback::End)> _End;
                            static inline REL::Relocation<decltype(Callback::CanProcess)> _CanProcess;
                            static inline REL::Relocation<decltype(Callback::Update)> _Update;
                    };
            };

            struct FPP {
                    inline static const float Vertical_Clamp_Angle = 1.0f;

                    struct Install {
                            /* TesCameraState */
                            static bool Begin();
                            static bool End();
                            static bool Update();

                            /* PlayerInputHandler */
                            static bool CanProcess();
                    };

                    struct Callback {
                            static void Begin(RE::FirstPersonState *a_this);
                            static void End(RE::FirstPersonState *a_this);
                            static void Update(RE::FirstPersonState *a_this, RE::BSTSmartPointer<RE::TESCameraState> &a_nextState);
                            static bool CanProcess(RE::FirstPersonState *a_this, RE::InputEvent *a_event);
                    };

                    struct OG {
                            static inline REL::Relocation<decltype(Callback::Begin)> _Begin;
                            static inline REL::Relocation<decltype(Callback::End)> _End;
                            static inline REL::Relocation<decltype(Callback::Update)> _Update;
                            static inline REL::Relocation<decltype(Callback::CanProcess)> _CanProcess;
                    };
            };

            struct FreeCam { /* UNUSED */
                    struct Install {
                            static bool Begin();
                            static bool End();
                    };

                    struct Callback {
                            static void Begin(RE::FreeCameraState *a_this);
                            static void End(RE::FreeCameraState *a_this);
                    };

                    struct OG {
                            static inline REL::Relocation<decltype(Callback::Begin)> _Begin;
                            static inline REL::Relocation<decltype(Callback::End)> _End;
                    };
            };
    };
}  // namespace Hooks

// Install

bool Hooks::CameraHandler::InstallCamStateHooks() {
    bool res = false;

    // res &= TPP::Install::Begin();
    res &= TPP::Install::End();
    res &= TPP::Install::Update();
    res &= TPP::Install::CanProcess();

    // res &= FPP::Install::Begin();
    res &= FPP::Install::End();
    res &= FPP::Install::Update();
    res &= FPP::Install::CanProcess();

    return res;
}

/* Third person */
bool Hooks::CameraHandler::TPP::Install::CanProcess() {
    /* VTABLE 0 ->TesCameraState /  1 ->PlayerInputHandler */

    REL::Relocation<uintptr_t> vtblInput{RE::VTABLE_ThirdPersonState[1]};
    OG::_CanProcess = vtblInput.write_vfunc(0x1, &Callback::CanProcess);

    if (!OG::_CanProcess.address()) {
        CRITICAL("TPP CanProcess Hook Not Installed");
        return false;
    }
    return true;
}
bool Hooks::CameraHandler::TPP::Install::Begin() {
    /* VTABLE 0 ->TesCameraState /  1 ->PlayerInputHandler */

    REL::Relocation<uintptr_t> vtbl{RE::VTABLE_ThirdPersonState[0]};
    OG::_Begin = vtbl.write_vfunc(0x1, &Callback::Begin);

    if (!OG::_Begin.address()) {
        CRITICAL("TPP Begin Hook Not Installed");
        return false;
    }
    return true;
}
bool Hooks::CameraHandler::TPP::Install::End() {
    /* VTABLE 0 ->TesCameraState /  1 ->PlayerInputHandler */

    REL::Relocation<uintptr_t> vtbl{RE::VTABLE_ThirdPersonState[0]};
    OG::_End = vtbl.write_vfunc(0x2, &Callback::End);

    if (!OG::_End.address()) {
        CRITICAL("TPP End Hook Not Installed");
        return false;
    }
    return true;
}
bool Hooks::CameraHandler::TPP::Install::Update() {
    /* VTABLE 0 ->TesCameraState /  1 ->PlayerInputHandler */

    REL::Relocation<uintptr_t> vtblInput{RE::VTABLE_ThirdPersonState[0]};
    OG::_Update = vtblInput.write_vfunc(0x3, &Callback::Update);

    if (!OG::_Update.address()) {
        CRITICAL("TPP Update Hook Not Installed");
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
        CRITICAL("FPP State Hook Not Installed");
        return false;
    }
    return true;
}
bool Hooks::CameraHandler::FPP::Install::Begin() {
    /* VTABLE 0 ->TesCameraState /  1 ->PlayerInputHandler */

    REL::Relocation<uintptr_t> vtbl{RE::VTABLE_FirstPersonState[0]};
    OG::_Begin = vtbl.write_vfunc(0x1, &Callback::Begin);

    if (!OG::_Begin.address()) {
        CRITICAL("FPP Begin Hook Not Installed");
        return false;
    }
    return true;
}
bool Hooks::CameraHandler::FPP::Install::End() {
    /* VTABLE 0 ->TesCameraState /  1 ->PlayerInputHandler */

    REL::Relocation<uintptr_t> vtbl{RE::VTABLE_FirstPersonState[0]};
    OG::_End = vtbl.write_vfunc(0x2, &Callback::End);

    if (!OG::_End.address()) {
        CRITICAL("FPP End Hook Not Installed");
        return false;
    }
    return true;
}
bool Hooks::CameraHandler::FPP::Install::Update() {
    /* VTABLE 0 ->TesCameraState /  1 ->PlayerInputHandler */

    REL::Relocation<uintptr_t> vtblInput{RE::VTABLE_FirstPersonState[0]};
    OG::_Update = vtblInput.write_vfunc(0x3, &Callback::Update);

    if (!OG::_Update.address()) {
        CRITICAL("FPP Update Hook Not Installed");
        return false;
    }
    return true;
}
/* ------------------------------------------------------------- */

// Callbacks

/* Third person */
bool Hooks::CameraHandler::TPP::Callback::CanProcess(RE::ThirdPersonState *a_this, RE::InputEvent *a_event) {
    if (ModSettings::Mod_Enabled) {
        if (RuntimeVariables::ParkourInProgress) {
            return false;
        }
    }

    return OG::_CanProcess(a_this, a_event);
}
void Hooks::CameraHandler::TPP::Callback::Begin(RE::ThirdPersonState *a_this) {
    OG::_Begin(a_this);
}
void Hooks::CameraHandler::TPP::Callback::End(RE::ThirdPersonState *a_this) {
    // On cam state exit, invalidate vars. FPP or TPP will pick up and update when re-entered.
    Parkouring::InvalidateVars();

    OG::_End(a_this);
}
void Hooks::CameraHandler::TPP::Callback::Update(RE::ThirdPersonState *a_this, RE::BSTSmartPointer<RE::TESCameraState> &a_nextState) {
    if (ModSettings::Mod_Enabled) {
        Parkouring::UpdateParkourPoint();
    }

    if (RuntimeVariables::ParkourInProgress) {
        const auto &ctrl = GET_PLAYER->GetCharController();

        /* TDM swim pitch angle thing */
        if (Compatibility::TrueDirectionalMovement) {
            float pitch = ctrl->pitchAngle;
            if (pitch > TDM_Pitch_Clamp) {
                ctrl->pitchAngle = TDM_Pitch_Clamp;
            }
            else if (pitch < -TDM_Pitch_Clamp) {
                ctrl->pitchAngle = -TDM_Pitch_Clamp;
            }
        }

        a_this->targetZoomOffset = a_this->currentZoomOffset;
        a_this->stateNotActive = false;
    }

    OG::_Update(a_this, a_nextState);
}
/* ------------------------------------------------------------- */

/* First person */
bool Hooks::CameraHandler::FPP::Callback::CanProcess(RE::FirstPersonState *a_this, RE::InputEvent *a_event) {
    if (ModSettings::Mod_Enabled) {
        if (RuntimeVariables::ParkourInProgress) {
            return false;
        }
    }

    return OG::_CanProcess(a_this, a_event);
}
void Hooks::CameraHandler::FPP::Callback::Begin(RE::FirstPersonState *a_this) {
    OG::_Begin(a_this);
}
void Hooks::CameraHandler::FPP::Callback::End(RE::FirstPersonState *a_this) {
    // On cam state exit, invalidate vars. FPP or TPP will pick up and update when re-entered.
    Parkouring::InvalidateVars();

    OG::_End(a_this);
}
void Hooks::CameraHandler::FPP::Callback::Update(RE::FirstPersonState *a_this, RE::BSTSmartPointer<RE::TESCameraState> &a_nextState) {
    if (ModSettings::Mod_Enabled) {
        Parkouring::UpdateParkourPoint();
    }

    if (RuntimeVariables::ParkourInProgress) {
        /* Clamp Player looking angle to prevent weird visuals */
        const auto &player = GET_PLAYER;

        /* Vert */
        auto &vertAngle = player->data.angle.x;
        if (vertAngle > Vertical_Clamp_Angle) {
            vertAngle = Vertical_Clamp_Angle;
        }
        else if (vertAngle < -Vertical_Clamp_Angle) {
            vertAngle = -Vertical_Clamp_Angle;
        }

        /* Horz */
        auto &camAngle = a_this->sittingRotation;
        camAngle = 0.0f;
    }

    OG::_Update(a_this, a_nextState);
}
/* ------------------------------------------------------------- */