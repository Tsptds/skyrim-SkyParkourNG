#pragma once

namespace ScaleUtility {
    [[nodiscard]] RE::NiAVObject* FindBoneNode(const RE::Actor* a_actorptr, const std::string_view a_nodeName,
                                               bool a_isFirstPerson);

    [[nodiscard]] float GetModelScale(const RE::Actor* a_actor);

    [[nodiscard]] float GetNodeScale(const RE::Actor* a_actor, const std::string_view a_boneName);

    [[nodiscard]] float GetScale();
}