#pragma once

namespace BehaviorGraph {
    /* --Parkour-- */
    // Anim Events
    constexpr const char *SPPF_NOTIFY = "SkyParkour";
    constexpr const char *SPPF_START = "SkyParkour_Start";
    constexpr const char *SPPF_STOP = "SkyParkour_Stop";
    constexpr const char *SPPF_RECOVERY = "SkyParkour_Recovery";
    constexpr const char *SPPF_INTERRUPT = "SkyParkour_Interrupt";
    constexpr const char *SPPF_GOTO_EXIT = "SkyParkour_GoToExit";
    constexpr const char *SPPF_STAMINA_HIT = "SkyParkour_HitStamina";

    // Graph Variables
    constexpr const char *SPPF_Ledge = "SkyParkourLedge";
    constexpr const char *SPPF_Leg = "SkyParkourStepLeg";
    constexpr const char *SPPF_Grab_Variant = "SkyParkourGrabVariant";  // false is low, true is high variant
    constexpr const char *SPPF_ONGOING = "SkyParkourOngoing";
    constexpr const char *SPPF_SPEEDMULT = "SkyParkourSpeedMult";
    constexpr const char *SPPF_Lower_Body_Only = "SkyParkourLowerBody";
    
    // Anim Event Payload
    constexpr const char* SPPF_LOWEFFORTPAYLOAD = "LowEffort";
    /*----------------------------------------------------------------*/

    /* --Crouch Slide-- */
    // Anim Events
    constexpr const char *SPPF_NOTIFY_SLIDE = "SkyParkour_Slide";
    constexpr const char *SPPF_SLIDE_START = "SkyParkour_SlideStart";
    constexpr const char *SPPF_SLIDE_STOP = "SkyParkour_SlideStop";
    
    // Graph Variables
    constexpr const char *SPPF_SLIDE_ONGOING = "SkyParkourSliding";
    constexpr const char *SPPF_SLIDE_IS_ROLL = "SkyParkourIsLandingRoll";
    
    // Anim Event Payload
    constexpr const char *SPPF_SLIDEPAYLOAD = "Slide";
    constexpr const char *SPPF_ROLLPAYLOAD = "LandRoll";
    /*----------------------------------------------------------------*/

    /* --Leap-- */
    // Anim Events
    // constexpr const char *SPPF_LEAP_START = "SkyParkour_LeapStart";
    // constexpr const char *SPPF_LEAP_STOP = "SkyParkour_LeapStop";

    // Graph Variables
    // constexpr const char *SPPF_LEAP_ONGOING = "SkyParkourLeaping";
    /*----------------------------------------------------------------*/

    /*----------------------------------------------------------------*/
    // Graph Variables to Check if Behavior Patches are Installed
    constexpr const char *SPPF_TPP_INSTALLED = "SkyParkourTPPInstalled";
    constexpr const char *SPPF_FPP_INSTALLED = "SkyParkourFPPInstalled";
    /*----------------------------------------------------------------*/

}  // namespace BehaviorGraph