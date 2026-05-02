#pragma once

enum class ParkourType : int32_t;

namespace Parkouring {

    ParkourType GetLedgePoint(RayCastResult &out_LedgeRay);
    ParkourType ClimbCheck(RE::NiPoint3 &ledgePoint, RE::NiPoint3 checkDir, float minLedgeHeight, float maxLedgeHeight,
                           RayCastResult &out_LedgeRay);
    ParkourType VaultCheck(RE::NiPoint3 &ledgePoint, RE::NiPoint3 checkDir, float vaultLength, float maxElevationIncrease,
                           float minVaultHeight, float maxVaultHeight, RayCastResult &out_LedgeRay, bool &out_isForwardBlocked);
    ParkourType ChooseClimbHeight(RE::Actor *player, const float playerHeight, RE::NiPoint3 &ledgePoint, const RE::NiPoint3 &playerPos,
                                  RE::NiPoint3 actorDirFlat);

    void OnStartStop(bool isStop, RE::Actor *actor);  // Stop is true, start is false
    bool CalculateStartingPosition(RE::Actor *actor, ParkourType ledgeType, RE::NiPoint3 &out);
    void InvalidateVars();
    bool TryActivateParkour();
    void UpdateIndicatorMenu();
    void UpdateParkourPoint();
    void ParkourReadyRun(ParkourType ledgeType);
    void PostParkourStaminaDamage(RE::Actor *player, bool isLowEffort, bool isSwimming);
    void SetParkourOnOff(bool turnOn);

    // Unused since 3.5.0
    // void InterpolateRefToPosition(const RE::Actor *movingRef, RE::NiPoint3 to, float seconds);
    // void StopInterpolatingRef(const RE::Actor *actor);
}  // namespace Parkouring