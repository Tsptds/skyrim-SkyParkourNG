#include "MenuListener.h"

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

    /*bool MainMenuShowing() {
        auto ui = RE::UI::GetSingleton();
        if (ui->IsMenuOpen(RE::MainMenu::MENU_NAME)) {
            return true;
        }
        return false;
    }*/
}  // namespace Menus

bool MenuListener::Register() {
    auto listener = MenuListener::GetSingleton();
    if (listener) {
        RE::UI::GetSingleton()->AddEventSink<RE::MenuOpenCloseEvent>(listener);  // :contentReference[oaicite:0]{index=0}
        return true;
    }
    return false;
}
bool MenuListener::Unregister() {
    auto listener = MenuListener::GetSingleton();
    if (listener) {
        RE::UI::GetSingleton()->RemoveEventSink<RE::MenuOpenCloseEvent>(listener);  // :contentReference[oaicite:0]{index=0}
        return true;
    }
    return false;
}

RE::BSEventNotifyControl MenuListener::ProcessEvent(const RE::MenuOpenCloseEvent* ev, RE::BSTEventSource<RE::MenuOpenCloseEvent>*) {
    if (ev->opening) {
        //logger::info("Menu {} opened", ev->menuName.c_str());

        if (Menus::CheckMenuOpen()) {
            RuntimeVariables::IsMenuOpen = true;
        }
        /*if (Menus::MainMenuShowing() && ModSettings::ShouldModSuspend == false) {
            ModSettings::ShouldModSuspend = true;
        }*/

    } else {
        //logger::info("Menu {} closed", ev->menuName.c_str());

        // Treating this as save loaded event, fires on COC command and new game, when area along with player loads.
        if (ev->menuName == RE::LoadingMenu::MENU_NAME) {
            AnimEventListener::Register();
        }

        // If player changes char, it needs to re-register to the graph.
        if (ev->menuName == RE::RaceSexMenu::MENU_NAME) {
            AnimEventListener::Register();
        }

        if (!Menus::CheckMenuOpen()) {
            RuntimeVariables::IsMenuOpen = false;
        }

        /*if (!Menus::MainMenuShowing() && ModSettings::ShouldModSuspend == true) {
            ModSettings::ShouldModSuspend = false;
            Parkouring::SetParkourOnOff(true);
        }*/
    }
    return RE::BSEventNotifyControl::kContinue;
}