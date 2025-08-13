#include "Listeners/RaceChangeListener.h"
#include "Util/ParkourUtility.h"
#include "Listeners/ButtonListener.h"
#include "_References/ModSettings.h"
#include "Parkouring.h"

void RaceChangeListener::Register() {
    auto g_raceChangeSink = RaceChangeListener::GetSingleton();

    if (g_raceChangeSink) {
        RE::ScriptEventSourceHolder::GetSingleton()->GetEventSource<RE::TESSwitchRaceCompleteEvent>()->AddEventSink(g_raceChangeSink);

        //LOG(">> RaceChange - Listening");
    }
}
void RaceChangeListener::Unregister() {
    auto g_raceChangeSink = RaceChangeListener::GetSingleton();

    if (g_raceChangeSink) {
        RE::ScriptEventSourceHolder::GetSingleton()->GetEventSource<RE::TESSwitchRaceCompleteEvent>()->RemoveEventSink(g_raceChangeSink);

        //LOG("RaceChange - Not Listening");
    }
}

RE::BSEventNotifyControl RaceChangeListener::ProcessEvent(const RE::TESSwitchRaceCompleteEvent* ev,
                                                          RE::BSTEventSource<RE::TESSwitchRaceCompleteEvent>*) {
    auto actorRef = ev->subject.get();
    if (!actorRef)
        return RE::BSEventNotifyControl::kContinue;

    auto player = GET_PLAYER;
    if (actorRef->formID != player->formID)
        return RE::BSEventNotifyControl::kContinue;

    /* On race switch graph vars reset, fix it */
    player->SetGraphVariableFloat(SPPF_SPEEDMULT, ModSettings::Playback_Speed);

    const auto playerPreTransformData = player->GetPlayerRuntimeData().preTransformationData;
    if (playerPreTransformData) {
        //LOG(">> Entering Beast Form");
        Parkouring::SetParkourOnOff(false);
    }
    else {
        //LOG(">> Exiting Beast Form");
        if (ModSettings::Mod_Enabled) {
            Parkouring::SetParkourOnOff(true);
        }
    }
    return RE::BSEventNotifyControl::kContinue;
}
