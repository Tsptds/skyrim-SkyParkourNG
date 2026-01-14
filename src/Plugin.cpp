#include "Util/ParkourUtility.h"
#include "Listeners/RaceChangeListener.h"
#include "Listeners/MenuListener.h"
#include "_References/Compatibility.h"
#include "_References/IniSettings.h"
#include "Papyrus/PapyrusInterface.h"
#include "PCH.h"
#include "Hooks/InputHandler.hpp"
#include "Hooks/AnimEventHandler.hpp"
#include "Hooks/CameraHandler.hpp"
// #include "Hooks/HavokHandler.hpp"
#include "API/API_Handles.h"
#include "HUD/Scaleform/SkyParkourMenu.hpp"

bool RegisterPapyrusFunctions(RE::BSScript::IVirtualMachine *vm) {
    SkyParkour_Papyrus::Internal::RegisterPapyrusFuncsToVM(vm);
    return true;
}
void HandleMissingESPWarning() {
    ERROR("ESP NOT FOUND: |{}|", IniSettings::ESP_NAME);

    std::string err;

    /* ESP not loaded by the game, check if game is pre 1.6.1130 */
    const auto IsPreExtendedEslVersion = REL::Module::get().version() < SKSE::RUNTIME_SSE_1_6_1130;
    if (IsPreExtendedEslVersion) {
        const auto BEES = GetModuleHandleA("BackportedESLSupport.dll");

        if (BEES) {
            Compatibility::BackportedESLSupport = true;
            LOG("BEES found on a pre-extended ESL Skyrim version");

            err = "SkyParkour Warning\n\n" + IniSettings::ESP_NAME + " isn't loaded by Skyrim." +
                  "\n\n'Backported Extended ESL Support' is installed on your pre 1.6.1130 Skyrim version, so it's not the problem." +
                  "\n\nMake sure SkyParkour ESP is enabled in your load order." + "\nMod will not function properly.";
        }
        else {
            ERROR("BEES not installed on a pre-extended ESL Skyrim version");

            err = "SkyParkour Warning\n\n" + IniSettings::ESP_NAME + " isn't loaded by Skyrim." +
                  "\n\nThis could be caused by not having 'Backported Extended ESL Support' installed." +
                  "\n\nIt's a required mod for ESL plugins made with the latest Creation Kit on pre 1.6.1130 Skyrim versions." +
                  "\n\nMod will not function properly.";

            // RE::DebugMessageBox(err.c_str());
        }
    }
    else {
        err = "SkyParkour Warning\n\n" + IniSettings::ESP_NAME + " isn't loaded by Skyrim." +
              "\n\nMake sure SkyParkour ESP is enabled in your load order." + "\n\nMod will not function properly.";
    }

    // RE::DebugMessageBox(err.c_str());
    ERROR("----SkyParkour Failed To Load Due To Missing ESP----");
    SKSE::stl::report_and_error(err);
}
void Install_Hooks_And_Listeners() {
    RaceChangeListener::Register();
    MenuListener::Register();
    //ButtonEventListener::Register();  // Do it when player loads, unregister inside Menu Listener

    if (Hooks::InputHandler::InstallInputHooks()) {
        LOG("Installed Hooks: |Input|");
    }

    if (Hooks::AnimationEventHook::InstallAnimEventHook()) {
        LOG("Installed Hooks: |AnimEvent|");
    }

    if (Hooks::NotifyGraphHandler::InstallGraphNotifyHook()) {
        LOG("Installed Hooks: |NotifyGraph|");
    }

    if (Hooks::CameraHandler::InstallCamStateHooks()) {
        LOG("Installed Hooks: |Camera|");
    }

    // if (Hooks::HavokHandler::InstallHooks()) {
    //     LOG("Installed Hooks: |Havok|");
    // }
}
void ShowSkyParkourMenu() {
    using menu = Scaleform::SkyParkourMenu;
    const auto &ui = RE::UI::GetSingleton();
    if (ui) {
        const auto &sppf = ui->GetMenu<menu>(menu::MENU_NAME);
        sppf->Show();
    }
}
void MessageEvent(SKSE::MessagingInterface::Message *message) {
    if (message->type == SKSE::MessagingInterface::kPostPostLoad) {
        RuntimeMethods::SetupDLLCompatibility();

        if (!RuntimeMethods::ReadPluginConfigFromINI()) {
            /* Ini does not exist and failed to create */
            return;
        }

        SkyParkour_Papyrus::Internal::Read_All_MCM_From_INI_and_Cache_Settings();

        API_Handles::TrueHUD::RequestTrueHUDAPI();
    }
    else if (message->type == SKSE::MessagingInterface::kDataLoaded) {
        if (!RuntimeMethods::IsESPLoaded()) {
            HandleMissingESPWarning();
            return;
        }

        RuntimeMethods::SetupESPCompatibility();
        Install_Hooks_And_Listeners();

        LOG("|>_SkyParkour Loaded_<|");
    }
    else if (message->type == SKSE::MessagingInterface::kPreLoadGame) {
        RuntimeMethods::ResetRuntimeVariables();
    }
    else if (message->type == SKSE::MessagingInterface::kPostLoadGame) {
        const auto &player = GET_PLAYER;
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
    else if (message->type == SKSE::MessagingInterface::kInputLoaded) {
        Scaleform::SkyParkourMenu::Register();
        ShowSkyParkourMenu();
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

extern "C" DLLEXPORT bool SKSEPlugin_Load(const LoadInterface *skse) {
    plugin::InitializeLogging();

    Init(skse, false);
    LOG("'{} {}' by {} / Skyrim '{}'", Plugin::Name, Plugin::VersionString, Plugin::Author, REL::Module::get().version().string());

    SKSE::GetPapyrusInterface()->Register(RegisterPapyrusFunctions);
    SKSE::GetMessagingInterface()->RegisterListener(MessageEvent);
    return true;
}