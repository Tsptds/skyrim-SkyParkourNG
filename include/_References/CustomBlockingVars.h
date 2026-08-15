#pragma once

namespace CustomBlockingVars
{
    inline std::string FilePath{"./Data/SKSE/Plugins/SkyParkourNG_CustomVars.ini"};
    extern std::unique_ptr<CSimpleIniA> GetHandle();
    extern bool ReadAndCacheVars();

    inline std::vector<RE::BSFixedString> ParkourList{};
    inline std::vector<RE::BSFixedString> SlideList{};

}  // namespace CustomBlockingVars