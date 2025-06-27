namespace Hooks {
    template <class T>
    class AnimationEventHook : public T {
        public:
            using Fn_t = decltype(&T::ProcessEvent);
            static inline REL::Relocation<Fn_t> _ProcessEvent;  // 01
            inline RE::BSEventNotifyControl Hook(const RE::BSAnimationGraphEvent* a_event,
                                                 RE::BSTEventSource<RE::BSAnimationGraphEvent>* a_eventSource) {
                if (ModSettings::ModEnabled) {
                    if (a_event) {
                        auto actor = a_event->holder;
                        if (actor && actor->IsPlayerRef()) {
                            /*player->IsInRagdoll() does not fully cover the getting up animation, which I also can't allow at all. Set this to false on GetUpExit anim event*/
                            if (a_event->tag == "GetUpExit") {
                                RuntimeVariables::IsInRagdollOrGettingUp = false;
                            }

                            if (RuntimeVariables::ParkourInProgress) {
                                //logger::info(">> AnimEvent: {} Payload: {}", a_event->tag.c_str(), a_event->payload.c_str());
                                const auto player = RE::PlayerCharacter::GetSingleton();
                                if (a_event->tag == "SkyParkour_Begin") {
                                    const auto payload = a_event->payload.c_str();
                                    Parkouring::InterpolateRefToPosition(player, RuntimeVariables::ledgePoint,
                                                                         std::strtof(payload, nullptr));
                                }
                                else if (a_event->tag == "SkyParkour_OverShoot") {
                                    const auto payload = a_event->payload.c_str();
                                    Parkouring::InterpolateRefToPosition(player, (RuntimeVariables::ledgePoint + RE::NiPoint3(0, 0, 50.0f)),
                                                                         std::strtof(payload, nullptr));
                                }
                                else if (a_event->tag == "SkyParkour_UnderShoot") {
                                    const auto payload = a_event->payload.c_str();
                                    Parkouring::InterpolateRefToPosition(
                                        player, (RuntimeVariables::ledgePoint + RE::NiPoint3(0, 0, -90.0f * RuntimeVariables::PlayerScale)),
                                        std::strtof(payload, nullptr));
                                }
                                else if (a_event->tag == "SkyParkour_VaultForward") {
                                    const auto payload = a_event->payload.c_str();

                                    auto onlyForwardVec = player->GetPosition() + RuntimeVariables::playerDirFlat * 20;

                                    Parkouring::InterpolateRefToPosition(player, onlyForwardVec, std::strtof(payload, nullptr));
                                }
                                else if (a_event->tag == "SkyParkour_RelativeForward") {
                                    const auto payload = a_event->payload.c_str();

                                    auto onlyForwardVec = player->GetPosition() + RuntimeVariables::playerDirFlat * 20;

                                    Parkouring::InterpolateRefToPosition(player, onlyForwardVec, std::strtof(payload, nullptr));
                                }
                                else if (a_event->tag == "SkyParkour_RelativeDownward") {
                                    const auto payload = a_event->payload.c_str();

                                    auto onlyForwardVec = player->GetPosition() + RE::NiPoint3{0, 0, -30};

                                    Parkouring::InterpolateRefToPosition(player, onlyForwardVec, std::strtof(payload, nullptr));
                                }

                                else if (a_event->tag == "SkyParkour_Start") {
                                    ParkourUtility::ToggleControlsForParkour(false);
                                    ParkourUtility::StopInteractions(*player);
                                }
                                else if (a_event->tag == "SkyParkour_Stop") {
                                    player->As<RE::IAnimationGraphManagerHolder>()->SetGraphVariableInt("SkyParkourLedge",
                                                                                                        ParkourType::NoLedge);
                                    /* Reenable controls */
                                    ParkourUtility::ToggleControlsForParkour(true);
                                    RuntimeVariables::ParkourInProgress = false;

                                    Parkouring::StopInterpolationToPosition();
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
    if (a_eventName == "testmove1") {
        auto player = RE::PlayerCharacter::GetSingleton();
        auto curPos = player->GetPosition();
        auto diff = RE::NiPoint3{0, 0, 120};
        auto newPos = curPos + diff;
        auto speed = diff.Length() * 0.5f;  // 2s

        logger::info("{}", player->GetPosition());
        Parkouring::InterpolateRefToPosition(player, newPos, speed);
    }
    else if (a_eventName == "testmove2") {
        auto player = RE::PlayerCharacter::GetSingleton();
        auto curPos = player->GetPosition();
        auto diff = RE::NiPoint3{0, 0, 120};
        auto newPos = curPos + diff;
        auto speed = diff.Length();  // 1s

        logger::info("{}", player->GetPosition());
        Parkouring::InterpolateRefToPosition(player, newPos, speed);
    }
    else if (a_eventName == "testmove3") {
        auto player = RE::PlayerCharacter::GetSingleton();
        auto curPos = player->GetPosition();
        auto diff = RE::NiPoint3{0, 0, 120};
        auto newPos = curPos + diff;
        auto speed = diff.Length() * 2;  // 1s

        logger::info("{}", player->GetPosition());
        Parkouring::InterpolateRefToPosition(player, newPos, speed);
    }

    if (a_eventName == "Ragdoll") {
        /*player->IsInRagdoll() does not fully cover the getting up animation, which I also can't allow at all. Set this to false on GetUpExit anim event*/
        RuntimeVariables::IsInRagdollOrGettingUp = true;
        if (RuntimeVariables::ParkourInProgress) {
            /*Unlock controls on ragdoll*/
            bool didRagdoll = _origPlayerCharacter(a_this, a_eventName);
            if (didRagdoll) {
                ParkourUtility::ToggleControlsForParkour(true);
                RuntimeVariables::ParkourInProgress = false;
            }
            return didRagdoll;
        }
    }
    return _origPlayerCharacter(a_this, a_eventName);
}