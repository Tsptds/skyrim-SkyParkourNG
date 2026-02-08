#pragma once
#include "_References/ModSettings.h"
#include "_References/RuntimeMethods.h"
#include "_References/RuntimeVariables.h"

#include "Parkouring.h"
#include "Util/ParkourUtility.h"
#include "Util/HookingUtil.hpp"
#include "API/API_Handles.h"

#include "HUD/Scaleform/SkyParkourMenu.hpp"

namespace Hooks {

    class AnimationEventHook {
        public:
            static bool InstallAnimEventHook();

        private:
            struct Signatures {
                    using ProcessEvent_t = RE::BSEventNotifyControl(RE::BSAnimationGraphManager *a_this,
                                                                    const RE::BSAnimationGraphEvent *a_event,
                                                                    RE::BSTEventSource<RE::BSAnimationGraphEvent> *a_eventSource);
            };

            struct Callback {
                    static Signatures::ProcessEvent_t ProcessEvent;
            };

            struct OG {
                    static inline REL::Relocation<Signatures::ProcessEvent_t *> _ProcessEvent;  // 01
            };
    };

    class NotifyGraphHandler {
        public:
            static bool InstallGraphNotifyHook();

        private:
            struct Signatures {
                    using Notify_TESObjectRefr_t = bool(RE::IAnimationGraphManagerHolder *a_this, const RE::BSFixedString &a_eventName);
                    using Notify_Character_t = bool(RE::IAnimationGraphManagerHolder *a_this, const RE::BSFixedString &a_eventName);
                    using Notify_PlayerCharacter_t = bool(RE::IAnimationGraphManagerHolder *a_this, const RE::BSFixedString &a_eventName);
            };

            struct Callback {
                    static Signatures::Notify_TESObjectRefr_t Notify_TESObjectRefr;
                    static Signatures::Notify_Character_t Notify_Character;
                    static Signatures::Notify_PlayerCharacter_t Notify_PlayerCharacter;
            };

            struct OG {
                    static inline REL::Relocation<Signatures::Notify_TESObjectRefr_t *> _Notify_TESObjectRefr;
                    static inline REL::Relocation<Signatures::Notify_Character_t *> _Notify_Character;
                    static inline REL::Relocation<Signatures::Notify_PlayerCharacter_t *> _Notify_PlayerCharacter;
            };
    };

#pragma region  // AnimEvent
    bool AnimationEventHook::InstallAnimEventHook() {
        // This is the anim event hook, global event sink for everyone. Event will go regardless. Don't return anything in this except the OG func.
        // Sink gets destroyed when graph deletes, so using this
        REL::Relocation<std::uintptr_t> vtbl{RE::VTABLE_BSAnimationGraphManager[0]};

        const bool res = Hooking::InstallVFuncHook(vtbl, 0x1, OG::_ProcessEvent, &Callback::ProcessEvent);
        if (!res) CRITICAL("AnimEvent Hook Not Installed");
        return res;
    }

    RE::BSEventNotifyControl AnimationEventHook::Callback::ProcessEvent(RE::BSAnimationGraphManager *a_this,
                                                                        const RE::BSAnimationGraphEvent *a_event,
                                                                        RE::BSTEventSource<RE::BSAnimationGraphEvent> *a_eventSource) {
        if (!a_event) return OG::_ProcessEvent(a_this, a_event, a_eventSource);
        if (!ModSettings::Parkour_Enabled && !ModSettings::Crouch_Slide_Enabled) return OG::_ProcessEvent(a_this, a_event, a_eventSource);

        const auto &actor = a_this->graphs[a_this->GetRuntimeData().activeGraph]->holder;

        if (!actor || !actor->IsPlayerRef()) {
            return OG::_ProcessEvent(a_this, a_event, a_eventSource);
        }

        /* Sneak roll without perk fix, works on its own */
        if (actor->IsSneaking()) {
            if (a_event->tag == "JumpLandEnd") {
                bool sprinting;
                if (actor->GetGraphVariableBool("IsSprinting", sprinting) && sprinting) {
                    actor->NotifyAnimationGraph("SprintStop");
                }

                return OG::_ProcessEvent(a_this, a_event, a_eventSource);
            }
        }

        if (a_event->tag == "GetUpExit") {
            /* Reset vars on ragdoll exit */
            RuntimeMethods::ResetAll();

            return OG::_ProcessEvent(a_this, a_event, a_eventSource);
        }

        if (a_event->tag == SPPF_SLIDE_STOP) {
            if (RuntimeVariables::SlideOngoing) RuntimeVariables::SlideOngoing = false;

            /* Other POV bugging out shit again, figures why bethesda stopped running both graphs FO4 and onwards */
            actor->NotifyAnimationGraph(SPPF_SLIDE_STOP);

            /* Fix swimstart not triggerring if entered water through crouch slide */
            auto res = OG::_ProcessEvent(a_this, a_event, a_eventSource);

            const auto &ctrl = actor->GetCharController();
            if (ctrl->context.currentState == RE::hkpCharacterStateTypes::kSwimming) actor->NotifyAnimationGraph("SwimStart");

            return res;
        }

        if (a_event->tag == SPPF_SLIDE_START) {
            RuntimeVariables::SlideOngoing = true;

            if (!actor->IsInMidair()) {
                actor->SetGraphVariableInt("iIsInSneak", true);
                actor->AsActorState()->actorState1.sneaking = true;
            }

            return OG::_ProcessEvent(a_this, a_event, a_eventSource);
        }

        if (RuntimeVariables::ParkourInProgress) {
            //LOG(">> AnimEvent: {} Payload: {}", a_event->tag.c_str(), a_event->payload.c_str());

            if (a_event->tag == SPPF_START) {
                constexpr bool Start = false;
                Parkouring::OnStartStop(Start, actor);
            }
            else if (a_event->tag == SPPF_RECOVERY) {
                RuntimeVariables::RecoveryFramesActive = true;

                const bool closeToGround = [actor] {
                    const RE::NiPoint3 start{actor->GetPosition()};
                    constexpr RE::NiPoint3 dir{0, 0, -1};
                    constexpr float dist = 35.0f;
                    constexpr COL_LAYER_EXTEND mask{COL_LAYER_EXTEND::kClimbLedge};

                    return ParkourUtility::RayCast(start, dir, dist, mask).didHit;
                }();

                if (!closeToGround) actor->NotifyAnimationGraph(SPPF_STOP);
            }
            else if (a_event->tag == SPPF_STOP) {
                constexpr bool Stop = true;
                Parkouring::OnStartStop(Stop, actor);
            }
            else if (a_event->tag == SPPF_STAMINA_HIT) {
                /* Steps don't consume stamina anymore */
                const bool isLowEffort = a_event->payload == "LowEffort";
                const bool isSwimming = actor->AsActorState()->IsSwimming();
                Parkouring::PostParkourStaminaDamage(actor, isLowEffort, isSwimming);
            }
        }

        return OG::_ProcessEvent(a_this, a_event, a_eventSource);
    }

#pragma endregion

#pragma region  // NotifyGraph
    bool NotifyGraphHandler::InstallGraphNotifyHook() {
        // TESObjectREFR
        //REL::Relocation<uintptr_t> vtblTES{RE::VTABLE_TESObjectREFR[3]};
        //_origTESObjectREFR = vtblTES.write_vfunc(0x1, OnTESObjectREFR);

        // Character
        //REL::Relocation<uintptr_t> vtblChar{RE::VTABLE_Character[3]};
        //_origCharacter = vtblChar.write_vfunc(0x1, OnCharacter);

        // PlayerCharacter
        REL::Relocation<uintptr_t> vtblPlayer{RE::VTABLE_PlayerCharacter[3]};

        const bool res = Hooking::InstallVFuncHook(vtblPlayer, 0x1, OG::_Notify_PlayerCharacter, &Callback::Notify_PlayerCharacter);
        if (!res) CRITICAL("Notify Hook Not Installed");
        return res;
    }

    bool NotifyGraphHandler::Callback::Notify_TESObjectRefr(RE::IAnimationGraphManagerHolder *a_this,
                                                            const RE::BSFixedString &a_eventName) {
        bool result = OG::_Notify_TESObjectRefr(a_this, a_eventName);

        LOG(">> Object Anim Event: {}", a_eventName.c_str());
        return result;
    }

    bool NotifyGraphHandler::Callback::Notify_Character(RE::IAnimationGraphManagerHolder *a_this, const RE::BSFixedString &a_eventName) {
        bool result = OG::_Notify_Character(a_this, a_eventName);

        LOG(">> Char Anim Event: {}", a_eventName.c_str());
        return result;
    }

    bool NotifyGraphHandler::Callback::Notify_PlayerCharacter(RE::IAnimationGraphManagerHolder *a_this,
                                                              const RE::BSFixedString &a_eventName) {
        if (a_eventName == "sppf_debug") {
            if (API_Handles::TrueHUD::Get()) {
                auto &draw = ModSettings::_Debug_Draw_Lines;
                draw = !draw;
                const char *msg = (std::string("SkyParkour Visual Debugging ") + (draw ? "Enabled" : "Disabled")).c_str();
                LOG("{}", msg);
                RE::ConsoleLog::GetSingleton()->Print(msg);

                return true;
            }
            else {
                WARN("Can't enable debug line drawing, TrueHud handle not found");
                RE::ConsoleLog::GetSingleton()->Print("TrueHUD not found, SkyParkour debugging isn't available");

                return false;
            }
        }

        if (a_eventName == SPPF_STOP && RuntimeVariables::ParkourInProgress) {
            /* If stop event is sent forcibly, flow to correct graph state. */
            const_cast<RE::BSFixedString &>(a_eventName) = SPPF_INTERRUPT;

            return OG::_Notify_PlayerCharacter(a_this, a_eventName);
        }

        if (a_eventName == "Ragdoll") {
            if (RuntimeVariables::ParkourInProgress) {
                /*Unlock controls on ragdoll*/

                bool didRagdoll = OG::_Notify_PlayerCharacter(a_this, a_eventName);
                if (didRagdoll) {
                    constexpr bool Stop = true;
                    RE::Actor *actor = GET_PLAYER;
                    Parkouring::OnStartStop(Stop, actor);
                }
                return didRagdoll;
            }
        }

        if (a_eventName == SPPF_STOP) {
            constexpr bool Start = true;
            RE::Actor *actor = GET_PLAYER;
            Parkouring::OnStartStop(Start, actor);

            return OG::_Notify_PlayerCharacter(a_this, a_eventName);
        }

        if (a_eventName == SPPF_SLIDE_STOP) {
            RuntimeVariables::SlideOngoing = false;
            return OG::_Notify_PlayerCharacter(a_this, a_eventName);
        }

        return OG::_Notify_PlayerCharacter(a_this, a_eventName);
    }

#pragma endregion

}  // namespace Hooks
