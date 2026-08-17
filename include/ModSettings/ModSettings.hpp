#pragma once
#include "API/SKSEMenuFramework.h"

namespace ModSettings
{
    inline bool _Debug_Enabled{false};

    inline bool Parkour_Enabled{true};
    inline bool Use_Indicators{true};
    inline float Playback_Speed{1.15f};

    inline bool Use_Preset_Parkour_Key{true};
    inline uint32_t Custom_Parkour_Key{0};
    inline int32_t Preset_Parkour_Key{SkyParkour::PresetKeys::kJump};
    inline float Parkour_Delay{0.0f};
    inline int32_t Auto_Parkour{SkyParkour::AutoParkourOptions::kNonCombatOnly};

    inline bool Enable_Stamina_Consumption{true};
    inline bool Must_Have_Stamina{true};
    inline float Stamina_Damage{20.f};

    inline bool Smart_Steps{true};
    inline bool Smart_Vault{true};
    inline bool Smart_Climb{true};

    inline bool Crouch_Slide_Enabled{true};
    inline bool Advanced_Slide_Sneak{true};
    inline bool ExpSlideTackle{false};

    inline bool Land_Rolling_Enabled{true};
}

