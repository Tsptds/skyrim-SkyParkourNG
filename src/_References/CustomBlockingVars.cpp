#include "_References/CustomBlockingVars.h"

namespace CustomBlockingVars {
    std::unique_ptr<CSimpleIniA> GetHandle() {
        const auto path = FilePath.c_str();

        auto ini = std::make_unique<CSimpleIniA>();
        ini->SetUnicode();

        SI_Error rc = ini->LoadFile(path);
        if (rc < 0) {
            return nullptr;
        }

        return ini;
    }

    bool ReadAndCacheVars() {
        auto ini = CustomBlockingVars::GetHandle();

        if (ini) {
            std::list<CSimpleIniA::Entry> lst_par;
            std::list<CSimpleIniA::Entry> lst_sld;
            ini->GetAllKeys("Parkour", lst_par);
            ini->GetAllKeys("Slide", lst_sld);

            for (auto &&i: lst_par) {
                CustomBlockingVars::ParkourList.push_back(RE::BSFixedString(i.pItem));
            }

            for (auto &&i: lst_sld) {
                CustomBlockingVars::SlideList.push_back(RE::BSFixedString(i.pItem));
            }
            return true;
        }
        return false;
    }
}  // namespace CustomBlockingVars