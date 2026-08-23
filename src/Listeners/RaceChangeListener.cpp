#include "Listeners/RaceChangeListener.h"
#include "Util/ParkourUtility.h"
#include "Listeners/ButtonListener.h"
#include "ModSettings/ModSettings.hpp"
#include "Parkouring.h"
#include "CrouchSliding.h"
#include "Util/HavokUtil.hpp"
#include "_References/RuntimeMethods.h"

void RaceChangeListener::Register()
{
    auto g_raceChangeSink = RaceChangeListener::GetSingleton();

    if (g_raceChangeSink)
    {
        RE::ScriptEventSourceHolder::GetSingleton()->GetEventSource<RE::TESSwitchRaceCompleteEvent>()->AddEventSink(g_raceChangeSink);

        //INFO(">> RaceChange - Listening");
    }
}
void RaceChangeListener::Unregister()
{
    auto g_raceChangeSink = RaceChangeListener::GetSingleton();

    if (g_raceChangeSink)
    {
        RE::ScriptEventSourceHolder::GetSingleton()->GetEventSource<RE::TESSwitchRaceCompleteEvent>()->RemoveEventSink(g_raceChangeSink);

        //INFO("RaceChange - Not Listening");
    }
}

RE::BSEventNotifyControl RaceChangeListener::ProcessEvent(const RE::TESSwitchRaceCompleteEvent *ev,
                                                          RE::BSTEventSource<RE::TESSwitchRaceCompleteEvent> *)
{
    auto actorRef = ev->subject.get();
    if (!actorRef) return RE::BSEventNotifyControl::kContinue;

    if (!actorRef->IsPlayerRef()) return RE::BSEventNotifyControl::kContinue;
    const auto pl = GET_PLAYER;

    /* On race switch graph vars reset, fix it */
    pl->SetGraphVariableFloat(SPPF_SPEEDMULT, ModSettings::Playback_Speed);

    const auto playerPreTransformData = pl->GetPlayerRuntimeData().preTransformationData;
    if (playerPreTransformData)
    {  // Entered beast form

        Parkouring::SetParkourOnOff(false);
        CrouchSliding::SetSlideOnOff(false);
    }
    else
    {  // Changed race but it's not a beast form, reset stuff
        //INFO(">> Exiting Beast Form");
        RuntimeMethods::ResetAll();
        if (ModSettings::Parkour_Enabled) Parkouring::SetParkourOnOff(true);
        if (ModSettings::Crouch_Slide_Enabled || ModSettings::Land_Rolling_Enabled) CrouchSliding::SetSlideOnOff(true);
        RuntimeMethods::ResetAll();
        HavokUtil::CreateBoundGraphChannels(GET_PLAYER);
    }
    return RE::BSEventNotifyControl::kContinue;
}
