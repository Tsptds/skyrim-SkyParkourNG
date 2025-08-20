#pragma once
#include "_References/ModSettings.h"
#include "_References/RuntimeVariables.h"
#include "_References/ParkourType.h"
#include "_References/Compatibility.h"

#include "Util/ParkourUtility.h"

namespace Hooks {

    class InputHandlerEx {
        public:
            static InputHandlerEx* GetSingleton() {
                static InputHandlerEx singleton;
                return &singleton;
            }

            static bool InstallInputHooks();

        private:
            static bool InstallJumpHook();
            static bool InstallProcessJumpHook();
            static bool InstallSneakHook();
            static bool InstallMovementHook();
            static bool InstallActivateHook();
            static bool InstallPOVHook();
            static bool InstallWeaponHook();
            static bool InstallLookHook();

            /* Callbacks */
            static bool CanProcess_Jump(RE::JumpHandler* a_this, RE::InputEvent* a_event);
            static void ProcessButton_Jump(RE::JumpHandler* a_this, RE::ButtonEvent* a_event, RE::PlayerControlsData* a_data);
            static bool CanProcess_Sneak(RE::SneakHandler* a_this, RE::InputEvent* a_event);
            static bool CanProcess_Movement(RE::MovementHandler* a_this, RE::InputEvent* a_event);
            static bool CanProcess_Activate(RE::ActivateHandler* a_this, RE::InputEvent* a_event);
            static bool CanProcess_POV(RE::TogglePOVHandler* a_this, RE::InputEvent* a_event);
            static bool CanProcess_Weapon(RE::ReadyWeaponHandler* a_this, RE::InputEvent* a_event);
            static bool CanProcess_Look(RE::LookHandler* a_this, RE::InputEvent* a_event);

            /* OG */
            static inline REL::Relocation<decltype(CanProcess_Jump)> _CanProcessJump;
            static inline REL::Relocation<decltype(ProcessButton_Jump)> _ProcessButtonJump;
            static inline REL::Relocation<decltype(CanProcess_Sneak)> _CanProcessSneak;
            static inline REL::Relocation<decltype(CanProcess_Movement)> _CanProcessMovement;
            static inline REL::Relocation<decltype(CanProcess_Activate)> _CanProcessActivate;
            static inline REL::Relocation<decltype(CanProcess_POV)> _CanProcessPOV;
            static inline REL::Relocation<decltype(CanProcess_Weapon)> _CanProcessWeapon;
            static inline REL::Relocation<decltype(CanProcess_Look)> _CanProcessLook;
    };

    /* Callbacks */
    bool InputHandlerEx::CanProcess_Jump(RE::JumpHandler* a_this, RE::InputEvent* a_event) {
        if (ModSettings::Mod_Enabled) {
            if (ModSettings::Use_Preset_Parkour_Key && ModSettings::Preset_Parkour_Key == PARKOUR_PRESET_KEYS::kJump &&
                ModSettings::Parkour_Delay == 0 && RuntimeVariables::selectedLedgeType != ParkourType::NoLedge) {
                //LOG("Prevented Jump");

                return false;
            }

            if (RuntimeVariables::ParkourInProgress) {
                return false;
            }
        }

        return _CanProcessJump(a_this, a_event);
    }

    void InputHandlerEx::ProcessButton_Jump(RE::JumpHandler* a_this, RE::ButtonEvent* a_event, RE::PlayerControlsData* a_data) {
        if (ModSettings::Mod_Enabled && !ParkourUtility::IsOnMount()) {
            if (ModSettings::Use_Preset_Parkour_Key && ModSettings::Preset_Parkour_Key == PARKOUR_PRESET_KEYS::kJump) {
                const auto& btn = a_event->AsButtonEvent();
                if (btn && btn->QUserEvent() == "Jump" && ModSettings::Parkour_Delay != 0.0f) {
                    if (btn->IsDown()) {
                        return;
                    }
                    else if (btn->IsUp()) {
                        const float& held = btn->HeldDuration();
                        const auto& dev = btn->GetDevice();
                        const auto& id = btn->GetIDCode();

                        // create a delayed Down
                        RE::ButtonEvent* downEvt =
                            (held < ModSettings::Parkour_Delay) ? RE::ButtonEvent::Create(dev, "Jump", id, 1.0f, 0.0f) : nullptr;
                        // for a tap, also create a delayed Up
                        RE::ButtonEvent* upEvt = downEvt ? RE::ButtonEvent::Create(dev, "Jump", id, 0, held) : nullptr;

                        if (downEvt || upEvt) {
                            if (downEvt) {
                                _ProcessButtonJump(a_this, downEvt, a_data);
                                delete downEvt;
                            }
                            if (upEvt) {
                                _ProcessButtonJump(a_this, upEvt, a_data);
                                delete upEvt;
                            }

                            return;  // don’t let the engine see the original Up
                        }
                    }
                }
            }
        }

        _ProcessButtonJump(a_this, a_event, a_data);
    }

    bool InputHandlerEx::CanProcess_Sneak(RE::SneakHandler* a_this, RE::InputEvent* a_event) {
        if (ModSettings::Mod_Enabled) {
            if (RuntimeVariables::ParkourInProgress) {
                return false;
            }
        }

        return _CanProcessSneak(a_this, a_event);
    }

    bool InputHandlerEx::CanProcess_Movement(RE::MovementHandler* a_this, RE::InputEvent* a_event) {
        if (ModSettings::Mod_Enabled) {
            if (RuntimeVariables::ParkourInProgress) {
                /**/
                /* Recovery Frame Early Exit Logic */
                if (RuntimeVariables::RecoveryFramesActive) {
                    bool res = _CanProcessMovement(a_this, a_event);
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

        return _CanProcessMovement(a_this, a_event);
    }

    bool InputHandlerEx::CanProcess_Activate(RE::ActivateHandler* a_this, RE::InputEvent* a_event) {
        if (ModSettings::Mod_Enabled) {
            if (RuntimeVariables::ParkourInProgress) {
                return false;
            }
        }

        return _CanProcessActivate(a_this, a_event);
    }

    bool InputHandlerEx::CanProcess_POV(RE::TogglePOVHandler* a_this, RE::InputEvent* a_event) {
        /* This disables holding F and setting the zoom thing */
        if (ModSettings::Mod_Enabled) {
            if (RuntimeVariables::ParkourInProgress) {
                return false;
            }
        }

        return _CanProcessPOV(a_this, a_event);
    }

    bool InputHandlerEx::CanProcess_Weapon(RE::ReadyWeaponHandler* a_this, RE::InputEvent* a_event) {
        /* Stops Weapon Ready button process, mostly fixes weapon state getting stuck and redrawn */
        if (ModSettings::Mod_Enabled) {
            if (RuntimeVariables::ParkourInProgress) {
                return false;
            }
        }

        return _CanProcessWeapon(a_this, a_event);
    }

    bool InputHandlerEx::CanProcess_Look(RE::LookHandler* a_this, RE::InputEvent* a_event) {
        if (ModSettings::Mod_Enabled) {
            if (RuntimeVariables::ParkourInProgress) {
                return false;
            }
        }

        return _CanProcessLook(a_this, a_event);
    }

    /* Install */
    bool InputHandlerEx::InstallInputHooks() {
        bool res = false;

        res &= InstallJumpHook();
        res &= InstallProcessJumpHook();
        res &= InstallSneakHook();
        res &= InstallMovementHook();
        res &= InstallActivateHook();
        res &= InstallPOVHook();
        res &= InstallWeaponHook();

        if (!Compatibility::TrueDirectionalMovement) {
            res &= Hooks::InputHandlerEx::InstallLookHook(); /* Block rotate camera, use for compatibility */
        }

        return res;
    }

    bool InputHandlerEx::InstallJumpHook() {
        auto a_vtbl = REL::Relocation<std::uintptr_t>(RE::VTABLE_JumpHandler[0]);
        std::uint64_t a_offset = 0x1;

        _CanProcessJump = a_vtbl.write_vfunc(a_offset, &InputHandlerEx::CanProcess_Jump);

        if (!_CanProcessJump.address()) {
            CRITICAL("Jump Hook Not Installed");
            return false;
        }

        return true;
    }

    bool InputHandlerEx::InstallProcessJumpHook() {
        auto a_vtbl = REL::Relocation<std::uintptr_t>(RE::VTABLE_JumpHandler[0]);
        std::uint64_t a_offset = 0x4;

        _ProcessButtonJump = a_vtbl.write_vfunc(a_offset, &InputHandlerEx::ProcessButton_Jump);

        if (!_ProcessButtonJump.address()) {
            CRITICAL("Jump Process Hook Not Installed");
            return false;
        }
        return true;
    }

    bool InputHandlerEx::InstallSneakHook() {
        auto a_vtbl = REL::Relocation<std::uintptr_t>(RE::VTABLE_SneakHandler[0]);
        std::uint64_t a_offset = 0x1;

        _CanProcessSneak = a_vtbl.write_vfunc(a_offset, &InputHandlerEx::CanProcess_Sneak);

        if (!_CanProcessSneak.address()) {
            CRITICAL("Sneak Hook Not Installed");
            return false;
        }
        return true;
    }

    bool InputHandlerEx::InstallMovementHook() {
        auto a_vtbl = REL::Relocation<std::uintptr_t>(RE::VTABLE_MovementHandler[0]);
        std::uint64_t a_offset = 0x1;

        _CanProcessMovement = a_vtbl.write_vfunc(a_offset, &InputHandlerEx::CanProcess_Movement);

        if (!_CanProcessMovement.address()) {
            CRITICAL("Movement Hook Not Installed");
            return false;
        }
        return true;
    }

    bool InputHandlerEx::InstallActivateHook() {
        auto a_vtbl = REL::Relocation<std::uintptr_t>(RE::VTABLE_ActivateHandler[0]);
        std::uint64_t a_offset = 0x1;

        _CanProcessActivate = a_vtbl.write_vfunc(a_offset, &InputHandlerEx::CanProcess_Activate);

        if (!_CanProcessActivate.address()) {
            CRITICAL("Movement Hook Not Installed");
            return false;
        }
        return true;
    }

    bool InputHandlerEx::InstallPOVHook() {
        auto a_vtbl = REL::Relocation<std::uintptr_t>(RE::VTABLE_TogglePOVHandler[0]);
        std::uint64_t a_offset = 0x1;

        _CanProcessPOV = a_vtbl.write_vfunc(a_offset, &InputHandlerEx::CanProcess_POV);

        if (!_CanProcessPOV.address()) {
            CRITICAL("POV Hook Not Installed");
            return false;
        }
        return true;
    }

    bool InputHandlerEx::InstallWeaponHook() {
        auto a_vtbl = REL::Relocation<std::uintptr_t>(RE::VTABLE_ReadyWeaponHandler[0]);
        std::uint64_t a_offset = 0x1;

        _CanProcessWeapon = a_vtbl.write_vfunc(a_offset, &InputHandlerEx::CanProcess_Weapon);

        if (!_CanProcessWeapon.address()) {
            CRITICAL("Weapon Hook Not Installed");
            return false;
        }
        return true;
    }

    bool InputHandlerEx::InstallLookHook() {
        auto a_vtbl = REL::Relocation<std::uintptr_t>(RE::VTABLE_LookHandler[0]);
        std::uint64_t a_offset = 0x1;

        _CanProcessLook = a_vtbl.write_vfunc(a_offset, &InputHandlerEx::CanProcess_Look);

        if (!_CanProcessLook.address()) {
            CRITICAL("Look Hook Not Installed");
            return false;
        }
        return true;
    }

}  // namespace Hooks