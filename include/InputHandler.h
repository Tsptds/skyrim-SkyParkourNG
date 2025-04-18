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
            // Separate static variables for each hook type.
            static inline REL::Relocation<CanProcess_t> _CanProcessJump;
            static inline REL::Relocation<ProcessButton_t> _ProcessButtonJump;
            static inline REL::Relocation<CanProcess_t> _CanProcessSneak;

            bool CanProcess_Jump(RE::InputEvent* a_event);
            void ProcessButton_Jump(RE::ButtonEvent* a_event, RE::PlayerControlsData* a_data);
            bool CanProcess_Sneak(RE::InputEvent* a_event);

            static void InstallJumpHook();
            static void InstallProcessJumpHook();
            static void InstallSneakHook();
    };

    template <class T>
    inline bool InputHandlerEx<T>::CanProcess_Jump(RE::InputEvent* a_event) {
        if (ModSettings::ModEnabled) {
            if (ModSettings::UsePresetParkourKey && ModSettings::PresetParkourKey == ModSettings::ParkourKeyOptions::kJump &&
                ModSettings::parkourDelay == 0 && RuntimeVariables::selectedLedgeType != ParkourType::NoLedge) {
                //logger::info("Prevented Jump");
                return false;
            }

            if (RuntimeVariables::ParkourEndQueued) {
                return false;
            }

           /* auto buttonEvent = a_event->AsButtonEvent();
            if (buttonEvent && buttonEvent->QUserEvent() == RE::UserEvents::GetSingleton()->jump) {
                if (!RuntimeVariables::ParkourEndQueued) {
                    logger::info("Block");
                    return false;
                } else {
                    return _CanProcessJump(this, a_event);
                }
            }*/
        }

        return _CanProcessJump(this, a_event);
    }

    template <class T>
    inline void InputHandlerEx<T>::ProcessButton_Jump(RE::ButtonEvent* a_event, RE::PlayerControlsData* a_data) {
        return _ProcessButtonJump(this, a_event, a_data);
    }

    template <class T>
    inline bool InputHandlerEx<T>::CanProcess_Sneak(RE::InputEvent* a_event) {
        if (ModSettings::ModEnabled) {
            if (RuntimeVariables::ParkourEndQueued) {
                return false;
            }
        }

        return _CanProcessSneak(this, a_event);
    }

    template <class T>
    inline void InputHandlerEx<T>::InstallJumpHook() {
        auto a_vtbl = REL::Relocation<std::uintptr_t>(RE::VTABLE_JumpHandler[0]);
        std::uint64_t a_offset = 0x1;
        _CanProcessJump = a_vtbl.write_vfunc(a_offset, &InputHandlerEx<T>::CanProcess_Jump);
        logger::info("Jump Hook Installed");
    }

    template <class T>
    inline void InputHandlerEx<T>::InstallProcessJumpHook() {
        auto a_vtbl = REL::Relocation<std::uintptr_t>(RE::VTABLE_JumpHandler[0]);
        std::uint64_t a_offset = 0x4;
        _ProcessButtonJump = a_vtbl.write_vfunc(a_offset, &InputHandlerEx<T>::ProcessButton_Jump);
        logger::info("Jump Process Hook Installed");
    }

    template <class T>
    inline void InputHandlerEx<T>::InstallSneakHook() {
        auto a_vtbl = REL::Relocation<std::uintptr_t>(RE::VTABLE_SneakHandler[0]);
        std::uint64_t a_offset = 0x1;
        _CanProcessSneak = a_vtbl.write_vfunc(a_offset, &InputHandlerEx<T>::CanProcess_Sneak);
        logger::info("Sneak Hook Installed");
    }
}  // namespace Hooks