#pragma once
#include "ModSettings/ModSettings.hpp"
#include "_References/RuntimeMethods.h"
#include "_References/RuntimeVariables.h"

#include "Parkouring.h"
#include "CrouchSliding.h"
#include "Util/ParkourUtility.h"
#include "Util/HookingUtil.hpp"
#include "Util/HavokUtil.hpp"
#include "API/API_Handles.h"

#include "HUD/Scaleform/SkyParkourMenu.hpp"

namespace Hooks
{

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
    bool AnimationEventHook::InstallAnimEventHook()
    {
        // This is the anim event hook, global event sink for everyone. Event will go regardless. Don't return anything in this except the OG func.
        // Sink gets destroyed when graph deletes, so using this
        REL::Relocation<std::uintptr_t> vtbl{RE::VTABLE_BSAnimationGraphManager[0]};

        const bool res = Hooking::InstallVFuncHook(vtbl, 0x1, OG::_ProcessEvent, &Callback::ProcessEvent);
        if (!res) CRITICAL("AnimEvent Hook Not Installed");
        return res;
    }

    RE::BSEventNotifyControl AnimationEventHook::Callback::ProcessEvent(RE::BSAnimationGraphManager *a_this,
                                                                        const RE::BSAnimationGraphEvent *a_event,
                                                                        RE::BSTEventSource<RE::BSAnimationGraphEvent> *a_eventSource)
    {
        if (!a_event) return OG::_ProcessEvent(a_this, a_event, a_eventSource);
        if (!ModSettings::Parkour_Enabled && !ModSettings::Crouch_Slide_Enabled) return OG::_ProcessEvent(a_this, a_event, a_eventSource);

        const auto actor = a_this->graphs[a_this->GetRuntimeData().activeGraph]->holder;

        if (!actor) return OG::_ProcessEvent(a_this, a_event, a_eventSource);
        if (!actor->IsPlayerRef()) return OG::_ProcessEvent(a_this, a_event, a_eventSource);

        if (a_event->tag == "GetUpExit")
        {
            /* Reset vars on ragdoll exit */
            RuntimeMethods::ResetAll();

            return OG::_ProcessEvent(a_this, a_event, a_eventSource);
        }

        if (RuntimeVariables::SlideOngoing)
        {
            if (a_event->tag == SPPF_SLIDE_STOP)
            {
                const bool isRoll = a_event->payload == SPPF_ROLLPAYLOAD;

                constexpr bool is_stop = true;
                CrouchSliding::OnStartStop(is_stop, actor, isRoll);

                /* Fix swimstart not triggerring if entered water through crouch slide */
                auto res = OG::_ProcessEvent(a_this, a_event, a_eventSource);

                const auto ctrl = actor->GetCharController();
                if (ctrl->context.currentState == RE::hkpCharacterStateType::kSwimming)
                {
                    actor->NotifyAnimationGraph("SwimStart");
                }

                return res;
            }

            else if (a_event->tag == SPPF_SLIDE_START)
            {
                if (a_event->payload == SPPF_SLIDEPAYLOAD)
                {
                    constexpr bool is_start = false;
                    constexpr bool isRoll = false;
                    CrouchSliding::OnStartStop(is_start, actor, isRoll);
                }
                // else if (a_event->payload == "LandRoll") {}
            }

            else if (a_event->tag == SPPF_RECOVERY)
            {
                RuntimeVariables::RecoveryFramesActive = true;
            }

            else if (a_event->tag == SPPF_STAMINA_HIT)
            {
                bool isLowEffort = a_event->payload == SPPF_LOWEFFORTPAYLOAD;
                constexpr bool isSwimming = false;
                Parkouring::PostParkourStaminaDamage(actor, isLowEffort, isSwimming);

                /* Reduce fall damage by decreasing fall start height */
                auto &fst = actor->GetCharController()->fallStartHeight;
                if (fst - actor->GetPositionZ() > 200) fst -= 100;
            }

            return OG::_ProcessEvent(a_this, a_event, a_eventSource);
        }

        if (RuntimeVariables::ParkourInProgress)
        {
            //INFO(">> AnimEvent: {} Payload: {}", a_event->tag.c_str(), a_event->payload.c_str());

            if (a_event->tag == SPPF_START)
            {
                constexpr bool Start = false;
                Parkouring::OnStartStop(Start, actor);
            }
            else if (a_event->tag == SPPF_RECOVERY)
            {
                RuntimeVariables::RecoveryFramesActive = true;

                const bool closeToGround = [actor] {
                    const RE::NiPoint3 start{actor->GetPosition()};
                    constexpr RE::NiPoint3 dir{0, 0, -1};
                    constexpr float dist = 35.0f;
                    constexpr COL_LAYER_EXTEND mask{COL_LAYER_EXTEND::kClimbLedge};

                    return HavokUtil::RayCast(start, dir, dist, mask).didHit;
                }();

                if (!closeToGround) actor->NotifyAnimationGraph(SPPF_STOP);
            }
            else if (a_event->tag == SPPF_STOP)
            {
                constexpr bool Stop = true;
                Parkouring::OnStartStop(Stop, actor);
            }
            else if (a_event->tag == SPPF_STAMINA_HIT)
            {
                /* Steps don't consume stamina anymore */
                const bool isLowEffort = a_event->payload == SPPF_LOWEFFORTPAYLOAD;
                const bool isSwimming = actor->AsActorState()->IsSwimming();
                Parkouring::PostParkourStaminaDamage(actor, isLowEffort, isSwimming);
            }
            return OG::_ProcessEvent(a_this, a_event, a_eventSource);
        }

        return OG::_ProcessEvent(a_this, a_event, a_eventSource);
    }

#pragma endregion

#pragma region  // NotifyGraph
    bool NotifyGraphHandler::InstallGraphNotifyHook()
    {
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

    bool NotifyGraphHandler::Callback::Notify_TESObjectRefr(RE::IAnimationGraphManagerHolder *a_this, const RE::BSFixedString &a_eventName)
    {
        bool result = OG::_Notify_TESObjectRefr(a_this, a_eventName);

        INFO(">> Object Anim Event: {}", a_eventName.c_str());
        return result;
    }

    bool NotifyGraphHandler::Callback::Notify_Character(RE::IAnimationGraphManagerHolder *a_this, const RE::BSFixedString &a_eventName)
    {
        bool result = OG::_Notify_Character(a_this, a_eventName);

        INFO(">> Char Anim Event: {}", a_eventName.c_str());
        return result;
    }

    bool NotifyGraphHandler::Callback::Notify_PlayerCharacter(RE::IAnimationGraphManagerHolder *a_this,
                                                              const RE::BSFixedString &a_eventName)
    {
        if (a_eventName == SPPF_STOP && RuntimeVariables::ParkourInProgress)
        {
            // Pass the interrupt event instead of modifying the parameter directly
            return OG::_Notify_PlayerCharacter(a_this, SPPF_INTERRUPT);
        }

        if (a_eventName == "Ragdoll")
        {
            /*Unlock controls on ragdoll*/
            bool didRagdoll = OG::_Notify_PlayerCharacter(a_this, a_eventName);
            if (didRagdoll)
            {
                constexpr bool Stop = true;
                RE::Actor *actor = GET_PLAYER;
                if (RuntimeVariables::ParkourInProgress)
                {
                    Parkouring::OnStartStop(Stop, actor);
                }
                else if (RuntimeVariables::SlideOngoing)
                {
                    CrouchSliding::OnStartStop(Stop, actor, false);
                }
                return didRagdoll;
            }
        }

        if (a_eventName == SPPF_STOP)
        {
            constexpr bool Start = true;
            RE::Actor *actor = GET_PLAYER;
            Parkouring::OnStartStop(Start, actor);

            return OG::_Notify_PlayerCharacter(a_this, a_eventName);
        }

        if (a_eventName == SPPF_SLIDE_STOP)
        {
            RuntimeVariables::SlideOngoing = false;
            // GET_PLAYER->GetCharController()->flags.reset(RE::CHARACTER_FLAGS::kNoFriction);
            return OG::_Notify_PlayerCharacter(a_this, a_eventName);
        }

        if (a_eventName == "SneakStart" && RuntimeVariables::SlideOngoing && RuntimeVariables::RecoveryFramesActive)
        {
            bool res = OG::_Notify_PlayerCharacter(a_this, a_eventName);

            if (res)
            {
                RE::Actor *actor = GET_PLAYER;
                actor->AsActorState()->actorState1.sneaking = true;
                actor->SetGraphVariableInt("iIsInSneak", 1);
            }
            return res;
        }

        return OG::_Notify_PlayerCharacter(a_this, a_eventName);
    }

#pragma endregion

}  // namespace Hooks
