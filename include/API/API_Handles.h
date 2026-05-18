#pragma once
#include "API/TrueHUDAPI.h"
#include "API/TrueDirectionalMovementAPI.h"

namespace API_Handles {

    class TrueHUD {
        public:
            // Request the API handle before accessing the handle with Get
            inline static TRUEHUD_API::IVTrueHUD4 *Get() { return APIHandle; };
            static bool RequestTrueHUDAPI();

        private:
            inline static TRUEHUD_API::IVTrueHUD4 *APIHandle;
    };
    class TDM {
        public:
            // Request the API handle before accessing the handle with Get
            inline static TDM_API::IVTDM3 *Get() { return APIHandle; };
            static bool IsLockedOn();
            static bool RequestTDMAPI();
            static void ObtainYawControl(bool);
            static void LockDirectional(bool);

        private:
            inline static TDM_API::PluginHandle pluginHandle;  // For this plugin not the API
            inline static TDM_API::IVTDM3 *APIHandle;
    };

    inline bool RequestAllHandles() {
        bool res{true};
        res &= TrueHUD::RequestTrueHUDAPI();
        res &= TDM::RequestTDMAPI();

        return res;
    }
}  // namespace API_Handles

/* TrueHUD debug draw colors */
#define COLOR_HEX_R 0xFF0000FF
#define COLOR_HEX_G 0x00FF00FF
#define COLOR_HEX_B 0x0000FFFF
#define COLOR_HEX_Y 0xFFF000FF