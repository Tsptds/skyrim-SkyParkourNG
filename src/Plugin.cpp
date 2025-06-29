#include "ParkourUtility.h"
#include "Parkouring.h"
#include "ButtonListener.h"
#include "RaceChangeListener.h"
#include "References.h"
#include "PCH.h"

#include "InputHandler.hpp"
#include "AnimEventHandler.hpp"

using namespace ParkourUtility;
using namespace Parkouring;

void RegisterCustomParkourKey(RE::StaticFunctionTag *, int32_t dxcode) {
    ButtonStates::DXCODE = dxcode;
    logger::info(">Custom Key: '{}'", dxcode);
}

void RegisterPresetParkourKey(RE::StaticFunctionTag *, int32_t presetKey) {
    ModSettings::PresetParkourKey = presetKey;
    logger::info(">Preset Key: '{}'", ModSettings::PresetParkourKey);
}

void RegisterParkourDelay(RE::StaticFunctionTag *, float delay) {
    ModSettings::parkourDelay = delay;
    logger::info(">Delay '{}'", ModSettings::parkourDelay);
}

void RegisterStaminaDamage(RE::StaticFunctionTag *, bool enabled, bool staminaBlocks, float damage) {
    ModSettings::Enable_Stamina_Consumption = enabled;
    ModSettings::Is_Stamina_Required = staminaBlocks;
    ModSettings::Stamina_Damage = damage;
    logger::info("|Stamina|> On:'{}' >Must:'{}' >Dmg:'{}'", ModSettings::Enable_Stamina_Consumption, ModSettings::Is_Stamina_Required,
                 ModSettings::Stamina_Damage);
}

void RegisterParkourSettings(RE::StaticFunctionTag *, bool _usePresetKey, bool _enableMod, bool _smartParkour, bool _useIndicators) {
    ModSettings::UsePresetParkourKey = _usePresetKey;
    ModSettings::Smart_Parkour_Enabled = _smartParkour;
    ModSettings::UseIndicators = _useIndicators;

    ModSettings::ModEnabled = _enableMod;

    // Turn on if setting is on and is not beast form. Same logic on race change listener.
    Parkouring::SetParkourOnOff(ModSettings::ModEnabled && !ParkourUtility::IsBeastForm());
}

bool PapyrusFunctions(RE::BSScript::IVirtualMachine *vm) {
    vm->RegisterFunction("RegisterParkourSettings", "SkyParkourPapyrus", RegisterParkourSettings);

    vm->RegisterFunction("RegisterCustomParkourKey", "SkyParkourPapyrus", RegisterCustomParkourKey);

    vm->RegisterFunction("RegisterPresetParkourKey", "SkyParkourPapyrus", RegisterPresetParkourKey);

    vm->RegisterFunction("RegisterParkourDelay", "SkyParkourPapyrus", RegisterParkourDelay);

    vm->RegisterFunction("RegisterStaminaDamage", "SkyParkourPapyrus", RegisterStaminaDamage);

    return true;
}

void Install_Hooks_And_Listeners() {
    RaceChangeListener::Register();
    MenuListener::Register();
    //ButtonEventListener::Register();  // Do it inside Menu Listener, when main menu closes

    Hooks::InputHandlerEx<RE::JumpHandler>::InstallJumpHook();
    Hooks::InputHandlerEx<RE::JumpHandler>::InstallProcessJumpHook();
    Hooks::InputHandlerEx<RE::SneakHandler>::InstallSneakHook();
    Hooks::AnimationEventHook<RE::BSAnimationGraphManager>::InstallAnimEventHook();
    Hooks::NotifyGraphHandler::InstallGraphNotifyHook();
}

bool RegisterIndicators() {
    GameReferences::indicatorRef_Blue =
        RE::NiPointer(RE::TESDataHandler::GetSingleton()->LookupForm<RE::TESObjectREFR>(0x000014, IniSettings::ESP_NAME));
    GameReferences::indicatorRef_Red =
        RE::NiPointer(RE::TESDataHandler::GetSingleton()->LookupForm<RE::TESObjectREFR>(0x00000C, IniSettings::ESP_NAME));

    if (!GameReferences::indicatorRef_Blue || !GameReferences::indicatorRef_Red) {
        logger::error("!Indicator Refs Are Null!");
        return false;
    }

    GameReferences::currentIndicatorRef = GameReferences::indicatorRef_Blue;
    return true;
}

void MessageEvent(SKSE::MessagingInterface::Message *message) {
    if (message->type == SKSE::MessagingInterface::kPostPostLoad) {
        RuntimeMethods::ReadIni();

        if (IniSettings::IgnoreRequirements) {
            logger::warn(">>> Requirement Checks Skipped <<<");
            return;
        }
        //RuntimeMethods::CheckRequirements();
    }
    else if (message->type == SKSE::MessagingInterface::kDataLoaded) {
        // Check for ESP
        if (!RuntimeMethods::CheckESPLoaded()) {
            logger::error("ESP NOT FOUND: |{}|", IniSettings::ESP_NAME);
            std::string err = "SkyParkour Warning\n\n" + IniSettings::ESP_NAME + " is not enabled in your load order. Mod will not work.";

            RE::DebugMessageBox(err.c_str());
            logger::info(">> SkyParkour *failed* to load <<");
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
            ParkourUtility::ToggleControlsForParkour(true);
            player->NotifyAnimationGraph("JumpLandEnd");
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

    void initializeLogging() {
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

using namespace plugin;

extern "C" DLLEXPORT bool SKSEPlugin_Load(const LoadInterface *skse) {
    initializeLogging();

    Init(skse, false);
    logger::info("'{} {}' / Skyrim '{}'", Plugin::Name, Plugin::VersionString, REL::Module::get().version().string());

    SKSE::GetPapyrusInterface()->Register(PapyrusFunctions);
    SKSE::GetMessagingInterface()->RegisterListener(MessageEvent);
    return true;
}