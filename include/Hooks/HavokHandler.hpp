#pragma once
namespace Hooks {
    class HavokHandler {
        public:
            static inline bool InstallHooks();

        private:
            struct hkbClipGenerator {
                    struct Install {
                            static bool Activate();
                    };

                    struct Callback {
                            static void Activate(RE::hkbClipGenerator *a_this, const RE::hkbContext &a_context);
                    };

                    struct OG {
                            static inline REL::Relocation<decltype(Callback::Activate)> _Activate;
                    };
            };
    };
}  // namespace Hooks

// Install

bool Hooks::HavokHandler::InstallHooks() {
    bool res = false;

    res &= hkbClipGenerator::Install::Activate();

    return res;
}

bool Hooks::HavokHandler::hkbClipGenerator::Install::Activate() {
    REL::Relocation<uintptr_t> vtbl{RE::VTABLE_hkbClipGenerator[0]};
    OG::_Activate = vtbl.write_vfunc(0x4, &Callback::Activate);

    if (!OG::_Activate.address()) {
        CRITICAL("ClipGenerator Activate Hook Not Installed");
        return false;
    }
    return true;
}

// Callback

void Hooks::HavokHandler::hkbClipGenerator::Callback::Activate(RE::hkbClipGenerator *a_this, const RE::hkbContext &a_context) {
    OG::_Activate(a_this, a_context);
}