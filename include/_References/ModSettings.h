#pragma once

namespace ModSettings {
    inline bool _Debug_Draw_Lines{false};

    inline bool Mod_Enabled{true};
    inline bool Use_Indicators{true};
    inline float Playback_Speed{1.15f};
    
    inline bool Use_Preset_Parkour_Key{true};
    inline int Preset_Parkour_Key{PARKOUR_PRESET_KEYS::kJump};
    inline uint32_t Custom_Parkour_Key{0};
    inline float Parkour_Delay{0.0f};
    
    inline bool Enable_Stamina_Consumption{true};
    inline bool Must_Have_Stamina{true};
    inline float Stamina_Damage{20.f};
    
    inline bool Smart_Steps{true};
    inline bool Smart_Vault{true};
    inline bool Smart_Climb{true};
    
    inline bool Crouch_Slide_Enabled{true};
}  // namespace ModSettings