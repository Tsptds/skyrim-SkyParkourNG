#pragma once
#include "References.h"
#include "ScaleUtility.h"

namespace ParkourUtility {
    void StopInteractions(RE::Actor &actor);
    bool ToggleControls(bool enable);
    RE::NiPoint3 GetPlayerDirFlat(RE::Actor *player);
    RayCastResult RayCast(RE::NiPoint3 rayStart, RE::NiPoint3 rayDir, float maxDist, RE::COL_LAYER layerMask);
    // Ragdoll & Get Up Sequence
    bool IsKnockedOut(RE::Actor *actor);
    bool IsPlayerAlreadyAnimationDriven(RE::PlayerCharacter *player);
    // Also includes mounts
    bool IsSitting(RE::PlayerCharacter *);
    bool ChargenHandsBound(RE::PlayerCharacter *);
    bool IsBeastForm();
    bool IsOnMount();
    bool IsGamePaused();
    bool IsPlayerInSyncedAnimation(RE::PlayerCharacter *);
    float CalculateParkourStamina();
    bool PlayerHasEnoughStamina();
    bool DamageActorStamina(RE::Actor *actor, float amount);
    bool ShouldReplaceMarkerWithFailed();
    bool CheckActionRequiresLowEffort(int32_t selectedLedgeType);
    bool PlayerIsGroundedOrSliding();
    bool PlayerIsMidairAndNotSliding();
    bool PlayerIsSwimming();
    bool PlayerWantsToDrawSheath();
    bool IsParkourActive();
    bool PlayerIsOnStairs();
}  // namespace ParkourUtility