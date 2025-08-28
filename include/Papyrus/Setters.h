#pragma once

namespace SkyParkour_Papyrus {

    class Setters {
        public:
            static void RegisterFuncs(RE::BSScript::IVirtualMachine *vm);

        private:
            static void SetEnableMod(RE::StaticFunctionTag *, bool value);
            static void SetShowIndicators(RE::StaticFunctionTag *, bool value);
            static void SetPlaybackSpeed(RE::StaticFunctionTag *, float value);
            static void SetEnableStaminaSystem(RE::StaticFunctionTag *, bool value);
            static void SetMustHaveStamina(RE::StaticFunctionTag *, bool value);
            static void SetBaseStaminaDamage(RE::StaticFunctionTag *, float value);
            static void SetUsePresetKey(RE::StaticFunctionTag *, bool value);
            static void SetCustomParkourKey(RE::StaticFunctionTag *, int32_t value);
            static void SetPresetParkourKey(RE::StaticFunctionTag *, int32_t value);
            static void SetParkourDelay(RE::StaticFunctionTag *, float value);
            static void SetSmartSteps(RE::StaticFunctionTag *, bool value);
            static void SetSmartVault(RE::StaticFunctionTag *, bool value);
            static void SetSmartClimb(RE::StaticFunctionTag *, bool value);

            static void save(const std::unique_ptr<CSimpleIniA> &ini);
    };
}  // namespace SkyParkour_Papyrus