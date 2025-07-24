#include "References.h"
#include "Parkouring.h"
#include "Util/ParkourUtility.h"

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
                            if (a_event->tag == "GetUpExit") {
                                /* Reset vars on ragdoll exit */
                                RuntimeMethods::ResetRuntimeVariables();
                            }

                            if (RuntimeVariables::ParkourInProgress) {
                                //logger::info(">> AnimEvent: {} Payload: {}", a_event->tag.c_str(), a_event->payload.c_str());

                                if (a_event->tag == SPPF_MOVE) {
                                    if (!a_event->payload.empty()) {
                                        const auto player = RE::PlayerCharacter::GetSingleton();
                                        player->GetCharController()->SetLinearVelocityImpl(ZERO_VECTOR);

                                        const ParsedPayload parsed = ParsePayload(a_event->payload.c_str());

                                        const auto relativePos = RE::NiPoint3{parsed.x, parsed.y, parsed.z};
                                        const auto seconds = parsed.sec;
                                        constexpr bool isRelative = true;

                                        Parkouring::InterpolateRefToPosition(player, relativePos, seconds, isRelative);
                                    }
                                }
                                else if (a_event->tag == SPPF_START) {
                                    const auto player = RE::PlayerCharacter::GetSingleton();
                                    ParkourUtility::ToggleControls(false);
                                    ParkourUtility::StopInteractions(*player);
                                }
                                else if (a_event->tag == SPPF_RECOVERY) {
                                    RuntimeVariables::RecoveryFramesActive = true;
                                    const auto player = RE::PlayerCharacter::GetSingleton();

                                    if (!ParkourUtility::RayCast(player->GetPosition(), RE::NiPoint3(0, 0, -1), 50,
                                                                 COL_LAYER_EXTEND::kClimbLedge)
                                             .didHit) {
                                        player->NotifyAnimationGraph(SPPF_STOP);
                                    }
                                }
                                else if (a_event->tag == SPPF_STOP) {
                                    const auto player = RE::PlayerCharacter::GetSingleton();
                                    Parkouring::StopInterpolatingRef(player);

                                    player->As<RE::IAnimationGraphManagerHolder>()->SetGraphVariableInt(SPPF_Ledge, ParkourType::NoLedge);
                                    /* Reenable controls */
                                    ParkourUtility::ToggleControls(true);
                                    RuntimeVariables::PlayerStartPosition = RE::NiPoint3{0, 0, 0};
                                    RuntimeVariables::RecoveryFramesActive = false;
                                    RuntimeVariables::ParkourInProgress = false;
                                    Parkouring::UpdateParkourPoint(); /* Update the ledge point once */
                                }
                            }
                        }
                    }
                }
                return _ProcessEvent(this, a_event, a_eventSource);
            }

            static bool InstallAnimEventHook() {
                // This is the Event notify hook, equivalent of an event sink. Event will go regardless. Don't return anything in this except the OG func.
                auto vtbl = REL::Relocation<std::uintptr_t>(RE::VTABLE_BSAnimationGraphManager[0]);
                constexpr std::size_t idx = 0x1;
                _ProcessEvent = vtbl.write_vfunc(idx, &Hook);

                if (!_ProcessEvent.address()) {
                    logger::critical("AnimEvent Hook Not Installed");
                    return false;
                }
                return true;
            }

        private:
            struct ParsedPayload {
                    float x, y, z, sec;
            };

            static float parse_next(const char*& ptr, char delim) {
                char* end;
                float v = std::strtof(ptr, &end);
                if (end == ptr) {
                    return 0;
                }
                ptr = end;
                if (*ptr == delim)
                    ++ptr;
                return v;
            }

            ParsedPayload ParsePayload(const char* payload) {
                /* Expected anim event - payload format: SkyParkour_Move.x|y|z@s */
                const char* ptr = payload;

                float x = parse_next(ptr, '|');
                float y = parse_next(ptr, '|');
                float z = parse_next(ptr, '@');
                float sec = parse_next(ptr, '\0');

                return {x, y, z, sec};
            }
    };

    class NotifyGraphHandler {
        public:
            static bool InstallGraphNotifyHook();

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

bool Hooks::NotifyGraphHandler::InstallGraphNotifyHook() {
    // TESObjectREFR
    //REL::Relocation<uintptr_t> vtblTES{RE::VTABLE_TESObjectREFR[3]};
    //_origTESObjectREFR = vtblTES.write_vfunc(0x1, OnTESObjectREFR);

    // Character
    //REL::Relocation<uintptr_t> vtblChar{RE::VTABLE_Character[3]};
    //_origCharacter = vtblChar.write_vfunc(0x1, OnCharacter);

    // PlayerCharacter
    REL::Relocation<uintptr_t> vtblPlayer{RE::VTABLE_PlayerCharacter[3]};
    _origPlayerCharacter = vtblPlayer.write_vfunc(0x1, OnPlayerCharacter);

    if (!_origPlayerCharacter.address()) {
        logger::critical("Notify Hook Not Installed");
        return false;
    }
    return true;
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
    if (a_eventName == SPPF_NOTIFY &&
        (!RuntimeVariables::IsParkourActive || (RuntimeVariables::ParkourInProgress && !RuntimeVariables::EnableNotifyWindow))) {
        return false;
    }

    if (a_eventName == "Ragdoll") {
        if (RuntimeVariables::ParkourInProgress) {
            /*Unlock controls on ragdoll*/

            bool didRagdoll = _origPlayerCharacter(a_this, a_eventName);
            if (didRagdoll) {
                const auto player = RE::PlayerCharacter::GetSingleton();
                Parkouring::StopInterpolatingRef(player);
                ParkourUtility::ToggleControls(true);
                RuntimeVariables::ParkourInProgress = false;
            }
            return didRagdoll;
        }
    }
    if (a_eventName == SPPF_STOP) {
        const auto player = RE::PlayerCharacter::GetSingleton();
        Parkouring::StopInterpolatingRef(player);

        player->As<RE::IAnimationGraphManagerHolder>()->SetGraphVariableInt(SPPF_Ledge, ParkourType::NoLedge);

        /* Reenable controls */
        ParkourUtility::ToggleControls(true);
        RuntimeVariables::PlayerStartPosition = RE::NiPoint3{0, 0, 0};
        RuntimeVariables::RecoveryFramesActive = false;
        RuntimeVariables::ParkourInProgress = false;
        Parkouring::UpdateParkourPoint(); /* Update the ledge point once */
    }
    return _origPlayerCharacter(a_this, a_eventName);
}