#include "Papyrus/Getters.h"
#include "_References/ModSettings.h"
#include "_References/RuntimeMethods.h"

namespace SkyParkour_Papyrus {

    using namespace ModSettings;

    void Getters::RegisterFuncs(RE::BSScript::IVirtualMachine *vm){
        vm->RegisterFunction("GetEnableMod", "SkyParkourPapyrus", GetEnableMod);
        vm->RegisterFunction("GetShowIndicators", "SkyParkourPapyrus", GetShowIndicators);
        vm->RegisterFunction("GetPlaybackSpeed", "SkyParkourPapyrus", GetPlaybackSpeed);
        vm->RegisterFunction("GetEnableStaminaSystem", "SkyParkourPapyrus", GetEnableStaminaSystem);
        vm->RegisterFunction("GetMustHaveStamina", "SkyParkourPapyrus", GetMustHaveStamina);
        vm->RegisterFunction("GetBaseStaminaDamage", "SkyParkourPapyrus", GetBaseStaminaDamage);
        vm->RegisterFunction("GetUsePresetKey", "SkyParkourPapyrus", GetUsePresetKey);
        vm->RegisterFunction("GetCustomParkourKey", "SkyParkourPapyrus", GetCustomParkourKey);
        vm->RegisterFunction("GetPresetParkourKey", "SkyParkourPapyrus", GetPresetParkourKey);
        vm->RegisterFunction("GetParkourDelay", "SkyParkourPapyrus", GetParkourDelay);
        vm->RegisterFunction("GetSmartSteps", "SkyParkourPapyrus", GetSmartSteps);
        vm->RegisterFunction("GetSmartVault", "SkyParkourPapyrus", GetSmartVault);
        vm->RegisterFunction("GetSmartClimb", "SkyParkourPapyrus", GetSmartClimb);
    }

    bool Getters::GetEnableMod(RE::StaticFunctionTag *) {
        return Mod_Enabled;
    }
    bool Getters::GetShowIndicators(RE::StaticFunctionTag *) {
        return Use_Indicators;
    }
    float Getters::GetPlaybackSpeed(RE::StaticFunctionTag *) {
        return Playback_Speed;
    }
    bool Getters::GetEnableStaminaSystem(RE::StaticFunctionTag *) {
        return Enable_Stamina_Consumption;
    }
    bool Getters::GetMustHaveStamina(RE::StaticFunctionTag *) {
        return Must_Have_Stamina;
    }
    float Getters::GetBaseStaminaDamage(RE::StaticFunctionTag *) {
        return Stamina_Damage;
    }
    bool Getters::GetUsePresetKey(RE::StaticFunctionTag *) {
        return Use_Preset_Parkour_Key;
    }
    int32_t Getters::GetCustomParkourKey(RE::StaticFunctionTag *) {
        return Custom_Parkour_Key;
    }
    int32_t Getters::GetPresetParkourKey(RE::StaticFunctionTag *) {
        return Preset_Parkour_Key;
    }
    float Getters::GetParkourDelay(RE::StaticFunctionTag *) {
        return Parkour_Delay;
    }
    bool Getters::GetSmartSteps(RE::StaticFunctionTag *) {
        return Smart_Steps;
    }
    bool Getters::GetSmartVault(RE::StaticFunctionTag *) {
        return Smart_Vault;
    }
    bool Getters::GetSmartClimb(RE::StaticFunctionTag *) {
        return Smart_Climb;
    }
}  // namespace SkyParkour_Papyrus