#pragma once

namespace ParkourUtility {
    bool IsParkourActive();
    bool StepsExtraChecks(RE::Actor *player, const RayCastResult ray);
    bool IsStepNormalValid(const RayCastResult ray, bool isMoving);
    bool VaultExtraChecks(RE::Actor *actor);
    bool GrabExtraChecks(const float ledgePlayerDiff, const RayCastResult ray);
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
    bool IsSupportGroundedOrSliding(RE::Actor *);
    bool IsSupportUnsupported(RE::Actor *);
    bool IsSupportSliding(RE::Actor *);
    bool IsSupportGrounded(RE::Actor *);
    bool PlayerIsSwimming();
    bool IsInDrawSheath(RE::Actor *);
}  // namespace ParkourUtility