#include "ParkourUtility.h"
#include "RaceChangeListener.h"
#include "References.h"
#include "PapyrusInterface.h"
#include "PCH.h"
#include "InputHandler.hpp"
#include "AnimEventHandler.hpp"

using namespace ParkourUtility;
using namespace Parkouring;

bool RegisterPapyrusFunctions(RE::BSScript::IVirtualMachine* vm) {
    SkyParkour_Papyrus::Internal::RegisterPapyrusFuncsToVM(vm);
    return true;
}

void Install_Hooks_And_Listeners() {
    RaceChangeListener::Register();
    MenuListener::Register();
    //ButtonEventListener::Register();  // Do it inside Menu Listener, when main menu closes

    Hooks::InputHandlerEx<RE::JumpHandler>::InstallJumpHook();
    Hooks::InputHandlerEx<RE::JumpHandler>::InstallProcessJumpHook();
    Hooks::InputHandlerEx<RE::SneakHandler>::InstallSneakHook();
    Hooks::InputHandlerEx<RE::MovementHandler>::InstallMovementHook();
    Hooks::AnimationEventHook<RE::BSAnimationGraphManager>::InstallAnimEventHook();
    Hooks::NotifyGraphHandler::InstallGraphNotifyHook();
}

bool RegisterIndicators() {
    auto ini = RuntimeMethods::GetIniHandle();
    int blueForm = 0x000014;
    int redForm = 0x00000C;

    if (ini) {
        const char* blueStr = ini->GetValue("ESP", "iBlueMarkerRefID");
        const char* redStr = ini->GetValue("ESP", "iRedMarkerRefID");

        char* endBlue = nullptr;
        char* endRed = nullptr;

        const int blueParse = std::strtol(blueStr, &endBlue, 16);
        const int redParse = std::strtol(redStr, &endRed, 16);

        if (!blueStr || endBlue == blueStr || *endBlue != '\0' || !redStr || endRed == redStr || *endRed != '\0' || !blueParse ||
            !redParse) {
            logger::error("Indicator refs in INI are corrupt: Blue='{}', Red='{}'", blueStr, redStr);
            logger::warn("Using Default Indicator Refs");
        }
        else {
            blueForm = blueParse;
            redForm = redParse;
        }
    }

    GameReferences::indicatorRef_Blue =
        RE::NiPointer(RE::TESDataHandler::GetSingleton()->LookupForm<RE::TESObjectREFR>(blueForm, IniSettings::ESP_NAME));
    GameReferences::indicatorRef_Red =
        RE::NiPointer(RE::TESDataHandler::GetSingleton()->LookupForm<RE::TESObjectREFR>(redForm, IniSettings::ESP_NAME));

    if (!GameReferences::indicatorRef_Blue || !GameReferences::indicatorRef_Red) {
        logger::error("!Indicator Refs Are Null!");
        return false;
    }

    GameReferences::currentIndicatorRef = GameReferences::indicatorRef_Blue;
    logger::info("Successfully Registered Indicators");
    return true;
}

void MessageEvent(SKSE::MessagingInterface::Message* message) {
    if (message->type == SKSE::MessagingInterface::kPostPostLoad) {
        RuntimeMethods::ReadPluginConfigFromINI();

        if (IniSettings::IgnoreRequirements) {
            logger::warn(">>> Requirement Checks Skipped <<<");
            return;
        }

        SkyParkour_Papyrus::Internal::Read_All_MCM_From_INI_and_Cache_Settings();
        //RuntimeMethods::CheckRequirements();
    }
    else if (message->type == SKSE::MessagingInterface::kDataLoaded) {
        // Check for ESP
        if (!RuntimeMethods::CheckESPLoaded()) {
            logger::error("ESP NOT FOUND: |{}|", IniSettings::ESP_NAME);
            std::string err = "SkyParkour Warning\n\n" + IniSettings::ESP_NAME + " is not enabled in your load order. Mod will not work.";

            RE::DebugMessageBox(err.c_str());
            return;
        }

        RegisterIndicators();
        Install_Hooks_And_Listeners();
        RuntimeMethods::SetupModCompatibility();

        logger::info(">> SkyParkour Loaded <<");
    }
    else if (message->type == SKSE::MessagingInterface::kPreLoadGame) {
        RuntimeMethods::ResetRuntimeVariables();
    }
    else if (message->type == SKSE::MessagingInterface::kPostLoadGame) {
        auto player = RE::PlayerCharacter::GetSingleton();
        int32_t out;
        if (player->GetGraphVariableInt("SkyParkourLedge", out) && out && out != -1) {
            logger::warn(">>SAVE LOADED WITH ONGOING PARKOUR, FIXING IT<<");
            player->NotifyAnimationGraph("SkyParkour_Stop");
        }

        RuntimeMethods::ResetRuntimeVariables();
    }
    else if (message->type == SKSE::MessagingInterface::kNewGame) {
        RuntimeMethods::ResetRuntimeVariables();
    }
}

using namespace SKSE;
using namespace SKSE::log;
using namespace SKSE::stl;

#include "Plugin.h"

namespace plugin {
    std::optional<std::filesystem::path> getLogDirectory() {
        using namespace std::filesystem;
        PWSTR buf;
        SHGetKnownFolderPath(FOLDERID_Documents, KF_FLAG_DEFAULT, nullptr, &buf);
        std::unique_ptr<wchar_t, decltype(&CoTaskMemFree)> documentsPath{buf, CoTaskMemFree};
        path directory{documentsPath.get()};
        directory.append("My Games"sv);

        if (exists("steam_api64.dll"sv)) {
            if (exists("openvr_api.dll") || exists("Data/SkyrimVR.esm")) {
                directory.append("Skyrim VR"sv);
            }
            else {
                directory.append("Skyrim Special Edition"sv);
            }
        }
        else if (exists("Galaxy64.dll"sv)) {
            directory.append("Skyrim Special Edition GOG"sv);
        }
        else if (exists("eossdk-win64-shipping.dll"sv)) {
            directory.append("Skyrim Special Edition EPIC"sv);
        }
        else {
            return current_path().append("skselogs");
        }
        return directory.append("SKSE"sv).make_preferred();
    }

    static void InitializeLogging() {
        auto path = getLogDirectory();
        if (!path) {
            report_and_fail("Can't find SKSE log directory");
        }
        *path /= std::format("{}.log"sv, Plugin::Name);

        std::shared_ptr<spdlog::logger> log;
        if (IsDebuggerPresent()) {
            log = std::make_shared<spdlog::logger>("Global", std::make_shared<spdlog::sinks::msvc_sink_mt>());
        }
        else {
            log = std::make_shared<spdlog::logger>("Global", std::make_shared<spdlog::sinks::basic_file_sink_mt>(path->string(), true));
        }
        log->set_level(spdlog::level::info);
        log->flush_on(spdlog::level::info);

        spdlog::set_default_logger(std::move(log));
        spdlog::set_pattern(PLUGIN_LOGPATTERN_RELEASE);
    }
}  // namespace plugin

extern "C" DLLEXPORT bool SKSEPlugin_Load(const LoadInterface* skse) {
    plugin::InitializeLogging();

    Init(skse, false);
    logger::info("'{} {}' / Skyrim '{}'", Plugin::Name, Plugin::VersionString, REL::Module::get().version().string());

    SKSE::GetPapyrusInterface()->Register(RegisterPapyrusFunctions);
    SKSE::GetMessagingInterface()->RegisterListener(MessageEvent);
    return true;
}