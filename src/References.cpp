#include "References.h"

namespace ModSettings {

    bool UsePresetParkourKey = true;
    int PresetParkourKey = 0;  // enum ParkourKeyOptions

    bool ModEnabled = true;
    bool UseIndicators = true;

    float parkourDelay = 0.0f;  // Set initial delay

    bool Enable_Stamina_Consumption = true;
    bool Is_Stamina_Required = true;
    float Stamina_Damage = 20.0f;

    bool Smart_Parkour_Enabled = true;  // Don't use high/failed ledge when moving, don't use vault when standing still
}  // namespace ModSettings

/*=========================================================================*/
// Alternate step animations
void RuntimeMethods::SwapLegs() {
    RuntimeVariables::shouldUseRightStep = !RuntimeVariables::shouldUseRightStep;
    RE::PlayerCharacter::GetSingleton()->SetGraphVariableBool("SkyParkourStepLeg", RuntimeVariables::shouldUseRightStep);

    //logger::info("Right Step Next: {}", RuntimeVariables::shouldUseRightStep);
}

// Things that are not handled by MCM and persistent throughout saves without being reset on game load
void RuntimeMethods::ResetRuntimeVariables() {
    RuntimeVariables::ParkourInProgress = false;
    RuntimeVariables::selectedLedgeType = ParkourType::NoLedge;
}
bool RuntimeMethods::CheckESPLoaded() {
    auto dh = RE::TESDataHandler::GetSingleton();
    return dh && (dh->GetSingleton()->LookupLoadedLightModByName(IniSettings::ESP_NAME) ||
                  dh->GetSingleton()->LookupLoadedModByName(IniSettings::ESP_NAME));
}
void RuntimeMethods::ReadIni() {
    CSimpleIniA ini;
    ini.SetUnicode();

    SI_Error rc = ini.LoadFile("./Data/SKSE/Plugins/SkyParkourNG.ini");
    if (rc < 0) {
        std::string f = std::filesystem::current_path().string();
        logger::error("** SkyParkourNG.ini not found in: |{}| >Using default configs", f);
    }
    else {
        const char *name = ini.GetValue("ESP", "sEspName");
        if (!name) {
            logger::error("SkyParkour: sEspName reading failed, using default config: SkyParkourV2");
        }
        else {
            logger::info("sEspName: {}", name);
            IniSettings::ESP_NAME = name;
        }

        const bool devMode = ini.GetBoolValue("DEV", "bIgnoreRequirements");
        if (devMode) {
            IniSettings::IgnoreRequirements = true;
        }
    }
}
void RuntimeMethods::CheckRequirements() {
    struct Requirements {
            const char *BDI = "BehaviorDataInjector.dll";
            const char *AMR = "AnimationMotionRevolution.dll";
            const char *OAR = "OpenAnimationReplacer.dll";
            const char *OAR_Math = "OpenAnimationReplacer-Math.dll";

            static Requirements *Get() {
                static Requirements req;
                return &req;
            }
    };

    auto BDI = GetModuleHandleA(Requirements::Get()->BDI);
    auto AMR = GetModuleHandleA(Requirements::Get()->AMR);
    auto OAR = GetModuleHandleA(Requirements::Get()->OAR);
    auto OAR_Math = GetModuleHandleA(Requirements::Get()->OAR_Math);

    if (!BDI || !AMR || !OAR || !OAR_Math) {
        std::string msg = "\nSkyParkourV2: Loading aborted, required mods not found:\n\n";

        if (!BDI)
            msg += Requirements::Get()->BDI + std::string("\n");
        if (!AMR)
            msg += Requirements::Get()->AMR + std::string("\n");
        if (!OAR)
            msg += Requirements::Get()->OAR + std::string("\n");
        if (!OAR_Math)
            msg += Requirements::Get()->OAR_Math + std::string("\n");

        SKSE::stl::report_and_fail(msg);
    }
}
void RuntimeMethods::SetupModCompatibility() {
    auto TDM = GetModuleHandleA("TrueDirectionalMovement.dll");
    if (TDM) {
        Compatibility::TrueDirectionalMovement = true;
        logger::info("** TDM Found, 360 Parkour Enabled");
    }
}
/*=========================================================================*/

namespace Compatibility {
    bool TrueDirectionalMovement = false;
}  // namespace Compatibility

namespace HardCodedVariables {
    // Lower - upper limits for ledge - vault detection.
    const float climbMaxHeight = 250.0f;
    const float climbMinHeight = 20.0f;

    const float vaultMaxHeight = 115.0f;
    const float vaultMinHeight = 40.5f;

    // These are the height ranges for parkour type selection, represent low limits.
    const float highestLedgeLimit = 220.0f;
    const float highLedgeLimit = 170.0f;
    const float medLedgeLimit = 123.0f;
    const float lowLedgeLimit = 80.0f;
    const float highStepLimit = 40.0f;

    // These are the ending heights for each animation, they are dependent on animmotion data.
    const float highestLedgeElevation = 250.0f;
    const float highLedgeElevation = 200.0f;
    const float medLedgeElevation = 153.0f;
    const float lowLedgeElevation = 110.0f;

    const float stepHighElevation = 70.0f;
    const float stepLowElevation = 50.0f;

    const float vaultElevation = 60.0f;

    const float grabElevation = 60.0f;
}  // namespace HardCodedVariables

namespace ParkourType {
    const int Highest = 8;
    const int High = 7;

    const int Medium = 6;
    const int Low = 5;
    const int StepHigh = 4;
    const int StepLow = 3;

    const int Vault = 2;

    const int Grab = 1;

    const int Failed = 0;

    const int NoLedge = -1;
}  // namespace ParkourType

namespace RuntimeVariables {
    bool IsParkourActive = true;

    RE::COL_LAYER lastHitObject;

    float PlayerScale = 1.0f;

    int selectedLedgeType = -1;
    RE::NiPoint3 ledgePoint = {0, 0, 0};
    RE::NiPoint3 playerDirFlat = {0, 0, 0};
    RE::NiPoint3 backwardAdjustment = {0, 0, 0};

    bool ParkourInProgress = false;
    bool ParkourQueuedForStart = false;
    bool IsMenuOpen = false;
    bool IsInMainMenu = true;

    bool shouldUseRightStep = true;
}  // namespace RuntimeVariables

namespace GameReferences {
    RE::NiPointer<RE::TESObjectREFR> indicatorRef_Blue;
    RE::NiPointer<RE::TESObjectREFR> indicatorRef_Red;

    RE::NiPointer<RE::TESObjectREFR> currentIndicatorRef;

}  // namespace GameReferences

namespace IniSettings {
    std::string ESP_NAME = "SkyParkourV2.esp";
    bool IgnoreRequirements = false;
}  // namespace IniSettings