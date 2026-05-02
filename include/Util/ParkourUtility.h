#pragma once

enum class ParkourType : int32_t;

namespace ParkourUtility {
    bool IsParkourActiveFor(RE::Actor *actor);
    bool ClimbExtraChecks(RE::NiPoint3 start, const float check_height, RE::NiPoint3 fwdDir);
    bool SmartClimbCheck(RE::Actor *);
    bool StepsExtraChecks(RE::Actor *actor, const float ledgePlayerDiff, const RE::NiPoint3 ledgePoint);
    bool VaultExtraChecks(RE::Actor *actor);
    bool GrabExtraChecks(RE::Actor *actor, const float ledgePlayerDiff, bool &out_grabHighVariant, const RE::NiPoint3 ledgePoint);
    void StopInteractions(RE::Actor &actor);
    RE::NiPoint3 GetActorDirFlat(RE::Actor *actor);

    bool IsKnockedOut(RE::Actor *); // Ragdoll & Get Up Sequence
    bool IsPlayerAlreadyAnimationDriven(RE::Actor *);

    bool IsSitting(RE::Actor *); // Also includes mounts
    bool IsCrosshairRefActivator();
    bool IsChargenHandsBound(RE::PlayerCharacter *);
    bool IsBeastForm();
    bool IsOnMount();
    bool IsGamePaused();
    bool IsInSyncedAnimation(RE::Actor *);
    float CalculateStaminaReqFromEquipLoad(RE::Actor *);
    bool ActorHasEnoughStamina(RE::Actor *);
    bool DamageActorStamina(RE::Actor *actor, float amount);
    bool ShouldClimbActionFail(RE::Actor *);
    bool CheckActionRequiresLowEffort(ParkourType selectedLedgeType);
    inline bool IsSupportGroundedOrSliding(RE::Actor *actor)    {return actor->GetCharController()->surfaceInfo.supportedState != RE::hkpSurfaceInfo::SupportedState::kUnsupported;}
    inline bool IsSupportUnsupported(RE::Actor *actor)          {return actor->GetCharController()->surfaceInfo.supportedState == RE::hkpSurfaceInfo::SupportedState::kUnsupported;}
    inline bool IsSupportSliding(RE::Actor *actor)              {return actor->GetCharController()->surfaceInfo.supportedState == RE::hkpSurfaceInfo::SupportedState::kSliding;}
    inline bool IsSupportGrounded(RE::Actor *actor)             {return actor->GetCharController()->surfaceInfo.supportedState == RE::hkpSurfaceInfo::SupportedState::kSupported;}
    bool PlayerIsSwimming();
    bool IsActorWeaponOut(RE::Actor *actor);
    bool IsInDrawSheath(RE::Actor *);
    bool IsAttacking(RE::Actor *actor);
    bool IsCrouchSliding(RE::Actor *actor);
    float GetCharForwardVelocity(RE::Actor *act);
    float GetRelativeVelocityToMT(RE::Actor *actor); // 0 - 1
}  // namespace ParkourUtility