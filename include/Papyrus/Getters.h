#pragma once

namespace SkyParkour_Papyrus {
    class Getters {
        public:
            static bool GetEnableMod(RE::StaticFunctionTag *);

            static bool GetShowIndicators(RE::StaticFunctionTag *);

            static float GetPlaybackSpeed(RE::StaticFunctionTag *);

            static bool GetEnableStaminaSystem(RE::StaticFunctionTag *);

            static bool GetMustHaveStamina(RE::StaticFunctionTag *);

            static float GetBaseStaminaDamage(RE::StaticFunctionTag *);

            static bool GetUsePresetKey(RE::StaticFunctionTag *);

            static int32_t GetCustomParkourKey(RE::StaticFunctionTag *);

            static int32_t GetPresetParkourKey(RE::StaticFunctionTag *);

            static float GetParkourDelay(RE::StaticFunctionTag *);

            static bool GetSmartSteps(RE::StaticFunctionTag *);

            static bool GetSmartVault(RE::StaticFunctionTag *);

            static bool GetSmartClimb(RE::StaticFunctionTag *);
    };
}  // namespace SkyParkour_Papyrus