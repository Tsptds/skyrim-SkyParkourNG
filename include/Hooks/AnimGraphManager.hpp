#pragma once
#include "Util/HookingUtil.hpp"
#include "_References/ModSettings.h"

namespace Hooks
{
    class GraphManagerHandler {
        public:
            inline static bool InstallGraphManagerHooks()
            {
                bool res{true};
                res &= Install::PostCreate();

                return res;
            }

        private:
            struct Signatures {
                    using PostCreate_t = void(RE::IAnimationGraphManagerHolder *a_this,
                                              RE::BSTSmartPointer<RE::BSAnimationGraphManager> &a_animGraphMgr);
            };

            struct Install {
                    static bool PostCreate();
            };

            struct Callback {
                    static Signatures::PostCreate_t PostCreate;
            };

            struct OG {
                    static inline REL::Relocation<Signatures::PostCreate_t *> _PostCreate;
            };
    };

    bool GraphManagerHandler::Install::PostCreate()
    {
        REL::Relocation<std::uintptr_t> vtbl{RE::VTABLE_PlayerCharacter[3]};
        const bool res = Hooking::InstallVFuncHook(vtbl, 0xB, OG::_PostCreate, &Callback::PostCreate);
        if (!res) CRITICAL("GraphManager Hook Not Installed");
        return res;
    }

    void GraphManagerHandler::Callback::PostCreate(RE::IAnimationGraphManagerHolder *a_this,
                                                   RE::BSTSmartPointer<RE::BSAnimationGraphManager> &a_animGraphMgr)
    {
        OG::_PostCreate(a_this, a_animGraphMgr);

        const auto actor = a_animGraphMgr->graphs[a_animGraphMgr->GetRuntimeData().activeGraph].get()->holder;
        INFO("Post Load Graph: {}", actor->GetName());

        if (actor->IsPlayerRef()) {
            HavokUtil::CreateBoundGraphChannels(actor, a_animGraphMgr);
            Parkouring::SetParkourOnOff(ModSettings::Parkour_Enabled);
            CrouchSliding::SetSlideOnOff(ModSettings::Crouch_Slide_Enabled);
        }
    }
}  // namespace Hooks