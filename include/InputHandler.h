#pragma once
#include "References.h"

// Taken from Skyrim Souls RE -> https://github.com/Vermunds/SkyrimSoulsRE.git
namespace JumpHook {

    template <class T>
    class InputHandlerEx : public T {
        public:
            using CanProcess_t = decltype(&T::CanProcess);
            static inline REL::Relocation<CanProcess_t> _CanProcess;

            bool CanProcess_Hook(RE::InputEvent* a_event);

            static void InstallHook(REL::Relocation<std::uintptr_t> a_vtbl, std::uint64_t a_offset);
    };

    template <class T>
    inline bool InputHandlerEx<T>::CanProcess_Hook(RE::InputEvent* a_event) {
        if (ModSettings::ModEnabled) {
            if (ModSettings::UsePresetParkourKey && ModSettings::PresetParkourKey == ModSettings::ParkourKeyOptions::kJump &&
                ModSettings::parkourDelay == 0) {
                if (RuntimeVariables::selectedLedgeType != ParkourType::NoLedge) {
                    //logger::info("Prevented Jump");
                    return false;
                }
            }
        }

        return _CanProcess(this, a_event);
    }

    template <class T>
    inline void InputHandlerEx<T>::InstallHook(REL::Relocation<std::uintptr_t> a_vtbl, std::uint64_t a_offset) {
        _CanProcess = a_vtbl.write_vfunc(a_offset, &CanProcess_Hook);
        logger::info("Jump Hook Installed");
    }
}  // namespace JumpHook