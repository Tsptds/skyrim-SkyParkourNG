#pragma once

namespace Compatibility {
    extern bool TrueDirectionalMovement;
    extern bool ImprovedCamera;

    extern RE::NiNode* FindNode(RE::NiNode* node, const char* name);
    extern void FixHands(RE::Actor*);
}  // namespace Compatibility