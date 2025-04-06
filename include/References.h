#pragma once

namespace ModSettings {
    extern bool UsePresetParkourKey;
    extern int PresetParkourKey;
    const enum ParkourKeyOptions { kJump = 0, kSprint, kActivate, k_Custom };  // k_Custom is unused for now
    extern bool ModEnabled;
    //extern bool ShouldModSuspend;
    extern float parkourDelay;
    extern bool Enable_Stamina_Consumption;
    extern bool Is_Stamina_Required;
    extern float Stamina_Damage;
    extern bool Smart_Parkour_Enabled;
}  // namespace ModSettings

namespace HardCodedVariables {
    extern const float climbMaxHeight;
    extern const float climbMinHeight;

    extern const float vaultMaxHeight;
    extern const float vaultMinHeight;

    // 220
    extern const float highestLedgeLimit;
    // 175
    extern const float highLedgeLimit;
    // 110
    extern const float medLedgeLimit;
    // 70
    extern const float highStepLimit;

    // 250
    extern const float highestLedgeElevation;
    // 200
    extern const float highLedgeElevation;
    // 155
    extern const float medLedgeElevation;
    // 70
    extern const float stepHighElevation;
    // 50
    extern const float stepLowElevation;
    // 40
    extern const float vaultElevation;
    // 60
    extern const float grabElevation;
}  // namespace HardCodedVariables

namespace ParkourType {
    // 7
    extern const int Highest;
    // 6
    extern const int High;
    // 5
    extern const int Medium;
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

namespace RuntimeMethods {
    extern void SwapLegs();
}

namespace RuntimeVariables {
    extern RE::COL_LAYER lastHitObject;
    extern float PlayerScale;
    extern int selectedLedgeType;
    extern RE::NiPoint3 ledgePoint;
    extern RE::NiPoint3 playerDirFlat;
    extern RE::NiPoint3 backwardAdjustment;

    extern bool wasFirstPerson;

    extern bool ParkourEndQueued;
    extern bool IsMenuOpen;
    extern bool IsBeastForm;

    extern bool shouldUseRightStep;
}  // namespace RuntimeVariables

namespace GameReferences {

    extern RE::TESObjectREFR* indicatorRef_Blue;
    extern RE::TESObjectREFR* indicatorRef_Red;
    extern RE::TESObjectREFR* currentIndicatorRef;
}  // namespace GameReferences
