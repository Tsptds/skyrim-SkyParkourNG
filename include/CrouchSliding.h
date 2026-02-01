#pragma once

namespace CrouchSliding {
    bool TrySprintSlide();
    bool IsSlideActiveFor(RE::Actor*);
    void SetSlideOnOff(bool turnOn);
}  // namespace CrouchSliding