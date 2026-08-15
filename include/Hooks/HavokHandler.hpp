#pragma once
#include "Util/HookingUtil.hpp"

namespace Hooks
{
    class HavokHandler {
        public:
            static inline bool InstallHooks();

            // void ControllerSubroutine(RE::bhkCharacterController *a_controller) {
            //     using func_t = decltype(ControllerSubroutine);
            //     REL::Relocation<func_t> func{RELOCATION_ID(76440, 78280)}; /*dc08e0*/
            //     return func(a_controller);
            // }

        private:
            class hkbClipGenerator {
                private:
                    /* Return types of funcs. helper InstallVFuncHooks function does not work cause MSVC can't deduce type
                       with decltype so using the type as alias and deriving the pointer type for OG funcs */
                    struct Signatures {
                            using Activate_t = void(RE::hkbClipGenerator *a_this, const RE::hkbContext &a_context);
                    };

                public:
                    /* Return type is bool, did it install or not */
                    struct InstallHook {
                            static bool Activate();
                    };

                    struct Callback {
                            /* Function signature defined above */
                            static Signatures::Activate_t Activate;
                    };

                    struct OG {
                            /* Original function of the game, pointer derived from the signature */
                            static inline REL::Relocation<Signatures::Activate_t *> _Activate;
                    };
            };

            class bhkCharacterStateClimbing {
                private:
                    /* Return types of funcs. helper InstallVFuncHooks function does not work cause MSVC can't deduce type
                       with decltype so using the type as alias and deriving the pointer type for OG funcs */
                    struct Signatures {
                            using Update_t = void(RE::bhkCharacterStateOnGround *, RE::hkpCharacterContext &, const RE::hkpCharacterInput &,
                                                  RE::hkpCharacterOutput &);
                    };

                public:
                    struct InstallHook {
                            static bool Update();
                    };

                    struct Callback {
                            static Signatures::Update_t Update;
                    };

                    struct OG {
                            static inline REL::Relocation<Signatures::Update_t *> _Update;
                    };
            };
    };
}  // namespace Hooks

#pragma region  // Install All

using clipgenerator = Hooks::HavokHandler::hkbClipGenerator;
using climbing = Hooks::HavokHandler::bhkCharacterStateClimbing;

bool Hooks::HavokHandler::InstallHooks()
{
    bool res = true;

    // res &= clipgenerator::InstallHook::Activate();
    // res &= climbing::InstallHook::Update();

    return res;
}

#pragma endregion

#pragma region  // hkbClipGenerator

// Install
bool clipgenerator::InstallHook::Activate()
{
    REL::Relocation<uintptr_t> vtbl{RE::VTABLE_hkbClipGenerator[0]};

    const bool res = Hooking::InstallVFuncHook(vtbl, 0x4, OG::_Activate, &Callback::Activate);
    if (!res) CRITICAL("ClipGenerator Activate Hook Not Installed");
    return res;
}

// Callback
void clipgenerator::Callback::Activate(RE::hkbClipGenerator *a_this, const RE::hkbContext &a_context)
{
    OG::_Activate(a_this, a_context);
}

#pragma endregion

#pragma region  // bhkCharacterState

// Install
bool climbing::InstallHook::Update()
{
    REL::Relocation<uintptr_t> vtbl{RE::VTABLE_bhkCharacterStateOnGround[0]};

    const bool res = Hooking::InstallVFuncHook(vtbl, 0x6, OG::_Update, &Callback::Update);
    if (!res) CRITICAL("Climbing Update Hook Not Installed");
    return res;
}

// Callback
void climbing::Callback::Update(RE::bhkCharacterStateOnGround *a_this, RE::hkpCharacterContext &a_context,
                                const RE::hkpCharacterInput &a_input, RE::hkpCharacterOutput &a_output)
{
    INFO("CHAR STATE UPDATE");
    return OG::_Update(a_this, a_context, a_input, a_output);
}

#pragma endregion