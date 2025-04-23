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

    int GetLedgePoint(float backwardOffset);
    void AdjustPlayerPosition(int ledge);

    bool TryActivateParkour();
    void UpdateParkourPoint();
    void ParkourReadyRun(int ledge);
    void PostParkourStaminaDamage(RE::PlayerCharacter *player, bool isVault);

    void SetParkourOnOff(bool turnOn);
}  // namespace Parkouring