#include "RaceChangeListener.h"

void RaceChangeListener::Register() {
    auto g_raceChangeSink = RaceChangeListener::GetSingleton();

    if (g_raceChangeSink) {
        RE::ScriptEventSourceHolder::GetSingleton()->GetEventSource<RE::TESSwitchRaceCompleteEvent>()->AddEventSink(g_raceChangeSink);

        logger::info("RaceChange - Listening");
    }
}
void RaceChangeListener::Unregister() {
    auto g_raceChangeSink = RaceChangeListener::GetSingleton();

    if (g_raceChangeSink) {
        RE::ScriptEventSourceHolder::GetSingleton()->GetEventSource<RE::TESSwitchRaceCompleteEvent>()->RemoveEventSink(
            g_raceChangeSink);  // RE::ScriptEventSourceHolder::GetEventSource<T> :contentReference[oaicite:0]{index=0}

        logger::info("RaceChange - Not Listening");
    }
}

RE::BSEventNotifyControl RaceChangeListener::ProcessEvent(const RE::TESSwitchRaceCompleteEvent* ev,
                                                          RE::BSTEventSource<RE::TESSwitchRaceCompleteEvent>*) {
    auto actorRef = ev->subject.get();
    if (!actorRef)
        return RE::BSEventNotifyControl::kContinue;

    // is it the player?
    auto player = RE::PlayerCharacter::GetSingleton();
    if (actorRef->formID != player->formID)
        return RE::BSEventNotifyControl::kContinue;

    // it *is* the player, if it has pre transformation data, then it is a beast race. Unregister button listener to stop parkour
    const auto playerPreTransformData = player->GetPlayerRuntimeData().preTransformationData;
    if (playerPreTransformData) {
        RuntimeVariables::IsBeastForm = true;
        ParkourUtility::ToggleControlsForParkour(true);
        RuntimeVariables::ParkourEndQueued = false;

        //logger::info(">> Entering Beast Form");

    } else {
        if (ModSettings::ModEnabled) {
            Parkouring::SetParkourOnOff(true);
        }

        RuntimeVariables::IsBeastForm = false;

        //logger::info(">> Exiting Beast Form");
    }
    return RE::BSEventNotifyControl::kContinue;
}
