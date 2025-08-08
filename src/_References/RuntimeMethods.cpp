#include "_References/RuntimeMethods.h"
#include "_References/RuntimeVariables.h"
#include "_References/ParkourType.h"
#include "_References/ModSettings.h"
#include "_References/IniSettings.h"
#include "_References/Compatibility.h"

namespace RuntimeMethods {

    void SwapLegs() {
        RuntimeVariables::shouldUseRightStep = !RuntimeVariables::shouldUseRightStep;
        GET_PLAYER->SetGraphVariableBool(SPPF_Leg, RuntimeVariables::shouldUseRightStep);
#ifdef LOG_STEPS
        logger::info("Next Step: {}", RuntimeVariables::shouldUseRightStep ? "Right" : "Left");
#endif  // LOG_STEPS
    }

    // Things that are not handled by MCM and persistent throughout saves without being reset on game load
    void ResetRuntimeVariables() {
        RuntimeVariables::ParkourInProgress = false;
        RuntimeVariables::EnableNotifyWindow = false;
        RuntimeVariables::RecoveryFramesActive = false;
        RuntimeVariables::selectedLedgeType = ParkourType::NoLedge;
        auto player = GET_PLAYER;
        if (player) {
            player->SetGraphVariableInt(SPPF_Ledge, -1);
            player->SetGraphVariableFloat(SPPF_SPEEDMULT, ModSettings::Playback_Speed);
        }
    }
    bool CheckESPLoaded() {
        auto dh = RE::TESDataHandler::GetSingleton();
        return dh && (dh->GetSingleton()->LookupLoadedLightModByName(IniSettings::ESP_NAME) ||
                      dh->GetSingleton()->LookupLoadedModByName(IniSettings::ESP_NAME));
    }
    std::unique_ptr<CSimpleIniA> GetIniHandle() {
        constexpr const char *path = "./Data/SKSE/Plugins/SkyParkourNG.ini";

        auto ini = std::make_unique<CSimpleIniA>();
        ini->SetUnicode();

        SI_Error rc = ini->LoadFile(path);
        if (rc < 0) {
            logger::warn("SkyParkourNG.ini not found, creating with default values");

            // Set default values here
            ini->SetValue("ESP", "sEspName", "SkyParkourV2.esp");
            ini->SetValue("ESP", "iBlueMarkerRefID", "0x000014");
            ini->SetValue("ESP", "iRedMarkerRefID", "0x00000C");

            ini->SetValue("MCM", "bEnableMod", "true");
            ini->SetValue("MCM", "bSmartParkour", "true");
            ini->SetValue("MCM", "bShowIndicators", "true");

            ini->SetValue("MCM", "bEnableStaminaSystem", "true");
            ini->SetValue("MCM", "bMustHaveStamina", "true");
            ini->SetValue("MCM", "iBaseStaminaDamage", "20");

            ini->SetValue("MCM", "bUsePresetKey", "true");
            ini->SetValue("MCM", "iPresetKeyIndex", "0");
            ini->SetValue("MCM", "iCustomKeybind", "0");
            ini->SetValue("MCM", "fInputDelay", "0.0");

            if (ini->SaveFile(path) < 0) {
                logger::error("Failed to create default SkyParkourNG.ini");
                return nullptr;
            }
        }

        return ini;
    }

    bool ReadPluginConfigFromINI() {
        auto ini = GetIniHandle();
        if (!ini) {
            logger::error("INI FILE DOES NOT EXIST AND FAILED TO CREATE");
            return false;
        }

        const char *name = ini->GetValue("ESP", "sEspName");
        if (!name) {
            logger::error("EspName not found, using default name");
        }

        logger::info("ESP Name: '{}'", name);
        IniSettings::ESP_NAME = name;
        return true;
    }

    void SetupModCompatibility() {
        auto TDM = GetModuleHandleA("TrueDirectionalMovement.dll");
        if (TDM) {
            Compatibility::TrueDirectionalMovement = true;
            logger::info("Patch: True Directional Movement |360|Swim Pitch|");
        }
    }
}  // namespace RuntimeMethods