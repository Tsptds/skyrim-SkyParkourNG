#pragma once

namespace ParkourUtility {
    bool IsParkourActiveFor(RE::Actor *actor);
    bool ClimbExtraChecks(RE::NiPoint3 start, const float check_height);
    bool SmartClimbCheck(RE::Actor *);
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
    bool ActorHasEnoughStamina(RE::Actor *);
    bool DamageActorStamina(RE::Actor *actor, float amount);
    bool ShouldClimbActionFail(RE::Actor *);
    bool CheckActionRequiresLowEffort(int32_t selectedLedgeType);
    inline bool IsSupportGroundedOrSliding(RE::Actor *actor)    {return actor->GetCharController()->surfaceInfo.supportedState != RE::hkpSurfaceInfo::SupportedState::kSupported;}
    inline bool IsSupportUnsupported(RE::Actor *actor)          {return actor->GetCharController()->surfaceInfo.supportedState == RE::hkpSurfaceInfo::SupportedState::kUnsupported;}
    inline bool IsSupportSliding(RE::Actor *actor)              {return actor->GetCharController()->surfaceInfo.supportedState == RE::hkpSurfaceInfo::SupportedState::kSliding;}
    inline bool IsSupportGrounded(RE::Actor *actor)             {return actor->GetCharController()->surfaceInfo.supportedState == RE::hkpSurfaceInfo::SupportedState::kSupported;}
    bool PlayerIsSwimming();
    bool IsActorWeaponOut(RE::Actor *actor);
    bool IsInDrawSheath(RE::Actor *);
    bool IsAttacking(RE::Actor *actor);
    bool IsCrouchSliding(RE::Actor *actor);
    bool TooSlowStuckToObject(RE::Actor *actor, float maxValAbs);
}  // namespace ParkourUtility