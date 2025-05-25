#pragma once
#include "Parkouring.h"
#include "References.h"

namespace ButtonStates {

    extern int32_t DXCODE;

    extern std::unordered_map<int32_t, int32_t> xinputToCKMap;

    extern int32_t MapToCKIfPossible(int32_t dxcode);

    extern void RegisterActivation(RE::InputEvent* event);
}  // namespace ButtonStates

class ButtonEventListener : public RE::BSTEventSink<RE::InputEvent*> {
    public:
        static ButtonEventListener* GetSingleton() {
            static ButtonEventListener singleton;
            return &singleton;
        }

        static void Register();
        static void Unregister();

        bool SinkRegistered;

    private:
        virtual RE::BSEventNotifyControl ProcessEvent(RE::InputEvent* const* a_event, RE::BSTEventSource<RE::InputEvent*>*) override;

        ButtonEventListener() = default;
        ~ButtonEventListener() = default;
};