#include "Papyrus/PapyrusInterface.h"
#include "Util/ParkourUtility.h"
#include "References.h"
#include "Parkouring.h"
namespace SkyParkour_Papyrus {

    using namespace ModSettings;
    class Getters {
        public:
            static bool GetEnableMod(RE::StaticFunctionTag *) {
                return Mod_Enabled;
            }

            static bool GetShowIndicators(RE::StaticFunctionTag *) {
                return Use_Indicators;
            }

            static float GetPlaybackSpeed(RE::StaticFunctionTag *) {
                return Playback_Speed;
            }

            static bool GetEnableStaminaSystem(RE::StaticFunctionTag *) {
                return Enable_Stamina_Consumption;
            }

            static bool GetMustHaveStamina(RE::StaticFunctionTag *) {
                return Must_Have_Stamina;
            }

            static float GetBaseStaminaDamage(RE::StaticFunctionTag *) {
                return Stamina_Damage;
            }

            static bool GetUsePresetKey(RE::StaticFunctionTag *) {
                return Use_Preset_Parkour_Key;
            }

            static int32_t GetCustomParkourKey(RE::StaticFunctionTag *) {
                return Custom_Parkour_Key;
            }

            static int32_t GetPresetParkourKey(RE::StaticFunctionTag *) {
                return Preset_Parkour_Key;
            }

            static float GetParkourDelay(RE::StaticFunctionTag *) {
                return Parkour_Delay;
            }

            static bool GetSmartSteps(RE::StaticFunctionTag *) {
                return Smart_Steps;
            }

            static bool GetSmartVault(RE::StaticFunctionTag *) {
                return Smart_Vault;
            }

            static bool GetSmartClimb(RE::StaticFunctionTag *) {
                return Smart_Climb;
            }
    };

    class Setters {
        public:
            static void SetEnableMod(RE::StaticFunctionTag *, bool value) {
                auto ini = RuntimeMethods::GetIniHandle();
                ini->SetBoolValue("MCM", "bEnableMod", value);
                save(ini);

                Mod_Enabled = value;

                // Turn on if setting is on and is not beast form. Same logic on race change listener.
                Parkouring::SetParkourOnOff(Mod_Enabled && !ParkourUtility::IsBeastForm());
            }

            static void SetShowIndicators(RE::StaticFunctionTag *, bool value) {
                auto ini = RuntimeMethods::GetIniHandle();
                ini->SetBoolValue("MCM", "bShowIndicators", value);
                save(ini);

                Use_Indicators = value;
            }

            static void SetPlaybackSpeed(RE::StaticFunctionTag *, float value) {
                auto ini = RuntimeMethods::GetIniHandle();
                ini->SetValue("MCM", "fPlaybackSpeed", std::to_string(value).c_str());
                save(ini);

                Playback_Speed = value;
                /* Set the graph variable as well, above is internal */
                GET_PLAYER->SetGraphVariableFloat(SPPF_SPEEDMULT, value);
            }

            static void SetEnableStaminaSystem(RE::StaticFunctionTag *, bool value) {
                auto ini = RuntimeMethods::GetIniHandle();
                ini->SetBoolValue("MCM", "bEnableStaminaSystem", value);
                save(ini);

                Enable_Stamina_Consumption = value;
            }

            static void SetMustHaveStamina(RE::StaticFunctionTag *, bool value) {
                auto ini = RuntimeMethods::GetIniHandle();
                ini->SetBoolValue("MCM", "bMustHaveStamina", value);
                save(ini);

                Must_Have_Stamina = value;
            }

            static void SetBaseStaminaDamage(RE::StaticFunctionTag *, float value) {
                auto ini = RuntimeMethods::GetIniHandle();
                ini->SetValue("MCM", "iBaseStaminaDamage", std::to_string(value).c_str());
                save(ini);

                Stamina_Damage = value;
            }

            static void SetUsePresetKey(RE::StaticFunctionTag *, bool value) {
                auto ini = RuntimeMethods::GetIniHandle();
                ini->SetBoolValue("MCM", "bUsePresetKey", value);
                save(ini);

                Use_Preset_Parkour_Key = value;
            }

            static void SetCustomParkourKey(RE::StaticFunctionTag *, int32_t value) {
                auto ini = RuntimeMethods::GetIniHandle();
                ini->SetValue("MCM", "iCustomKeybind", std::to_string(value).c_str());
                save(ini);

                Custom_Parkour_Key = value;
            }

            static void SetPresetParkourKey(RE::StaticFunctionTag *, int32_t value) {
                auto ini = RuntimeMethods::GetIniHandle();
                ini->SetValue("MCM", "iPresetKeyIndex", std::to_string(value).c_str());
                save(ini);

                Preset_Parkour_Key = value;
            }

            static void SetParkourDelay(RE::StaticFunctionTag *, float value) {
                auto ini = RuntimeMethods::GetIniHandle();
                ini->SetValue("MCM", "fInputDelay", std::to_string(value).c_str());
                save(ini);

                Parkour_Delay = value;
            }

            static void SetSmartSteps(RE::StaticFunctionTag *, bool value) {
                auto ini = RuntimeMethods::GetIniHandle();
                ini->SetBoolValue("MCM", "bSmartSteps", value);
                save(ini);

                Smart_Steps = value;
            }

            static void SetSmartVault(RE::StaticFunctionTag *, bool value) {
                auto ini = RuntimeMethods::GetIniHandle();
                ini->SetBoolValue("MCM", "bSmartVault", value);
                save(ini);

                Smart_Vault = value;
            }

            static void SetSmartClimb(RE::StaticFunctionTag *, bool value) {
                auto ini = RuntimeMethods::GetIniHandle();
                ini->SetBoolValue("MCM", "bSmartClimb", value);
                save(ini);

                Smart_Climb = value;
            }

        private:
            static void save(const std::unique_ptr<CSimpleIniA> &ini) {
                ini->SaveFile(IniSettings::INIPath);
            }
    };

    void Internal::AlertPlayerLoaded(RE::StaticFunctionTag *) {
        // Turn on if setting is on and is not beast form. Same logic on race change listener.
        Parkouring::SetParkourOnOff(Mod_Enabled && !ParkourUtility::IsBeastForm());
        GET_PLAYER->SetGraphVariableFloat(SPPF_SPEEDMULT, Playback_Speed);
    }

    void Internal::Read_All_MCM_From_INI_and_Cache_Settings() {
        using namespace ModSettings;

        auto ini = RuntimeMethods::GetIniHandle();

        /* Parkour Settings */
        Mod_Enabled = ini->GetBoolValue("MCM", "bEnableMod");
        Use_Indicators = ini->GetBoolValue("MCM", "bShowIndicators");
        Playback_Speed = static_cast<float>(ini->GetDoubleValue("MCM", "fPlaybackSpeed"));

        /* Stamina Settings */
        Enable_Stamina_Consumption = ini->GetBoolValue("MCM", "bEnableStaminaSystem");
        Must_Have_Stamina = ini->GetBoolValue("MCM", "bMustHaveStamina");
        Stamina_Damage = static_cast<float>(ini->GetDoubleValue("MCM", "iBaseStaminaDamage"));

        /* Input Settings */
        Use_Preset_Parkour_Key = ini->GetBoolValue("MCM", "bUsePresetKey");
        Preset_Parkour_Key = static_cast<int32_t>(ini->GetDoubleValue("MCM", "iPresetKeyIndex"));
        Custom_Parkour_Key = static_cast<int32_t>(ini->GetDoubleValue("MCM", "iCustomKeybind"));
        Parkour_Delay = static_cast<float>(ini->GetDoubleValue("MCM", "fInputDelay"));

        Smart_Steps = ini->GetBoolValue("MCM", "bSmartSteps");
        Smart_Vault = ini->GetBoolValue("MCM", "bSmartVault");
        Smart_Climb = ini->GetBoolValue("MCM", "bSmartClimb");
    }
    void Internal::RegisterPapyrusFuncsToVM(RE::BSScript::IVirtualMachine *vm) {
        // Maintenance calls this to start polling updates
        vm->RegisterFunction("AlertPlayerLoaded", "SkyParkourPapyrus", Internal::AlertPlayerLoaded);

        // Getter functions
        vm->RegisterFunction("GetEnableMod", "SkyParkourPapyrus", Getters::GetEnableMod);
        vm->RegisterFunction("GetShowIndicators", "SkyParkourPapyrus", Getters::GetShowIndicators);
        vm->RegisterFunction("GetPlaybackSpeed", "SkyParkourPapyrus", Getters::GetPlaybackSpeed);
        vm->RegisterFunction("GetEnableStaminaSystem", "SkyParkourPapyrus", Getters::GetEnableStaminaSystem);
        vm->RegisterFunction("GetMustHaveStamina", "SkyParkourPapyrus", Getters::GetMustHaveStamina);
        vm->RegisterFunction("GetBaseStaminaDamage", "SkyParkourPapyrus", Getters::GetBaseStaminaDamage);
        vm->RegisterFunction("GetUsePresetKey", "SkyParkourPapyrus", Getters::GetUsePresetKey);
        vm->RegisterFunction("GetCustomParkourKey", "SkyParkourPapyrus", Getters::GetCustomParkourKey);
        vm->RegisterFunction("GetPresetParkourKey", "SkyParkourPapyrus", Getters::GetPresetParkourKey);
        vm->RegisterFunction("GetParkourDelay", "SkyParkourPapyrus", Getters::GetParkourDelay);
        vm->RegisterFunction("GetSmartSteps", "SkyParkourPapyrus", Getters::GetSmartSteps);
        vm->RegisterFunction("GetSmartVault", "SkyParkourPapyrus", Getters::GetSmartVault);
        vm->RegisterFunction("GetSmartClimb", "SkyParkourPapyrus", Getters::GetSmartClimb);

        // Setter functions
        vm->RegisterFunction("SetEnableMod", "SkyParkourPapyrus", Setters::SetEnableMod);
        vm->RegisterFunction("SetShowIndicators", "SkyParkourPapyrus", Setters::SetShowIndicators);
        vm->RegisterFunction("SetPlaybackSpeed", "SkyParkourPapyrus", Setters::SetPlaybackSpeed);
        vm->RegisterFunction("SetEnableStaminaSystem", "SkyParkourPapyrus", Setters::SetEnableStaminaSystem);
        vm->RegisterFunction("SetMustHaveStamina", "SkyParkourPapyrus", Setters::SetMustHaveStamina);
        vm->RegisterFunction("SetBaseStaminaDamage", "SkyParkourPapyrus", Setters::SetBaseStaminaDamage);
        vm->RegisterFunction("SetUsePresetKey", "SkyParkourPapyrus", Setters::SetUsePresetKey);
        vm->RegisterFunction("SetCustomParkourKey", "SkyParkourPapyrus", Setters::SetCustomParkourKey);
        vm->RegisterFunction("SetPresetParkourKey", "SkyParkourPapyrus", Setters::SetPresetParkourKey);
        vm->RegisterFunction("SetParkourDelay", "SkyParkourPapyrus", Setters::SetParkourDelay);
        vm->RegisterFunction("SetSmartSteps", "SkyParkourPapyrus", Setters::SetSmartSteps);
        vm->RegisterFunction("SetSmartVault", "SkyParkourPapyrus", Setters::SetSmartVault);
        vm->RegisterFunction("SetSmartClimb", "SkyParkourPapyrus", Setters::SetSmartClimb);
    }

}  // namespace SkyParkour_Papyrus