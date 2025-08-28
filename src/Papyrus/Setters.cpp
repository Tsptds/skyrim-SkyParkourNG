#include "Papyrus/Setters.h"
#include "_References/ModSettings.h"
#include "_References/RuntimeMethods.h"
#include "Util/ParkourUtility.h"
#include "_References/IniSettings.h"
#include "Parkouring.h"

namespace SkyParkour_Papyrus {

    using namespace ModSettings;

    void Setters::RegisterFuncs(RE::BSScript::IVirtualMachine *vm) {
        vm->RegisterFunction("SetEnableMod", className, SetEnableMod);
        vm->RegisterFunction("SetShowIndicators", className, SetShowIndicators);
        vm->RegisterFunction("SetPlaybackSpeed", className, SetPlaybackSpeed);
        vm->RegisterFunction("SetEnableStaminaSystem", className, SetEnableStaminaSystem);
        vm->RegisterFunction("SetMustHaveStamina", className, SetMustHaveStamina);
        vm->RegisterFunction("SetBaseStaminaDamage", className, SetBaseStaminaDamage);
        vm->RegisterFunction("SetUsePresetKey", className, SetUsePresetKey);
        vm->RegisterFunction("SetCustomParkourKey", className, SetCustomParkourKey);
        vm->RegisterFunction("SetPresetParkourKey", className, SetPresetParkourKey);
        vm->RegisterFunction("SetParkourDelay", className, SetParkourDelay);
        vm->RegisterFunction("SetSmartSteps", className, SetSmartSteps);
        vm->RegisterFunction("SetSmartVault", className, SetSmartVault);
        vm->RegisterFunction("SetSmartClimb", className, SetSmartClimb);
    }

    void Setters::SetEnableMod(RE::StaticFunctionTag *, bool value) {
        auto ini = RuntimeMethods::GetIniHandle();
        ini->SetBoolValue(Section, "bEnableMod", value);
        save(ini);

        Mod_Enabled = value;

        // Turn on if setting is on and is not beast form. Same logic on race change listener.
        Parkouring::SetParkourOnOff(Mod_Enabled && !ParkourUtility::IsBeastForm());
    }
    void Setters::SetShowIndicators(RE::StaticFunctionTag *, bool value) {
        auto ini = RuntimeMethods::GetIniHandle();
        ini->SetBoolValue(Section, "bShowIndicators", value);
        save(ini);

        Use_Indicators = value;
    }
    void Setters::SetPlaybackSpeed(RE::StaticFunctionTag *, float value) {
        auto ini = RuntimeMethods::GetIniHandle();
        ini->SetValue(Section, "fPlaybackSpeed", std::to_string(value).c_str());
        save(ini);

        Playback_Speed = value;
        /* Set the graph variable as well, above is internal */
        GET_PLAYER->SetGraphVariableFloat(SPPF_SPEEDMULT, value);
    }
    void Setters::SetEnableStaminaSystem(RE::StaticFunctionTag *, bool value) {
        auto ini = RuntimeMethods::GetIniHandle();
        ini->SetBoolValue(Section, "bEnableStaminaSystem", value);
        save(ini);

        Enable_Stamina_Consumption = value;
    }
    void Setters::SetMustHaveStamina(RE::StaticFunctionTag *, bool value) {
        auto ini = RuntimeMethods::GetIniHandle();
        ini->SetBoolValue(Section, "bMustHaveStamina", value);
        save(ini);

        Must_Have_Stamina = value;
    }
    void Setters::SetBaseStaminaDamage(RE::StaticFunctionTag *, float value) {
        auto ini = RuntimeMethods::GetIniHandle();
        ini->SetValue(Section, "iBaseStaminaDamage", std::to_string(value).c_str());
        save(ini);

        Stamina_Damage = value;
    }
    void Setters::SetUsePresetKey(RE::StaticFunctionTag *, bool value) {
        auto ini = RuntimeMethods::GetIniHandle();
        ini->SetBoolValue(Section, "bUsePresetKey", value);
        save(ini);

        Use_Preset_Parkour_Key = value;
    }
    void Setters::SetCustomParkourKey(RE::StaticFunctionTag *, int32_t value) {
        auto ini = RuntimeMethods::GetIniHandle();
        ini->SetValue(Section, "iCustomKeybind", std::to_string(value).c_str());
        save(ini);

        Custom_Parkour_Key = value;
    }
    void Setters::SetPresetParkourKey(RE::StaticFunctionTag *, int32_t value) {
        auto ini = RuntimeMethods::GetIniHandle();
        ini->SetValue(Section, "iPresetKeyIndex", std::to_string(value).c_str());
        save(ini);

        Preset_Parkour_Key = value;
    }
    void Setters::SetParkourDelay(RE::StaticFunctionTag *, float value) {
        auto ini = RuntimeMethods::GetIniHandle();
        ini->SetValue(Section, "fInputDelay", std::to_string(value).c_str());
        save(ini);

        Parkour_Delay = value;
    }
    void Setters::SetSmartSteps(RE::StaticFunctionTag *, bool value) {
        auto ini = RuntimeMethods::GetIniHandle();
        ini->SetBoolValue(Section, "bSmartSteps", value);
        save(ini);

        Smart_Steps = value;
    }
    void Setters::SetSmartVault(RE::StaticFunctionTag *, bool value) {
        auto ini = RuntimeMethods::GetIniHandle();
        ini->SetBoolValue(Section, "bSmartVault", value);
        save(ini);

        Smart_Vault = value;
    }
    void Setters::SetSmartClimb(RE::StaticFunctionTag *, bool value) {
        auto ini = RuntimeMethods::GetIniHandle();
        ini->SetBoolValue(Section, "bSmartClimb", value);
        save(ini);

        Smart_Climb = value;
    }
    void Setters::save(const std::unique_ptr<CSimpleIniA> &ini) {
        ini->SaveFile(IniSettings::INIPath.c_str());
    }
}  // namespace SkyParkour_Papyrus