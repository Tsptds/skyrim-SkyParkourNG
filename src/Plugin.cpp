#include "Util/ParkourUtility.h"
#include "Listeners/RaceChangeListener.h"
#include "Listeners/MenuListener.h"
#include "_References/Compatibility.h"
#include "_References/GameReferences.h"
#include "_References/IniSettings.h"
#include "Papyrus/PapyrusInterface.h"
#include "PCH.h"
#include "Hooks/InputHandler.hpp"
#include "Hooks/AnimEventHandler.hpp"
#include "Hooks/CameraHandler.hpp"
#include "API/API_Handles.h"

bool RegisterPapyrusFunctions(RE::BSScript::IVirtualMachine* vm) {
    SkyParkour_Papyrus::Internal::RegisterPapyrusFuncsToVM(vm);
    return true;
}

void Install_Hooks_And_Listeners() {
    RaceChangeListener::Register();
    MenuListener::Register();
    //ButtonEventListener::Register();  // Do it when player loads, unregister inside Menu Listener

    Hooks::InputHandlerEx::InstallInputHooks();

    Hooks::AnimationEventHook::InstallAnimEventHook();
    Hooks::NotifyGraphHandler::InstallGraphNotifyHook();

    Hooks::CameraHandler::InstallCamStateHooks();
}

bool RegisterIndicators() {
    auto ini = IniSettings::GetIniHandle();
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
            ERROR("Indicator refs in INI are corrupt: Blue='{}', Red='{}'", blueStr, redStr);
            WARN("Using Default Indicator Refs");
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
        ERROR("!Indicator Refs Are Null!");
        return false;
    }

    GameReferences::currentIndicatorRef = GameReferences::indicatorRef_Blue;
    LOG("Indicators: Registered");
    return true;
}

void MessageEvent(SKSE::MessagingInterface::Message* message) {
    if (message->type == SKSE::MessagingInterface::kPostPostLoad) {
        RuntimeMethods::SetupModCompatibility();
        
        if (!RuntimeMethods::ReadPluginConfigFromINI()) {
            /* Ini does not exist and failed to create */
            return;
        }
        
        SkyParkour_Papyrus::Internal::Read_All_MCM_From_INI_and_Cache_Settings();
        
        API_Handles::TrueHUD::RequestTrueHUDAPI();
    }
    else if (message->type == SKSE::MessagingInterface::kDataLoaded) {
        if (!RuntimeMethods::CheckESPLoaded()) {
            ERROR("ESP NOT FOUND: |{}|", IniSettings::ESP_NAME);

            /* ESP not loaded by the game, check if game is pre 1.6.1130 */
            auto IsPreExtendedEslVersion = REL::Module::get().version() < SKSE::RUNTIME_SSE_1_6_1130;
            if (IsPreExtendedEslVersion) {
                auto BEES = GetModuleHandleA("BackportedESLSupport.dll");
                if (BEES) {
                    Compatibility::BackportedESLSupport = true;
                    LOG("BEES found on a pre-extended ESL Skyrim version");
                }
                else {
                    ERROR("BEES not installed on a pre-extended ESL Skyrim version");
                    const std::string err = "SkyParkour Warning\n\n" + IniSettings::ESP_NAME + " isn't loaded." +
                                            "\n\n'Backported Extended ESL Support' is required on pre 1.6.1130 Skyrim versions." +
                                            "\n\nMod is disabled.";
                    RE::DebugMessageBox(err.c_str());
                    return;
                }
            }

            const std::string err =
                "SkyParkour Warning\n\n" + IniSettings::ESP_NAME + " is not enabled in your load order." + "\nMod is disabled.";

            RE::DebugMessageBox(err.c_str());
            return;
        }

        RegisterIndicators();
        Install_Hooks_And_Listeners();

        LOG("|>_SkyParkour Loaded_<|");
    }
    else if (message->type == SKSE::MessagingInterface::kPreLoadGame) {
        RuntimeMethods::ResetRuntimeVariables();
    }
    else if (message->type == SKSE::MessagingInterface::kPostLoadGame) {
        auto player = GET_PLAYER;
        int32_t out;
        if (player->GetGraphVariableInt(SPPF_Ledge, out) && out != -1) {
            WARN("Fix: Save with ongoing parkour");
            player->NotifyAnimationGraph(SPPF_STOP);
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
        spdlog::set_pattern(
#ifdef _DEBUG
            PLUGIN_LOGPATTERN_DEBUG
#else
            PLUGIN_LOGPATTERN_RELEASE
#endif
        );
    }
}  // namespace plugin

extern "C" DLLEXPORT bool SKSEPlugin_Load(const LoadInterface* skse) {
    plugin::InitializeLogging();

    Init(skse, false);
    LOG("'{} {}' by {} / Skyrim '{}'", Plugin::Name, Plugin::VersionString, Plugin::Author, REL::Module::get().version().string());

    SKSE::GetPapyrusInterface()->Register(RegisterPapyrusFunctions);
    SKSE::GetMessagingInterface()->RegisterListener(MessageEvent);
    return true;
}