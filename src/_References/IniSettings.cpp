#include "_References/IniSettings.h"

namespace IniSettings
{
    void CreateDefault(std::unique_ptr<CSimpleIniA> &ini)
    {
        // Set default values here
        ini->SetValue("ESP", "sEspName", IniSettings::ESP_NAME.c_str());

        ini->SetValue("MCM", "bEnableMod", "true");
        ini->SetValue("MCM", "bShowIndicators", "true");
        ini->SetValue("MCM", "fPlaybackSpeed", "1.15");
        ini->SetValue("MCM", "bEnableCrouchSlide", "true");

        ini->SetValue("MCM", "bUsePresetKey", "true");
        ini->SetValue("MCM", "iPresetKeyIndex", "0");
        ini->SetValue("MCM", "iCustomKeybind", "0");
        ini->SetValue("MCM", "fInputDelay", "0.0");

        ini->SetValue("MCM", "bEnableStaminaSystem", "true");
        ini->SetValue("MCM", "bMustHaveStamina", "true");
        ini->SetValue("MCM", "iBaseStaminaDamage", "20");

        ini->SetValue("MCM", "iAutoParkour", "1");
        ini->SetValue("MCM", "bSmartSteps", "true");
        ini->SetValue("MCM", "bSmartVault", "true");
        ini->SetValue("MCM", "bSmartClimb", "true");

        ini->SetValue("Experimental", "bSlideTackle", "false");
    }

    std::unique_ptr<CSimpleIniA> GetIniHandle()
    {
        const auto path = IniSettings::INIPath.c_str();

        auto ini = std::make_unique<CSimpleIniA>();
        ini->SetUnicode();

        SI_Error rc = ini->LoadFile(path);
        if (rc < 0) {
            WARN("SkyParkourNG.ini not found, creating with default values");

            IniSettings::CreateDefault(ini);

            if (ini->SaveFile(path) < 0) {
                ERROR("Failed to create default SkyParkourNG.ini");
                return nullptr;
            }
        }

        return ini;
    }
}  // namespace IniSettings