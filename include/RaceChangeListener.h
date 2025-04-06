#pragma once
#include "ButtonListener.h"
#include "Parkouring.h"

struct RaceChangeListener : RE::BSTEventSink<RE::TESSwitchRaceCompleteEvent> {
    public:
        static RaceChangeListener* GetSingleton() {
            static RaceChangeListener singleton;
            return &singleton;
        }

        static void Register();
        static void Unregister();

    private:
        virtual RE::BSEventNotifyControl ProcessEvent(const RE::TESSwitchRaceCompleteEvent* ev,
                                              RE::BSTEventSource<RE::TESSwitchRaceCompleteEvent>*);

        RaceChangeListener() = default;
        ~RaceChangeListener() = default;
};