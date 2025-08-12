#include "_References/Compatibility.h"

namespace Compatibility {
    bool TrueDirectionalMovement = false;
    bool ImprovedCamera = false;

    RE::NiNode* FindNode(RE::NiNode* node, const char* name) {
        if (!node)
            return nullptr;

        if (strcmp(node->name.c_str(), name) == 0)
            return node;

        for (auto it = node->children.begin(); it != node->children.end(); ++it) {
            // Need this check as some containers can be null.
            if (it->get() == nullptr)
                continue;

            auto findNode = FindNode(it->get()->AsNode(), name);
            if (findNode)
                return findNode;
        }
        return nullptr;
    }
    void FixHands(RE::Actor* actor) {
        const auto& cam = RE::PlayerCamera::GetSingleton();
        if (cam->IsInFirstPerson()) {
            _THREAD_POOL.enqueue([actor] {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                SKSE::GetTaskInterface()->AddTask([actor] {
                    constexpr bool isFirstPerson = true;
                    const auto fppNode = actor->Get3D(isFirstPerson);

                    const auto rightArmNode = Compatibility::FindNode(fppNode->AsNode(), "NPC R UpperArm [RUar]");
                    const auto leftArmNode = Compatibility::FindNode(fppNode->AsNode(), "NPC L UpperArm [LUar]");

                    rightArmNode->local.scale = 1.0f;
                    leftArmNode->local.scale = 1.0f;
                });
            });
        }
    }
}  // namespace Compatibility