#pragma once
#include <fmt/format.h>

enum class ParkourType : int32_t { NoLedge = -1, Failed, Grab, Vault, StepLow, StepHigh, Low, Medium, High, Highest };


template <>
struct fmt::formatter<ParkourType> : fmt::formatter<std::string_view>
{
    auto format(ParkourType type, fmt::format_context& ctx) const
    {
        using enum ParkourType;

        std::string_view name = "Invalid";

        switch (type) {
        case NoLedge:  name = "NoLedge";  break;
        case Failed:   name = "Failed";   break;
        case Grab:     name = "Grab";     break;
        case Vault:    name = "Vault";    break;
        case StepLow:  name = "StepLow";  break;
        case StepHigh: name = "StepHigh"; break;
        case Low:      name = "Low";      break;
        case Medium:   name = "Medium";   break;
        case High:     name = "High";     break;
        case Highest:  name = "Highest";  break;
        }

        return fmt::formatter<std::string_view>::format(name, ctx);
    }
};
