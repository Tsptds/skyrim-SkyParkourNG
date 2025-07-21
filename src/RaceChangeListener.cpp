#include "RaceChangeListener.h"
#include "ParkourUtility.h"

void RaceChangeListener::Register() {
    auto g_raceChangeSink = RaceChangeListener::GetSingleton();

    if (g_raceChangeSink) {
        RE::ScriptEventSourceHolder::GetSingleton()->GetEventSource<RE::TESSwitchRaceCompleteEvent>()->AddEventSink(g_raceChangeSink);

        //logger::info(">> RaceChange - Listening");
    }
}
void RaceChangeListener::Unregister() {
    auto g_raceChangeSink = RaceChangeListener::GetSingleton();

    if (g_raceChangeSink) {
        RE::ScriptEventSourceHolder::GetSingleton()->GetEventSource<RE::TESSwitchRaceCompleteEvent>()->RemoveEventSink(g_raceChangeSink);

        //logger::info("RaceChange - Not Listening");
    }
}

RE::BSEventNotifyControl RaceChangeListener::ProcessEvent(const RE::TESSwitchRaceCompleteEvent* ev,
                                                          RE::BSTEventSource<RE::TESSwitchRaceCompleteEvent>*) {
    auto actorRef = ev->subject.get();
    if (!actorRef)
        return RE::BSEventNotifyControl::kContinue;

    auto player = RE::PlayerCharacter::GetSingleton();
    if (actorRef->formID != player->formID)
        return RE::BSEventNotifyControl::kContinue;

    const auto playerPreTransformData = player->GetPlayerRuntimeData().preTransformationData;
    if (playerPreTransformData) {
        //logger::info(">> Entering Beast Form");
        Parkouring::SetParkourOnOff(false);
    }
    else {
        //logger::info(">> Exiting Beast Form");
        if (ModSettings::ModEnabled) {
            Parkouring::SetParkourOnOff(true);
        }
    }
    return RE::BSEventNotifyControl::kContinue;
}
