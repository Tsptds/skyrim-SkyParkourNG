#pragma once

#include "References.h"
#include "Parkouring.h"

struct MenuListener : public RE::BSTEventSink<RE::MenuOpenCloseEvent> {
    public:
        static MenuListener* GetSingleton() {
            static MenuListener instance;
            return &instance;
        }

        static bool Register();
        static bool Unregister();

    private:
        // called on every open/close
        virtual RE::BSEventNotifyControl ProcessEvent(const RE::MenuOpenCloseEvent* ev,
                                                      RE::BSTEventSource<RE::MenuOpenCloseEvent>*) override;

        MenuListener() = default;
        ~MenuListener() = default;
};
