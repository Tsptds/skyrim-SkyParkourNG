#pragma once

namespace RuntimeVariables {
    extern bool IsParkourActive;
    extern float PlayerScale;
    extern int selectedLedgeType;
    extern RE::NiPoint3 ledgePoint;
    extern RE::NiPoint3 playerDirFlat;

    extern RE::NiPoint3 PlayerStartPosition;

    extern bool ParkourInProgress;
    extern bool EnableNotifyWindow;
    extern bool RecoveryFramesActive;
    extern bool IsMenuOpen;
    extern bool IsInMainMenu;

    extern bool shouldUseRightStep;

}  // namespace RuntimeVariables
