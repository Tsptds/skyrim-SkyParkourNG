#pragma once
#include "Parkouring.h"
#include "CrouchSliding.h"
#include "Util/ParkourUtility.h"
#include "HUD/Scaleform/SkyParkourMenu.hpp"
#include "SkyParkourINI.hpp"
#include "ModSettings.hpp"
#include "Util/HavokUtil.hpp"

namespace INISetters
{
    namespace ms = ModSettings;
    auto SectionMCM = SkyParkourINI::SectionMCM;
    auto SectionDebug = SkyParkourINI::SectionDebug;
    auto SectionExperimental = SkyParkourINI::SectionExperimental;

    void save(const std::unique_ptr<CSimpleIniA> &ini) { ini->SaveFile(SkyParkourINI::INIPath.c_str()); }
    std::unique_ptr<CSimpleIniA> GetINI() { return SkyParkourINI::GetIniHandle(); }

    /* Parkour */
    void SetEnableParkour(bool value)
    {
        auto ini = GetINI();
        ini->SetBoolValue(SectionMCM, "bEnableMod", value);
        save(ini);

        // Turn on if setting is on and is not beast form. Same logic on race change listener.
        Parkouring::SetParkourOnOff(ms::Parkour_Enabled && !ParkourUtility::IsBeastForm());
        Parkouring::UpdateIndicatorMenu();
    }
    void SetShowIndicators(bool value)
    {
        auto ini = GetINI();
        ini->SetBoolValue(SectionMCM, "bShowIndicators", value);
        save(ini);

        if (!ms::Use_Indicators)
        {
            using sppf = Scaleform::SkyParkourMenu;
            const auto menu = sppf::GetSingleton();
            if (menu && menu->IsOpen()) menu->SetActiveIndicatorType(sppf::IndicatorType::kInvisible);
        }
    }
    void SetPlaybackSpeed(float value)
    {
        auto ini = GetINI();
        ini->SetValue(SectionMCM, "fPlaybackSpeed", std::to_string(value).c_str());
        save(ini);

        /* Set the graph variable as well, above is internal */
        // GET_PLAYER->SetGraphVariableFloat(SPPF_SPEEDMULT, value);
        // Use the new bound channel for this
        HavokUtil::SetBoundSpeedMult(GET_PLAYER, ms::Playback_Speed);
    }

    /* Input */
    void SetUsePresetKey(bool value)
    {
        auto ini = GetINI();
        ini->SetBoolValue(SectionMCM, "bUsePresetKey", value);
        save(ini);
    }
    void SetCustomParkourKey(int32_t value)
    {
        auto ini = GetINI();
        ini->SetValue(SectionMCM, "iCustomKeybind", std::to_string(value).c_str());
        save(ini);
    }
    void SetPresetParkourKey(int32_t value)
    {
        auto ini = GetINI();
        ini->SetValue(SectionMCM, "iPresetKeyIndex", std::to_string(value).c_str());
        save(ini);
    }
    void SetParkourDelay(float value)
    {
        auto ini = GetINI();
        ini->SetValue(SectionMCM, "fInputDelay", std::to_string(value).c_str());
        save(ini);
    }

    /* Stamina System */
    void SetEnableStaminaSystem(bool value)
    {
        auto ini = GetINI();
        ini->SetBoolValue(SectionMCM, "bEnableStaminaSystem", value);
        save(ini);
    }
    void SetMustHaveStamina(bool value)
    {
        auto ini = GetINI();
        ini->SetBoolValue(SectionMCM, "bMustHaveStamina", value);
        save(ini);
    }
    void SetBaseStaminaDamage(float value)
    {
        auto ini = GetINI();
        ini->SetValue(SectionMCM, "iBaseStaminaDamage", std::to_string(value).c_str());
        save(ini);
    }

    /* Smart Parkour */
    void SetAutoParkour(int32_t value)
    {
        auto ini = GetINI();
        ini->SetValue(SectionMCM, "iAutoParkour", std::to_string(value).c_str());
        save(ini);
    }
    void SetSmartSteps(bool value)
    {
        auto ini = GetINI();
        ini->SetBoolValue(SectionMCM, "bSmartSteps", value);
        save(ini);
    }
    void SetSmartVault(bool value)
    {
        auto ini = GetINI();
        ini->SetBoolValue(SectionMCM, "bSmartVault", value);
        save(ini);
    }
    void SetSmartClimb(bool value)
    {
        auto ini = GetINI();
        ini->SetBoolValue(SectionMCM, "bSmartClimb", value);
        save(ini);
    }

    /* Slide */
    void SetEnableCrouchSlide(bool value)
    {
        auto ini = GetINI();
        ini->SetBoolValue(SectionMCM, "bEnableCrouchSlide", value);
        save(ini);

        // Turn on if setting is on and is not beast form. Same logic on race change listener.
        CrouchSliding::SetSlideOnOff((ms::Crouch_Slide_Enabled || ms::Land_Rolling_Enabled) && !ParkourUtility::IsBeastForm());
    }
    void SetEnableAdvancedSlide(bool value)
    {
        auto ini = GetINI();
        ini->SetBoolValue(SectionMCM, "bEnableAdvancedSlide", value);
        save(ini);
    }
    void SetEnableSlideTackle(bool value)
    {
        auto ini = GetINI();
        ini->SetBoolValue(SectionExperimental, "bSlideTackle", value);
        save(ini);
    }

    /* Rolling */
    void SetEnableRolling(bool value)
    {
        auto ini = GetINI();
        ini->SetBoolValue(SectionMCM, "bEnableLandRolling", value);
        save(ini);

        // Turn on if setting is on and is not beast form. Same logic on race change listener.
        CrouchSliding::SetSlideOnOff((ms::Crouch_Slide_Enabled || ms::Land_Rolling_Enabled) && !ParkourUtility::IsBeastForm());
    }

    /* Debug */
    void SetEnableDebug(bool value)
    {
        bool swapped{false};
        if (API_Handles::TrueHUD::Get())
        {
            Scaleform::SkyParkourMenu::GetSingleton()->ShowDebugOverlay(value);
            swapped = true;
        }
        else
        {
            WARN("Can't enable debug, TrueHud handle not found");
            // RE::ConsoleLog::GetSingleton()->Print("TrueHUD not found, SkyParkour debugging isn't available");
            RE::DebugMessageBox("TrueHUD not found, SkyParkour debugging isn't available");
        }

        if (swapped)
        {
            auto ini = GetINI();
            ini->SetBoolValue(SectionDebug, "bDebugEnabled", value);
            save(ini);
        }
    }

}