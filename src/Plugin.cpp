#include "PCH.h"
#include "Util/ParkourUtility.h"
// #include "Listeners/RaceChangeListener.h"
#include "Listeners/MenuListener.h"
#include "_References/Compatibility.h"
#include "ModSettings/SkyParkourINI.hpp"
#include "ModSettings/ModSettingsMenu.hpp"
#include "_References/CustomBlockingVars.h"
#include "Hooks/InputHandler.hpp"
#include "Hooks/AnimEventHandler.hpp"
#include "Hooks/CameraHandler.hpp"
#include "Hooks/AnimGraphManager.hpp"
// #include "Hooks/HavokHandler.hpp"
#include "API/API_Handles.h"
#include "HUD/Scaleform/SkyParkourMenu.hpp"

void Install_Hooks_And_Listeners()
{
    // RaceChangeListener::Register(); // Not needed, post graph create includes this
    //ButtonEventListener::Register(); // Do it when player loads, unregister inside Menu Listener
    MenuListener::Register();

    if (Hooks::InputHandler::InstallInputHooks())
    {
        INFO("Installed Hooks: |Input|");
    }

    if (Hooks::AnimationEventHook::InstallAnimEventHook())
    {
        INFO("Installed Hooks: |AnimEvent|");
    }

    if (Hooks::NotifyGraphHandler::InstallGraphNotifyHook())
    {
        INFO("Installed Hooks: |NotifyGraph|");
    }

    if (Hooks::CameraHandler::InstallCamStateHooks())
    {
        INFO("Installed Hooks: |Camera|");
    }

    if (Hooks::GraphManagerHandler::InstallGraphManagerHooks())
    {
        INFO("Installed Hooks: |GraphManager|");
    }

    // if (Hooks::HavokHandler::InstallHooks()) {
    //     INFO("Installed Hooks: |Havok|");
    // }
}
void ShowSkyParkourMenu()
{
    using menu = Scaleform::SkyParkourMenu;
    const auto ui = RE::UI::GetSingleton();
    if (ui)
    {
        ui->GetMenu<menu>(menu::MENU_NAME)->Show();
    }
}
void MessageEvent(SKSE::MessagingInterface::Message *message)
{
    if (message->type == SKSE::MessagingInterface::kPostPostLoad)
    {
        RuntimeMethods::SetupDLLCompatibility();

        SkyParkourINI::Read_All_MCM_From_INI_and_Cache_Settings();
        if (!API_Handles::RequestAllHandles()) WARN("Some API handles not registered");
        if (!CustomBlockingVars::ReadAndCacheVars()) WARN("No custom rule file found, skipping");
    }
    else if (message->type == SKSE::MessagingInterface::kDataLoaded)
    {
        RuntimeMethods::SetupESPCompatibility();
        Install_Hooks_And_Listeners();
        ModSettingsMenu::Register();

        INFO("|>_SkyParkour Loaded_<|");
    }
    else if (message->type == SKSE::MessagingInterface::kPreLoadGame)
    {
        RuntimeMethods::ResetAll();
    }
    else if (message->type == SKSE::MessagingInterface::kPostLoadGame)
    {
        const auto player = GET_PLAYER;
        int32_t out;
        if (player->GetGraphVariableInt(SPPF_Ledge, out) && out != std::to_underlying(ParkourType::NoLedge))
        {
            WARN("Fix: Save with ongoing parkour");
            player->NotifyAnimationGraph(SPPF_STOP);
        }

        RuntimeMethods::ResetAll();
    }
    else if (message->type == SKSE::MessagingInterface::kNewGame)
    {
        RuntimeMethods::ResetAll();
    }
    else if (message->type == SKSE::MessagingInterface::kInputLoaded)
    {
        Scaleform::SkyParkourMenu::Register();
        ShowSkyParkourMenu();
    }
}

#include "Plugin.h"

namespace plugin
{
    std::optional<std::filesystem::path> getLogDirectory()
    {
        using namespace std::filesystem;
        PWSTR buf;
        SHGetKnownFolderPath(FOLDERID_Documents, KF_FLAG_DEFAULT, nullptr, &buf);
        std::unique_ptr<wchar_t, decltype(&CoTaskMemFree)> documentsPath{buf, CoTaskMemFree};
        path directory{documentsPath.get()};
        directory.append("My Games"sv);

        if (exists("steam_api64.dll"sv))
        {
            if (exists("openvr_api.dll") || exists("Data/SkyrimVR.esm"))
            {
                directory.append("Skyrim VR"sv);
            }
            else
            {
                directory.append("Skyrim Special Edition"sv);
            }
        }
        else if (exists("Galaxy64.dll"sv))
        {
            directory.append("Skyrim Special Edition GOG"sv);
        }
        else if (exists("eossdk-win64-shipping.dll"sv))
        {
            directory.append("Skyrim Special Edition EPIC"sv);
        }
        else
        {
            return current_path().append("skselogs");
        }
        return directory.append("SKSE"sv).make_preferred();
    }

    static void InitializeLogging()
    {
        auto path = getLogDirectory();
        if (!path)
        {
            SKSE::stl::report_and_fail("Can't find SKSE log directory");
        }
        *path /= std::format("{}.log"sv, Plugin::Name);

        std::shared_ptr<spdlog::logger> log;
        if (IsDebuggerPresent())
        {
            log = std::make_shared<spdlog::logger>("Global", std::make_shared<spdlog::sinks::msvc_sink_mt>());
        }
        else
        {
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

extern "C" DLLEXPORT bool SKSEPlugin_Load(const SKSE::LoadInterface *skse)
{
    plugin::InitializeLogging();

    SKSE::Init(skse, false);
    INFO("'{} {}' by {} / Skyrim '{}'", Plugin::Name, Plugin::VersionString, Plugin::Author, REL::Module::get().version().string());

    SKSE::GetMessagingInterface()->RegisterListener(MessageEvent);

    return true;
}