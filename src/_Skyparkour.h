#pragma once

#include "_References/BehaviorGraph.h"
#include "_References/ExclusionLists.h"
#include "_References/HavokRayShapeCast.h"
#include "_References/Fmt.h"

namespace SkyParkour
{
    const enum PresetKeys { kJump = 0, kSprint, kActivate, kTotalKeys };
    const enum AutoParkourOptions { kDisabled = 0, kNonCombatOnly, kAlways, kTotalOpts };

    static RE::NiPoint3 Vec4_To_Vec3(RE::hkVector4 vec) { return {vec.quad.m128_f32[0], vec.quad.m128_f32[1], vec.quad.m128_f32[2]}; }

    static hkVector4 Vec3_To_Vec4(NiPoint3 vec) { return {vec.x, vec.y, vec.z, 0}; }
}  // namespace SkyParkour

/* Log macros */
using TRACE = SKSE::log::trace;
using INFO = SKSE::log::info;
using WARN = SKSE::log::warn;
using ERROR = SKSE::log::error;
using CRITICAL = SKSE::log::critical;

/* Task Queue Macro */
#define _TASK_Q SKSE::GetTaskInterface()->AddTask

/* Generic Stuff */
#define GET_PLAYER RE::PlayerCharacter::GetSingleton()

#define VEC3_TO_VEC4 SkyParkour::Vec3_To_Vec4               // Only assign the properties
#define VEC4_TO_VEC3 SkyParkour::Vec4_To_Vec3               // Only assign the properties
#define nipoint_to_hkvector(a) VEC3_TO_VEC4(a / 69.99125f)  // Scale from ni to havok
#define hkvector_to_nipoint(a) VEC4_TO_VEC3(a * 69.99125f)  // Scale from havok to ni

#define RayCastResult SkyParkour::RayCastResult
#define ShapeCastResult SkyParkour::ShapeCastResult
using namespace BehaviorGraph;