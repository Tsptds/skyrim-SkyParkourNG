#pragma once
#include "ParkourUtility.h"
#include "Parkouring.h"
#include "References.h"

class AnimEventListener : public RE::BSTEventSink<RE::BSAnimationGraphEvent> {
    public:
        static AnimEventListener* GetSingleton() {
            static AnimEventListener singleton;
            return &singleton;
        }

        static bool Register();
        static bool Unregister();

    private:
        virtual RE::BSEventNotifyControl ProcessEvent(const RE::BSAnimationGraphEvent* a_event,
                                                      RE::BSTEventSource<RE::BSAnimationGraphEvent>*) override;

        AnimEventListener() = default;
        ~AnimEventListener() = default;

};

// namespace {
//     // This flag ensures we register only once.
//     bool g_registered = false;
//
//     // Define the function type of the original ProcessEvent virtual function.
//     using ProcessEvent_t = RE::BSEventNotifyControl(__fastcall*)(
//         RE::BSTEventSink<RE::BSAnimationGraphEvent>* sink, const RE::BSAnimationGraphEvent* a_event,
//         RE::BSTEventSource<RE::BSAnimationGraphEvent>* a_eventSource);
//
//     ProcessEvent_t originalProcessEvent = nullptr;
// }

//// This is our detour function.
// RE::BSEventNotifyControl __fastcall HookedProcessEvent(RE::BSTEventSink<RE::BSAnimationGraphEvent>* sink,
//                                                        const RE::BSAnimationGraphEvent* a_event,
//                                                        RE::BSTEventSource<RE::BSAnimationGraphEvent>* a_eventSource)
//                                                        {
//     if (!g_registered) {
//         if (auto player = RE::PlayerCharacter::GetSingleton()) {
//             // Create a static instance of your listener
//             static AnimEventListener listener;
//             // Register the listener on the player so that it receives animation events.
//             player->AddAnimationGraphEventSink(&listener);
//         }
//         g_registered = true;
//     }
//     // Call the original function.
//     return originalProcessEvent(sink, a_event, a_eventSource);
// }
//
//// Call this function during plugin initialization to install the hook.
// void InstallAnimationEventHook() {
//     auto& trampoline = SKSE::GetTrampoline();
//     REL::Relocation<uintptr_t> hook{RELOCATION_ID(43030, 44222)};  // 754820, 7821A0
//
//     trampoline.write_call<6>(hook.address() + /*0x318,*/ 0x6FF,
//                              Func183);  // 754B24, 78289F // vfunc call
//     trampoline.write_call<5>(hook.address() + RELOCATION_OFFSET(0x3B8, 0x78A), InitProjectile);
// }