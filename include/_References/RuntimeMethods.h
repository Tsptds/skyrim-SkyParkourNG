#pragma once

namespace RuntimeMethods {
    extern void SetupModCompatibility();
    extern void SwapLegs();
    extern void ResetRuntimeVariables();
    extern bool CheckESPLoaded();
    extern std::unique_ptr<CSimpleIniA> GetIniHandle();
    extern bool ReadPluginConfigFromINI();
}  // namespace RuntimeMethods