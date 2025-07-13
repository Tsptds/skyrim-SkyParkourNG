#pragma once
#include "PCH.h"
#include "References.h"
#include "Parkouring.h"

namespace SkyParkour_Papyrus {

    extern void AddFuncsToVm(RE::BSScript::IVirtualMachine *vm);

    extern void RegisterCustomParkourKey(RE::StaticFunctionTag *, int32_t dxcode);
    extern void RegisterPresetParkourKey(RE::StaticFunctionTag *, int32_t presetKey);
    extern void RegisterParkourDelay(RE::StaticFunctionTag *, float delay);
    extern void RegisterStaminaDamage(RE::StaticFunctionTag *, bool enabled, bool staminaBlocks, float damage);
    extern void RegisterParkourSettings(RE::StaticFunctionTag *, bool _usePresetKey, bool _enableMod, bool _smartParkour,
                                        bool _useIndicators);

};  // namespace SkyParkour_PapyrusInterface