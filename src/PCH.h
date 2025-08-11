#pragma once

//Standard Library C++20
#include <algorithm>
#include <any>
#include <array>
#include <atomic>
#include <barrier>
#include <bit>
#include <bitset>
#include <cassert>
#include <cctype>
#include <cerrno>
#include <cfenv>
#include <cfloat>
#include <charconv>
#include <chrono>
#include <cinttypes>
#include <climits>
#include <clocale>
#include <cmath>
#include <codecvt>
#include <compare>
#include <complex>
#include <concepts>
#include <condition_variable>
#include <coroutine>
#include <csetjmp>
#include <csignal>
#include <cstdarg>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <cuchar>
#include <cwchar>
#include <cwctype>
#include <deque>
#include <exception>
#include <execution>
#include <filesystem>
#include <format>
#include <forward_list>
#include <fstream>
#include <functional>
#include <future>
#include <initializer_list>
#include <iomanip>
#include <ios>
#include <iosfwd>
#include <iostream>
#include <istream>
#include <iterator>
#include <latch>
#include <limits>
#include <list>
#include <locale>
#include <map>
#include <memory>
#include <memory_resource>
#include <mutex>
#include <new>
#include <numbers>
#include <numeric>
#include <optional>
#include <ostream>
#include <queue>
#include <random>
#include <ranges>
#include <ratio>
#include <regex>
#include <scoped_allocator>
#include <semaphore>
#include <set>
#include <shared_mutex>
#include <source_location>
#include <span>
#include <sstream>
#include <stack>
#include <stdexcept>
#include <stop_token>
#include <streambuf>
#include <string>
#include <string_view>
#include <strstream>
#include <syncstream>
#include <system_error>
#include <thread>
#include <tuple>
#include <type_traits>
#include <typeindex>
#include <typeinfo>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <valarray>
#include <variant>
#include <vector>
#include <version>

#define WIN32_LEAN_AND_MEAN
#define NOGDICAPMASKS     // CC_*, LC_*, PC_*, CP_*, TC_*, RC_
#define NOWINSTYLES       // WS_*, CS_*, ES_*, LBS_*, SBS_*, CBS_*
#define NOSYSMETRICS      // SM_*
#define NOMENUS           // MF_*
#define NOICONS           // IDI_*
#define NOSYSCOMMANDS     // SC_*
#define NORASTEROPS       // Binary and Tertiary raster ops
#define NOSHOWWINDOW      // SW_*
#define OEMRESOURCE       // OEM Resource values
#define NOATOM            // Atom Manager routines
#define NOCLIPBOARD       // Clipboard routines
#define NOCOLOR           // Screen colors
#define NODRAWTEXT        // DrawText() and DT_*
#define NOGDI             // All GDI defines and routines
#define NOMB              // MB_* and MessageBox()
#define NOMETAFILE        // typedef METAFILEPICT
#define NOMINMAX          // Macros min(a,b) and max(a,b)
#define NOSCROLL          // SB_* and scrolling routines
#define NOSERVICE         // All Service Controller routines, SERVICE_ equates, etc.
#define NOSOUND           // Sound driver routines
#define NOTEXTMETRIC      // typedef TEXTMETRIC and associated routines
#define NOWINOFFSETS      // GWL_*, GCL_*, associated routines
#define NOCOMM            // COMM driver routines
#define NOKANJI           // Kanji support stuff.
#define NOHELP            // Help engine interface.
#define NOPROFILER        // Profiler interface.
#define NODEFERWINDOWPOS  // DeferWindowPos routines
#define NOMCX             // Modem Configuration Extensions

#include "BuildOptions.h"

//spdlog
#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/msvc_sink.h>

#define PLUGIN_LOGPATTERN_DEFAULT "[%b %d %H:%M:%S.%e] [%l] [%t] %v"
#define PLUGIN_LOGPATTERN_DEBUG "[%b %d %H:%M:%S.%e] [%l] [%t] [%s:%#] %v"
#define PLUGIN_LOGPATTERN_RELEASE "[%l] [%H:%M:%S.%e] %v"

//commonlibsse-ng and skse
#include <RE/Skyrim.h>
#include <SKSE/SKSE.h>
#include <REL/Relocation.h>

//winapi
#include <ShlObj_core.h>
#include <Windows.h>

#if DETOURS_LIBRARY
#include <detours/detours.h>
#endif

#undef cdecl  // Workaround for Clang 14 CMake configure error.

//local
#include <SimpleIni.h>
#include "Util/ThreadPool.hpp"

#define DLLEXPORT __declspec(dllexport)

using namespace std::literals;
namespace logger = SKSE::log;

/* Macro func */
#define Mask_OR(x, y) (static_cast<uint32_t>(x) | static_cast<uint32_t>(y))
#define Mask_AND(x, y) (static_cast<uint32_t>(x) & static_cast<uint32_t>(y))
#define Mask_DIFF(x, y) (static_cast<uint32_t>(x) & ~static_cast<uint32_t>(y))

namespace SkyParkourUtil {
    using namespace RE;
    static const std::unordered_map<COL_LAYER, std::string> colLayerMap{{COL_LAYER::kUnidentified, "Unidentified"},
                                                                        {COL_LAYER::kStatic, "Static"},
                                                                        {COL_LAYER::kAnimStatic, "AnimStatic"},
                                                                        {COL_LAYER::kTransparent, "Transparent"},
                                                                        {COL_LAYER::kClutter, "Clutter"},
                                                                        {COL_LAYER::kWeapon, "Weapon"},
                                                                        {COL_LAYER::kProjectile, "Projectile"},
                                                                        {COL_LAYER::kSpell, "Spell"},
                                                                        {COL_LAYER::kBiped, "Biped"},
                                                                        {COL_LAYER::kTrees, "Trees"},
                                                                        {COL_LAYER::kProps, "Props"},
                                                                        {COL_LAYER::kWater, "Water"},
                                                                        {COL_LAYER::kTrigger, "Trigger"},
                                                                        {COL_LAYER::kTerrain, "Terrain"},
                                                                        {COL_LAYER::kTrap, "Trap"},
                                                                        {COL_LAYER::kNonCollidable, "NonCollidable"},
                                                                        {COL_LAYER::kCloudTrap, "CloudTrap"},
                                                                        {COL_LAYER::kGround, "Ground"},
                                                                        {COL_LAYER::kPortal, "Portal"},
                                                                        {COL_LAYER::kDebrisSmall, "DebrisSmall"},
                                                                        {COL_LAYER::kDebrisLarge, "DebrisLarge"},
                                                                        {COL_LAYER::kAcousticSpace, "AcousticSpace"},
                                                                        {COL_LAYER::kActorZone, "ActorZone"},
                                                                        {COL_LAYER::kProjectileZone, "ProjectileZone"},
                                                                        {COL_LAYER::kGasTrap, "GasTrap"},
                                                                        {COL_LAYER::kShellCasting, "ShellCasting"},
                                                                        {COL_LAYER::kTransparentWall, "TransparentWall"},
                                                                        {COL_LAYER::kInvisibleWall, "InvisibleWall"},
                                                                        {COL_LAYER::kTransparentSmallAnim, "TransparentSmallAnim"},
                                                                        {COL_LAYER::kClutterLarge, "ClutterLarge"},
                                                                        {COL_LAYER::kCharController, "CharController"},
                                                                        {COL_LAYER::kStairHelper, "StairHelper"},
                                                                        {COL_LAYER::kDeadBip, "DeadBip"},
                                                                        {COL_LAYER::kBipedNoCC, "BipedNoCC"},
                                                                        {COL_LAYER::kAvoidBox, "AvoidBox"},
                                                                        {COL_LAYER::kCollisionBox, "CollisionBox"},
                                                                        {COL_LAYER::kCameraSphere, "CameraSphere"},
                                                                        {COL_LAYER::kDoorDetection, "DoorDetection"},
                                                                        {COL_LAYER::kConeProjectile, "ConeProjectile"},
                                                                        {COL_LAYER::kCamera, "Camera"},
                                                                        {COL_LAYER::kItemPicker, "ItemPicker"},
                                                                        {COL_LAYER::kLOS, "LOS"},
                                                                        {COL_LAYER::kPathingPick, "PathingPick"},
                                                                        {COL_LAYER::kUnused0, "Unused0"},
                                                                        {COL_LAYER::kUnused1, "Unused1"},
                                                                        {COL_LAYER::kSpellExplosion, "SpellExplosion"},
                                                                        {COL_LAYER::kDroppingPick, "DroppingPick"}};
    inline std::string ColLayerToString(COL_LAYER layer) {
        auto it = colLayerMap.find(layer);
        return it != colLayerMap.end() ? it->second : "Unknown";
    }

    /* Mark ledge point layers that are considered invalid for climbing */
    static const std::unordered_set<COL_LAYER> ClimbLayerExclusionList{
        COL_LAYER::kNonCollidable, COL_LAYER::kCharController, COL_LAYER::kAnimStatic, COL_LAYER::kWeapon,
        COL_LAYER::kProjectile,    COL_LAYER::kTransparent,    COL_LAYER::kClutter,    COL_LAYER::kBiped};

    /* Head Level Check Layers. If hit, consider vault has obstruction behind */
    static const std::unordered_set<COL_LAYER> VaultForwardRayList{
        COL_LAYER::kStatic, COL_LAYER::kTerrain,      COL_LAYER::kGround,     COL_LAYER::kProps,       COL_LAYER::kDoorDetection,
        COL_LAYER::kTrees,  COL_LAYER::kClutterLarge, COL_LAYER::kAnimStatic, COL_LAYER::kDebrisLarge, COL_LAYER::kTransparent};

    /* Ledge Point Layers. If hit, consider vault point invalid. */
    static const std::unordered_set<COL_LAYER> VaultDownRayList{
        COL_LAYER::kGround,         COL_LAYER::kTerrain,       COL_LAYER::kWeapon,  COL_LAYER::kProjectile,
        COL_LAYER::kCharController, COL_LAYER::kNonCollidable, COL_LAYER::kClutter, COL_LAYER::kBiped};

    static ThreadPool threads;
    static RE::hkVector4 zeroVector{0, 0, 0, 0};

    struct RayCastResult {
            float distance = -1.0f;
            RE::COL_LAYER layer = RE::COL_LAYER::kUnidentified;
            RE::hkVector4 normalOut = RE::hkVector4(0, 0, 0, 0);
            bool didHit = false;

            RayCastResult() = default;

            RayCastResult(float d, RE::COL_LAYER l, const RE::hkVector4& n, bool h)
                : distance(d), layer(l), normalOut(n), didHit(h) {}
    };

    enum class COL_LAYER_EXTEND {
        kClimbLedge = static_cast<uint32_t>(RE::COL_LAYER::kLOS),
        kClimbObstruction = Mask_OR(RE::COL_LAYER::kLOS, RE::COL_LAYER::kTransparent),
        kVaultDown = Mask_OR(RE::COL_LAYER::kLOS, RE::COL_LAYER::kTransparent),
        kVaultForward = static_cast<uint32_t>(RE::COL_LAYER::kTransparent),
    };

    const enum ParkourKeyOptions { kJump = 0, kSprint, kActivate, k_Custom };  // k_Custom is unused for now

}  // namespace SkyParkourUtil

#define GET_PLAYER RE::PlayerCharacter::GetSingleton()
#define _THREAD_POOL SkyParkourUtil::threads
#define ZERO_VECTOR SkyParkourUtil::zeroVector
#define RayCastResult SkyParkourUtil::RayCastResult
#define COL_LAYER_EXTEND SkyParkourUtil::COL_LAYER_EXTEND
#define PARKOUR_PRESET_KEYS SkyParkourUtil::ParkourKeyOptions

#define PRINT_LAYER(x) (SkyParkourUtil::ColLayerToString(x))

#define LAYERS_CLIMB_EXCLUDE SkyParkourUtil::ClimbLayerExclusionList
#define LAYERS_VAULT_DOWN_RAY SkyParkourUtil::VaultDownRayList
#define LAYERS_VAULT_FORWARD_RAY SkyParkourUtil::VaultForwardRayList

/* Anim Events */
#define SPPF_NOTIFY "SkyParkour"
#define SPPF_START "SkyParkour_Start"
#define SPPF_STOP "SkyParkour_Stop"
#define SPPF_RECOVERY "SkyParkour_Recovery"
#define SPPF_INTERRUPT "SkyParkour_Interrupt"

/* Graph Variables */
#define SPPF_Ledge "SkyParkourLedge"
#define SPPF_Leg "SkyParkourStepLeg"
#define SPPF_ONGOING "SkyparkourOngoing"
#define SPPF_SPEEDMULT "SkyParkourSpeedMult"

/* Bool def for func */
#define IS_STOP true
#define IS_START false

/* Log switches */
#ifdef _DEBUG

//#define LOG_CLIMB
//#define LOG_VAULT
//#define LOG_STEPS
//#define LOG_LEDGE_UPDATES
//#define LOG_CROSSHAIR
//#define LOG_STEPS_VELOCITY

#endif
