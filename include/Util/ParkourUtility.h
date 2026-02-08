#pragma once

namespace ParkourUtility {
    bool IsParkourActiveFor(RE::Actor *actor);
    bool ClimbExtraChecks(RE::NiPoint3 start, const float check_height);
    bool SmartClimbCheck(RE::Actor*);
    bool StepsExtraChecks(RE::Actor *player, const RayCastResult ray);
    bool IsStepNormalValid(const RayCastResult ray, bool isMoving);
    bool VaultExtraChecks(RE::Actor *actor);
    bool GrabExtraChecks(const float ledgePlayerDiff, const RayCastResult ray, bool &out_grabHighVariant);
    void StopInteractions(RE::Actor &actor);
    RE::NiPoint3 GetActorDirFlat(RE::Actor *actor);
    RayCastResult RayCast(RE::NiPoint3 rayStart, RE::NiPoint3 rayDir, float maxDist, COL_LAYER_EXTEND layerMask,
                          RE::Actor *actor = GET_PLAYER);
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
    bool ActorHasEnoughStamina(RE::Actor*);
    bool DamageActorStamina(RE::Actor *actor, float amount);
    bool ShouldClimbActionFail(RE::Actor*);
    bool CheckActionRequiresLowEffort(int32_t selectedLedgeType);
    bool IsSupportGroundedOrSliding(RE::Actor *);
    bool IsSupportUnsupported(RE::Actor *);
    bool IsSupportSliding(RE::Actor *);
    bool IsSupportGrounded(RE::Actor *);
    bool PlayerIsSwimming();
    bool IsActorWeaponOut(RE::Actor *actor);
    bool IsInDrawSheath(RE::Actor *);
    bool IsAttacking(RE::Actor *actor);
    bool IsCrouchSliding(RE::Actor *actor);
}  // namespace ParkourUtility