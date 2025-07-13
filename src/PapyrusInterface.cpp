#include "PapyrusInterface.h"
namespace SkyParkour_PapyrusInterface {

    bool ReadMCM() {
        auto ini = RuntimeMethods::GetIniHandle();
        if (!ini) {
            logger::error(">>> CAN'T READ MCM SETTINGS, INI FILE NOT FOUND <<<");
            return false;
        }
    }
    bool WriteMCM() {}
    void RegisterCustomParkourKey(RE::StaticFunctionTag *, int32_t dxcode) {
        ButtonStates::DXCODE = dxcode;
        logger::info(">Custom Key: '{}'", dxcode);
    }
    void RegisterPresetParkourKey(RE::StaticFunctionTag *, int32_t presetKey) {
        ModSettings::PresetParkourKey = presetKey;
        logger::info(">Preset Key: '{}'", ModSettings::PresetParkourKey);
    }
    void RegisterParkourDelay(RE::StaticFunctionTag *, float delay) {
        ModSettings::parkourDelay = delay;
        logger::info(">Delay '{}'", ModSettings::parkourDelay);
    }
    void RegisterStaminaDamage(RE::StaticFunctionTag *, bool enabled, bool staminaBlocks, float damage) {
        ModSettings::Enable_Stamina_Consumption = enabled;
        ModSettings::Is_Stamina_Required = staminaBlocks;
        ModSettings::Stamina_Damage = damage;
        logger::info("|Stamina|> On:'{}' >Must:'{}' >Dmg:'{}'", ModSettings::Enable_Stamina_Consumption, ModSettings::Is_Stamina_Required,
                     ModSettings::Stamina_Damage);
    }
    void RegisterParkourSettings(RE::StaticFunctionTag *, bool _usePresetKey, bool _enableMod, bool _smartParkour, bool _useIndicators) {
        ModSettings::UsePresetParkourKey = _usePresetKey;
        ModSettings::Smart_Parkour_Enabled = _smartParkour;
        ModSettings::UseIndicators = _useIndicators;

        ModSettings::ModEnabled = _enableMod;

        // Turn on if setting is on and is not beast form. Same logic on race change listener.
        Parkouring::SetParkourOnOff(ModSettings::ModEnabled && !ParkourUtility::IsBeastForm());
    }

}  // namespace SkyParkour_PapyrusInterface