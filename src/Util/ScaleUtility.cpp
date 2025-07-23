#include "Util/ScaleUtility.h"
// From BingusEx's SkyClimb Fork

namespace ScaleUtility {
    [[nodiscard]] RE::NiAVObject* FindBoneNode(const RE::Actor* a_actorptr, const std::string_view a_nodeName, const bool a_isFirstPerson) {
        if (!a_actorptr->Is3DLoaded())
            return nullptr;

        const auto model = a_actorptr->Get3D(a_isFirstPerson);

        if (!model)
            return nullptr;

        if (const auto node_lookup = model->GetObjectByName(a_nodeName))
            return node_lookup;

        // Game lookup failed we try and find it manually
        std::deque<RE::NiAVObject*> queue;
        queue.push_back(model);

        while (!queue.empty()) {
            const auto currentnode = queue.front();
            queue.pop_front();
            try {
                if (currentnode) {
                    if (const auto ninode = currentnode->AsNode()) {
                        for (const auto& child: ninode->GetChildren()) {
                            // Bredth first search
                            queue.push_back(child.get());
                            // Depth first search
                            // queue.push_front(child.get());
                        }
                    }
                    // Do smth
                    if (currentnode->name.c_str() == a_nodeName) {
                        return currentnode;
                    }
                }
            } catch (const std::overflow_error& e) {
                SKSE::log::warn("Find Bone Overflow: {}", e.what());
            }  // this executes if f() throws std::overflow_error (same type rule)
            catch (const std::runtime_error& e) {
                SKSE::log::warn("Find Bone Underflow: {}", e.what());
            }  // this executes if f() throws std::underflow_error (base class rule)
            catch (const std::exception& e) {
                SKSE::log::warn("Find Bone Exception: {}", e.what());
            }  // this executes if f() throws std::logic_error (base class rule)
            catch (...) {
                SKSE::log::warn("Find Bone Exception Other");
            }
        }

        return nullptr;
    }

    [[nodiscard]] float GetModelScale(const RE::Actor* a_actor) {
        if (!a_actor)
            return 1.0f;

        if (!a_actor->Is3DLoaded()) {
            return 1.0;
        }

        if (const auto model = a_actor->Get3D(false)) {
            return model->local.scale;
        }

        if (const auto first_model = a_actor->Get3D(true)) {
            return first_model->local.scale;
        }

        return 1.0;
    }

    [[nodiscard]] float GetNodeScale(const RE::Actor* a_actor, const std::string_view a_boneName) {
        if (!a_actor)
            return 1.0f;

        if (const auto Node = FindBoneNode(a_actor, a_boneName, false)) {
            return Node->local.scale;
        }
        if (const auto FPNode = FindBoneNode(a_actor, a_boneName, true)) {
            return FPNode->local.scale;
        }
        return 1.0;
    }

    [[nodiscard]] float GetScale() {
        const auto player = RE::PlayerCharacter::GetSingleton();
        if (!player)
            return false;

        float TargetScale = 1.0f;
        TargetScale *= GetModelScale(player);                    // Model scale, Scaling done by game
        TargetScale *= GetNodeScale(player, "NPC");              // NPC bone, Racemenu uses this.
        TargetScale *= GetNodeScale(player, "NPC Root [Root]");  // Child bone of "NPC" some other mods scale this bone instead

        if (TargetScale < 0.15f)
            TargetScale = 0.15f;
        if (TargetScale > 250.f)
            TargetScale = 250.f;
        return TargetScale;
    }
}  // namespace ScaleUtility