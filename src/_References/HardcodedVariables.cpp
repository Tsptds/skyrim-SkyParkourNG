#include "_References/HardcodedVariables.h"

namespace HardCodedVariables {
    // Lower - upper limits for ledge - vault - grab detection.
    const float climbMaxHeight = 250.0f;
    const float climbMinHeight = 30.0f;

    const float vaultMaxHeight = 120.0f;
    const float vaultMinHeight = 40.5f;

    const float grabPlayerBelowLedgeMaxDiff = 110.0f;
    const float grabPlayerAboveLedgeMaxDiff = -35.0f;

    // These are the height ranges for parkour type selection, represent low limits.
    const float highestLedgeLimit = 220.0f;
    const float highLedgeLimit = 170.0f;
    const float medLedgeLimit = 130.0f;
    const float lowLedgeLimit = 80.0f;
    const float highStepLimit = 60.0f;

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
