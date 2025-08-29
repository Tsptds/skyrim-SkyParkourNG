#include "Papyrus/PapyrusInterface.h"
#include "Papyrus/Getters.h"
#include "Papyrus/Setters.h"
#include "Papyrus/Translations.h"

#include "_References/ModSettings.h"
#include "Util/ParkourUtility.h"
#include "_References/IniSettings.h"
#include "Parkouring.h"

namespace SkyParkour_Papyrus {
    using namespace ModSettings;

    void Internal::AlertPlayerLoaded(RE::StaticFunctionTag *) {
        // Turn on if setting is on and is not beast form. Same logic on race change listener.
        Parkouring::SetParkourOnOff(Mod_Enabled && !ParkourUtility::IsBeastForm());
        GET_PLAYER->SetGraphVariableFloat(SPPF_SPEEDMULT, Playback_Speed);
    }

    void Internal::Read_All_MCM_From_INI_and_Cache_Settings() {
        auto ini = IniSettings::GetIniHandle();

        /* Parkour Settings */
        Mod_Enabled = ini->GetBoolValue(Section, "bEnableMod");
        Use_Indicators = ini->GetBoolValue(Section, "bShowIndicators");
        Playback_Speed = static_cast<float>(ini->GetDoubleValue(Section, "fPlaybackSpeed"));

        /* Stamina Settings */
        Enable_Stamina_Consumption = ini->GetBoolValue(Section, "bEnableStaminaSystem");
        Must_Have_Stamina = ini->GetBoolValue(Section, "bMustHaveStamina");
        Stamina_Damage = static_cast<float>(ini->GetDoubleValue(Section, "iBaseStaminaDamage"));

        /* Input Settings */
        Use_Preset_Parkour_Key = ini->GetBoolValue(Section, "bUsePresetKey");
        Preset_Parkour_Key = static_cast<int32_t>(ini->GetDoubleValue(Section, "iPresetKeyIndex"));
        Custom_Parkour_Key = static_cast<int32_t>(ini->GetDoubleValue(Section, "iCustomKeybind"));
        Parkour_Delay = static_cast<float>(ini->GetDoubleValue(Section, "fInputDelay"));

        Smart_Steps = ini->GetBoolValue(Section, "bSmartSteps");
        Smart_Vault = ini->GetBoolValue(Section, "bSmartVault");
        Smart_Climb = ini->GetBoolValue(Section, "bSmartClimb");
    }
    void Internal::RegisterPapyrusFuncsToVM(RE::BSScript::IVirtualMachine *vm) {
        // Maintenance calls this to start polling updates on player load
        vm->RegisterFunction("AlertPlayerLoaded", className, Internal::AlertPlayerLoaded);

        Getters::RegisterFuncs(vm);
        Setters::RegisterFuncs(vm);
        Translations::RegisterFuncs(vm);
    }

}  // namespace SkyParkour_Papyrus