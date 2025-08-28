#pragma once

namespace SkyParkour_Papyrus {

    class Translations {
        public:
            static void RegisterFuncs(RE::BSScript::IVirtualMachine *vm);

        private:
            struct Core {
                    inline static const char *Section = "Core";

                    static RE::BSFixedString Header(RE::StaticFunctionTag *);
                    static RE::BSFixedString OnOff(RE::StaticFunctionTag *);
                    static RE::BSFixedString Indicator(RE::StaticFunctionTag *);
                    static RE::BSFixedString PlaybackSpeed(RE::StaticFunctionTag *);
            };

            struct Input {
                    inline static const char *Section = "Input";

                    static RE::BSFixedString Header(RE::StaticFunctionTag *);
                    static RE::BSFixedString PresetKeyList(RE::StaticFunctionTag *);
                    static RE::BSFixedString PresetKey0(RE::StaticFunctionTag *);
                    static RE::BSFixedString PresetKey1(RE::StaticFunctionTag *);
                    static RE::BSFixedString PresetKey2(RE::StaticFunctionTag *);
                    static RE::BSFixedString CustomKey(RE::StaticFunctionTag *);
                    static RE::BSFixedString Delay(RE::StaticFunctionTag *);
            };

            struct Stamina {
                    inline static const char *Section = "Stamina";

                    static RE::BSFixedString Header(RE::StaticFunctionTag *);
                    static RE::BSFixedString StaminaSystem(RE::StaticFunctionTag *);
                    static RE::BSFixedString MustHaveStamina(RE::StaticFunctionTag *);
                    static RE::BSFixedString BaseStaminaCost(RE::StaticFunctionTag *);
            };

            struct SmartParkour {
                    inline static const char *Section = "SmartParkour";

                    static RE::BSFixedString Header(RE::StaticFunctionTag *);
                    static RE::BSFixedString Steps(RE::StaticFunctionTag *);
                    static RE::BSFixedString Vault(RE::StaticFunctionTag *);
                    static RE::BSFixedString Climb(RE::StaticFunctionTag *);
            };

            struct MCM_Info {
                    inline static const char *Section = "MCM_Info";

                    static RE::BSFixedString OnOff(RE::StaticFunctionTag *);
                    static RE::BSFixedString Indicator(RE::StaticFunctionTag *);
                    static RE::BSFixedString PlaybackSpeed(RE::StaticFunctionTag *);
                    static RE::BSFixedString UsePresetKey(RE::StaticFunctionTag *);
                    static RE::BSFixedString CustomKey(RE::StaticFunctionTag *);
                    static RE::BSFixedString Delay(RE::StaticFunctionTag *);
                    static RE::BSFixedString StaminaSystem(RE::StaticFunctionTag *);
                    static RE::BSFixedString MustHaveStamina(RE::StaticFunctionTag *);
                    static RE::BSFixedString StaminaCost(RE::StaticFunctionTag *);
                    static RE::BSFixedString SmartSteps(RE::StaticFunctionTag *);
                    static RE::BSFixedString SmartVault(RE::StaticFunctionTag *);
                    static RE::BSFixedString SmartClimb(RE::StaticFunctionTag *);
            };

            struct MCM_Warn {
                    inline static const char *Section = "MCM_Warn";

                    static RE::BSFixedString AreYouSure(RE::StaticFunctionTag *);
                    static RE::BSFixedString KeyAlreadyMapped(RE::StaticFunctionTag *);
            };
    };
}  // namespace SkyParkour_Papyrus