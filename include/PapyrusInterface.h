#pragma once
#include "PCH.h"
#include "References.h"
#include "Parkouring.h"

namespace SkyParkour_Papyrus {

    class Internal {
        public:
            static void Read_All_MCM_From_INI_and_Cache_Settings();
            static void RegisterPapyrusFuncsToVM(RE::BSScript::IVirtualMachine *vm);

        private:
            static void AlertPlayerLoaded(RE::StaticFunctionTag *);
    };

};  // namespace SkyParkour_Papyrus