#pragma once
#include "Parkouring.h"
#include "References.h"

namespace ButtonStates {

    extern int32_t DXCODE;

    extern std::unordered_map<int32_t, int32_t> xinputToCKMap;

    extern int32_t MapToCKIfPossible(int32_t dxcode);

    extern void RegisterActivation(RE::ButtonEvent* event);
}

class ButtonEventListener : public RE::BSTEventSink<RE::InputEvent*> {
public:
    static ButtonEventListener* GetSingleton() {
        static ButtonEventListener singleton;
        return &singleton;
    }

    static void Register();
    static void Unregister();


private:
    virtual RE::BSEventNotifyControl ProcessEvent(RE::InputEvent* const* a_event,
                                          RE::BSTEventSource<RE::InputEvent*>*);

    ButtonEventListener() = default;
    ~ButtonEventListener() = default;
};

//class JumpHandlerEx : public RE::JumpHandler {
//public:
//    using ProcessButton_t = decltype(&RE::JumpHandler::ProcessButton);
//    static inline REL::Relocation<ProcessButton_t> _ProcessButton;
//
//    void ProcessButton(RE::ButtonEvent* a_event, RE::PlayerControlsData* a_data) override {
//        
//            
//        
//        // Custom logic before the original function
//        logger::info("Jump button pressed");
//        // Call the original function
//        _ProcessButton(this, a_event, a_data);
//        
//
//
//        // Custom logic after the original function
//    }
//
//    static void InstallHook() {
//        REL::Relocation<std::uintptr_t> vtable{RE::VTABLE_JumpHandler[0]};
//        _ProcessButton = vtable.write_vfunc(0x4, &JumpHandlerEx::ProcessButton);
//        logger::info("Jump Hook Installed");
//
//        logger::info("Original ProcessButton address: {}", reinterpret_cast<void*>(_ProcessButton.address()));
//    }
//};