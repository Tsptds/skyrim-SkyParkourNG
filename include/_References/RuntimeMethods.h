#pragma once

namespace RuntimeMethods
{
    extern const RE::TESFile *GetPlugin(RE::TESDataHandler *const &dh, std::string_view esp_name);
    extern void SetupDLLCompatibility();
    extern void SetupESPCompatibility();
    extern void ResetAll();
    extern void ResetParkour();
    extern void ResetSlide();
    extern std::unique_ptr<CSimpleIniA> GetIniHandle();
    extern bool RequestTrueHUDAPI();
}  // namespace RuntimeMethods