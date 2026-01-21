#include "_References/Localized_ini.h"

namespace Localized_ini {

    std::unique_ptr<CSimpleIniA> GetIniHandle() {
        const auto &path = INIPath.c_str();

        auto ini = std::make_unique<CSimpleIniA>();
        ini->SetUnicode();

        SI_Error rc = ini->LoadFile(path);
        if (rc < 0) {
            if (!fileDoesNotExistReported) {
                ERROR("Localization file not found, make sure SKSE/Plugins/SkyParkourNG_Localization.ini exists");
                fileDoesNotExistReported = true;
            }

            return nullptr;
        }

        return ini;
    }
}  // namespace Localized_ini