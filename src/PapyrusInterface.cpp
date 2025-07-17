#include "PapyrusInterface.h"
namespace SkyParkour_Papyrus {

    using namespace ModSettings;
    class Getters {
        public:
            static bool GetEnableMod(RE::StaticFunctionTag *) {
                return ModEnabled;
            }

            static bool GetSmartParkour(RE::StaticFunctionTag *) {
                return Smart_Parkour_Enabled;
            }

            static bool GetShowIndicators(RE::StaticFunctionTag *) {
                return UseIndicators;
            }

            static bool GetEnableStaminaSystem(RE::StaticFunctionTag *) {
                return Enable_Stamina_Consumption;
            }

            static bool GetMustHaveStamina(RE::StaticFunctionTag *) {
                return Is_Stamina_Required;
            }

            static float GetBaseStaminaDamage(RE::StaticFunctionTag *) {
                return Stamina_Damage;
            }

            static bool GetUsePresetKey(RE::StaticFunctionTag *) {
                return UsePresetParkourKey;
            }

            static int32_t GetCustomParkourKey(RE::StaticFunctionTag *) {
                return ButtonStates::DXCODE;
            }

            static int32_t GetPresetParkourKey(RE::StaticFunctionTag *) {
                return PresetParkourKey;
            }

            static float GetParkourDelay(RE::StaticFunctionTag *) {
                return parkourDelay;
            }
    };

    class Setters {
        public:
            static void SetEnableMod(RE::StaticFunctionTag *, bool value) {
                auto ini = RuntimeMethods::GetIniHandle();
                ini->SetBoolValue("MCM", "bEnableMod", value);
                save(ini);

                ModEnabled = value;

                // Turn on if setting is on and is not beast form. Same logic on race change listener.
                Parkouring::SetParkourOnOff(ModEnabled && !ParkourUtility::IsBeastForm());
            }

            static void SetSmartParkour(RE::StaticFunctionTag *, bool value) {
                auto ini = RuntimeMethods::GetIniHandle();
                ini->SetBoolValue("MCM", "bSmartParkour", value);
                save(ini);

                Smart_Parkour_Enabled = value;
            }

            static void SetShowIndicators(RE::StaticFunctionTag *, bool value) {
                auto ini = RuntimeMethods::GetIniHandle();
                ini->SetBoolValue("MCM", "bShowIndicators", value);
                save(ini);

                UseIndicators = value;
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

                Is_Stamina_Required = value;
            }

            static void SetBaseStaminaDamage(RE::StaticFunctionTag *, float value) {
                auto ini = RuntimeMethods::GetIniHandle();
                ini->SetDoubleValue("MCM", "iBaseStaminaDamage", static_cast<double>(value));
                save(ini);

                Stamina_Damage = value;
            }

            static void SetUsePresetKey(RE::StaticFunctionTag *, bool value) {
                auto ini = RuntimeMethods::GetIniHandle();
                ini->SetBoolValue("MCM", "bUsePresetKey", value);
                save(ini);

                UsePresetParkourKey = value;
            }

            static void SetCustomParkourKey(RE::StaticFunctionTag *, int32_t value) {
                auto ini = RuntimeMethods::GetIniHandle();
                ini->SetDoubleValue("MCM", "iCustomKeybind", static_cast<double>(value));
                save(ini);

                /* TODO: Move this mod settings and rename to custom key */
                ButtonStates::DXCODE = value;
            }

            static void SetPresetParkourKey(RE::StaticFunctionTag *, int32_t value) {
                auto ini = RuntimeMethods::GetIniHandle();
                ini->SetDoubleValue("MCM", "iPresetKeyIndex", static_cast<double>(value));
                save(ini);

                PresetParkourKey = value;
            }

            static void SetParkourDelay(RE::StaticFunctionTag *, float value) {
                auto ini = RuntimeMethods::GetIniHandle();
                ini->SetDoubleValue("MCM", "fInputDelay", static_cast<double>(value));
                save(ini);

                parkourDelay = value;
            }

        private:
            static void save(const std::unique_ptr<CSimpleIniA> &ini) {
                ini->SaveFile(IniSettings::INIPath);
            }
    };

    void Internal::AlertPlayerLoaded(RE::StaticFunctionTag *) {
        // Turn on if setting is on and is not beast form. Same logic on race change listener.
        Parkouring::SetParkourOnOff(ModEnabled && !ParkourUtility::IsBeastForm());
    }

    void Internal::Read_All_MCM_From_INI_and_Cache_Settings() {
        auto ini = RuntimeMethods::GetIniHandle();

        /* Parkour Settings */
        ModSettings::ModEnabled = ini->GetBoolValue("MCM", "bEnableMod");
        ModSettings::Smart_Parkour_Enabled = ini->GetBoolValue("MCM", "bSmartParkour");
        ModSettings::UseIndicators = ini->GetBoolValue("MCM", "bShowIndicators");

        /* Stamina Settings */
        ModSettings::Enable_Stamina_Consumption = ini->GetBoolValue("MCM", "bEnableStaminaSystem");
        ModSettings::Is_Stamina_Required = ini->GetBoolValue("MCM", "bMustHaveStamina");
        ModSettings::Stamina_Damage = static_cast<float>(ini->GetDoubleValue("MCM", "iBaseStaminaDamage"));

        /* Input Settings */
        ModSettings::UsePresetParkourKey = ini->GetBoolValue("MCM", "bUsePresetKey");
        ModSettings::PresetParkourKey = static_cast<int32_t>(ini->GetDoubleValue("MCM", "iPresetKeyIndex"));
        ButtonStates::DXCODE = static_cast<int32_t>(ini->GetDoubleValue("MCM", "iCustomKeybind"));
        ModSettings::parkourDelay = static_cast<float>(ini->GetDoubleValue("MCM", "fInputDelay"));
    }
    void Internal::RegisterPapyrusFuncsToVM(RE::BSScript::IVirtualMachine *vm) {
        // Maintenance Calls this to starts polling updates
        vm->RegisterFunction("AlertPlayerLoaded", "SkyParkourPapyrus", Internal::AlertPlayerLoaded);

        // Read functions
        vm->RegisterFunction("GetEnableMod", "SkyParkourPapyrus", Getters::GetEnableMod);
        vm->RegisterFunction("GetSmartParkour", "SkyParkourPapyrus", Getters::GetSmartParkour);
        vm->RegisterFunction("GetShowIndicators", "SkyParkourPapyrus", Getters::GetShowIndicators);
        vm->RegisterFunction("GetEnableStaminaSystem", "SkyParkourPapyrus", Getters::GetEnableStaminaSystem);
        vm->RegisterFunction("GetMustHaveStamina", "SkyParkourPapyrus", Getters::GetMustHaveStamina);
        vm->RegisterFunction("GetBaseStaminaDamage", "SkyParkourPapyrus", Getters::GetBaseStaminaDamage);
        vm->RegisterFunction("GetUsePresetKey", "SkyParkourPapyrus", Getters::GetUsePresetKey);
        vm->RegisterFunction("GetCustomParkourKey", "SkyParkourPapyrus", Getters::GetCustomParkourKey);
        vm->RegisterFunction("GetPresetParkourKey", "SkyParkourPapyrus", Getters::GetPresetParkourKey);
        vm->RegisterFunction("GetParkourDelay", "SkyParkourPapyrus", Getters::GetParkourDelay);

        // Write functions
        vm->RegisterFunction("SetEnableMod", "SkyParkourPapyrus", Setters::SetEnableMod);
        vm->RegisterFunction("SetSmartParkour", "SkyParkourPapyrus", Setters::SetSmartParkour);
        vm->RegisterFunction("SetShowIndicators", "SkyParkourPapyrus", Setters::SetShowIndicators);
        vm->RegisterFunction("SetEnableStaminaSystem", "SkyParkourPapyrus", Setters::SetEnableStaminaSystem);
        vm->RegisterFunction("SetMustHaveStamina", "SkyParkourPapyrus", Setters::SetMustHaveStamina);
        vm->RegisterFunction("SetBaseStaminaDamage", "SkyParkourPapyrus", Setters::SetBaseStaminaDamage);
        vm->RegisterFunction("SetUsePresetKey", "SkyParkourPapyrus", Setters::SetUsePresetKey);
        vm->RegisterFunction("SetCustomParkourKey", "SkyParkourPapyrus", Setters::SetCustomParkourKey);
        vm->RegisterFunction("SetPresetParkourKey", "SkyParkourPapyrus", Setters::SetPresetParkourKey);
        vm->RegisterFunction("SetParkourDelay", "SkyParkourPapyrus", Setters::SetParkourDelay);
    }

}  // namespace SkyParkour_Papyrus