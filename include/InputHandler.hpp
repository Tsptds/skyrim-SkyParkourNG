#pragma once
#include "References.h"

// Taken from Skyrim Souls RE -> https://github.com/Vermunds/SkyrimSoulsRE.git
namespace Hooks {

    template <class T>
    class InputHandlerEx : public T {
        public:
            static InputHandlerEx* GetSingleton() {
                static InputHandlerEx singleton;
                return &singleton;
            }

            InputHandlerEx() = default;
            ~InputHandlerEx() = default;

            using CanProcess_t = decltype(&T::CanProcess);
            using ProcessButton_t = decltype(&T::ProcessButton);

            // Separate static variables for each hook type. Default value is 0.
            static inline REL::Relocation<CanProcess_t> _CanProcessJump;
            static inline REL::Relocation<ProcessButton_t> _ProcessButtonJump;
            static inline REL::Relocation<CanProcess_t> _CanProcessSneak;
            static inline REL::Relocation<CanProcess_t> _CanProcessMovement;
            static inline REL::Relocation<CanProcess_t> _CanProcessActivate;

            bool CanProcess_Jump(RE::InputEvent* a_event);
            void ProcessButton_Jump(RE::ButtonEvent* a_event, RE::PlayerControlsData* a_data);
            bool CanProcess_Sneak(RE::InputEvent* a_event);
            bool CanProcess_Movement(RE::InputEvent* a_event);
            bool CanProcess_Activate(RE::InputEvent* a_event);

            static bool InstallJumpHook();
            static bool InstallProcessJumpHook();
            static bool InstallSneakHook();
            static bool InstallMovementHook();
            static bool InstallActivateHook();
    };

    /* Hooks */
    template <class T>
    inline bool InputHandlerEx<T>::CanProcess_Jump(RE::InputEvent* a_event) {
        if (ModSettings::ModEnabled) {
            if (ModSettings::UsePresetParkourKey && ModSettings::PresetParkourKey == ModSettings::ParkourKeyOptions::kJump &&
                ModSettings::parkourDelay == 0 && RuntimeVariables::selectedLedgeType != ParkourType::NoLedge) {
                //logger::info("Prevented Jump");

                return false;
            }

            if (RuntimeVariables::ParkourInProgress) {
                return false;
            }
        }

        return _CanProcessJump(this, a_event);
    }

    template <class T>
    void InputHandlerEx<T>::ProcessButton_Jump(RE::ButtonEvent* a_event, RE::PlayerControlsData* a_data) {
        if (ModSettings::ModEnabled && !ParkourUtility::IsOnMount()) {
            if (ModSettings::UsePresetParkourKey && ModSettings::PresetParkourKey == ModSettings::ParkourKeyOptions::kJump) {
                auto btn = a_event->AsButtonEvent();
                if (btn && btn->QUserEvent() == "Jump" && ModSettings::parkourDelay != 0.0f) {
                    if (btn->IsDown()) {
                        return;
                    }
                    else if (btn->IsUp()) {
                        float held = btn->HeldDuration();
                        auto dev = btn->GetDevice();
                        auto id = btn->GetIDCode();

                        // create a delayed Down
                        RE::ButtonEvent* downEvt =
                            (held < ModSettings::parkourDelay) ? RE::ButtonEvent::Create(dev, "Jump", id, 1.0f, 0.0f) : nullptr;
                        // for a tap, also create a delayed Up
                        RE::ButtonEvent* upEvt = downEvt ? RE::ButtonEvent::Create(dev, "Jump", id, 0, held) : nullptr;

                        if (downEvt || upEvt) {
                            _THREAD_POOL.enqueue([this, downEvt, upEvt, a_data]() {
                                SKSE::GetTaskInterface()->AddTask([this, downEvt, upEvt, a_data]() {
                                    if (downEvt) {
                                        _ProcessButtonJump(this, downEvt, a_data);
                                        delete downEvt;
                                    }
                                    if (upEvt) {
                                        _ProcessButtonJump(this, upEvt, a_data);
                                        delete upEvt;
                                    }
                                });
                            });
                            return;  // don’t let the engine see the original Up
                        }
                    }
                }
            }
        }

        _ProcessButtonJump(this, a_event, a_data);
    }

    template <class T>
    inline bool InputHandlerEx<T>::CanProcess_Sneak(RE::InputEvent* a_event) {
        if (ModSettings::ModEnabled) {
            if (RuntimeVariables::ParkourInProgress) {
                return false;
            }
        }

        return _CanProcessSneak(this, a_event);
    }

    template <class T>
    inline bool InputHandlerEx<T>::CanProcess_Movement(RE::InputEvent* a_event) {
        if (ModSettings::ModEnabled) {
            if (RuntimeVariables::ParkourInProgress) {
                return false;
            }
        }

        return _CanProcessMovement(this, a_event);
    }

    template <class T>
    inline bool InputHandlerEx<T>::CanProcess_Activate(RE::InputEvent* a_event) {
        if (ModSettings::ModEnabled) {
            if (RuntimeVariables::ParkourInProgress) {
                return false;
            }
        }

        return _CanProcessActivate(this, a_event);
    }

    /* Install */
    template <class T>
    inline bool InputHandlerEx<T>::InstallJumpHook() {
        auto a_vtbl = REL::Relocation<std::uintptr_t>(RE::VTABLE_JumpHandler[0]);
        std::uint64_t a_offset = 0x1;

        _CanProcessJump = a_vtbl.write_vfunc(a_offset, &InputHandlerEx<T>::CanProcess_Jump);

        if (!_CanProcessJump.address()) {
            logger::critical("Jump Hook Not Installed");
            return false;
        }

        return true;
    }

    template <class T>
    inline bool InputHandlerEx<T>::InstallProcessJumpHook() {
        auto a_vtbl = REL::Relocation<std::uintptr_t>(RE::VTABLE_JumpHandler[0]);
        std::uint64_t a_offset = 0x4;

        _ProcessButtonJump = a_vtbl.write_vfunc(a_offset, &InputHandlerEx<T>::ProcessButton_Jump);

        if (!_ProcessButtonJump.address()) {
            logger::critical("Jump Process Hook Not Installed");
            return false;
        }
        return true;
    }

    template <class T>
    inline bool InputHandlerEx<T>::InstallSneakHook() {
        auto a_vtbl = REL::Relocation<std::uintptr_t>(RE::VTABLE_SneakHandler[0]);
        std::uint64_t a_offset = 0x1;

        _CanProcessSneak = a_vtbl.write_vfunc(a_offset, &InputHandlerEx<T>::CanProcess_Sneak);

        if (!_CanProcessSneak.address()) {
            logger::critical("Sneak Hook Not Installed");
            return false;
        }
        return true;
    }

    template <class T>
    inline bool InputHandlerEx<T>::InstallMovementHook() {
        auto a_vtbl = REL::Relocation<std::uintptr_t>(RE::VTABLE_MovementHandler[0]);
        std::uint64_t a_offset = 0x1;

        _CanProcessMovement = a_vtbl.write_vfunc(a_offset, &InputHandlerEx<T>::CanProcess_Movement);

        if (!_CanProcessMovement.address()) {
            logger::critical("Movement Hook Not Installed");
            return false;
        }
        return true;
    }

    template <class T>
    inline bool InputHandlerEx<T>::InstallActivateHook() {
        auto a_vtbl = REL::Relocation<std::uintptr_t>(RE::VTABLE_ActivateHandler[0]);
        std::uint64_t a_offset = 0x1;

        _CanProcessActivate = a_vtbl.write_vfunc(a_offset, &InputHandlerEx<T>::CanProcess_Activate);

        if (!_CanProcessActivate.address()) {
            logger::critical("Movement Hook Not Installed");
            return false;
        }
        return true;
    }
}  // namespace Hooks