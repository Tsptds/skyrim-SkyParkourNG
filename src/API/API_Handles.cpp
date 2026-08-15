#include "API/API_Handles.h"

namespace API_Handles
{
    bool TrueHUD::RequestTrueHUDAPI()
    {
        if (APIHandle)
        {
            WARN("TrueHUD API handle already registered but requested again");
            return true;
        }

        const auto res = TRUEHUD_API::RequestPluginAPI();
        if (res)
        {
            INFO("TrueHUD Found: Visual Debugging Available");
            APIHandle = static_cast<decltype(TrueHUD::APIHandle)>(res);
            return true;
        }

        WARN("TrueHUD API not found, debugging isn't available");
        return false;
    }
    bool TDM::RequestTDMAPI()
    {
        if (APIHandle)
        {
            WARN("TDM API handle already registered but requested again");
            return true;
        }

        const auto res = TDM_API::RequestPluginAPI();
        if (res)
        {
            INFO("TDM API Found");
            APIHandle = static_cast<decltype(TDM::APIHandle)>(res);
            return true;
        }

        WARN("TDM API not found");
        return false;
    }

    bool TDM::IsLockedOn()
    {
        auto tdm = Get();
        return tdm && tdm->GetDirectionalMovementMode() == TDM_API::DirectionalMovementMode::kTargetLock;
    }

    void TDM::ReleaseYaw()
    {
        if (!APIHandle)
        {
            ERROR("{}", "TDM API Handle is null, can't modify yaw lock");
            return;
        }

        static_cast<void>(APIHandle->ReleaseYawControl(pluginHandle));
    }

    void TDM::ObtainYaw(bool canStillTurn)
    {
        if (!APIHandle)
        {
            ERROR("{}", "TDM API Handle is null, can't modify yaw lock");
            return;
        }

        static_cast<void>(APIHandle->RequestYawControl(pluginHandle, canStillTurn ? 2.f : 0));
    }

    void TDM::SyncTppYaw()
    {
        auto cam = RE::PlayerCamera::GetSingleton();
        if (!cam) return;

        auto tpp = skyrim_cast<RE::ThirdPersonState *>(cam->currentState.get());
        if (APIHandle && tpp)
        {
            const float diff = fabsf(GET_PLAYER->data.angle.z - tpp->currentYaw);

            if (diff <= 0.5f)
            {
                APIHandle->SetPlayerYaw(pluginHandle, tpp->currentYaw);
            }
            else if (diff > 2.64f)
            {
                APIHandle->SetPlayerYaw(pluginHandle, tpp->currentYaw + 3.14f);
            }
        }
    }

    // void TDM::LockDirectional(bool isLock)
    // {
    //     auto tdm = API_Handles::TDM::Get();
    //     if (!tdm)
    //     {
    //         ERROR("{}", "TDM API Handle is null, can't modify directional movement");
    //         return;
    //     }

    //     if (isLock)
    //     {
    //         static_cast<void>(tdm->RequestDisableDirectionalMovement(API_Handles::TDM::pluginHandle));
    //     }
    //     else
    //     {
    //         static_cast<void>(tdm->ReleaseDisableDirectionalMovement(API_Handles::TDM::pluginHandle));
    //     }
    // }

}  // namespace API_Handles