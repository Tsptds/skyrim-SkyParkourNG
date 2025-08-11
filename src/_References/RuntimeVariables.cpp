#include "_References/RuntimeVariables.h"

namespace RuntimeVariables {
    bool IsParkourActive = true;

    float PlayerScale = 1.0f;

    int selectedLedgeType = -1;
    RE::NiPoint3 ledgePoint = {0, 0, 0};
    RE::NiPoint3 playerDirFlat = {0, 0, 0};

    RE::NiPoint3 PlayerStartPosition = {0, 0, 0};

    bool ParkourInProgress = false;
    bool EnableNotifyWindow = false;
    bool RecoveryFramesActive = false;
    bool IsMenuOpen = false;
    bool IsInMainMenu = true;

    bool shouldUseRightStep = true;

    bool BlockSheatheNotifyWindow = false;
}  // namespace RuntimeVariables
