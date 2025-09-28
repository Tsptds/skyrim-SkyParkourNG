#pragma once

namespace Parkouring {
    int GetLedgePoint();
    int ClimbCheck(RE::NiPoint3 &ledgePoint, RE::NiPoint3 checkDir, float minLedgeHeight, float maxLedgeHeight);
    int VaultCheck(RE::NiPoint3 &ledgePoint, RE::NiPoint3 checkDir, float vaultLength, float maxElevationIncrease, float minVaultHeight,
                   float maxVaultHeight);
    void OnStartStop(bool isStop);
    bool PlaceAndShowIndicator();
    void InterpolateRefToPosition(const RE::Actor *movingRef, RE::NiPoint3 to, float seconds);
    void StopInterpolatingRef(const RE::Actor *actor);
    void CalculateStartingPosition(const RE::Actor *actor, int ledgeType, RE::NiPoint3 &out);
    void InvalidateVars();
    bool TryActivateParkour();
    void UpdateParkourPoint();
    void ParkourReadyRun(int32_t ledgeType, bool isSwimming);
    void PostParkourStaminaDamage(RE::PlayerCharacter *player, bool isLowEffort, bool isSwimming);

    void SetParkourOnOff(bool turnOn);
}  // namespace Parkouring