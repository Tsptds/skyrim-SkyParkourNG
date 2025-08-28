#include "Papyrus/Translations.h"
#include "_References/Localized_ini.h"

namespace SkyParkour_Papyrus {

    void Translations::RegisterFuncs(RE::BSScript::IVirtualMachine *vm) {
        const auto pf_core = "Core_"s;
        const auto pf_input = "Input_"s;
        const auto pf_stamina = "Stamina_"s;
        const auto pf_smart = "SmartParkour_"s;
        const auto pf_tooltip = "Tooltip_"s;
        const auto pf_warn = "Warn_"s;

        // Core Settings
        vm->RegisterFunction(pf_core + "Header", className, Core::Header);
        vm->RegisterFunction(pf_core + "OnOff", className, Core::OnOff);
        vm->RegisterFunction(pf_core + "Indicator", className, Core::Indicator);
        vm->RegisterFunction(pf_core + "PlaybackSpeed", className, Core::PlaybackSpeed);

        // Input Settings
        vm->RegisterFunction(pf_input + "Header", className, Input::Header);
        vm->RegisterFunction(pf_input + "UsePresetKey", className, Input::UsePresetKey);
        vm->RegisterFunction(pf_input + "PresetKeyIndex", className, Input::PresetKeyIndex);
        vm->RegisterFunction(pf_input + "PresetKey0", className, Input::PresetKey0);
        vm->RegisterFunction(pf_input + "PresetKey1", className, Input::PresetKey1);
        vm->RegisterFunction(pf_input + "PresetKey2", className, Input::PresetKey2);
        vm->RegisterFunction(pf_input + "CustomKey", className, Input::CustomKey);
        vm->RegisterFunction(pf_input + "Delay", className, Input::Delay);

        // Stamina Settings
        vm->RegisterFunction(pf_stamina + "Header", className, Stamina::Header);
        vm->RegisterFunction(pf_stamina + "StaminaSystem", className, Stamina::StaminaSystem);
        vm->RegisterFunction(pf_stamina + "MustHaveStamina", className, Stamina::MustHaveStamina);
        vm->RegisterFunction(pf_stamina + "BaseStaminaCost", className, Stamina::BaseStaminaCost);

        // Smart Parkour Settings
        vm->RegisterFunction(pf_smart + "Header", className, SmartParkour::Header);
        vm->RegisterFunction(pf_smart + "Steps", className, SmartParkour::Steps);
        vm->RegisterFunction(pf_smart + "Vault", className, SmartParkour::Vault);
        vm->RegisterFunction(pf_smart + "Climb", className, SmartParkour::Climb);

        // MCM Tooltip Info
        vm->RegisterFunction(pf_tooltip + "OnOff", className, MCM_Info::OnOff);
        vm->RegisterFunction(pf_tooltip + "Indicator", className, MCM_Info::Indicator);
        vm->RegisterFunction(pf_tooltip + "PlaybackSpeed", className, MCM_Info::PlaybackSpeed);
        vm->RegisterFunction(pf_tooltip + "UsePresetKey", className, MCM_Info::UsePresetKey);
        vm->RegisterFunction(pf_tooltip + "CustomKey", className, MCM_Info::CustomKey);
        vm->RegisterFunction(pf_tooltip + "Delay", className, MCM_Info::Delay);
        vm->RegisterFunction(pf_tooltip + "StaminaSystem", className, MCM_Info::StaminaSystem);
        vm->RegisterFunction(pf_tooltip + "MustHaveStamina", className, MCM_Info::MustHaveStamina);
        vm->RegisterFunction(pf_tooltip + "StaminaCost", className, MCM_Info::StaminaCost);
        vm->RegisterFunction(pf_tooltip + "SmartSteps", className, MCM_Info::SmartSteps);
        vm->RegisterFunction(pf_tooltip + "SmartVault", className, MCM_Info::SmartVault);
        vm->RegisterFunction(pf_tooltip + "SmartClimb", className, MCM_Info::SmartClimb);

        // MCM Warnings
        vm->RegisterFunction(pf_warn + "AreYouSure", className, MCM_Warn::AreYouSure);
        vm->RegisterFunction(pf_warn + "KeyAlreadyMapped", className, MCM_Warn::KeyAlreadyMapped);
    }

    // Core Settings
    RE::BSFixedString Translations::Core::Header(RE::StaticFunctionTag *) {
        const auto &ini = Localized_ini::GetIniHandle();

        const auto &val = ini->GetValue(Section, "Header");
        return RE::BSFixedString(val);
    }
    RE::BSFixedString Translations::Core::OnOff(RE::StaticFunctionTag *) {
        const auto &ini = Localized_ini::GetIniHandle();

        const auto &val = ini->GetValue(Section, "OnOff");
        return RE::BSFixedString(val);
    }
    RE::BSFixedString Translations::Core::Indicator(RE::StaticFunctionTag *) {
        const auto &ini = Localized_ini::GetIniHandle();

        const auto &val = ini->GetValue(Section, "Indicator");
        return RE::BSFixedString(val);
    }
    RE::BSFixedString Translations::Core::PlaybackSpeed(RE::StaticFunctionTag *) {
        const auto &ini = Localized_ini::GetIniHandle();

        const auto &val = ini->GetValue(Section, "PlaybackSpeed");
        return RE::BSFixedString(val);
    }

    // Input Settings
    RE::BSFixedString Translations::Input::Header(RE::StaticFunctionTag *) {
        const auto &ini = Localized_ini::GetIniHandle();

        const auto &val = ini->GetValue(Section, "Header");
        return RE::BSFixedString(val);
    }
    RE::BSFixedString Translations::Input::UsePresetKey(RE::StaticFunctionTag *) {
        const auto &ini = Localized_ini::GetIniHandle();

        const auto &val = ini->GetValue(Section, "UsePresetKey");
        return RE::BSFixedString(val);
    }
    RE::BSFixedString Translations::Input::PresetKeyIndex(RE::StaticFunctionTag *) {
        const auto &ini = Localized_ini::GetIniHandle();

        const auto &val = ini->GetValue(Section, "PresetKeyIndex");
        return RE::BSFixedString(val);
    }
    RE::BSFixedString Translations::Input::PresetKey0(RE::StaticFunctionTag *) {
        const auto &ini = Localized_ini::GetIniHandle();

        const auto &val = ini->GetValue(Section, "PresetKey0");
        return RE::BSFixedString(val);
    }
    RE::BSFixedString Translations::Input::PresetKey1(RE::StaticFunctionTag *) {
        const auto &ini = Localized_ini::GetIniHandle();

        const auto &val = ini->GetValue(Section, "PresetKey1");
        return RE::BSFixedString(val);
    }
    RE::BSFixedString Translations::Input::PresetKey2(RE::StaticFunctionTag *) {
        const auto &ini = Localized_ini::GetIniHandle();

        const auto &val = ini->GetValue(Section, "PresetKey2");
        return RE::BSFixedString(val);
    }
    RE::BSFixedString Translations::Input::CustomKey(RE::StaticFunctionTag *) {
        const auto &ini = Localized_ini::GetIniHandle();

        const auto &val = ini->GetValue(Section, "CustomKey");
        return RE::BSFixedString(val);
    }
    RE::BSFixedString Translations::Input::Delay(RE::StaticFunctionTag *) {
        const auto &ini = Localized_ini::GetIniHandle();

        const auto &val = ini->GetValue(Section, "Delay");
        return RE::BSFixedString(val);
    }

    // Stamina Settings
    RE::BSFixedString Translations::Stamina::Header(RE::StaticFunctionTag *) {
        const auto &ini = Localized_ini::GetIniHandle();

        const auto &val = ini->GetValue(Section, "Header");
        return RE::BSFixedString(val);
    }
    RE::BSFixedString Translations::Stamina::StaminaSystem(RE::StaticFunctionTag *) {
        const auto &ini = Localized_ini::GetIniHandle();

        const auto &val = ini->GetValue(Section, "StaminaSystem");
        return RE::BSFixedString(val);
    }
    RE::BSFixedString Translations::Stamina::MustHaveStamina(RE::StaticFunctionTag *) {
        const auto &ini = Localized_ini::GetIniHandle();

        const auto &val = ini->GetValue(Section, "MustHaveStamina");
        return RE::BSFixedString(val);
    }
    RE::BSFixedString Translations::Stamina::BaseStaminaCost(RE::StaticFunctionTag *) {
        const auto &ini = Localized_ini::GetIniHandle();

        const auto &val = ini->GetValue(Section, "BaseStaminaCost");
        return RE::BSFixedString(val);
    }

    // Smart Parkour Settings
    RE::BSFixedString Translations::SmartParkour::Header(RE::StaticFunctionTag *) {
        const auto &ini = Localized_ini::GetIniHandle();

        const auto &val = ini->GetValue(Section, "Header");
        return RE::BSFixedString(val);
    }
    RE::BSFixedString Translations::SmartParkour::Steps(RE::StaticFunctionTag *) {
        const auto &ini = Localized_ini::GetIniHandle();

        const auto &val = ini->GetValue(Section, "Steps");
        return RE::BSFixedString(val);
    }
    RE::BSFixedString Translations::SmartParkour::Vault(RE::StaticFunctionTag *) {
        const auto &ini = Localized_ini::GetIniHandle();

        const auto &val = ini->GetValue(Section, "Vault");
        return RE::BSFixedString(val);
    }
    RE::BSFixedString Translations::SmartParkour::Climb(RE::StaticFunctionTag *) {
        const auto &ini = Localized_ini::GetIniHandle();

        const auto &val = ini->GetValue(Section, "Climb");
        return RE::BSFixedString(val);
    }

    // MCM Tooltip Info
    RE::BSFixedString Translations::MCM_Info::OnOff(RE::StaticFunctionTag *) {
        const auto &ini = Localized_ini::GetIniHandle();

        const auto &val = ini->GetValue(Section, "OnOff");
        return RE::BSFixedString(val);
    }
    RE::BSFixedString Translations::MCM_Info::Indicator(RE::StaticFunctionTag *) {
        const auto &ini = Localized_ini::GetIniHandle();

        const auto &val = ini->GetValue(Section, "Indicator");
        return RE::BSFixedString(val);
    }
    RE::BSFixedString Translations::MCM_Info::PlaybackSpeed(RE::StaticFunctionTag *) {
        const auto &ini = Localized_ini::GetIniHandle();

        const auto &val = ini->GetValue(Section, "PlaybackSpeed");
        return RE::BSFixedString(val);
    }
    RE::BSFixedString Translations::MCM_Info::UsePresetKey(RE::StaticFunctionTag *) {
        const auto &ini = Localized_ini::GetIniHandle();

        const auto &val = ini->GetValue(Section, "UsePresetKey");
        return RE::BSFixedString(val);
    }
    RE::BSFixedString Translations::MCM_Info::CustomKey(RE::StaticFunctionTag *) {
        const auto &ini = Localized_ini::GetIniHandle();

        const auto &val = ini->GetValue(Section, "CustomKey");
        return RE::BSFixedString(val);
    }
    RE::BSFixedString Translations::MCM_Info::Delay(RE::StaticFunctionTag *) {
        const auto &ini = Localized_ini::GetIniHandle();

        const auto &val = ini->GetValue(Section, "Delay");
        return RE::BSFixedString(val);
    }
    RE::BSFixedString Translations::MCM_Info::StaminaSystem(RE::StaticFunctionTag *) {
        const auto &ini = Localized_ini::GetIniHandle();

        const auto &val = ini->GetValue(Section, "StaminaSystem");
        return RE::BSFixedString(val);
    }
    RE::BSFixedString Translations::MCM_Info::MustHaveStamina(RE::StaticFunctionTag *) {
        const auto &ini = Localized_ini::GetIniHandle();

        const auto &val = ini->GetValue(Section, "MustHaveStamina");
        return RE::BSFixedString(val);
    }
    RE::BSFixedString Translations::MCM_Info::StaminaCost(RE::StaticFunctionTag *) {
        const auto &ini = Localized_ini::GetIniHandle();

        const auto &val = ini->GetValue(Section, "StaminaCost");
        return RE::BSFixedString(val);
    }
    RE::BSFixedString Translations::MCM_Info::SmartSteps(RE::StaticFunctionTag *) {
        const auto &ini = Localized_ini::GetIniHandle();

        const auto &val = ini->GetValue(Section, "SmartSteps");
        return RE::BSFixedString(val);
    }
    RE::BSFixedString Translations::MCM_Info::SmartVault(RE::StaticFunctionTag *) {
        const auto &ini = Localized_ini::GetIniHandle();

        const auto &val = ini->GetValue(Section, "SmartVault");
        return RE::BSFixedString(val);
    }
    RE::BSFixedString Translations::MCM_Info::SmartClimb(RE::StaticFunctionTag *) {
        const auto &ini = Localized_ini::GetIniHandle();

        const auto &val = ini->GetValue(Section, "SmartClimb");
        return RE::BSFixedString(val);
    }

    // MCM Warnings
    RE::BSFixedString Translations::MCM_Warn::AreYouSure(RE::StaticFunctionTag *) {
        const auto &ini = Localized_ini::GetIniHandle();

        const auto &val = ini->GetValue(Section, "AreYouSure");
        return RE::BSFixedString(val);
    }
    RE::BSFixedString Translations::MCM_Warn::KeyAlreadyMapped(RE::StaticFunctionTag *) {
        const auto &ini = Localized_ini::GetIniHandle();

        const auto &val = ini->GetValue(Section, "KeyAlreadyMapped");
        return RE::BSFixedString(val);
    }
}  // namespace SkyParkour_Papyrus