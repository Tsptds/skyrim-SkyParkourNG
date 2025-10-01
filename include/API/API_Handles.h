#pragma once
#include "API/TrueHUDAPI.h"

namespace API_Handles {
    class TrueHUD {
        public:
            static TRUEHUD_API::IVTrueHUD4* Get();
            static bool RequestTrueHUDAPI();

        private:
            static void* Handle;
    };
}  // namespace API_Handles