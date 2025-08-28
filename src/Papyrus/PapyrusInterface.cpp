#include "Papyrus/PapyrusInterface.h"
#include "Papyrus/Getters.h"
#include "Papyrus/Setters.h"
#include "Papyrus/Translations.h"

#include "_References/ModSettings.h"
#include "Util/ParkourUtility.h"
#include "_References/RuntimeMethods.h"
#include "Parkouring.h"

namespace SkyParkour_Papyrus {
    using namespace ModSettings;
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