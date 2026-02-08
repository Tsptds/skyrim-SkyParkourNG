#pragma once

namespace CrouchSliding {
    bool TrySprintSlide();
    bool IsSlideActiveFor(RE::Actor*, bool&);
    void SetSlideOnOff(bool turnOn);
}  // namespace CrouchSliding