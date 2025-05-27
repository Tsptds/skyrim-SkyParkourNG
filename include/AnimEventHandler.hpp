namespace Hooks {
    template <class T>
    class AnimationEventHook : public T {
        public:
            using Fn_t = decltype(&T::ProcessEvent);
            static inline REL::Relocation<Fn_t> _ProcessEvent;  // 01
            inline RE::BSEventNotifyControl Hook(const RE::BSAnimationGraphEvent* a_event,
                                                 RE::BSTEventSource<RE::BSAnimationGraphEvent>* a_eventSource) {
                if (ModSettings::ModEnabled) {
                    if (RuntimeVariables::ParkourInProgress) {
                        if (a_event) {
                            auto actor = a_event->holder;
                            if (actor && actor->IsPlayerRef()) {
                                logger::info(">> AnimEvent: {} Payload: {}", a_event->tag.c_str(), a_event->payload.c_str());

                                const auto player = RE::PlayerCharacter::GetSingleton();
                                if (a_event->tag == "SkyParkour_Begin") {
                                    const auto payload = a_event->payload.c_str();
                                    if (payload)
                                        Parkouring::InterpolateRefToPosition(player, RuntimeVariables::ledgePoint,
                                                                             std::strtof(payload, nullptr), 1000);
                                }
                                else if (a_event->tag == "SkyParkour_End" || a_event->tag == "JumpLandEnd") {
                                    player->NotifyAnimationGraph("JumpLandEnd");
                                    player->As<RE::IAnimationGraphManagerHolder>()->SetGraphVariableInt("SkyParkourLedge",
                                                                                                        ParkourType::NoLedge);
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
                logger::info(">> AnimEvent Hook Installed");
            }
    };
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
    if (RuntimeVariables::ParkourInProgress) {
        if (a_eventName == "JumpLandEnd") {
            // Reenable controls
            ParkourUtility::ToggleControlsForParkour(true);
            Parkouring::UpdateParkourPoint();
            RuntimeVariables::ParkourInProgress = false;
        }
    }
    return _origPlayerCharacter(a_this, a_eventName);
}