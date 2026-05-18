#include "API/API_Handles.h"

namespace API_Handles {

    bool TrueHUD::RequestTrueHUDAPI() {
        if (APIHandle) {
            WARN("TrueHUD API handle already registered but requested again");
            return true;
        }

        const auto res = TRUEHUD_API::RequestPluginAPI();
        if (res) {
            LOG("TrueHUD Found: Visual Debugging Available");
            APIHandle = static_cast<decltype(TrueHUD::APIHandle)>(res);
            return true;
        }

        WARN("TrueHUD API not found, debugging isn't available");
        return false;
    }
    bool TDM::RequestTDMAPI() {
        if (APIHandle) {
            WARN("TDM API handle already registered but requested again");
            return true;
        }

        const auto res = TDM_API::RequestPluginAPI();
        if (res) {
            LOG("TDM API Found");
            APIHandle = static_cast<decltype(TDM::APIHandle)>(res);
            return true;
        }

        WARN("TDM API not found");
        return false;
    }

    bool TDM::IsLockedOn() {
        auto tdm = Get();
        return tdm && tdm->GetDirectionalMovementMode() == TDM_API::DirectionalMovementMode::kTargetLock;
    }

    void TDM::ObtainYawControl(bool isLock) {
        auto tdm = API_Handles::TDM::Get();
        if (!tdm) {
            ERROR("{}", "TDM API Handle is null, can't modify yaw lock");
            return;
        }

        if (isLock) {
            static_cast<void>(tdm->RequestYawControl(API_Handles::TDM::pluginHandle, 0));
        }
        else {
            static_cast<void>(tdm->ReleaseYawControl(API_Handles::TDM::pluginHandle));
        }
    }

    void TDM::LockDirectional(bool isLock) {
        auto tdm = API_Handles::TDM::Get();
        if (!tdm) {
            ERROR("{}", "TDM API Handle is null, can't modify directional movement");
            return;
        }

        if (isLock) {
            static_cast<void>(tdm->RequestDisableDirectionalMovement(API_Handles::TDM::pluginHandle));
        }
        else {
            static_cast<void>(tdm->ReleaseDisableDirectionalMovement(API_Handles::TDM::pluginHandle));
        }
    }

}  // namespace API_Handles