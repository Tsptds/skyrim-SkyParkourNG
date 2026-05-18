#include "_References/RuntimeMethods.h"
#include "_References/RuntimeVariables.h"
#include "_References/ParkourType.h"
#include "_References/ModSettings.h"
#include "_References/IniSettings.h"
#include "_References/Compatibility.h"
#include "API/API_Handles.h"

namespace RuntimeMethods {
    const RE::TESFile *GetPlugin(RE::TESDataHandler *const &dh, std::string_view esp_name) {
        if (!dh) {
            return nullptr;
        }

        if (auto file = dh->GetSingleton()->LookupLoadedLightModByName(esp_name)) {
            return file;
        }
        return dh->GetSingleton()->LookupLoadedModByName(esp_name);
    }

    // Things that are not handled by MCM and persistent throughout saves without being reset on game load
    void ResetAll() {
        ResetParkour();
        ResetSlide();
    }

    void ResetParkour() {
        RuntimeVariables::ParkourInProgress = false;
        RuntimeVariables::selectedLedgeType = ParkourType::NoLedge;
        RuntimeVariables::IsParkourActive = true;

        if (Compatibility::TrueDirectionalMovement::found) {
            API_Handles::TDM::ObtainYawControl(false);
        }
    }

    void ResetSlide() {
        RuntimeVariables::SlideOngoing = false;
        if (Compatibility::TrueDirectionalMovement::found) {
            API_Handles::TDM::ObtainYawControl(false);
        }
    }

    bool IsESPLoaded() {
        const auto dh = RE::TESDataHandler::GetSingleton();
        return dh && (dh->GetSingleton()->LookupLoadedLightModByName(IniSettings::ESP_NAME) ||
                      dh->GetSingleton()->LookupLoadedModByName(IniSettings::ESP_NAME));
    }

    bool ReadPluginConfigFromINI() {
        const auto ini = IniSettings::GetIniHandle();
        if (!ini) {
            ERROR("INI FILE DOES NOT EXIST AND FAILED TO CREATE");
            return false;
        }

        const char *name = ini->GetValue("ESP", "sEspName");
        if (!name) {
            ERROR("EspName not found, using default name");
        }

        LOG("ESP Name: '{}'", name);
        IniSettings::ESP_NAME = name;
        return true;
    }

    void SetupDLLCompatibility() {
        const auto TDM = GetModuleHandleA(Compatibility::TrueDirectionalMovement::dll_name);
        if (TDM) {
            Compatibility::TrueDirectionalMovement::found = true;
            LOG("Patch: True Directional Movement |360|Swim Pitch|Yaw Lock|");
        }
        const auto CS = GetModuleHandleA(Compatibility::ClassicSprintingRedone::dll_name);
        if (CS) {
            Compatibility::ClassicSprintingRedone::found = true;
            LOG("Patch: Classic Sprinting Redone |No Sprint State Conservation|");
        }
    }

    void SetupESPCompatibility() {
        const auto dh = RE::TESDataHandler::GetSingleton();
        const auto JA = GetPlugin(dh, Compatibility::JumpingAttack::esp_name);
        if (JA) {
            Compatibility::JumpingAttack::found = true;
            LOG("Patch: Jumping Attack |Weapon State Fix|");
        }
    }
}  // namespace RuntimeMethods