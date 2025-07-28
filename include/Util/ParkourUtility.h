#pragma once

namespace ParkourUtility {
    bool IsParkourActive();
    bool StepsExtraChecks(RE::Actor *player, RE::NiPoint3 ledgePoint, RE::NiPoint3 playerPos, float ledgePlayerDiff,
                          float playerToLedgeHypotenuse);
    void StopInteractions(RE::Actor &actor);
    RE::NiPoint3 GetPlayerDirFlat(RE::Actor *player);
    RayCastResult RayCast(RE::NiPoint3 rayStart, RE::NiPoint3 rayDir, float maxDist, COL_LAYER_EXTEND layerMask);
    // Ragdoll & Get Up Sequence
    bool IsKnockedOut(RE::Actor *);
    bool IsPlayerAlreadyAnimationDriven(RE::Actor *);
    // Also includes mounts
    bool IsSitting(RE::Actor *);
    bool IsCrosshairRefActivator();
    bool IsChargenHandsBound(RE::PlayerCharacter *);
    bool IsBeastForm();
    bool IsOnMount();
    bool IsGamePaused();
    bool IsInSyncedAnimation(RE::Actor *);
    float CalculateParkourStamina(RE::Actor *);
    bool PlayerHasEnoughStamina();
    bool DamageActorStamina(RE::Actor *actor, float amount);
    bool ShouldReplaceMarkerWithFailed();
    bool CheckActionRequiresLowEffort(int32_t selectedLedgeType);
    bool PlayerIsGroundedOrSliding();
    bool PlayerIsMidairAndNotSliding();
    bool PlayerIsSwimming();
    bool IsInDrawSheath(RE::Actor *);
    bool PlayerIsOnStairs();
}  // namespace ParkourUtility