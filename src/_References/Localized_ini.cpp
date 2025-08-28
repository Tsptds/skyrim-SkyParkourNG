#include "_References/Localized_ini.h"

namespace Localized_ini {
    std::string INIPath = "./Data/SKSE/Plugins/SkyParkourNG_Localization.ini";

    std::unique_ptr<CSimpleIniA> GetIniHandle() {
        const auto &path = INIPath.c_str();

        auto ini = std::make_unique<CSimpleIniA>();
        ini->SetUnicode();

        SI_Error rc = ini->LoadFile(path);
        if (rc < 0) {
            WARN("Localization file not found");

            return nullptr;
        }

        return ini;
    }
}