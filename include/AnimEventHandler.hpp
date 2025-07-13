namespace Hooks {
    bool didActivate = false;

    template <class T>
    class AnimationEventHook : public T {
        public:
            struct ParsedPayload {
                    float x, y, z, sec;
            };

            using Fn_t = decltype(&T::ProcessEvent);
            static inline REL::Relocation<Fn_t> _ProcessEvent;  // 01
            inline RE::BSEventNotifyControl Hook(const RE::BSAnimationGraphEvent* a_event,
                                                 RE::BSTEventSource<RE::BSAnimationGraphEvent>* a_eventSource) {
                if (ModSettings::ModEnabled) {
                    if (a_event) {
                        auto actor = a_event->holder;
                        if (actor && actor->IsPlayerRef()) {
                            if (a_event->tag == "GetUpExit") {
                                /*player->IsInRagdoll() does not fully cover the getting up animation, which I also can't allow at all. Set this to false on GetUpExit anim event*/
                                RuntimeVariables::IsInRagdollOrGettingUp = false;
                            }

                            if (RuntimeVariables::ParkourInProgress) {
                                //logger::info(">> AnimEvent: {} Payload: {}", a_event->tag.c_str(), a_event->payload.c_str());

                                if (a_event->tag == "SkyParkour_Move") {
                                    if (!a_event->payload.empty()) {
                                        const auto player = RE::PlayerCharacter::GetSingleton();
                                        const ParsedPayload parsed = ParsePayload(a_event->payload.c_str());

                                        const auto relativePos = RE::NiPoint3{parsed.x, parsed.y, parsed.z};
                                        const auto seconds = parsed.sec;

                                        constexpr bool isRelative = true;
                                        Parkouring::InterpolateRefToPosition(player, relativePos, seconds, isRelative);
                                    }
                                }
                                else if (a_event->tag == "SkyParkour_Start") {
                                    didActivate = true;
                                    const auto player = RE::PlayerCharacter::GetSingleton();
                                    ParkourUtility::ToggleControlsForParkour(false);
                                    ParkourUtility::StopInteractions(*player);
                                }
                                else if (a_event->tag == "SkyParkour_Recovery") {
                                    RuntimeVariables::RecoveryFramesActive = true;
                                }
                                else if (a_event->tag == "SkyParkour_Stop") {
                                    const auto player = RE::PlayerCharacter::GetSingleton();
                                    Parkouring::StopInterpolationToPosition();

                                    player->As<RE::IAnimationGraphManagerHolder>()->SetGraphVariableInt("SkyParkourLedge",
                                                                                                        ParkourType::NoLedge);
                                    /* Reenable controls */
                                    ParkourUtility::ToggleControlsForParkour(true);

                                    RuntimeVariables::RecoveryFramesActive = false;
                                    RuntimeVariables::ParkourInProgress = false;

                                    didActivate = false;
                                }
                            }
                        }
                    }
                }
                return _ProcessEvent(this, a_event, a_eventSource);
            }
            static void InstallAnimEventHook() {
                // This is the Event notify hook, equivalent of an event sink. Event will go regardless. Don't return anything in this except the OG func.
                auto vtbl = REL::Relocation<std::uintptr_t>(RE::VTABLE_BSAnimationGraphManager[0]);
                constexpr std::size_t idx = 0x1;
                _ProcessEvent = vtbl.write_vfunc(idx, &Hook);
                logger::info(">> AnimEvent Hook Installed");
            }

        private:
            /* TODO: UNSAFE, fix this */
            ParsedPayload ParsePayload(const char* payload) {
                /* Expected anim event - payload format: SkyParkour_Move.x|y|z@s */
                /* Separators are actually just non numeric chars, but for clarity this one seems fine */
                const char* ptr = payload;

                float x = std::strtof(ptr, const_cast<char**>(&ptr));
                ++ptr;  // Skip '|'

                float y = std::strtof(ptr, const_cast<char**>(&ptr));
                ++ptr;  // Skip '|'

                float z = std::strtof(ptr, const_cast<char**>(&ptr));
                ++ptr;  // Skip '@'

                float sec = std::strtof(ptr, const_cast<char**>(&ptr));

                return {x, y, z, sec};
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
    if (didActivate && RuntimeVariables::ParkourInProgress && a_eventName == "SkyParkour") {
        /* Don't send event during parkour. TODO: Move this into behavior patch later by disabling self transition. */
        return false;
    }

    if (a_eventName == "SkyParkour" && !RuntimeVariables::IsParkourActive) {
        return false;
    }

    if (a_eventName == "Ragdoll") {
        /*player->IsInRagdoll() does not fully cover the getting up animation, which I also can't allow at all. Set this to false on GetUpExit anim event*/
        RuntimeVariables::IsInRagdollOrGettingUp = true;
        if (RuntimeVariables::ParkourInProgress) {
            /*Unlock controls on ragdoll*/
            bool didRagdoll = _origPlayerCharacter(a_this, a_eventName);
            if (didRagdoll) {
                Parkouring::StopInterpolationToPosition();
                ParkourUtility::ToggleControlsForParkour(true);
                RuntimeVariables::ParkourInProgress = false;
            }
            return didRagdoll;
        }
    }
    else if (a_eventName == "SkyParkour_Stop") {
        const auto player = RE::PlayerCharacter::GetSingleton();
        Parkouring::StopInterpolationToPosition();

        player->As<RE::IAnimationGraphManagerHolder>()->SetGraphVariableInt("SkyParkourLedge", ParkourType::NoLedge);
        /* Reenable controls */
        ParkourUtility::ToggleControlsForParkour(true);

        RuntimeVariables::RecoveryFramesActive = false;
        RuntimeVariables::ParkourInProgress = false;

        didActivate = false;
    }
    return _origPlayerCharacter(a_this, a_eventName);
}