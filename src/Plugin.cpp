#include "ParkourUtility.h"
#include "Parkouring.h"
#include "ButtonListener.h"
#include "RaceChangeListener.h"
#include "AnimationListener.h"
#include "References.h"
#include "PCH.h"

using namespace ParkourUtility;
using namespace Parkouring;

void RegisterCustomParkourKey(RE::StaticFunctionTag *, int32_t dxcode) {
    ButtonStates::DXCODE = dxcode;
    logger::info("-Custom Key Set: {}", dxcode);
}

void RegisterPresetParkourKey(RE::StaticFunctionTag *, int32_t presetKey) {
    ModSettings::PresetParkourKey = presetKey;
    logger::info("-Preset Key Set: {}", ModSettings::PresetParkourKey);

    // Reset jump flag to prevent getting stuck if jump is disabled and mod setting is changed
    // When Button delay is 0, jump is disabled on ledge point detection
    if (!RuntimeVariables::ParkourEndQueued) {
        RE::ControlMap::GetSingleton()->ToggleControls(RE::ControlMap::UEFlag::kJumping, true);
    }
}

void RegisterParkourDelay(RE::StaticFunctionTag *, float delay) {
    ModSettings::parkourDelay = delay;
    logger::info("-Delay Set {}", ModSettings::parkourDelay);

    // Reset jump flag to prevent getting stuck if jump is disabled and mod setting is changed
    // When Button delay is 0, jump is disabled on ledge point detection
    if (!RuntimeVariables::ParkourEndQueued) {
        RE::ControlMap::GetSingleton()->ToggleControls(RE::ControlMap::UEFlag::kJumping, true);
    }
}

void RegisterStaminaDamage(RE::StaticFunctionTag *, bool enabled, bool staminaBlocks, float damage) {
    ModSettings::Enable_Stamina_Consumption = enabled;
    ModSettings::Is_Stamina_Required = staminaBlocks;
    ModSettings::Stamina_Damage = damage;
    logger::info("**Stamina System**\n-Enabled:{} -Required:{} -Value:{}", ModSettings::Enable_Stamina_Consumption,
                 ModSettings::Is_Stamina_Required, ModSettings::Stamina_Damage);
}

void RegisterParkourSettings(RE::StaticFunctionTag *, bool _usePresetKey, bool _enableMod, bool _smartParkour) {
    ModSettings::UsePresetParkourKey = _usePresetKey;
    ModSettings::ModEnabled = _enableMod;
    ModSettings::Smart_Parkour_Enabled = _smartParkour;
}

void RegisterReferences(RE::StaticFunctionTag *, RE::TESObjectREFR *indicatorRef_Blue, RE::TESObjectREFR *indicatorRef_Red) {
    if (!indicatorRef_Blue || !indicatorRef_Red) {
        logger::error("!Indicator Refs Are Null!");
    }

    GameReferences::indicatorRef_Blue = indicatorRef_Blue;
    GameReferences::indicatorRef_Red = indicatorRef_Red;

    GameReferences::currentIndicatorRef = indicatorRef_Blue;
}

void Register(RE::StaticFunctionTag *) {
    AnimEventListener::Register();
}

bool PapyrusFunctions(RE::BSScript::IVirtualMachine *vm) {
    vm->RegisterFunction("RegisterParkourSettings", "SkyParkourPapyrus", RegisterParkourSettings);

    vm->RegisterFunction("RegisterCustomParkourKey", "SkyParkourPapyrus", RegisterCustomParkourKey);

    vm->RegisterFunction("RegisterPresetParkourKey", "SkyParkourPapyrus", RegisterPresetParkourKey);

    vm->RegisterFunction("RegisterParkourDelay", "SkyParkourPapyrus", RegisterParkourDelay);

    vm->RegisterFunction("RegisterStaminaDamage", "SkyParkourPapyrus", RegisterStaminaDamage);

    vm->RegisterFunction("RegisterReferences", "SkyParkourPapyrus", RegisterReferences);

    return true;
}

void MessageEvent(SKSE::MessagingInterface::Message *message) {
    if (message->type == SKSE::MessagingInterface::kDataLoaded) {
        //JumpHandlerEx::InstallHook();
        RaceChangeListener::Register();
        ButtonEventListener::Register();
        logger::info("Done");

    } else if (message->type == SKSE::MessagingInterface::kPreLoadGame) {
        // Parkour Point updates with button listener, remove it when player unloads to prevent crash
        ButtonEventListener::Unregister();

        RuntimeVariables::ParkourEndQueued = false;

    } else if (message->type == SKSE::MessagingInterface::kPostLoadGame) {
        ButtonEventListener::Register();
        ParkourUtility::ToggleControlsForParkour(true);

    } else if (message->type == SKSE::MessagingInterface::kNewGame) {
        ButtonEventListener::Register();

        ParkourUtility::ToggleControlsForParkour(true);
        RuntimeVariables::ParkourEndQueued = false;
    }
}

using namespace SKSE;
using namespace SKSE::log;
using namespace SKSE::stl;

#include "Plugin.h"
#include "GameEventHandler.h"

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
            } else {
                directory.append("Skyrim Special Edition"sv);
            }
        } else if (exists("Galaxy64.dll"sv)) {
            directory.append("Skyrim Special Edition GOG"sv);
        } else if (exists("eossdk-win64-shipping.dll"sv)) {
            directory.append("Skyrim Special Edition EPIC"sv);
        } else {
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
        } else {
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
    //GameEventHandler::getInstance().onLoad();
    //logger::info("{} has finished loading.", Plugin::Name);
    return true;
}