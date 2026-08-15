#include "Papyrus/Setters.h"
#include "_References/ModSettings.h"
#include "Util/ParkourUtility.h"
#include "Util/HavokUtil.hpp"
#include "_References/IniSettings.h"
#include "Parkouring.h"
#include "CrouchSliding.h"
#include "HUD/Scaleform/SkyParkourMenu.hpp"

namespace SkyParkour_Papyrus
{
    using namespace ModSettings;

    void Setters::RegisterFuncs(RE::BSScript::IVirtualMachine *vm)
    {
        vm->RegisterFunction("SetEnableDebug", className, SetEnableDebug);
        vm->RegisterFunction("SetEnableMod", className, SetEnableMod);
        vm->RegisterFunction("SetShowIndicators", className, SetShowIndicators);
        vm->RegisterFunction("SetPlaybackSpeed", className, SetPlaybackSpeed);
        vm->RegisterFunction("SetEnableCrouchSlide", className, SetEnableCrouchSlide);
        vm->RegisterFunction("SetEnableStaminaSystem", className, SetEnableStaminaSystem);
        vm->RegisterFunction("SetMustHaveStamina", className, SetMustHaveStamina);
        vm->RegisterFunction("SetBaseStaminaDamage", className, SetBaseStaminaDamage);
        vm->RegisterFunction("SetUsePresetKey", className, SetUsePresetKey);
        vm->RegisterFunction("SetCustomParkourKey", className, SetCustomParkourKey);
        vm->RegisterFunction("SetPresetParkourKey", className, SetPresetParkourKey);
        vm->RegisterFunction("SetParkourDelay", className, SetParkourDelay);
        vm->RegisterFunction("SetAutoParkour", className, SetAutoParkour);
        vm->RegisterFunction("SetSmartSteps", className, SetSmartSteps);
        vm->RegisterFunction("SetSmartVault", className, SetSmartVault);
        vm->RegisterFunction("SetSmartClimb", className, SetSmartClimb);
    }

    bool Setters::SetEnableDebug(RE::StaticFunctionTag *, bool value)
    {
        bool debugSwapped{false};
        if (API_Handles::TrueHUD::Get())
        {
            auto &draw = _Debug_Enabled;
            draw = !draw;
            const char *msg = (std::string("SkyParkour Visual Debugging ") + (draw ? "Enabled" : "Disabled")).c_str();
            INFO("{}", msg);
            RE::ConsoleLog::GetSingleton()->Print(msg);

            Scaleform::SkyParkourMenu::GetSingleton()->ShowDebugOverlay(draw);

            debugSwapped = true;
        }
        else
        {
            WARN("Can't enable debug, TrueHud handle not found");
            // RE::ConsoleLog::GetSingleton()->Print("TrueHUD not found, SkyParkour debugging isn't available");
            RE::DebugMessageBox("TrueHUD not found, SkyParkour debugging isn't available");
        }

        if (debugSwapped)
        {
            auto ini = GetINI();
            ini->SetBoolValue(SectionDebug, "bDebugEnabled", value);
            save(ini);

            _Debug_Enabled = value;
        }
        return debugSwapped;
    }
    void Setters::SetEnableMod(RE::StaticFunctionTag *, bool value)
    {
        auto ini = GetINI();
        ini->SetBoolValue(SectionMCM, "bEnableMod", value);
        save(ini);

        Parkour_Enabled = value;

        // Turn on if setting is on and is not beast form. Same logic on race change listener.
        Parkouring::SetParkourOnOff(Parkour_Enabled && !ParkourUtility::IsBeastForm());
    }
    void Setters::SetShowIndicators(RE::StaticFunctionTag *, bool value)
    {
        auto ini = GetINI();
        ini->SetBoolValue(SectionMCM, "bShowIndicators", value);
        save(ini);

        Use_Indicators = value;

        if (!Use_Indicators)
        {
            using sppf = Scaleform::SkyParkourMenu;
            const auto menu = sppf::GetSingleton();
            if (menu && menu->IsOpen()) menu->SetActiveIndicatorType(sppf::IndicatorType::kInvisible);
        }
    }
    void Setters::SetPlaybackSpeed(RE::StaticFunctionTag *, float value)
    {
        auto ini = GetINI();
        ini->SetValue(SectionMCM, "fPlaybackSpeed", std::to_string(value).c_str());
        save(ini);

        Playback_Speed = value;
        /* Set the graph variable as well, above is internal */
        // GET_PLAYER->SetGraphVariableFloat(SPPF_SPEEDMULT, value);
        // Use the new bound channel for this
        HavokUtil::SetBoundSpeedMult(GET_PLAYER, Playback_Speed);
    }
    void Setters::SetEnableCrouchSlide(RE::StaticFunctionTag *, bool value)
    {
        auto ini = GetINI();
        ini->SetBoolValue(SectionMCM, "bEnableCrouchSlide", value);
        save(ini);

        Crouch_Slide_Enabled = value;

        // Turn on if setting is on and is not beast form. Same logic on race change listener.
        CrouchSliding::SetSlideOnOff(Crouch_Slide_Enabled && !ParkourUtility::IsBeastForm());
    }
    void Setters::SetEnableStaminaSystem(RE::StaticFunctionTag *, bool value)
    {
        auto ini = GetINI();
        ini->SetBoolValue(SectionMCM, "bEnableStaminaSystem", value);
        save(ini);

        Enable_Stamina_Consumption = value;
    }
    void Setters::SetMustHaveStamina(RE::StaticFunctionTag *, bool value)
    {
        auto ini = GetINI();
        ini->SetBoolValue(SectionMCM, "bMustHaveStamina", value);
        save(ini);

        Must_Have_Stamina = value;
    }
    void Setters::SetBaseStaminaDamage(RE::StaticFunctionTag *, float value)
    {
        auto ini = GetINI();
        ini->SetValue(SectionMCM, "iBaseStaminaDamage", std::to_string(value).c_str());
        save(ini);

        Stamina_Damage = value;
    }
    void Setters::SetUsePresetKey(RE::StaticFunctionTag *, bool value)
    {
        auto ini = GetINI();
        ini->SetBoolValue(SectionMCM, "bUsePresetKey", value);
        save(ini);

        Use_Preset_Parkour_Key = value;
    }
    void Setters::SetCustomParkourKey(RE::StaticFunctionTag *, int32_t value)
    {
        auto ini = GetINI();
        ini->SetValue(SectionMCM, "iCustomKeybind", std::to_string(value).c_str());
        save(ini);

        Custom_Parkour_Key = value;
    }
    void Setters::SetPresetParkourKey(RE::StaticFunctionTag *, int32_t value)
    {
        auto ini = GetINI();
        ini->SetValue(SectionMCM, "iPresetKeyIndex", std::to_string(value).c_str());
        save(ini);

        Preset_Parkour_Key = value;
    }
    void Setters::SetParkourDelay(RE::StaticFunctionTag *, float value)
    {
        auto ini = GetINI();
        ini->SetValue(SectionMCM, "fInputDelay", std::to_string(value).c_str());
        save(ini);

        Parkour_Delay = value;
    }
    void Setters::SetAutoParkour(RE::StaticFunctionTag *, int32_t value)
    {
        auto ini = GetINI();
        ini->SetValue(SectionMCM, "iAutoParkour", std::to_string(value).c_str());
        save(ini);

        Auto_Parkour = value;
    }
    void Setters::SetSmartSteps(RE::StaticFunctionTag *, bool value)
    {
        auto ini = GetINI();
        ini->SetBoolValue(SectionMCM, "bSmartSteps", value);
        save(ini);

        Smart_Steps = value;
    }
    void Setters::SetSmartVault(RE::StaticFunctionTag *, bool value)
    {
        auto ini = GetINI();
        ini->SetBoolValue(SectionMCM, "bSmartVault", value);
        save(ini);

        Smart_Vault = value;
    }
    void Setters::SetSmartClimb(RE::StaticFunctionTag *, bool value)
    {
        auto ini = GetINI();
        ini->SetBoolValue(SectionMCM, "bSmartClimb", value);
        save(ini);

        Smart_Climb = value;
    }

    void Setters::save(const std::unique_ptr<CSimpleIniA> &ini) { ini->SaveFile(IniSettings::INIPath.c_str()); }
    std::unique_ptr<CSimpleIniA> Setters::GetINI() { return IniSettings::GetIniHandle(); }
}  // namespace SkyParkour_Papyrus