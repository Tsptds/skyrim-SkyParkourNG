#include "Papyrus/PapyrusInterface.h"
#include "Papyrus/Getters.h"
#include "Papyrus/Setters.h"

#include "_References/ModSettings.h"
#include "Util/ParkourUtility.h"
#include "_References/IniSettings.h"
#include "Parkouring.h"
#include "CrouchSliding.h"
#include "Util/HavokUtil.hpp"

namespace SkyParkour_Papyrus {
    using namespace ModSettings;

    void Internal::AlertPlayerLoaded(RE::StaticFunctionTag *) {
        // Turn on if setting is on and is not beast form. Same logic on race change listener.
        Parkouring::SetParkourOnOff(Parkour_Enabled && !ParkourUtility::IsBeastForm());
        CrouchSliding::SetSlideOnOff(Crouch_Slide_Enabled && !ParkourUtility::IsBeastForm());
        GET_PLAYER->SetGraphVariableFloat(SPPF_SPEEDMULT, Playback_Speed);
        HavokUtil::CreateBoundGraphChannels(GET_PLAYER);
    }

    void Internal::Read_All_MCM_From_INI_and_Cache_Settings() {
        auto ini = IniSettings::GetIniHandle();

        /* Parkour Settings */
        Parkour_Enabled = ini->GetBoolValue(Section, "bEnableMod", true);
        Use_Indicators = ini->GetBoolValue(Section, "bShowIndicators", true);
        Playback_Speed = static_cast<float>(ini->GetDoubleValue(Section, "fPlaybackSpeed", 1.15f));
        Crouch_Slide_Enabled = ini->GetBoolValue(Section, "bEnableCrouchSlide", true);

        /* Stamina Settings */
        Enable_Stamina_Consumption = ini->GetBoolValue(Section, "bEnableStaminaSystem", true);
        Must_Have_Stamina = ini->GetBoolValue(Section, "bMustHaveStamina", true);
        Stamina_Damage = static_cast<float>(ini->GetDoubleValue(Section, "iBaseStaminaDamage", 20.f));

        /* Input Settings */
        Use_Preset_Parkour_Key = ini->GetBoolValue(Section, "bUsePresetKey", true);
        Preset_Parkour_Key = static_cast<int32_t>(ini->GetDoubleValue(Section, "iPresetKeyIndex", 0));
        Custom_Parkour_Key = static_cast<int32_t>(ini->GetDoubleValue(Section, "iCustomKeybind", 0));
        Parkour_Delay = static_cast<float>(ini->GetDoubleValue(Section, "fInputDelay", 0));
        
        /* Smart Parkour */
        Auto_Parkour = static_cast<int32_t>(ini->GetDoubleValue(Section, "iAutoParkour", 1));
        Smart_Steps = ini->GetBoolValue(Section, "bSmartSteps", true);
        Smart_Vault = ini->GetBoolValue(Section, "bSmartVault", true);
        Smart_Climb = ini->GetBoolValue(Section, "bSmartClimb", true);

        ExpSlideTackle = ini->GetBoolValue("Experimental", "bSlideTackle", false);
        _Debug_Enabled = ini->GetBoolValue("Debug", "bDebugEnabled", false);
    }
    void Internal::RegisterPapyrusFuncsToVM(RE::BSScript::IVirtualMachine *vm) {
        // Maintenance calls this to start polling updates on player load
        vm->RegisterFunction("AlertPlayerLoaded", className, Internal::AlertPlayerLoaded);

        Getters::RegisterFuncs(vm);
        Setters::RegisterFuncs(vm);
    }

}  // namespace SkyParkour_Papyrus