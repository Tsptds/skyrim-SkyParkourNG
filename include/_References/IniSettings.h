#pragma once

namespace IniSettings {
    extern std::string INIPath;
    extern std::string ESP_NAME;
    extern std::string Blue_Indicator_RefID;
    extern std::string Red_Indicator_RefID;

    extern void CreateDefault(std::unique_ptr<CSimpleIniA> &ini);
}  // namespace IniSettings