#pragma once
#include "ModSettings/ModSettings.hpp"

namespace SkyParkourINI
{
    inline std::string INIPath{"./Data/SKSE/Plugins/SkyParkourNG.ini"};
    inline static const char *SectionMCM = "MCM";
    inline static const char *SectionDebug = "Debug";
    inline static const char *SectionExperimental = "Experimental";

    inline void CreateDefault(std::unique_ptr<CSimpleIniA> &ini)
    {
        // Default values

        /* Parkour */
        ini->SetValue("MCM", "bEnableMod", "true");
        ini->SetValue("MCM", "bShowIndicators", "true");
        ini->SetValue("MCM", "fPlaybackSpeed", "1.15");

        /* Input */
        ini->SetValue("MCM", "bUsePresetKey", "true");
        ini->SetValue("MCM", "iPresetKeyIndex", "0");
        ini->SetValue("MCM", "iCustomKeybind", "0");
        ini->SetValue("MCM", "fInputDelay", "0.0");

        /* Stamina */
        ini->SetValue("MCM", "bEnableStaminaSystem", "true");
        ini->SetValue("MCM", "bMustHaveStamina", "true");
        ini->SetValue("MCM", "iBaseStaminaDamage", "20");

        /* Smart Parkour */
        ini->SetValue("MCM", "iAutoParkour", "1");
        ini->SetValue("MCM", "bSmartSteps", "true");
        ini->SetValue("MCM", "bSmartVault", "true");
        ini->SetValue("MCM", "bSmartClimb", "true");

        /* Slide */
        ini->SetValue("MCM", "bEnableCrouchSlide", "true");
        ini->SetValue("MCM", "bEnableAdvancedSlide", "true");
        ini->SetValue("Experimental", "bSlideTackle", "false");

        /* Roll */
        ini->SetValue("MCM", "bEnableLandRolling", "true");

        /* Debug */
        ini->SetValue("Debug", "bDebugEnabled", "false");
    }

    inline std::unique_ptr<CSimpleIniA> GetIniHandle()
    {
        const auto path = INIPath.c_str();

        auto ini = std::make_unique<CSimpleIniA>();
        ini->SetUnicode();

        SI_Error rc = ini->LoadFile(path);
        if (rc < 0)
        {
            WARN("SkyParkourNG.ini not found, creating with default values");

            CreateDefault(ini);

            if (ini->SaveFile(path) < 0)
            {
                ERROR("Failed to create default SkyParkourNG.ini");
                return nullptr;
            }
        }

        return ini;
    }

    inline void Read_All_MCM_From_INI_and_Cache_Settings()
    {
        namespace ms = ModSettings;
        const auto ini = GetIniHandle();

        if (!ini)
        {
            ERROR("INI FILE DOES NOT EXIST AND FAILED TO CREATE");
            return;
        }

        /* Parkour Settings */
        ms::Parkour_Enabled = ini->GetBoolValue(SectionMCM, "bEnableMod", true);
        ms::Use_Indicators = ini->GetBoolValue(SectionMCM, "bShowIndicators", true);
        ms::Playback_Speed = static_cast<float>(ini->GetDoubleValue(SectionMCM, "fPlaybackSpeed", 1.15f));

        /* Input Settings */
        ms::Use_Preset_Parkour_Key = ini->GetBoolValue(SectionMCM, "bUsePresetKey", true);
        ms::Preset_Parkour_Key = static_cast<int32_t>(ini->GetDoubleValue(SectionMCM, "iPresetKeyIndex", 0));
        ms::Custom_Parkour_Key = static_cast<int32_t>(ini->GetDoubleValue(SectionMCM, "iCustomKeybind", 0));
        ms::Parkour_Delay = static_cast<float>(ini->GetDoubleValue(SectionMCM, "fInputDelay", 0));

        /* Stamina Settings */
        ms::Enable_Stamina_Consumption = ini->GetBoolValue(SectionMCM, "bEnableStaminaSystem", true);
        ms::Must_Have_Stamina = ini->GetBoolValue(SectionMCM, "bMustHaveStamina", true);
        ms::Stamina_Damage = static_cast<float>(ini->GetDoubleValue(SectionMCM, "iBaseStaminaDamage", 20.f));

        /* Smart Parkour */
        ms::Auto_Parkour = static_cast<int32_t>(ini->GetDoubleValue(SectionMCM, "iAutoParkour", 1));
        ms::Smart_Steps = ini->GetBoolValue(SectionMCM, "bSmartSteps", true);
        ms::Smart_Vault = ini->GetBoolValue(SectionMCM, "bSmartVault", true);
        ms::Smart_Climb = ini->GetBoolValue(SectionMCM, "bSmartClimb", true);

        /* Slide Settings */
        ms::Crouch_Slide_Enabled = ini->GetBoolValue(SectionMCM, "bEnableCrouchSlide", true);
        ms::Advanced_Slide_Sneak = ini->GetBoolValue(SectionMCM, "bEnableAdvancedSlide", true);
        ms::ExpSlideTackle = ini->GetBoolValue("Experimental", "bSlideTackle", false);

        /* Roll Settings */
        ms::Land_Rolling_Enabled = ini->GetBoolValue(SectionMCM, "bEnableLandRolling", true);

        /* Debug */
        ms::_Debug_Enabled = ini->GetBoolValue("Debug", "bDebugEnabled", false);
    }

}