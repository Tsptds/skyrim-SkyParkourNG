#pragma once

namespace CrouchSliding {
    bool TrySprintSlide(bool isHoldingKey);
    bool IsSlideActiveFor(RE::Actor *, bool &, bool);
    void SetSlideOnOff(bool turnOn);
    void OnStartStop(bool is_Stop, RE::Actor* actor, bool isRoll);
    bool TryKnockCollidedActor(RE::Actor* act);
}  // namespace CrouchSliding