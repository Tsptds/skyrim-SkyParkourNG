#include "Papyrus/Getters.h"
#include "_References/ModSettings.h"
#include "_References/RuntimeMethods.h"

namespace SkyParkour_Papyrus {

    using namespace ModSettings;

    void Getters::RegisterFuncs(RE::BSScript::IVirtualMachine *vm){
        vm->RegisterFunction("GetEnableMod", className, GetEnableMod);
        vm->RegisterFunction("GetShowIndicators", className, GetShowIndicators);
        vm->RegisterFunction("GetPlaybackSpeed", className, GetPlaybackSpeed);
        vm->RegisterFunction("GetEnableStaminaSystem", className, GetEnableStaminaSystem);
        vm->RegisterFunction("GetMustHaveStamina", className, GetMustHaveStamina);
        vm->RegisterFunction("GetBaseStaminaDamage", className, GetBaseStaminaDamage);
        vm->RegisterFunction("GetUsePresetKey", className, GetUsePresetKey);
        vm->RegisterFunction("GetCustomParkourKey", className, GetCustomParkourKey);
        vm->RegisterFunction("GetPresetParkourKey", className, GetPresetParkourKey);
        vm->RegisterFunction("GetParkourDelay", className, GetParkourDelay);
        vm->RegisterFunction("GetSmartSteps", className, GetSmartSteps);
        vm->RegisterFunction("GetSmartVault", className, GetSmartVault);
        vm->RegisterFunction("GetSmartClimb", className, GetSmartClimb);
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