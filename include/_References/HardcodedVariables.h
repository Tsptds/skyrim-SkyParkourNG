#pragma once

namespace HardCodedVariables {
    // 250
    extern const float climbMaxHeight;
    // 30
    extern const float climbMinHeight;

    // 120
    extern const float vaultMaxHeight;
    // 40.5
    extern const float vaultMinHeight;

    // 110
    extern const float grabPlayerBelowLedgeMaxDiff;
    // -35
    extern const float grabPlayerAboveLedgeMaxDiff;

    // 220
    extern const float highestLedgeLimit;
    // 170
    extern const float highLedgeLimit;
    // 130
    extern const float medLedgeLimit;
    // 80
    extern const float lowLedgeLimit;
    // 60
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
