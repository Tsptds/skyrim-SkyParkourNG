#pragma once

namespace Localized_ini {
    inline std::string INIPath{"./Data/SKSE/Plugins/SkyParkourNG_Localization.ini"};

    extern std::unique_ptr<CSimpleIniA> GetIniHandle();
}