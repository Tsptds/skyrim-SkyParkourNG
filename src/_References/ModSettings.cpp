#include "_References/ModSettings.h"

namespace ModSettings {

    bool Use_Preset_Parkour_Key = true;
    int Preset_Parkour_Key = PARKOUR_PRESET_KEYS::kJump;
    uint32_t Custom_Parkour_Key = 0;

    bool Mod_Enabled = true;
    bool Use_Indicators = true;
    float Playback_Speed = 1.15f;

    float Parkour_Delay = 0.0f;  // Set initial delay

    bool Enable_Stamina_Consumption = true;
    bool Must_Have_Stamina = true;
    float Stamina_Damage = 20.0f;

    bool Smart_Steps = true;
    bool Smart_Vault = true;
    bool Smart_Climb = true;
}  // namespace ModSettings
