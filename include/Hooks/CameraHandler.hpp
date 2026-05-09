#pragma once
#include "_References/ModSettings.h"
#include "_References/RuntimeVariables.h"
#include "_References/Compatibility.h"
#include "HUD/Scaleform/SkyParkourMenu.hpp"
#include "Util/HookingUtil.hpp"

namespace Hooks {

    class CameraHandler {
        public:
            static bool InstallCamStateHooks();

        private:
            class TPP {
                private:
                    inline static float TDM_Pitch_Clamp = 0.1f;

                    struct Signatures {
                            using Begin_t = void(RE::ThirdPersonState *a_this);
                            using End_t = void(RE::ThirdPersonState *a_this);
                            using Update_t = void(RE::ThirdPersonState *a_this, RE::BSTSmartPointer<RE::TESCameraState> &a_nextState);
                            using CanProcess_t = bool(RE::ThirdPersonState *a_this, RE::InputEvent *a_event);
                    };

                public:
                    struct Install {
                            /* TesCameraState */
                            static bool Begin();
                            static bool End();
                            static bool Update();

                            /* PlayerInputHandler */
                            static bool CanProcess();
                    };

                    struct Callback {
                            static Signatures::Begin_t Begin;
                            static Signatures::End_t End;
                            static Signatures::Update_t Update;
                            static Signatures::CanProcess_t CanProcess;
                    };

                    struct OG {
                            static inline REL::Relocation<Signatures::Begin_t *> _Begin;
                            static inline REL::Relocation<Signatures::End_t *> _End;
                            static inline REL::Relocation<Signatures::CanProcess_t *> _CanProcess;
                            static inline REL::Relocation<Signatures::Update_t *> _Update;
                    };
            };

            class FPP {
                private:
                    inline static const float Vertical_Clamp_Angle_Parkour = 1.0f;
                    inline static const float Vertical_Clamp_Angle_Slide = 0.6f;

                    struct Signatures {
                            using Begin_t = void(RE::FirstPersonState *a_this);
                            using End_t = void(RE::FirstPersonState *a_this);
                            using Update_t = void(RE::FirstPersonState *a_this, RE::BSTSmartPointer<RE::TESCameraState> &a_nextState);
                            using CanProcess_t = bool(RE::FirstPersonState *a_this, RE::InputEvent *a_event);
                    };

                public:
                    struct Install {
                            /* TesCameraState */
                            static bool Begin();
                            static bool End();
                            static bool Update();

                            /* PlayerInputHandler */
                            static bool CanProcess();
                    };

                    struct Callback {
                        private:
                            using _S = Signatures;

                        public:
                            static _S::Begin_t Begin;
                            static _S::End_t End;
                            static _S::Update_t Update;
                            static _S::CanProcess_t CanProcess;
                    };

                    struct OG {
                        private:
                            using _S = Signatures;

                        public:
                            static inline REL::Relocation<_S::Begin_t *> _Begin;
                            static inline REL::Relocation<_S::End_t *> _End;
                            static inline REL::Relocation<_S::CanProcess_t *> _CanProcess;
                            static inline REL::Relocation<_S::Update_t *> _Update;
                    };
            };
    };

#pragma region  // Install All

    bool CameraHandler::InstallCamStateHooks() {
        bool res = true;

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

#pragma endregion

#pragma region  // TPP Install

    bool CameraHandler::TPP::Install::CanProcess() {
        /* VTABLE 0 ->TesCameraState /  1 ->PlayerInputHandler */

        REL::Relocation<uintptr_t> vtbl{RE::VTABLE_ThirdPersonState[1]};

        const bool res = Hooking::InstallVFuncHook(vtbl, 0x1, OG::_CanProcess, &Callback::CanProcess);
        if (!res) CRITICAL("TPP CanProcess Hook Not Installed");

        return res;
    }
    bool CameraHandler::TPP::Install::Begin() {
        /* VTABLE 0 ->TesCameraState /  1 ->PlayerInputHandler */

        REL::Relocation<uintptr_t> vtbl{RE::VTABLE_ThirdPersonState[0]};

        const bool res = Hooking::InstallVFuncHook(vtbl, 0x1, OG::_Begin, &Callback::Begin);
        if (!res) CRITICAL("TPP Begin Hook Not Installed");

        return res;
    }
    bool CameraHandler::TPP::Install::End() {
        /* VTABLE 0 ->TesCameraState /  1 ->PlayerInputHandler */

        REL::Relocation<uintptr_t> vtbl{RE::VTABLE_ThirdPersonState[0]};

        const bool res = Hooking::InstallVFuncHook(vtbl, 0x2, OG::_End, &Callback::End);
        if (!res) CRITICAL("TPP End Hook Not Installed");

        return res;
    }
    bool CameraHandler::TPP::Install::Update() {
        /* VTABLE 0 ->TesCameraState /  1 ->PlayerInputHandler */

        REL::Relocation<uintptr_t> vtbl{RE::VTABLE_ThirdPersonState[0]};

        const bool res = Hooking::InstallVFuncHook(vtbl, 0x3, OG::_Update, &Callback::Update);
        if (!res) CRITICAL("TPP Update Hook Not Installed");

        return res;
    }

#pragma endregion

#pragma region  // FPP Install

    bool CameraHandler::FPP::Install::CanProcess() {
        /* VTABLE 0 ->TesCameraState /  1 ->PlayerInputHandler */

        REL::Relocation<uintptr_t> vtbl{RE::VTABLE_FirstPersonState[1]};

        const bool res = Hooking::InstallVFuncHook(vtbl, 0x1, OG::_CanProcess, &Callback::CanProcess);
        if (!res) CRITICAL("FPP State Hook Not Installed");

        return res;
    }
    bool CameraHandler::FPP::Install::Begin() {
        /* VTABLE 0 ->TesCameraState /  1 ->PlayerInputHandler */

        REL::Relocation<uintptr_t> vtbl{RE::VTABLE_FirstPersonState[0]};

        const bool res = Hooking::InstallVFuncHook(vtbl, 0x1, OG::_Begin, &Callback::Begin);
        if (!res) CRITICAL("FPP Begin Hook Not Installed");

        return res;
    }
    bool CameraHandler::FPP::Install::End() {
        /* VTABLE 0 ->TesCameraState /  1 ->PlayerInputHandler */

        REL::Relocation<uintptr_t> vtbl{RE::VTABLE_FirstPersonState[0]};

        const bool res = Hooking::InstallVFuncHook(vtbl, 0x2, OG::_End, &Callback::End);
        if (!res) CRITICAL("FPP End Hook Not Installed");

        return res;
    }
    bool CameraHandler::FPP::Install::Update() {
        /* VTABLE 0 ->TesCameraState /  1 ->PlayerInputHandler */

        REL::Relocation<uintptr_t> vtbl{RE::VTABLE_FirstPersonState[0]};

        const bool res = Hooking::InstallVFuncHook(vtbl, 0x3, OG::_Update, &Callback::Update);
        if (!res) CRITICAL("FPP Update Hook Not Installed");

        return res;
    }

#pragma endregion

#pragma region  // TPP Callback

    bool CameraHandler::TPP::Callback::CanProcess(RE::ThirdPersonState *a_this, RE::InputEvent *a_event) {
        if (ModSettings::Parkour_Enabled) {
            if (RuntimeVariables::ParkourInProgress) {
                return false;
            }
        }

        return OG::_CanProcess(a_this, a_event);
    }
    void CameraHandler::TPP::Callback::Begin(RE::ThirdPersonState *a_this) {
        OG::_Begin(a_this);
    }
    void CameraHandler::TPP::Callback::End(RE::ThirdPersonState *a_this) {
        // On cam state exit, invalidate vars. FPP or TPP will pick up and update when re-entered.
        Parkouring::InvalidateVars();
        auto menu = Scaleform::SkyParkourMenu::GetSingleton();
        if (menu) menu->ScaleToFirstPerson();

        OG::_End(a_this);
    }
    void CameraHandler::TPP::Callback::Update(RE::ThirdPersonState *a_this, RE::BSTSmartPointer<RE::TESCameraState> &a_nextState) {
        auto menu = Scaleform::SkyParkourMenu::GetSingleton();
        if (menu) menu->ScaleToThirdPersonZoom(a_this->currentZoomOffset);

        if (RuntimeVariables::ParkourInProgress) {
            const auto ctrl = GET_PLAYER->GetCharController();

            /* TDM swim pitch angle thing */
            if (Compatibility::TrueDirectionalMovement::found) {
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

#pragma endregion

#pragma region  // FPP Callback

    bool CameraHandler::FPP::Callback::CanProcess(RE::FirstPersonState *a_this, RE::InputEvent *a_event) {
        if (ModSettings::Parkour_Enabled) {
            if (RuntimeVariables::ParkourInProgress) {
                return false;
            }
        }

        return OG::_CanProcess(a_this, a_event);
    }
    void CameraHandler::FPP::Callback::Begin(RE::FirstPersonState *a_this) {
        OG::_Begin(a_this);
    }
    void CameraHandler::FPP::Callback::End(RE::FirstPersonState *a_this) {
        // On cam state exit, invalidate vars. FPP or TPP will pick up and update when re-entered.
        Parkouring::InvalidateVars();

        OG::_End(a_this);
    }
    void CameraHandler::FPP::Callback::Update(RE::FirstPersonState *a_this, RE::BSTSmartPointer<RE::TESCameraState> &a_nextState) {
        namespace rt = RuntimeVariables;
        if (rt::ParkourInProgress || rt::SlideOngoing) {
            /* Clamp Player looking angle to prevent weird visuals */

            const auto clamp = rt::ParkourInProgress ? Vertical_Clamp_Angle_Parkour : Vertical_Clamp_Angle_Slide;
            const auto player = GET_PLAYER;

            /* Vert */
            auto &vertAngle = player->data.angle.x;
            if (vertAngle > clamp) {
                vertAngle = clamp;
            }
            else if (vertAngle < -clamp) {
                vertAngle = -clamp;
            }

            /* Horz */
            auto &camAngle = a_this->sittingRotation;
            camAngle = 0.0f;
        }

        OG::_Update(a_this, a_nextState);
    }

#pragma endregion
}  // namespace Hooks