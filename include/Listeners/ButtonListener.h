#pragma once

namespace ButtonStates {

    extern std::unordered_map<uint32_t, uint32_t> xinputToCKMap;

    extern uint32_t MapToCKIfPossible(uint32_t dxcode);

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