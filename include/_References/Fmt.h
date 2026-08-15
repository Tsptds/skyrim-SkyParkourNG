#include <fmt/format.h>

template <>
struct fmt::formatter<RE::hkVector4> {
        // No custom format specifiers
        constexpr auto parse(format_parse_context &ctx)
        {
            return ctx.begin();
        }

        template <typename FormatContext>
        auto format(const RE::hkVector4 &v, FormatContext &ctx) const
        {
            return fmt::format_to(ctx.out(), "({:.3f}, {:.3f}, {:.3f}, {:.3f})", v.quad.m128_f32[0], v.quad.m128_f32[1], v.quad.m128_f32[2],
                                  v.quad.m128_f32[3]);
        }
};

template <>
struct fmt::formatter<RE::hkpSurfaceInfo::SupportedState> : fmt::formatter<std::string_view> {
        auto format(RE::hkpSurfaceInfo::SupportedState a_state, fmt::format_context &a_ctx) const
        {
            std::string_view name;
            switch (a_state) {
                case RE::hkpSurfaceInfo::SupportedState::kUnsupported:
                    name = "No";
                    break;
                case RE::hkpSurfaceInfo::SupportedState::kSliding:
                    name = "Slide";
                    break;
                case RE::hkpSurfaceInfo::SupportedState::kSupported:
                    name = "Yes";
                    break;
                default:
                    name = "Udf";
                    break;
            }
            return fmt::formatter<std::string_view>::format(name, a_ctx);
        }
};