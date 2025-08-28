#pragma once

namespace Localized_ini {
    extern std::string INIPath;

    extern std::unique_ptr<CSimpleIniA> GetIniHandle();
}