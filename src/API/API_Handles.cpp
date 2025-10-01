#include "API/API_Handles.h"

namespace API_Handles {
    void *TrueHUD::Handle = nullptr;

    TRUEHUD_API::IVTrueHUD4 *TrueHUD::Get() {
        return static_cast<TRUEHUD_API::IVTrueHUD4 *>(TrueHUD::Handle);
    }

    bool TrueHUD::RequestTrueHUDAPI() {
        if (Handle) {
            WARN("TrueHUD API handle already registered but requested again");
            return true;
        }

        const auto &res = TRUEHUD_API::RequestPluginAPI();
        if (res) {
            LOG("TrueHUD API found, click on player and 'sae sppf_debug' in console to toggle");
            Handle = res;
            return true;
        }

        WARN("TrueHUD API not found, debugging isn't available");
        return false;
    }

}  // namespace API_Handles