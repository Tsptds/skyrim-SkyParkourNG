#include "Papyrus/Translations.h"
#include "_References/Localized_ini.h"

namespace SkyParkour_Papyrus {

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
    RE::BSFixedString Translations::Input::PresetKeyList(RE::StaticFunctionTag *) {
        const auto &ini = Localized_ini::GetIniHandle();

        const auto &val = ini->GetValue(Section, "PresetKeyList");
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