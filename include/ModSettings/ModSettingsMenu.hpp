#pragma once
#include "ModSettings.hpp"
#include "INISetters.hpp"
#include "Listeners/ButtonListener.h"
#include "Translations.hpp"

namespace ModSettingsMenu
{
    using cached = MCM_Translation::CachedStrings;
    auto CS = cached::GetSingleton();
/**/ #define CV(x) CS->x.CachedValue.c_str()

    inline bool MenuRegistered{false};

    inline bool _CapturingKey{false};
    inline uint32_t *_CaptureTarget{nullptr};
    inline uint32_t *_LastCapturedTarget{nullptr};
    inline SKSEMenuFramework::Model::InputEvent *_InputListener{nullptr};

    inline bool OnInput(RE::InputEvent *event)
    {
        if (!_CapturingKey || !_CaptureTarget || !event)
        {
            return false;
        }

        auto button = event->AsButtonEvent();
        if (!button || !button->IsDown())
        {
            return false;
        }

        if (event->device == RE::INPUT_DEVICE::kKeyboard &&
            button->GetIDCode() == static_cast<uint32_t>(RE::BSWin32KeyboardDevice::Key::kEscape))
        {
            _CapturingKey = false;
            _CaptureTarget = nullptr;
            return true;
        }

        uint32_t code = button->GetIDCode();
        if (event->device == RE::INPUT_DEVICE::kGamepad)
        {
            code = SKSE::InputMap::GamepadMaskToKeycode(code);
        }
        else if (event->device == RE::INPUT_DEVICE::kMouse)
        {
            code = Buttons::MapToCKIfPossible(code);
        }

        *_CaptureTarget = code;
        _LastCapturedTarget = _CaptureTarget;
        _CapturingKey = false;
        _CaptureTarget = nullptr;
        return true;
    }

    inline bool DrawKeyBindRow(const char *label, uint32_t *key)
    {
        bool changed{false};
        if (_LastCapturedTarget == key)
        {
            changed = true;
            _LastCapturedTarget = nullptr;
        }

        ImGuiMCP::PushID(label);

        bool isCapturingThis = _CapturingKey && _CaptureTarget == key;
        std::string btnLabel = isCapturingThis ? CV(mcmWarnPressAnyKey) : (*key != 0 ? SKSE::InputMap::GetKeyName(*key) : CV(mcmUnbound));

        if (ImGuiMCP::Button(btnLabel.c_str(), ImGuiMCP::ImVec2(150, 0)) && !isCapturingThis)
        {
            _CapturingKey = true;
            _CaptureTarget = key;
        }

        ImGuiMCP::SameLine();
        ImGuiMCP::Text("%s", label);

        ImGuiMCP::PopID();
        return changed;
    }

    inline void ShowTooltip(const char *text)
    {
        if (ImGuiMCP::IsItemHovered())
        {
            ImGuiMCP::SetTooltip("%s", text);
        }
    }

    namespace IS = INISetters;

    inline void Render()
    {
        const char *presetKeyLabels[] = {CV(presetKey0), CV(presetKey1), CV(presetKey2)};
        const char *autoParkourLabels[] = {CV(autoParkourOpt0), CV(autoParkourOpt1), CV(autoParkourOpt2)};

        if (ImGuiMCP::BeginTabBar(CV(mainSettingsHeader)))
        {
            /* Parkour */
            if (ImGuiMCP::BeginTabItem(CV(parkourHeader)))
            {
                if (ImGuiMCP::Checkbox(CV(parkourOnOff), &ModSettings::Parkour_Enabled))
                {
                    INISetters::SetEnableParkour(ModSettings::Parkour_Enabled);
                }
                ShowTooltip(CV(mcmInfoOnOff));

                if (ImGuiMCP::Checkbox(CV(indicator), &ModSettings::Use_Indicators))
                {
                    IS::SetShowIndicators(ModSettings::Use_Indicators);
                }
                ShowTooltip(CV(mcmInfoIndicator));

                if (ImGuiMCP::SliderFloat(CV(playbackSpeed), &ModSettings::Playback_Speed, 0.5f, 3.0f, "%.2fx"))
                {
                    IS::SetPlaybackSpeed(ModSettings::Playback_Speed);
                }
                ShowTooltip(CV(mcmInfoPlaybackSpeed));

                ImGuiMCP::EndTabItem();
            }

            /* Inputs */
            if (ImGuiMCP::BeginTabItem(CV(inputHeader)))
            {
                if (ImGuiMCP::Checkbox(CV(usePresetKey), &ModSettings::Use_Preset_Parkour_Key))
                {
                    IS::SetUsePresetKey(ModSettings::Use_Preset_Parkour_Key);
                }
                ShowTooltip(CV(mcmInfoUsePresetKey));

                if (ModSettings::Use_Preset_Parkour_Key)
                {
                    if (ImGuiMCP::Combo(CV(presetKeyList), &ModSettings::Preset_Parkour_Key, presetKeyLabels,
                                        SkyParkour::PresetKeys::kTotalKeys))
                    {
                        IS::SetPresetParkourKey(ModSettings::Preset_Parkour_Key);
                    }
                }
                else
                {
                    if (DrawKeyBindRow(CV(customKey), &ModSettings::Custom_Parkour_Key))
                    {
                        IS::SetCustomParkourKey(static_cast<int32_t>(ModSettings::Custom_Parkour_Key));
                    }
                    ShowTooltip(CV(mcmInfoCustomKey));
                }

                if (ImGuiMCP::SliderFloat(CV(inputDelay), &ModSettings::Parkour_Delay, 0.0f, 2.0f, "%.2fs"))
                {
                    IS::SetParkourDelay(ModSettings::Parkour_Delay);
                }
                ShowTooltip(CV(mcmInfoDelay));

                ImGuiMCP::EndTabItem();
            }

            /* Stamina System */
            if (ImGuiMCP::BeginTabItem(CV(staminaHeader)))
            {
                if (ImGuiMCP::Checkbox(CV(enableStaminaSystem), &ModSettings::Enable_Stamina_Consumption))
                {
                    IS::SetEnableStaminaSystem(ModSettings::Enable_Stamina_Consumption);
                }
                ShowTooltip(CV(mcmInfoStaminaSystem));

                ImGuiMCP::BeginDisabled(!ModSettings::Enable_Stamina_Consumption);
                if (ImGuiMCP::Checkbox(CV(mustHaveStamina), &ModSettings::Must_Have_Stamina))
                {
                    IS::SetMustHaveStamina(ModSettings::Must_Have_Stamina);
                }
                ShowTooltip(CV(mcmInfoMustHaveStamina));

                if (ImGuiMCP::SliderFloat(CV(baseStaminaCost), &ModSettings::Stamina_Damage, 0.0f, 100.0f, "%.1f"))
                {
                    IS::SetBaseStaminaDamage(ModSettings::Stamina_Damage);
                }
                ShowTooltip(CV(mcmInfoStaminaCost));
                ImGuiMCP::EndDisabled();

                ImGuiMCP::EndTabItem();
            }

            /* Smart Parkour */
            if (ImGuiMCP::BeginTabItem(CV(smartParkourHeader)))
            {
                if (ImGuiMCP::Combo(CV(autoParkourList), &ModSettings::Auto_Parkour, autoParkourLabels,
                                    SkyParkour::AutoParkourOptions::kTotalOpts))
                {
                    IS::SetAutoParkour(ModSettings::Auto_Parkour);
                }
                ShowTooltip(CV(mcmInfoAutoParkour));
                ImGuiMCP::Separator();

                if (ImGuiMCP::Checkbox(CV(smartSteps), &ModSettings::Smart_Steps))
                {
                    IS::SetSmartSteps(ModSettings::Smart_Steps);
                }
                ShowTooltip(CV(mcmInfoSmartSteps));

                if (ImGuiMCP::Checkbox(CV(smartVault), &ModSettings::Smart_Vault))
                {
                    IS::SetSmartVault(ModSettings::Smart_Vault);
                }
                ShowTooltip(CV(mcmInfoSmartVault));

                if (ImGuiMCP::Checkbox(CV(smartClimb), &ModSettings::Smart_Climb))
                {
                    IS::SetSmartClimb(ModSettings::Smart_Climb);
                }
                ShowTooltip(CV(mcmInfoSmartClimb));

                ImGuiMCP::EndTabItem();
            }

            /* Sliding */
            if (ImGuiMCP::BeginTabItem(CV(slideHeader)))
            {
                if (ImGuiMCP::Checkbox(CV(crouchSlideOnOff), &ModSettings::Crouch_Slide_Enabled))
                {
                    IS::SetEnableCrouchSlide(ModSettings::Crouch_Slide_Enabled);
                }
                ShowTooltip(CV(mcmInfoCrouchSlide));

                ImGuiMCP::BeginDisabled(!ModSettings::Crouch_Slide_Enabled);
                if (ImGuiMCP::Checkbox(CV(advancedSlide), &ModSettings::Advanced_Slide_Sneak))
                {
                    IS::SetEnableAdvancedSlide(ModSettings::Advanced_Slide_Sneak);
                }
                ShowTooltip(CV(mcmInfoAdvancedSlide));

                if (ImGuiMCP::Checkbox(CV(slideTackle), &ModSettings::ExpSlideTackle))
                {
                    IS::SetEnableSlideTackle(ModSettings::ExpSlideTackle);
                }
                ShowTooltip(CV(mcmInfoSlideTackle));
                ImGuiMCP::EndDisabled();

                ImGuiMCP::EndTabItem();
            }

            /* Rolling */
            if (ImGuiMCP::BeginTabItem(CV(rollHeader)))
            {
                if (ImGuiMCP::Checkbox(CV(rollOnOff), &ModSettings::Land_Rolling_Enabled))
                {
                    IS::SetEnableRolling(ModSettings::Land_Rolling_Enabled);
                }
                ShowTooltip(CV(mcmInfoRoll));

                ImGuiMCP::EndTabItem();
            }

            /* Debug */
            if (ImGuiMCP::BeginTabItem(CV(debugHeader)))
            {
                if (ImGuiMCP::Checkbox(CV(debugOnOff), &ModSettings::_Debug_Enabled))
                {
                    IS::SetEnableDebug(ModSettings::_Debug_Enabled);
                }

                ImGuiMCP::EndTabItem();
            }

            ImGuiMCP::EndTabBar();
        }
    }

    inline void Register()
    {
        if (!SKSEMenuFramework::IsInstalled()) return;
        if (MenuRegistered) return;

        cached::Initialize();

        SKSEMenuFramework::SetSection("SkyParkour");
        SKSEMenuFramework::AddSectionItem(CV(mainSettingsHeader), Render);
        _InputListener = new SKSEMenuFramework::Model::InputEvent(OnInput);
        MenuRegistered = true;
    }
#undef CV
}