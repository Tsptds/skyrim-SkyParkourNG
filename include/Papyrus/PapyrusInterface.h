#pragma once

namespace SkyParkour_Papyrus {

    class Internal {
        public:
            static void Read_All_MCM_From_INI_and_Cache_Settings();
            static void RegisterPapyrusFuncsToVM(RE::BSScript::IVirtualMachine *vm);

        private:
            inline static const std::string className = "SkyParkourPapyrus"s;
            inline static const char *Section = "MCM";

            static void AlertPlayerLoaded(RE::StaticFunctionTag *);
    };

};  // namespace SkyParkour_Papyrus