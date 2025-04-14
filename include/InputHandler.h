#pragma once
#include "References.h"

// Taken from Skyrim Souls RE -> https://github.com/Vermunds/SkyrimSoulsRE.git
namespace Hooks {

    template <class T>
    class InputHandlerEx : public T {
        public:
            using CanProcess_t = decltype(&T::CanProcess);
            // Separate static variables for each hook type.
            static inline REL::Relocation<CanProcess_t> _CanProcessJump;
            static inline REL::Relocation<CanProcess_t> _CanProcessSneak;

            bool CanProcess_Jump(RE::InputEvent* a_event);
            bool CanProcess_Sneak(RE::InputEvent* a_event);

            static void InstallJumpHook();
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
        }

        if (RuntimeVariables::ParkourEndQueued) {
            return false;
        }

        return _CanProcessJump(this, a_event);
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
    inline void InputHandlerEx<T>::InstallSneakHook() {
        auto a_vtbl = REL::Relocation<std::uintptr_t>(RE::VTABLE_SneakHandler[0]);
        std::uint64_t a_offset = 0x1;
        _CanProcessSneak = a_vtbl.write_vfunc(a_offset, &InputHandlerEx<T>::CanProcess_Sneak);
        logger::info("Sneak Hook Installed");
    }
}  // namespace Hooks