#include "References.h"

namespace ModSettings {

    bool UsePresetParkourKey = true;
    int PresetParkourKey = 0;  // enum ParkourKeyOptions

    bool ModEnabled = true;
    //bool ShouldModSuspend = false;

    float parkourDelay = 0.0f;  // Set initial delay

    bool Enable_Stamina_Consumption = true;
    bool Is_Stamina_Required = true;
    float Stamina_Damage = 20.0f;

    bool Smart_Parkour_Enabled = true;  // Don't use high/failed ledge when moving, don't use vault when standing still
}  // namespace ModSettings

namespace HardCodedVariables {
    // Lower - upper limits for ledge - vault detection.
    const float climbMaxHeight = 250.0f;
    const float climbMinHeight = 20.0f;

    const float vaultMaxHeight = 90.0f;
    const float vaultMinHeight = 40.5f;

    // These are the height ranges for parkour type selection, represent low limits.
    const float highestLedgeLimit = 220.0f;
    const float highLedgeLimit = 160.0f;
    const float medLedgeLimit = 135.0f;
    const float lowLedgeLimit = 90.0f;
    const float highStepLimit = 70.0f;

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

// Alternate step animations
void RuntimeMethods::SwapLegs() {
    RuntimeVariables::shouldUseRightStep = !RuntimeVariables::shouldUseRightStep;
    RE::PlayerCharacter::GetSingleton()->SetGraphVariableBool("SkyParkourStepLeg", RuntimeVariables::shouldUseRightStep);

    //logger::info("Right Step Next: {}", RuntimeVariables::shouldUseRightStep);
}
// Things that are not handled by MCM and persistent throughout saves without being reset on game load
void RuntimeMethods::ResetRuntimeVariables() {
    RuntimeVariables::IsBeastForm = false;
    RuntimeVariables::ParkourEndQueued = false;
    RuntimeVariables::selectedLedgeType = ParkourType::NoLedge;
}

namespace RuntimeVariables {
    RE::COL_LAYER lastHitObject;

    float PlayerScale = 1.0f;

    int selectedLedgeType = -1;
    RE::NiPoint3 ledgePoint = {0, 0, 0};
    RE::NiPoint3 playerDirFlat = {0, 0, 0};
    RE::NiPoint3 backwardAdjustment = {0, 0, 0};

    bool wasFirstPerson = false;

    bool ParkourEndQueued = false;
    bool IsMenuOpen = false;
    bool IsInMainMenu = true;
    bool IsBeastForm = false;

    bool shouldUseRightStep = true;
}  // namespace RuntimeVariables

namespace GameReferences {
    RE::TESObjectREFR *indicatorRef_Blue;
    RE::TESObjectREFR *indicatorRef_Red;

    RE::TESObjectREFR *currentIndicatorRef;
}  // namespace GameReferences