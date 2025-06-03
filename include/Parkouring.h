#pragma once
#include "References.h"
#include "ParkourUtility.h"
#include "ButtonListener.h"
#include "MenuListener.h"
#include "ScaleUtility.h"

namespace Parkouring {
    int LedgeCheck(RE::NiPoint3 &ledgePoint, RE::NiPoint3 checkDir, float minLedgeHeight, float maxLedgeHeight);
    int VaultCheck(RE::NiPoint3 &ledgePoint, RE::NiPoint3 checkDir, float vaultLength, float maxElevationIncrease, float minVaultHeight,
                   float maxVaultHeight);
    bool PlaceAndShowIndicator();
    int GetLedgePoint(float backwardOffset);
    void InterpolateRefToPosition(const RE::TESObjectREFR *obj, RE::NiPoint3 position, float speed, bool useTimeout = false,
                                  int timeoutMS = 500);
    void AdjustPlayerPosition(int ledgeType);

    bool TryActivateParkour();
    void UpdateParkourPoint();
    void ParkourReadyRun(int ledge);
    void PostParkourStaminaDamage(RE::PlayerCharacter *player, bool isVault);

    void SetParkourOnOff(bool turnOn);
}  // namespace Parkouring