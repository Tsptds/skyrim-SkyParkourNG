#include "Util/ScaleUtility.h"
// From BingusEx's SkyClimb Fork, optimized

namespace ScaleUtility
{
    [[nodiscard]] RE::NiAVObject *FindBoneNode(const RE::Actor *a_actorptr, const RE::BSFixedString &a_nodeName, const bool a_isFirstPerson)
    {
        if (!a_actorptr->Is3DLoaded()) return nullptr;

        const auto model = a_actorptr->Get3D(a_isFirstPerson);
        if (!model) return nullptr;

        return model->GetObjectByName(a_nodeName);
    }

    [[nodiscard]] float GetModelScale(const RE::Actor *a_actor)
    {
        if (!a_actor) return 1.f;

        if (!a_actor->Is3DLoaded()) {
            return 1.f;
        }

        if (const auto model = a_actor->Get3D(false)) {
            return model->local.scale;
        }

        if (const auto first_model = a_actor->Get3D(true)) {
            return first_model->local.scale;
        }

        return 1.f;
    }

    [[nodiscard]] float GetNodeScale(const RE::Actor *a_actor, const RE::BSFixedString &a_boneName)
    {
        if (!a_actor) return 1.f;

        if (const auto Node = FindBoneNode(a_actor, a_boneName, false)) {
            return Node->local.scale;
        }
        if (const auto FPNode = FindBoneNode(a_actor, a_boneName, true)) {
            return FPNode->local.scale;
        }
        return 1.f;
    }

    [[nodiscard]] float GetScale(RE::Actor *actor)
    {
        float TargetScale{1.f};

        TargetScale *= GetModelScale(actor);  // Model scale, Scaling done by game

        const auto npcNode = GetNodeScale(actor, RE::BSFixedString("NPC"));  // NPC bone, Racemenu uses this.
        if (npcNode) TargetScale *= npcNode;

        const auto rootNode = GetNodeScale(actor, RE::BSFixedString("NPC Root [Root]"));  // Some other mods scale this bone instead
        if (rootNode) TargetScale *= rootNode;

        if (TargetScale < 0.15f) TargetScale = 0.15f;
        if (TargetScale > 250.f) TargetScale = 250.f;
        return TargetScale;
    }
}  // namespace ScaleUtility