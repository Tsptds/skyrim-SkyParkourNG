#pragma once
#include "_References/ModSettings.h"
#include "_References/RuntimeVariables.h"
#include "_References/ParkourType.h"
#include "_References/Compatibility.h"

#include "Util/ParkourUtility.h"
#include "Util/HookingUtil.hpp"

namespace Hooks {

    class InputHandler {
        public:
            static bool InstallInputHooks();

        private:
            struct Signatures {
                    using ProcessButton_Jump_t = void(RE::JumpHandler *a_this, RE::ButtonEvent *a_event, RE::PlayerControlsData *a_data);

                    using CanProcess_Jump_t = bool(RE::JumpHandler *a_this, RE::InputEvent *a_event);
                    using CanProcess_Sneak_t = bool(RE::SneakHandler *a_this, RE::InputEvent *a_event);
                    using CanProcess_Movement_t = bool(RE::MovementHandler *a_this, RE::InputEvent *a_event);
                    using CanProcess_Activate_t = bool(RE::ActivateHandler *a_this, RE::InputEvent *a_event);
                    using CanProcess_POV_t = bool(RE::TogglePOVHandler *a_this, RE::InputEvent *a_event);
                    using CanProcess_Weapon_t = bool(RE::ReadyWeaponHandler *a_this, RE::InputEvent *a_event);
                    using CanProcess_Look_t = bool(RE::LookHandler *a_this, RE::InputEvent *a_event);
                    using CanProcess_Sprint_t = bool(RE::SprintHandler *a_this, RE::InputEvent *a_event);
            };

            struct Install {
                    static bool ProcessButton_Jump();

                    static bool CanProcess_Jump();
                    static bool CanProcess_Sneak();
                    static bool CanProcess_Movement();
                    static bool CanProcess_Activate();
                    static bool CanProcess_POV();
                    static bool CanProcess_Weapon();
                    static bool CanProcess_Look();
                    static bool CanProcess_Sprint();
            };

            struct Callback {
                private:
                    using _S = Signatures;

                public:
                    static _S::ProcessButton_Jump_t ProcessButton_Jump;

                    static _S::CanProcess_Jump_t CanProcess_Jump;
                    static _S::CanProcess_Sneak_t CanProcess_Sneak;
                    static _S::CanProcess_Movement_t CanProcess_Movement;
                    static _S::CanProcess_Activate_t CanProcess_Activate;
                    static _S::CanProcess_POV_t CanProcess_POV;
                    static _S::CanProcess_Weapon_t CanProcess_Weapon;
                    static _S::CanProcess_Look_t CanProcess_Look;
                    static _S::CanProcess_Sprint_t CanProcess_Sprint;
            };

            struct OG {
                private:
                    using _S = Signatures;

                public:
                    static inline REL::Relocation<_S::ProcessButton_Jump_t *> _ProcessButtonJump;

                    static inline REL::Relocation<_S::CanProcess_Jump_t *> _CanProcessJump;
                    static inline REL::Relocation<_S::CanProcess_Sneak_t *> _CanProcessSneak;
                    static inline REL::Relocation<_S::CanProcess_Movement_t *> _CanProcessMovement;
                    static inline REL::Relocation<_S::CanProcess_Activate_t *> _CanProcessActivate;
                    static inline REL::Relocation<_S::CanProcess_POV_t *> _CanProcessPOV;
                    static inline REL::Relocation<_S::CanProcess_Weapon_t *> _CanProcessWeapon;
                    static inline REL::Relocation<_S::CanProcess_Look_t *> _CanProcessLook;
                    static inline REL::Relocation<_S::CanProcess_Sprint_t *> _CanProcessSprint;
            };
    };

#pragma region  // Callbacks
    bool InputHandler::Callback::CanProcess_Jump(RE::JumpHandler *a_this, RE::InputEvent *a_event) {
        if (ModSettings::Parkour_Enabled) {
            if (ModSettings::Use_Preset_Parkour_Key && ModSettings::Preset_Parkour_Key == PARKOUR_PRESET_KEYS::kJump &&
                ModSettings::Parkour_Delay == 0 && RuntimeVariables::selectedLedgeType != ParkourType::NoLedge) {
                //LOG("Prevented Jump");

                return false;
            }

            if (RuntimeVariables::ParkourInProgress) return false;
            if (RuntimeVariables::SlideOngoing) return false;
        }

        return OG::_CanProcessJump(a_this, a_event);
    }

    void InputHandler::Callback::ProcessButton_Jump(RE::JumpHandler *a_this, RE::ButtonEvent *a_event, RE::PlayerControlsData *a_data) {
        if (ModSettings::Parkour_Enabled && !ParkourUtility::IsOnMount()) {
            if (ModSettings::Use_Preset_Parkour_Key && ModSettings::Preset_Parkour_Key == PARKOUR_PRESET_KEYS::kJump) {
                const auto &btn = a_event->AsButtonEvent();
                if (btn && btn->QUserEvent() == "Jump" && ModSettings::Parkour_Delay != 0.0f) {
                    if (btn->IsDown()) {
                        return;
                    }
                    else if (btn->IsUp()) {
                        const float &held = btn->HeldDuration();
                        const auto &dev = btn->GetDevice();
                        const auto &id = btn->GetIDCode();

                        // create a delayed Down
                        RE::ButtonEvent *downEvt =
                            (held < ModSettings::Parkour_Delay) ? RE::ButtonEvent::Create(dev, "Jump", id, 1.0f, 0.0f) : nullptr;
                        // for a tap, also create a delayed Up
                        RE::ButtonEvent *upEvt = downEvt ? RE::ButtonEvent::Create(dev, "Jump", id, 0, held) : nullptr;

                        if (downEvt || upEvt) {
                            if (downEvt) {
                                OG::_ProcessButtonJump(a_this, downEvt, a_data);
                                delete downEvt;
                            }
                            if (upEvt) {
                                OG::_ProcessButtonJump(a_this, upEvt, a_data);
                                delete upEvt;
                            }

                            return;  // don’t let the engine see the original Up
                        }
                    }
                }
            }
        }

        OG::_ProcessButtonJump(a_this, a_event, a_data);
    }

    bool InputHandler::Callback::CanProcess_Sneak(RE::SneakHandler *a_this, RE::InputEvent *a_event) {
        if (ModSettings::Parkour_Enabled) {
            if (RuntimeVariables::ParkourInProgress) return false;
        }

        if (ModSettings::Crouch_Slide_Enabled) {
            if (RuntimeVariables::SlideOngoing) return false;
            const auto &pl = GET_PLAYER;
            if (pl->AsActorState()->IsSprinting()) return false;
            if (pl->IsInMidair()) return false;
        }

        return OG::_CanProcessSneak(a_this, a_event);
    }

    bool InputHandler::Callback::CanProcess_Movement(RE::MovementHandler *a_this, RE::InputEvent *a_event) {
        if (ModSettings::Parkour_Enabled) {
            if (RuntimeVariables::ParkourInProgress) {
                /**/
                /* Recovery Frame Early Exit Logic */
                if (RuntimeVariables::RecoveryFramesActive) {
                    bool res = OG::_CanProcessMovement(a_this, a_event);
                    if (res) {
                        GET_PLAYER->NotifyAnimationGraph(SPPF_INTERRUPT);
                    }
                    return res;
                }
                /**/
                else {
                    return false;
                }
            }
        }

        if (ModSettings::Crouch_Slide_Enabled) {
            if (RuntimeVariables::SlideOngoing) return false;
        }

        return OG::_CanProcessMovement(a_this, a_event);
    }

    bool InputHandler::Callback::CanProcess_Activate(RE::ActivateHandler *a_this, RE::InputEvent *a_event) {
        if (ModSettings::Parkour_Enabled) {
            if (RuntimeVariables::ParkourInProgress) return false;
            if (RuntimeVariables::SlideOngoing) return false;
        }

        return OG::_CanProcessActivate(a_this, a_event);
    }

    bool InputHandler::Callback::CanProcess_POV(RE::TogglePOVHandler *a_this, RE::InputEvent *a_event) {
        /* This disables holding F and setting the zoom thing */
        if (ModSettings::Parkour_Enabled) {
            if (RuntimeVariables::ParkourInProgress) return false;
        }

        return OG::_CanProcessPOV(a_this, a_event);
    }

    bool InputHandler::Callback::CanProcess_Weapon(RE::ReadyWeaponHandler *a_this, RE::InputEvent *a_event) {
        /* Stops Weapon Ready button process, mostly fixes weapon state getting stuck and redrawn */
        if (ModSettings::Parkour_Enabled) {
            if (RuntimeVariables::ParkourInProgress) return false;
            if (RuntimeVariables::SlideOngoing) return false;
        }

        return OG::_CanProcessWeapon(a_this, a_event);
    }

    bool InputHandler::Callback::CanProcess_Look(RE::LookHandler *a_this, RE::InputEvent *a_event) {
        if (ModSettings::Parkour_Enabled) {
            if (RuntimeVariables::ParkourInProgress) return false;
        }

        return OG::_CanProcessLook(a_this, a_event);
    }

    bool InputHandler::Callback::CanProcess_Sprint(RE::SprintHandler *a_this, RE::InputEvent *a_event) {
        if (ModSettings::Crouch_Slide_Enabled) {
            if (RuntimeVariables::SlideOngoing) return false;
        }

        return OG::_CanProcessSprint(a_this, a_event);
    }

#pragma endregion

#pragma region  // Install
    bool InputHandler::InstallInputHooks() {
        bool res = true;

        res &= Install::ProcessButton_Jump();

        res &= Install::CanProcess_Jump();
        res &= Install::CanProcess_Sneak();
        res &= Install::CanProcess_Movement();
        res &= Install::CanProcess_Activate();
        res &= Install::CanProcess_POV();
        res &= Install::CanProcess_Weapon();

        if (!Compatibility::TrueDirectionalMovement::found) {
            res &= Install::CanProcess_Look(); /* Block rotate camera, used for compatibility */
        }

        return res;
    }

    bool InputHandler::Install::CanProcess_Jump() {
        REL::Relocation<uintptr_t> vtbl{RE::VTABLE_JumpHandler[0]};

        const bool res = Hooking::InstallVFuncHook(vtbl, 0x1, OG::_CanProcessJump, &Callback::CanProcess_Jump);

        if (!res) CRITICAL("Jump Hook Not Installed");
        return res;
    }

    bool InputHandler::Install::ProcessButton_Jump() {
        REL::Relocation<uintptr_t> vtbl{RE::VTABLE_JumpHandler[0]};

        const bool res = Hooking::InstallVFuncHook(vtbl, 0x4, OG::_ProcessButtonJump, &Callback::ProcessButton_Jump);

        if (!res) CRITICAL("Jump Process Hook Not Installed");
        return res;
    }

    bool InputHandler::Install::CanProcess_Sneak() {
        REL::Relocation<std::uintptr_t> vtbl{RE::VTABLE_SneakHandler[0]};

        const bool res = Hooking::InstallVFuncHook(vtbl, 0x1, OG::_CanProcessSneak, &Callback::CanProcess_Sneak);

        if (!res) CRITICAL("Sneak Hook Not Installed");
        return res;
    }

    bool InputHandler::Install::CanProcess_Movement() {
        REL::Relocation<std::uintptr_t> vtbl{RE::VTABLE_MovementHandler[0]};

        const bool res = Hooking::InstallVFuncHook(vtbl, 0x1, OG::_CanProcessMovement, &Callback::CanProcess_Movement);

        if (!res) CRITICAL("Movement Hook Not Installed");
        return res;
    }

    bool InputHandler::Install::CanProcess_Activate() {
        REL::Relocation<std::uintptr_t> vtbl{RE::VTABLE_ActivateHandler[0]};

        const bool res = Hooking::InstallVFuncHook(vtbl, 0x1, OG::_CanProcessActivate, &Callback::CanProcess_Activate);

        if (!res) CRITICAL("Movement Hook Not Installed");
        return res;
    }

    bool InputHandler::Install::CanProcess_POV() {
        REL::Relocation<std::uintptr_t> vtbl{RE::VTABLE_TogglePOVHandler[0]};

        const bool res = Hooking::InstallVFuncHook(vtbl, 0x1, OG::_CanProcessPOV, &Callback::CanProcess_POV);

        if (!res) CRITICAL("POV Hook Not Installed");
        return res;
    }

    bool InputHandler::Install::CanProcess_Weapon() {
        REL::Relocation<std::uintptr_t> vtbl{RE::VTABLE_ReadyWeaponHandler[0]};

        const bool res = Hooking::InstallVFuncHook(vtbl, 0x1, OG::_CanProcessWeapon, &Callback::CanProcess_Weapon);

        if (!res) CRITICAL("Weapon Hook Not Installed");
        return res;
    }

    bool InputHandler::Install::CanProcess_Look() {
        REL::Relocation<std::uintptr_t> vtbl{RE::VTABLE_LookHandler[0]};

        const bool res = Hooking::InstallVFuncHook(vtbl, 0x1, OG::_CanProcessLook, &Callback::CanProcess_Look);

        if (!res) CRITICAL("Look Hook Not Installed");
        return res;
    }
    bool InputHandler::Install::CanProcess_Sprint() {
        REL::Relocation<std::uintptr_t> vtbl{RE::VTABLE_SprintHandler[0]};

        const bool res = Hooking::InstallVFuncHook(vtbl, 0x1, OG::_CanProcessSprint, &Callback::CanProcess_Sprint);

        if (!res) CRITICAL("Sprint Hook Not Installed");
        return res;
    }
#pragma endregion
}  // namespace Hooks