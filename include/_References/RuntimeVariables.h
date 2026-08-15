#pragma once

enum class ParkourType : int32_t;

namespace RuntimeVariables
{
    inline bool IsParkourActive{true};
    inline float PlayerScale{1.f};

    inline ParkourType selectedLedgeType{-1};
    inline RE::NiPoint3 ledgePoint{0, 0, 0};
    inline RE::NiPoint3 startPos{0, 0, 0};

    inline bool ParkourInProgress{false};
    inline bool RecoveryFramesActive{false};
    inline bool IsMenuOpen{false};
    inline bool IsInMainMenu{true};

    inline bool SlideOngoing{false};

    inline bool _DidWarnMissingFPP{false};
    inline bool _DidWarnMissingTPP{false};

}  // namespace RuntimeVariables
