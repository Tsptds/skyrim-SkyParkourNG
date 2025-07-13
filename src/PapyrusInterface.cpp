#include "PapyrusInterface.h"
namespace SkyParkour_Papyrus {

    class Read {
        public:
            static bool GetEnableMod(RE::StaticFunctionTag *) {
                auto ini = RuntimeMethods::GetIniHandle();
                return ini->GetBoolValue("MCM", "bEnableMod");
            }

            static bool GetSmartParkour(RE::StaticFunctionTag *) {
                auto ini = RuntimeMethods::GetIniHandle();
                return ini->GetBoolValue("MCM", "bSmartParkour");
            }

            static bool GetShowIndicators(RE::StaticFunctionTag *) {
                auto ini = RuntimeMethods::GetIniHandle();
                return ini->GetBoolValue("MCM", "bShowIndicators");
            }

            static bool GetEnableStaminaSystem(RE::StaticFunctionTag *) {
                auto ini = RuntimeMethods::GetIniHandle();
                return ini->GetBoolValue("MCM", "bEnableStaminaSystem");
            }

            static bool GetMustHaveStamina(RE::StaticFunctionTag *) {
                auto ini = RuntimeMethods::GetIniHandle();
                return ini->GetBoolValue("MCM", "bMustHaveStamina");
            }

            static float GetBaseStaminaDamage(RE::StaticFunctionTag *) {
                auto ini = RuntimeMethods::GetIniHandle();
                return static_cast<float>(ini->GetDoubleValue("MCM", "iBaseStaminaDamage"));
            }

            static bool GetUsePresetKey(RE::StaticFunctionTag *) {
                auto ini = RuntimeMethods::GetIniHandle();
                return ini->GetBoolValue("MCM", "bUsePresetKey");
            }

            static int32_t GetCustomParkourKey(RE::StaticFunctionTag *) {
                auto ini = RuntimeMethods::GetIniHandle();
                return static_cast<int32_t>(ini->GetDoubleValue("MCM", "iCustomKeybind"));
            }

            static int32_t GetPresetParkourKey(RE::StaticFunctionTag *) {
                auto ini = RuntimeMethods::GetIniHandle();
                return static_cast<int32_t>(ini->GetDoubleValue("MCM", "iPresetKeyIndex"));
            }

            static float GetParkourDelay(RE::StaticFunctionTag *) {
                auto ini = RuntimeMethods::GetIniHandle();
                return static_cast<float>(ini->GetDoubleValue("MCM", "fInputDelay"));
            }
    };

    class Write {
        public:
            static void SetEnableMod(RE::StaticFunctionTag *, bool value) {
                auto ini = RuntimeMethods::GetIniHandle();
                ini->SetBoolValue("MCM", "bEnableMod", value);
                save(ini);
            }

            static void SetSmartParkour(RE::StaticFunctionTag *, bool value) {
                auto ini = RuntimeMethods::GetIniHandle();
                ini->SetBoolValue("MCM", "bSmartParkour", value);
                save(ini);
            }

            static void SetShowIndicators(RE::StaticFunctionTag *, bool value) {
                auto ini = RuntimeMethods::GetIniHandle();
                ini->SetBoolValue("MCM", "bShowIndicators", value);
                save(ini);
            }

            static void SetEnableStaminaSystem(RE::StaticFunctionTag *, bool value) {
                auto ini = RuntimeMethods::GetIniHandle();
                ini->SetBoolValue("MCM", "bEnableStaminaSystem", value);
                save(ini);
            }

            static void SetMustHaveStamina(RE::StaticFunctionTag *, bool value) {
                auto ini = RuntimeMethods::GetIniHandle();
                ini->SetBoolValue("MCM", "bMustHaveStamina", value);
                save(ini);
            }

            static void SetBaseStaminaDamage(RE::StaticFunctionTag *, float value) {
                auto ini = RuntimeMethods::GetIniHandle();
                ini->SetDoubleValue("MCM", "iBaseStaminaDamage", static_cast<double>(value));
                save(ini);
            }

            static void SetUsePresetKey(RE::StaticFunctionTag *, bool value) {
                auto ini = RuntimeMethods::GetIniHandle();
                ini->SetBoolValue("MCM", "bUsePresetKey", value);
                save(ini);
            }

            static void SetCustomParkourKey(RE::StaticFunctionTag *, int32_t value) {
                auto ini = RuntimeMethods::GetIniHandle();
                ini->SetDoubleValue("MCM", "iCustomKeybind", static_cast<double>(value));
                save(ini);
            }

            static void SetPresetParkourKey(RE::StaticFunctionTag *, int32_t value) {
                auto ini = RuntimeMethods::GetIniHandle();
                ini->SetDoubleValue("MCM", "iPresetKeyIndex", static_cast<double>(value));
                save(ini);
            }

            static void SetParkourDelay(RE::StaticFunctionTag *, float value) {
                auto ini = RuntimeMethods::GetIniHandle();
                ini->SetDoubleValue("MCM", "fInputDelay", static_cast<double>(value));
                save(ini);
            }

        private:
            static void save(const std::unique_ptr<CSimpleIniA> &ini) {
                ini->SaveFile("./Data/SKSE/Plugins/SkyParkourNG.ini");
            }
    };

    void RegisterParkourSettings(RE::StaticFunctionTag *, bool _usePresetKey, bool _enableMod, bool _smartParkour, bool _useIndicators) {
        ModSettings::ModEnabled = _enableMod;
        ModSettings::Smart_Parkour_Enabled = _smartParkour;
        ModSettings::UseIndicators = _useIndicators;
        ModSettings::UsePresetParkourKey = _usePresetKey;

        Write::SetEnableMod(nullptr, _enableMod);
        Write::SetSmartParkour(nullptr, _smartParkour);
        Write::SetShowIndicators(nullptr, _useIndicators);
        Write::SetUsePresetKey(nullptr, _usePresetKey);

        // Turn on if setting is on and is not beast form. Same logic on race change listener.
        Parkouring::SetParkourOnOff(ModSettings::ModEnabled && !ParkourUtility::IsBeastForm());
    }
    void RegisterStaminaDamage(RE::StaticFunctionTag *, bool enabled, bool staminaBlocks, float damage) {
        ModSettings::Enable_Stamina_Consumption = enabled;
        ModSettings::Is_Stamina_Required = staminaBlocks;
        ModSettings::Stamina_Damage = damage;

        Write::SetEnableStaminaSystem(nullptr, enabled);
        Write::SetMustHaveStamina(nullptr, staminaBlocks);
        Write::SetBaseStaminaDamage(nullptr, damage);

        logger::info("|Stamina|> On:'{}' >Must:'{}' >Dmg:'{}'", ModSettings::Enable_Stamina_Consumption, ModSettings::Is_Stamina_Required,
                     ModSettings::Stamina_Damage);
    }
    void RegisterPresetParkourKey(RE::StaticFunctionTag *, int32_t presetKey) {
        ModSettings::PresetParkourKey = presetKey;
        Write::SetPresetParkourKey(nullptr, presetKey);

        logger::info(">Preset Key: '{}'", ModSettings::PresetParkourKey);
    }
    void RegisterCustomParkourKey(RE::StaticFunctionTag *, int32_t dxcode) {
        ButtonStates::DXCODE = dxcode;
        Write::SetCustomParkourKey(nullptr, dxcode);

        logger::info(">Custom Key: '{}'", dxcode);
    }
    void RegisterParkourDelay(RE::StaticFunctionTag *, float delay) {
        ModSettings::parkourDelay = delay;
        Write::SetParkourDelay(nullptr, delay);

        logger::info(">Delay '{}'", ModSettings::parkourDelay);
    }

    void AddFuncsToVm(RE::BSScript::IVirtualMachine *vm) {
        vm->RegisterFunction("RegisterParkourSettings", "SkyParkourPapyrus", RegisterParkourSettings);
        vm->RegisterFunction("RegisterCustomParkourKey", "SkyParkourPapyrus", RegisterCustomParkourKey);
        vm->RegisterFunction("RegisterPresetParkourKey", "SkyParkourPapyrus", RegisterPresetParkourKey);
        vm->RegisterFunction("RegisterParkourDelay", "SkyParkourPapyrus", RegisterParkourDelay);
        vm->RegisterFunction("RegisterStaminaDamage", "SkyParkourPapyrus", RegisterStaminaDamage);

        // Read functions
        vm->RegisterFunction("GetEnableMod", "SkyParkourPapyrus", Read::GetEnableMod);
        vm->RegisterFunction("GetSmartParkour", "SkyParkourPapyrus", Read::GetSmartParkour);
        vm->RegisterFunction("GetShowIndicators", "SkyParkourPapyrus", Read::GetShowIndicators);
        vm->RegisterFunction("GetEnableStaminaSystem", "SkyParkourPapyrus", Read::GetEnableStaminaSystem);
        vm->RegisterFunction("GetMustHaveStamina", "SkyParkourPapyrus", Read::GetMustHaveStamina);
        vm->RegisterFunction("GetBaseStaminaDamage", "SkyParkourPapyrus", Read::GetBaseStaminaDamage);
        vm->RegisterFunction("GetUsePresetKey", "SkyParkourPapyrus", Read::GetUsePresetKey);
        vm->RegisterFunction("GetCustomParkourKey", "SkyParkourPapyrus", Read::GetCustomParkourKey);
        vm->RegisterFunction("GetPresetParkourKey", "SkyParkourPapyrus", Read::GetPresetParkourKey);
        vm->RegisterFunction("GetParkourDelay", "SkyParkourPapyrus", Read::GetParkourDelay);

        // Write functions
        vm->RegisterFunction("SetEnableMod", "SkyParkourPapyrus", Write::SetEnableMod);
        vm->RegisterFunction("SetSmartParkour", "SkyParkourPapyrus", Write::SetSmartParkour);
        vm->RegisterFunction("SetShowIndicators", "SkyParkourPapyrus", Write::SetShowIndicators);
        vm->RegisterFunction("SetEnableStaminaSystem", "SkyParkourPapyrus", Write::SetEnableStaminaSystem);
        vm->RegisterFunction("SetMustHaveStamina", "SkyParkourPapyrus", Write::SetMustHaveStamina);
        vm->RegisterFunction("SetBaseStaminaDamage", "SkyParkourPapyrus", Write::SetBaseStaminaDamage);
        vm->RegisterFunction("SetUsePresetKey", "SkyParkourPapyrus", Write::SetUsePresetKey);
        vm->RegisterFunction("SetCustomParkourKey", "SkyParkourPapyrus", Write::SetCustomParkourKey);
        vm->RegisterFunction("SetPresetParkourKey", "SkyParkourPapyrus", Write::SetPresetParkourKey);
        vm->RegisterFunction("SetParkourDelay", "SkyParkourPapyrus", Write::SetParkourDelay);
    }

}  // namespace SkyParkour_Papyrus