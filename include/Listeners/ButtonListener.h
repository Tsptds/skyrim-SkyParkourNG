#pragma once

namespace Buttons
{

    const inline std::unordered_map<uint32_t, uint32_t> xinputToCKMap = {
        // Mouse
        {2, 258},  // Mouse middle
        {3, 259},  // M4
        {4, 260},  // M5
        // Not natively supported
        //{5, 261},  // M6
        //{6, 262},  // M7
        //{7, 263},  // ? tf is 8th button

        // These have no button up events, input gets stuck on wheel up down
        //{8, 264},  // Wheel Up
        //{9, 265},  // Wheel Down

        // Gamepad
        //{0x0001, 266},  // DPAD_UP
        //{0x0002, 267},  // DPAD_DOWN
        //{0x0004, 268},  // DPAD_LEFT
        //{0x0008, 269},  // DPAD_RIGHT
        //{0x0010, 270},  // START
        //{0x0020, 271},  // BACK
        //{0x0040, 272},  // LEFT_THUMB
        //{0x0080, 273},  // RIGHT_THUMB
        //{0x0100, 274},  // LEFT_SHOULDER
        //{0x0200, 275},  // RIGHT_SHOULDER
        //{0x1000, 276},  // A
        //{0x2000, 277},  // B
        //{0x4000, 278},  // X
        //{0x8000, 279}   // Y
    };

    inline uint32_t MapToCKIfPossible(uint32_t dxcode)
    {
        auto it = xinputToCKMap.find(dxcode);
        if (it != xinputToCKMap.end()) {
            //INFO("Alt. CK input found, mapping {}", it->second);
            return it->second;
        }
        return dxcode;  // Return default value if key not found
    }

    class ParkourListener : public RE::BSTEventSink<RE::InputEvent *> {
        public:
            static ParkourListener *GetSingleton()
            {
                static ParkourListener singleton;
                return &singleton;
            }

            static void Register();
            static void Unregister();

            bool SinkRegistered;

        private:
            static void Parkour(RE::ButtonEvent *event);
            virtual RE::BSEventNotifyControl ProcessEvent(RE::InputEvent *const *a_event, RE::BSTEventSource<RE::InputEvent *> *) override;

            ParkourListener() = default;
            ~ParkourListener() = default;
    };

    class SlideListener : public RE::BSTEventSink<RE::InputEvent *> {
        public:
            static SlideListener *GetSingleton()
            {
                static SlideListener singleton;
                return &singleton;
            }

            static void Register();
            static void Unregister();

            bool SinkRegistered;

        private:
            static void CrouchSlide(RE::ButtonEvent *event);
            virtual RE::BSEventNotifyControl ProcessEvent(RE::InputEvent *const *a_event, RE::BSTEventSource<RE::InputEvent *> *) override;

            SlideListener() = default;
            ~SlideListener() = default;
    };
}  // namespace Buttons