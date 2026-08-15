#pragma once

namespace IniSettings
{
    inline std::string INIPath{"./Data/SKSE/Plugins/SkyParkourNG.ini"};
    inline std::string ESP_NAME{"SkyParkour.esp"};

    extern void CreateDefault(std::unique_ptr<CSimpleIniA> &ini);
    extern std::unique_ptr<CSimpleIniA> GetIniHandle();
}  // namespace IniSettings