#pragma once

namespace RuntimeVariables {
    inline bool IsParkourActive{true};
    inline float PlayerScale{1.f};

    inline int32_t selectedLedgeType{-1};
    inline RE::NiPoint3 ledgePoint{0, 0, 0};
    inline RE::NiPoint3 playerDirFlat{0, 0, 0};

    inline bool ParkourInProgress{false};
    inline bool EnableNotifyWindow{false};
    inline bool RecoveryFramesActive{false};
    inline bool IsMenuOpen{false};
    inline bool IsInMainMenu{true};

    inline bool shouldUseRightStep{true};

    inline bool SlideOngoing{false};

}  // namespace RuntimeVariables
