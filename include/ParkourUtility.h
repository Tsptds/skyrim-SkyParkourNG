#pragma once
#include "References.h"
#include "ScaleUtility.h"

namespace ParkourUtility {
    void StopInteractions(RE::Actor &actor);
    bool ToggleControlsForParkour(bool enable);
    RE::NiPoint3 GetPlayerDirFlat(RE::Actor *player);
    RayCastResult RayCast(RE::NiPoint3 rayStart, RE::NiPoint3 rayDir, float maxDist, RE::COL_LAYER layerMask);
    bool IsPlayerAlreadyAnimationDriven(RE::PlayerCharacter *player);
    bool IsPlayerUsingFurniture(RE::PlayerCharacter *);
    bool IsPlayerInCharGen(RE::PlayerCharacter *);
    bool IsBeastForm();
    bool IsOnMount();
    bool IsPlayerInSyncedAnimation(RE::PlayerCharacter *);
    float CalculateParkourStamina();
    bool PlayerHasEnoughStamina();
    bool DamageActorStamina(RE::Actor *actor, float amount);
    bool ShouldReplaceMarkerWithFailed();
    bool CheckIsVaultActionFromType(int32_t selectedLedgeType);
    bool PlayerIsGroundedOrSliding();
    bool PlayerIsMidairAndNotSliding();
    bool PlayerIsSwimming();
    bool PlayerWantsToDrawSheath();
    bool IsParkourActive();
    bool PlayerIsOnStairs();
    float magnitudeXY(float x, float y);
}  // namespace ParkourUtility