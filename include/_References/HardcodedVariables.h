#pragma once

namespace HardCodedVariables
{
    // Lower - upper limits for ledge - vault - grab detection.
    constexpr float climbMaxHeight{250};
    constexpr float climbMinHeight{30};

    constexpr float vaultMaxHeight{120};
    constexpr float vaultMinHeight{42};

    constexpr float grabMaxHeight{125};
    constexpr float grabHighVariantThreshold{75};

    // These are the height ranges for parkour type selection, represent low limits.
    constexpr float highestLedgeLimit{220};
    constexpr float highLedgeLimit{170};
    constexpr float medLedgeLimit{130};
    constexpr float lowLedgeLimit{80};
    constexpr float highStepLimit{60};

    // These are the ending heights for each animation, they are dependent on animmotion data.
    constexpr float highestLedgeElevation{250};
    constexpr float highLedgeElevation{200};
    constexpr float medLedgeElevation{153};
    constexpr float lowLedgeElevation{110};
    constexpr float stepHighElevation{70};
    constexpr float stepLowElevation{50};
    constexpr float vaultElevation{60};  // This is exception, vault needs to put player further below. Elevation is 20, plus 40 adjustment
    constexpr float grabElevation{55};
    constexpr float grabHighElevation{100};
}  // namespace HardCodedVariables
