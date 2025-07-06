#pragma once

namespace ModSettings {
    extern bool UsePresetParkourKey;
    extern int PresetParkourKey;
    const enum ParkourKeyOptions { kJump = 0, kSprint, kActivate, k_Custom };  // k_Custom is unused for now
    extern bool ModEnabled;
    extern bool UseIndicators;
    extern float parkourDelay;
    extern bool Enable_Stamina_Consumption;
    extern bool Is_Stamina_Required;
    extern float Stamina_Damage;
    extern bool Smart_Parkour_Enabled;
}  // namespace ModSettings

namespace RuntimeMethods {
    extern void SetupModCompatibility();
    extern void SwapLegs();
    extern void ResetRuntimeVariables();
    extern bool CheckESPLoaded();
    extern void ReadIni();
    extern void CheckRequirements();
}  // namespace RuntimeMethods

namespace Compatibility {
    extern bool TrueDirectionalMovement;
    //extern bool ImprovedCamera;
}  // namespace Compatibility

namespace HardCodedVariables {
    extern const float climbMaxHeight;
    extern const float climbMinHeight;

    extern const float vaultMaxHeight;
    extern const float vaultMinHeight;

    // 220
    extern const float highestLedgeLimit;
    // 170
    extern const float highLedgeLimit;
    // 123
    extern const float medLedgeLimit;
    // 80
    extern const float lowLedgeLimit;
    // 40
    extern const float highStepLimit;

    // 250
    extern const float highestLedgeElevation;
    // 200
    extern const float highLedgeElevation;
    // 153
    extern const float medLedgeElevation;
    // 110
    extern const float lowLedgeElevation;
    // 70
    extern const float stepHighElevation;
    // 50
    extern const float stepLowElevation;
    // 60
    extern const float vaultElevation;  // This is exception, vault needs to put player further below. Elevation is 20, plus 40 adjustment
    // 60
    extern const float grabElevation;
}  // namespace HardCodedVariables

namespace ParkourType {
    // 8
    extern const int Highest;
    // 7
    extern const int High;
    // 6
    extern const int Medium;
    // 5
    extern const int Low;
    // 4
    extern int const StepHigh;
    // 3
    extern int const StepLow;
    // 2
    extern int const Vault;
    // 1
    extern int const Grab;
    // 0
    extern int const Failed;
    // -1
    extern const int NoLedge;
}  // namespace ParkourType

namespace RuntimeVariables {
    extern bool IsParkourActive;
    extern RE::COL_LAYER lastHitObject;
    extern float PlayerScale;
    extern int selectedLedgeType;
    extern RE::NiPoint3 ledgePoint;
    extern RE::NiPoint3 playerDirFlat;
    extern RE::NiPoint3 backwardAdjustment;

    extern RE::NiPoint3 PlayerStartPosition;

    extern bool ParkourInProgress;
    extern bool IsMenuOpen;
    extern bool IsInMainMenu;
    extern bool IsInRagdollOrGettingUp;

    extern bool shouldUseRightStep;
}  // namespace RuntimeVariables

namespace GameReferences {

    extern RE::NiPointer<RE::TESObjectREFR> indicatorRef_Blue;
    extern RE::NiPointer<RE::TESObjectREFR> indicatorRef_Red;
    extern RE::NiPointer<RE::TESObjectREFR> currentIndicatorRef;
}  // namespace GameReferences

namespace IniSettings {
    extern std::string ESP_NAME;
    extern bool IgnoreRequirements;
}  // namespace IniSettings