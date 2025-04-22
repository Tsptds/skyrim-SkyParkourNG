//namespace Hooks {
//
//    template <class T>
//    class AnimGraphHolderEx : public T {
//        public:
//            static AnimGraphHolderEx* GetSingleton() {
//                static AnimGraphHolderEx singleton;
//                return &singleton;
//            }
//
//            AnimGraphHolderEx() = default;
//            ~AnimGraphHolderEx() = default;
//
//            using Construct_t = decltype(&T::PostCreateAnimationGraphManager);
//            static inline REL::Relocation<Construct_t> _PostCreateAnimationGraphManager;
//
//            void PostCreateAnimationGraphManager(RE::BSTSmartPointer<RE::BSAnimationGraphManager>& a_animGraphMgr);
//
//            static void InstallConstructHook();
//    };
//
//    template <class T>
//    inline void AnimGraphHolderEx<T>::PostCreateAnimationGraphManager(RE::BSTSmartPointer<RE::BSAnimationGraphManager>& a_animGraphMgr) {
//        logger::info("Graph Refresh");
//        return _PostCreateAnimationGraphManager(this, a_animGraphMgr);
//    }
//
//    template <class T>
//    inline void AnimGraphHolderEx<T>::InstallConstructHook() {
//        auto a_vtbl = REL::Relocation<std::uintptr_t>(RE::VTABLE_IAnimationGraphManagerHolder[0]);
//        std::uint64_t a_offset = 0xB;
//        _PostCreateAnimationGraphManager = a_vtbl.write_vfunc(a_offset, &AnimGraphHolderEx<T>::PostCreateAnimationGraphManager);
//        logger::info("ConstructAnimationGraph Hook Installed");
//    }
//
//}  // namespace Hooks
//
//// somewhere in your plugin startup:
////void InstallAllHooks() {
////    Hooks::AnimGraphHolderEx<RE::SimpleAnimationGraphManagerHolder>::InstallConstructHook();
////}
namespace Hooks {
    template <class T>
    class AnimationEventHook : public T {
        public:
            //RE::BSAnimationGraphManager
            using Fn_t = decltype(&T::ProcessEvent);
            static inline REL::Relocation<Fn_t> _ProcessEvent;  // 01
            inline RE::BSEventNotifyControl Hook(const RE::BSAnimationGraphEvent* a_event,
                                                 RE::BSTEventSource<RE::BSAnimationGraphEvent>* a_eventSource) {
                if (ModSettings::ModEnabled) {
                    if (a_event) {
                        auto actor = a_event->holder;
                        if (actor && actor->IsPlayerRef()) {
                            //logger::info(">> AnimEvent: {}", a_event->tag.c_str());

                            if (RuntimeVariables::ParkourEndQueued) {
                                if (RE::PlayerCharacter::GetSingleton()->IsInRagdollState()) {
                                    ParkourUtility::ToggleControlsForParkour(true);
                                    RuntimeVariables::ParkourEndQueued = false;
                                }
                                // Reenable controls
                                else if (a_event->tag == "idleChairGetUp") {
                                    // Swap the leg for step animation
                                    RuntimeMethods::SwapLegs();

                                    ParkourUtility::ToggleControlsForParkour(true);
                                    Parkouring::UpdateParkourPoint();
                                    RuntimeVariables::ParkourEndQueued = false;
                                }
                            }
                        }
                    }
                }
                return _ProcessEvent(this, a_event, a_eventSource);
            }
            static void InstallAnimEventHook() {
                // Hooking the vfunc directly
                auto vtbl = REL::Relocation<std::uintptr_t>(RE::VTABLE_BSAnimationGraphManager[0]);
                constexpr std::size_t idx = 0x1;
                _ProcessEvent = vtbl.write_vfunc(idx, &Hook);
                logger::info("AnimEvent Hook Installed");
            }
    };
    //struct AnimEventSink {
    //        // The signature of the vfunc we’re patching:
    //        using Fn = RE::BSEventNotifyControl(__fastcall*)(RE::BSAnimationGraphManager*, const RE::BSAnimationGraphEvent*,
    //                                                         RE::BSTEventSource<RE::BSAnimationGraphEvent>*);

    //        static inline REL::Relocation<Fn> _ProcessEvent;

    //        // Our detour—fires on every SendAnimationEvent()/actor.Notify():
    //        static RE::BSEventNotifyControl __fastcall Hooked_ProcessEvent(RE::BSAnimationGraphManager* mgr,
    //                                                                       const RE::BSAnimationGraphEvent* ev,
    //                                                                       RE::BSTEventSource<RE::BSAnimationGraphEvent>* src) {
    //            if (ev && ev->holder) {
    //                if (auto pc = ev->holder; pc == RE::PlayerCharacter::GetSingleton()) {
    //                    logger::info(">> AnimEvent ({}): payload=\"{}\"", ev->tag.c_str(), ev->payload.c_str());
    //                }
    //            }
    //            return _ProcessEvent(mgr, ev, src);
    //        }

    //        static void Install() {
    //            // Grab the BSAnimationGraphManager vtable
    //            auto vtbl = REL::Relocation<std::uintptr_t>(
    //                RE::VTABLE_BSAnimationGraphManager[0]  // VTABLE at 0x17C78E8 :contentReference[oaicite:0]{index=0}
    //            );
    //            // Slot 0 = dtor, slot 1 = ProcessEvent (the BSTEventSink override) :contentReference[oaicite:1]{index=1}
    //            constexpr std::size_t idx = 1;
    //            _ProcessEvent = vtbl.write_vfunc(idx, &Hooked_ProcessEvent);
    //            logger::info("BSAnimationGraphManager::ProcessEvent hook installed");
    //        }
    //};

    //template <class T>
    //class AnimationEventHook : public T {
    //    public:
    //        static AnimationEventHook* GetSingleton() {
    //            static AnimationEventHook singleton;
    //            return &singleton;
    //        }
    //        AnimationEventHook() = default;
    //        ~AnimationEventHook() = default;

    //        using NotifyAnimationGraph_t = decltype(&T::NotifyAnimationGraph);
    //        static inline REL::Relocation<NotifyAnimationGraph_t> _NotifyAnimationGraphPlayer;     // 01
    //        static inline REL::Relocation<NotifyAnimationGraph_t> _NotifyAnimationGraphCharacter;  // 01

    //        bool NotifyAnimationGraph_Hooked(const RE::BSFixedString& a_eventName);

    //        static void InstallNotifyHook();
    //};
    //template <class T>
    //inline bool AnimationEventHook<T>::NotifyAnimationGraph_Hooked(const RE::BSFixedString& a_eventName) {
    //    logger::info(">> {}", a_eventName);
    //    return _NotifyAnimationGraphCharacter(this, a_eventName);
    //}

    //template <class T>
    //inline void AnimationEventHook<T>::InstallNotifyHook() {
    //    auto vtblPlayer = REL::Relocation<std::uintptr_t>(RE::VTABLE_PlayerCharacter[3]);
    //    auto vtblCharacter = REL::Relocation<std::uintptr_t>(RE::VTABLE_Character[3]);
    //    std::uint64_t a_offset = 0x1;
    //    _NotifyAnimationGraphPlayer = vtblPlayer.write_vfunc(a_offset, &AnimationEventHook<T>::NotifyAnimationGraph);
    //    _NotifyAnimationGraphCharacter = vtblCharacter.write_vfunc(a_offset, &AnimationEventHook<T>::NotifyAnimationGraph);
    //    logger::info("AnimEvent Hook Installed");
    //}
}  // namespace Hooks
namespace Hooks {
    class NotifyGraphHandler {
        public:
            static void InstallGraphNotifyHook();

        private:
            // Our hook callbacks
            static bool OnTESObjectREFR(RE::IAnimationGraphManagerHolder* a_this, const RE::BSFixedString& a_eventName);
            static bool OnCharacter(RE::IAnimationGraphManagerHolder* a_this, const RE::BSFixedString& a_eventName);
            static bool OnPlayerCharacter(RE::IAnimationGraphManagerHolder* a_this, const RE::BSFixedString& a_eventName);

            // Originals
            static inline REL::Relocation<decltype(OnTESObjectREFR)> _origTESObjectREFR;
            static inline REL::Relocation<decltype(OnCharacter)> _origCharacter;
            static inline REL::Relocation<decltype(OnPlayerCharacter)> _origPlayerCharacter;
    };
}  // namespace Hooks

void Hooks::NotifyGraphHandler::InstallGraphNotifyHook() {
    // TESObjectREFR
    //REL::Relocation<uintptr_t> vtblTES{RE::VTABLE_TESObjectREFR[3]};
    //_origTESObjectREFR = vtblTES.write_vfunc(0x1, OnTESObjectREFR);

    // Character
    //REL::Relocation<uintptr_t> vtblChar{RE::VTABLE_Character[3]};
    //_origCharacter = vtblChar.write_vfunc(0x1, OnCharacter);

    // PlayerCharacter
    REL::Relocation<uintptr_t> vtblPlayer{RE::VTABLE_PlayerCharacter[3]};
    _origPlayerCharacter = vtblPlayer.write_vfunc(0x1, OnPlayerCharacter);

    logger::info(">> Notify Graph Hook Installed");
}

bool Hooks::NotifyGraphHandler::OnTESObjectREFR(RE::IAnimationGraphManagerHolder* a_this, const RE::BSFixedString& a_eventName) {
    // pre‑hook logic...
    bool result = _origTESObjectREFR(a_this, a_eventName);
    // post‑hook logic...
    logger::info(">> Object Anim Event: {}", a_eventName.c_str());
    return result;
}

bool Hooks::NotifyGraphHandler::OnCharacter(RE::IAnimationGraphManagerHolder* a_this, const RE::BSFixedString& a_eventName) {
    bool result = _origCharacter(a_this, a_eventName);
    logger::info(">> Char Anim Event: {}", a_eventName.c_str());
    return result;
}

bool Hooks::NotifyGraphHandler::OnPlayerCharacter(RE::IAnimationGraphManagerHolder* a_this, const RE::BSFixedString& a_eventName) {
    if (RuntimeVariables::ParkourEndQueued) {
        if (a_eventName == "swimStart" || a_eventName == "swimStop") {
            logger::info(">> Cancelled: {}", a_eventName.c_str());

            return false;
        }
    }
    // Notify occurs on function call, return value is to evaluate something I guess.
    return _origPlayerCharacter(a_this, a_eventName);

    //logger::info(">> {}", a_eventName);
}