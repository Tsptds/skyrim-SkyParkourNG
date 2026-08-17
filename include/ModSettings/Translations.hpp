#pragma once

namespace MCM_Translation
{
    namespace translation = SKSE::Translation;

    template <const char *KeyStr>
    class TranslationKey {
        public:
            static constexpr const char *Key = KeyStr;
            std::string CachedValue;
    };

    // Parkour
    inline constexpr char s_ParkourHeader[] = "$ParkourHeader";
    inline constexpr char s_ParkourOnOff[] = "$ParkourOnOff";
    inline constexpr char s_MCMInfoOnOff[] = "$MCM_Info_OnOff";
    inline constexpr char s_Indicator[] = "$Indicator";
    inline constexpr char s_MCMInfoIndicator[] = "$MCM_Info_Indicator";
    inline constexpr char s_PlaybackSpeed[] = "$PlaybackSpeed";
    inline constexpr char s_MCMInfoPlaybackSpeed[] = "$MCM_Info_PlaybackSpeed";

    // Inputs
    inline constexpr char s_InputHeader[] = "$InputHeader";
    inline constexpr char s_UsePresetKey[] = "$UsePresetKey";
    inline constexpr char s_MCMInfoUsePresetKey[] = "$MCM_Info_UsePresetKey";
    inline constexpr char s_PresetKeyList[] = "$PresetKeyList";
    inline constexpr char s_PresetKey0[] = "$PresetKey0";
    inline constexpr char s_PresetKey1[] = "$PresetKey1";
    inline constexpr char s_PresetKey2[] = "$PresetKey2";
    inline constexpr char s_CustomKey[] = "$CustomKey";
    inline constexpr char s_MCMInfoCustomKey[] = "$MCM_Info_CustomKey";
    inline constexpr char s_InputDelay[] = "$InputDelay";
    inline constexpr char s_MCMInfoDelay[] = "$MCM_Info_Delay";

    // Stamina System
    inline constexpr char s_StaminaHeader[] = "$StaminaHeader";
    inline constexpr char s_EnableStaminaSystem[] = "$EnableStaminaSystem";
    inline constexpr char s_MCMInfoStaminaSystem[] = "$MCM_Info_StaminaSystem";
    inline constexpr char s_MustHaveStamina[] = "$MustHaveStamina";
    inline constexpr char s_MCMInfoMustHaveStamina[] = "$MCM_Info_MustHaveStamina";
    inline constexpr char s_BaseStaminaCost[] = "$BaseStaminaCost";
    inline constexpr char s_MCMInfoStaminaCost[] = "$MCM_Info_StaminaCost";

    // Smart Parkour
    inline constexpr char s_SmartParkourHeader[] = "$SmartParkourHeader";
    inline constexpr char s_AutoParkourList[] = "$AutoParkourList";
    inline constexpr char s_AutoParkourOpt0[] = "$AutoParkourOpt0";
    inline constexpr char s_AutoParkourOpt1[] = "$AutoParkourOpt1";
    inline constexpr char s_AutoParkourOpt2[] = "$AutoParkourOpt2";
    inline constexpr char s_MCMInfoAutoParkour[] = "$MCM_Info_AutoParkour";
    inline constexpr char s_SmartSteps[] = "$SmartSteps";
    inline constexpr char s_MCMInfoSmartSteps[] = "$MCM_Info_SmartSteps";
    inline constexpr char s_SmartVault[] = "$SmartVault";
    inline constexpr char s_MCMInfoSmartVault[] = "$MCM_Info_SmartVault";
    inline constexpr char s_SmartClimb[] = "$SmartClimb";
    inline constexpr char s_MCMInfoSmartClimb[] = "$MCM_Info_SmartClimb";

    // Sliding
    inline constexpr char s_SlideHeader[] = "$SlideHeader";
    inline constexpr char s_CrouchSlideOnOff[] = "$CrouchSlideOnOff";
    inline constexpr char s_MCMInfoCrouchSlide[] = "$MCM_Info_CrouchSlide";
    inline constexpr char s_AdvancedSlide[] = "$AdvancedSlide";
    inline constexpr char s_MCMInfoAdvancedSlide[] = "$MCM_Info_AdvancedSlide";
    inline constexpr char s_SlideTackle[] = "$SlideTackle";
    inline constexpr char s_MCMInfoSlideTackle[] = "$MCM_Info_SlideTackle";

    // Rolling
    inline constexpr char s_RollHeader[] = "$RollHeader";
    inline constexpr char s_RollOnOff[] = "$RollOnOff";
    inline constexpr char s_MCMInfoRoll[] = "$MCM_Info_Roll";

    // Debug
    inline constexpr char s_DebugHeader[] = "$DebugHeader";
    inline constexpr char s_DebugOnOff[] = "$DebugOnOff";

    // Warnings / Misc
    inline constexpr char s_MCMWarnPressAnyKey[] = "$MCM_Warn_PressAnyKey";
    inline constexpr char s_MainSettingsHeader[] = "$SettingsHeader";
    inline constexpr char s_MCMUnbound[] = "$Unbound";

    class CachedStrings {
        public:
            static CachedStrings *GetSingleton()
            {
                static CachedStrings instance;
                return &instance;
            }

            // Parkour
            TranslationKey<s_ParkourHeader> parkourHeader;
            TranslationKey<s_ParkourOnOff> parkourOnOff;
            TranslationKey<s_MCMInfoOnOff> mcmInfoOnOff;
            TranslationKey<s_Indicator> indicator;
            TranslationKey<s_MCMInfoIndicator> mcmInfoIndicator;
            TranslationKey<s_PlaybackSpeed> playbackSpeed;
            TranslationKey<s_MCMInfoPlaybackSpeed> mcmInfoPlaybackSpeed;

            // Inputs
            TranslationKey<s_InputHeader> inputHeader;
            TranslationKey<s_UsePresetKey> usePresetKey;
            TranslationKey<s_MCMInfoUsePresetKey> mcmInfoUsePresetKey;
            TranslationKey<s_PresetKeyList> presetKeyList;
            TranslationKey<s_PresetKey0> presetKey0;
            TranslationKey<s_PresetKey1> presetKey1;
            TranslationKey<s_PresetKey2> presetKey2;
            TranslationKey<s_CustomKey> customKey;
            TranslationKey<s_MCMInfoCustomKey> mcmInfoCustomKey;
            TranslationKey<s_InputDelay> inputDelay;
            TranslationKey<s_MCMInfoDelay> mcmInfoDelay;

            // Stamina System
            TranslationKey<s_StaminaHeader> staminaHeader;
            TranslationKey<s_EnableStaminaSystem> enableStaminaSystem;
            TranslationKey<s_MCMInfoStaminaSystem> mcmInfoStaminaSystem;
            TranslationKey<s_MustHaveStamina> mustHaveStamina;
            TranslationKey<s_MCMInfoMustHaveStamina> mcmInfoMustHaveStamina;
            TranslationKey<s_BaseStaminaCost> baseStaminaCost;
            TranslationKey<s_MCMInfoStaminaCost> mcmInfoStaminaCost;

            // Smart Parkour
            TranslationKey<s_SmartParkourHeader> smartParkourHeader;
            TranslationKey<s_AutoParkourList> autoParkourList;
            TranslationKey<s_AutoParkourOpt0> autoParkourOpt0;
            TranslationKey<s_AutoParkourOpt1> autoParkourOpt1;
            TranslationKey<s_AutoParkourOpt2> autoParkourOpt2;
            TranslationKey<s_MCMInfoAutoParkour> mcmInfoAutoParkour;
            TranslationKey<s_SmartSteps> smartSteps;
            TranslationKey<s_MCMInfoSmartSteps> mcmInfoSmartSteps;
            TranslationKey<s_SmartVault> smartVault;
            TranslationKey<s_MCMInfoSmartVault> mcmInfoSmartVault;
            TranslationKey<s_SmartClimb> smartClimb;
            TranslationKey<s_MCMInfoSmartClimb> mcmInfoSmartClimb;

            // Sliding
            TranslationKey<s_SlideHeader> slideHeader;
            TranslationKey<s_CrouchSlideOnOff> crouchSlideOnOff;
            TranslationKey<s_MCMInfoCrouchSlide> mcmInfoCrouchSlide;
            TranslationKey<s_AdvancedSlide> advancedSlide;
            TranslationKey<s_MCMInfoAdvancedSlide> mcmInfoAdvancedSlide;
            TranslationKey<s_SlideTackle> slideTackle;
            TranslationKey<s_MCMInfoSlideTackle> mcmInfoSlideTackle;

            // Rolling
            TranslationKey<s_RollHeader> rollHeader;
            TranslationKey<s_RollOnOff> rollOnOff;
            TranslationKey<s_MCMInfoRoll> mcmInfoRoll;

            // Debug
            TranslationKey<s_DebugHeader> debugHeader;
            TranslationKey<s_DebugOnOff> debugOnOff;

            // Warnings / Misc
            TranslationKey<s_MCMWarnPressAnyKey> mcmWarnPressAnyKey;
            TranslationKey<s_MainSettingsHeader> mainSettingsHeader;
            TranslationKey<s_MCMUnbound> mcmUnbound;

            inline static void Initialize()
            {
                auto Fetch = [](auto &target) { translation::Translate(target.Key, target.CachedValue); };
                auto cs = CachedStrings::GetSingleton();

                // Parkour
                Fetch(cs->parkourHeader);
                Fetch(cs->parkourOnOff);
                Fetch(cs->mcmInfoOnOff);
                Fetch(cs->indicator);
                Fetch(cs->mcmInfoIndicator);
                Fetch(cs->playbackSpeed);
                Fetch(cs->mcmInfoPlaybackSpeed);

                // Inputs
                Fetch(cs->inputHeader);
                Fetch(cs->usePresetKey);
                Fetch(cs->mcmInfoUsePresetKey);
                Fetch(cs->presetKeyList);
                Fetch(cs->presetKey0);
                Fetch(cs->presetKey1);
                Fetch(cs->presetKey2);
                Fetch(cs->customKey);
                Fetch(cs->mcmInfoCustomKey);
                Fetch(cs->inputDelay);
                Fetch(cs->mcmInfoDelay);

                // Stamina System
                Fetch(cs->staminaHeader);
                Fetch(cs->enableStaminaSystem);
                Fetch(cs->mcmInfoStaminaSystem);
                Fetch(cs->mustHaveStamina);
                Fetch(cs->mcmInfoMustHaveStamina);
                Fetch(cs->baseStaminaCost);
                Fetch(cs->mcmInfoStaminaCost);

                // Smart Parkour
                Fetch(cs->smartParkourHeader);
                Fetch(cs->autoParkourList);
                Fetch(cs->autoParkourOpt0);
                Fetch(cs->autoParkourOpt1);
                Fetch(cs->autoParkourOpt2);
                Fetch(cs->mcmInfoAutoParkour);
                Fetch(cs->smartSteps);
                Fetch(cs->mcmInfoSmartSteps);
                Fetch(cs->smartVault);
                Fetch(cs->mcmInfoSmartVault);
                Fetch(cs->smartClimb);
                Fetch(cs->mcmInfoSmartClimb);

                // Sliding
                Fetch(cs->slideHeader);
                Fetch(cs->crouchSlideOnOff);
                Fetch(cs->mcmInfoCrouchSlide);
                Fetch(cs->advancedSlide);
                Fetch(cs->mcmInfoAdvancedSlide);
                Fetch(cs->slideTackle);
                Fetch(cs->mcmInfoSlideTackle);

                // Rolling
                Fetch(cs->rollHeader);
                Fetch(cs->rollOnOff);
                Fetch(cs->mcmInfoRoll);

                // Debug
                Fetch(cs->debugHeader);
                Fetch(cs->debugOnOff);

                // Warnings / Misc
                Fetch(cs->mcmWarnPressAnyKey);
                Fetch(cs->mainSettingsHeader);
                Fetch(cs->mcmUnbound);
            }

        private:
            CachedStrings() = default;
    };
}