#include "Listeners/MenuListener.h"
#include "_References/RuntimeMethods.h"
#include "_References/RuntimeVariables.h"
#include "Parkouring.h"

namespace Menus {
    // List of disqualifying menu names
    const std::string_view excludedMenus[] = {RE::BarterMenu::MENU_NAME,       RE::ConsoleNativeUIMenu::MENU_NAME,
                                              RE::ContainerMenu::MENU_NAME,    RE::CraftingMenu::MENU_NAME,
                                              RE::CreationClubMenu::MENU_NAME, RE::DialogueMenu::MENU_NAME,
                                              RE::FavoritesMenu::MENU_NAME,    RE::GiftMenu::MENU_NAME,
                                              RE::InventoryMenu::MENU_NAME,    RE::JournalMenu::MENU_NAME,
                                              RE::LevelUpMenu::MENU_NAME,      RE::LockpickingMenu::MENU_NAME,
                                              RE::MagicMenu::MENU_NAME,        RE::MapMenu::MENU_NAME,
                                              RE::MessageBoxMenu::MENU_NAME,   RE::MistMenu::MENU_NAME,
                                              RE::RaceSexMenu::MENU_NAME,      RE::SleepWaitMenu::MENU_NAME,
                                              RE::StatsMenu::MENU_NAME,        RE::TrainingMenu::MENU_NAME,
                                              RE::Console::MENU_NAME,          RE::TweenMenu::MENU_NAME,
                                              RE::MainMenu::MENU_NAME};

    bool CheckMenuOpen() {
        auto ui = RE::UI::GetSingleton();
        // Check if any of the excluded menus are open
        for (const std::string_view menuName: Menus::excludedMenus) {
            if (ui->IsMenuOpen(menuName)) {
                return true;
            }
        }
        return false;
    }

    bool MainMenuShowing() {
        auto ui = RE::UI::GetSingleton();
        if (ui->IsMenuOpen(RE::MainMenu::MENU_NAME)) {
            return true;
        }
        return false;
    }
}  // namespace Menus

bool MenuListener::Register() {
    auto listener = MenuListener::GetSingleton();
    if (listener) {
        RE::UI::GetSingleton()->AddEventSink<RE::MenuOpenCloseEvent>(listener);
        return true;
    }
    return false;
}
bool MenuListener::Unregister() {
    auto listener = MenuListener::GetSingleton();
    if (listener) {
        RE::UI::GetSingleton()->RemoveEventSink<RE::MenuOpenCloseEvent>(listener);
        return true;
    }
    return false;
}

RE::BSEventNotifyControl MenuListener::ProcessEvent(const RE::MenuOpenCloseEvent* ev, RE::BSTEventSource<RE::MenuOpenCloseEvent>*) {
    if (ev->opening) {
        //LOG("Menu {} opened", ev->menuName.c_str());

        if (Menus::CheckMenuOpen()) {
            RuntimeVariables::IsMenuOpen = true;
        }

        if (!RuntimeVariables::IsInMainMenu && Menus::MainMenuShowing()) {
            Parkouring::SetParkourOnOff(false);
            RuntimeMethods::ResetRuntimeVariables();

            RuntimeVariables::IsInMainMenu = true;

            //LOG(">> In Menu");
        }
    }
    else {
        //LOG("Menu {} closed", ev->menuName.c_str());

        //// Treating this as save loaded event, fires on COC command and new game, when area along with player loads.
        //if (ev->menuName == RE::LoadingMenu::MENU_NAME) {
        //    if (!RuntimeVariables::IsBeastForm) {
        //        AnimEventListener::Register();
        //    }
        //}

        // Racemenu closed, reset some stuff
        if (ev->menuName == RE::RaceSexMenu::MENU_NAME) {
            RuntimeMethods::ResetRuntimeVariables();
        }

        if (!Menus::CheckMenuOpen()) {
            RuntimeVariables::IsMenuOpen = false;

            //LOG(">> Closed Menu");
        }

        if (RuntimeVariables::IsInMainMenu && !Menus::MainMenuShowing()) {
            RuntimeVariables::IsInMainMenu = false;

            //LOG(">> Closed Main Menu");
        }
    }
    return RE::BSEventNotifyControl::kContinue;
}