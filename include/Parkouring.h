#pragma once

namespace Parkouring {
    inline static float *g_gameTimeMult = (float *) RELOCATION_ID(508682, 380437).address();  // SGTM static pointer, dereference and use

    int GetLedgePoint();
    int ClimbCheck(RE::NiPoint3 &ledgePoint, RE::NiPoint3 checkDir, float minLedgeHeight, float maxLedgeHeight);
    int VaultCheck(RE::NiPoint3 &ledgePoint, RE::NiPoint3 checkDir, float vaultLength, float maxElevationIncrease, float minVaultHeight,
                   float maxVaultHeight);
    int ChooseClimbHeight(RE::Actor *player, const float playerHeight, RE::NiPoint3 &ledgePoint, const RE::NiPoint3 &playerPos,
                          RayCastResult &ledgeRay);
    // Stop is true, start is false
    void OnStartStop(bool isStop);
    void InterpolateRefToPosition(const RE::Actor *movingRef, RE::NiPoint3 to, float seconds);
    void StopInterpolatingRef(const RE::Actor *actor);
    void CalculateStartingPosition(const RE::Actor *actor, int ledgeType, RE::NiPoint3 &out);
    void InvalidateVars();
    bool TryActivateParkour();
    void UpdateIndicatorMenu();
    void UpdateParkourPoint();
    void ParkourReadyRun(int32_t ledgeType);
    void PostParkourStaminaDamage(RE::Actor *player, bool isLowEffort, bool isSwimming);

    void SetParkourOnOff(bool turnOn);
}  // namespace Parkouring