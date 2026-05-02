#pragma once

namespace Compatibility {
    inline bool BackportedESLSupport{false};
    struct TrueDirectionalMovement {
            inline static bool found{false};
            inline static constexpr const char *dll_name{"TrueDirectionalMovement.dll"};
    };
    struct JumpingAttack {
            inline static bool found{false};
            inline static constexpr const char *esp_name{"JumpAttack.esp"};
            inline static constexpr const char *event{"JumpAtkEquip"};
    };
    struct ClassicSprintingRedone {
            inline static bool found{false};
            inline static constexpr const char *dll_name{"ClassicSprintingRedone.dll"};
    };
}  // namespace Compatibility