#pragma once

namespace IniSettings {
    extern std::string INIPath;
    extern std::string ESP_NAME;

    extern void CreateDefault(std::unique_ptr<CSimpleIniA> &ini);
    extern std::unique_ptr<CSimpleIniA> GetIniHandle();
}  // namespace IniSettings