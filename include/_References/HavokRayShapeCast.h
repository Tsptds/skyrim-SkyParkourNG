#pragma once
#include "API/API_Handles.h"

namespace SkyParkour
{
    struct _CastResult {
            bool didHit = false;
            float distance = -1.0f;
            RE::COL_LAYER layer = RE::COL_LAYER::kUnidentified;
            RE::hkVector4 normalOut = RE::hkVector4(0, 0, 0, 0);
            RE::NiPoint3 startPos = RE::NiPoint3(0, 0, 0);
            RE::NiPoint3 hitPos = startPos;
            RE::NiPoint3 direction = RE::NiPoint3(0, 0, 0);

            // Do a null check before using this
            RE::TESObjectREFR *hitObjectRef = nullptr;

            _CastResult() = default;
            _CastResult(bool hit, float dist, RE::COL_LAYER l, const RE::hkVector4 &normal, RE::NiPoint3 start, RE::NiPoint3 hp,
                        RE::TESObjectREFR *ref)
                : didHit(hit), distance(dist), layer(l), normalOut(normal), startPos(start), hitPos(hp), hitObjectRef(ref)
            {
            }

            // Returns form type. Will not crash if there's no ref.
            RE::FormType GetHitObjectFormType_Safe() const
            {
                if (!hitObjectRef) return RE::FormType::None;
                return hitObjectRef->GetObjectReference()->GetFormType();
            }
    };

    struct RayCastResult : _CastResult {
            RE::hkInplaceArray<RE::hkpWorldRayCastOutput, 8> hits;

            // Color, duration, thickness
            void Debug_Visualize(uint64_t color, float duration = 0.f, float thickness = 1.f)
            {
                TRUEHUD_API::IVTrueHUD4 *th = API_Handles::TrueHUD::Get();
                if (th)
                {
                    RE::NiPoint3 endPos = startPos + direction * distance;
                    th->DrawArrow(startPos, endPos, 10.f, duration, color, thickness);
                }
            }

            // Red if hit, green if not hit
            void Debug_Visualize() { Debug_Visualize(didHit ? COLOR_HEX_R : COLOR_HEX_G); }

            RayCastResult() = default;
            RayCastResult(bool hit, float dist, RE::COL_LAYER l, const RE::hkVector4 &normal, RE::NiPoint3 start, RE::NiPoint3 hp,
                          RE::TESObjectREFR *ref, RE::hkInplaceArray<RE::hkpWorldRayCastOutput, 8> arr)
                : _CastResult(hit, dist, l, normal, start, hp, ref), hits(std::move(arr))
            {
            }
    };

    struct ShapeCastResult : _CastResult {
            RE::hkInplaceArray<RE::hkpRootCdPoint, 8> hits;
            float radius = 5.f;

            // Color, duration, thickness
            void Debug_Visualize(uint64_t color, float duration = 0.f, float thickness = 1.f)
            {
                TRUEHUD_API::IVTrueHUD4 *th = API_Handles::TrueHUD::Get();
                if (th)
                {
                    RE::NiPoint3 endPos = startPos + direction * distance;
                    th->DrawCapsule(startPos, endPos, radius, duration, color, thickness);
                }
            }

            // Red if hit, green if not hit
            void Debug_Visualize() { Debug_Visualize(didHit ? COLOR_HEX_R : COLOR_HEX_G); }

            ShapeCastResult() = default;
            ShapeCastResult(bool hit, float dist, RE::COL_LAYER l, const RE::hkVector4 &normal, RE::NiPoint3 start, RE::NiPoint3 hp,
                            float rad, RE::TESObjectREFR *ref, RE::hkInplaceArray<RE::hkpRootCdPoint, 8> arr)
                : _CastResult(hit, dist, l, normal, start, hp, ref), hits(std::move(arr)), radius(rad)
            {
            }
    };

    inline RayCastResult ShapeToRayCast(const ShapeCastResult &other)
    {
        return RayCastResult(other.didHit, other.distance, other.layer, other.normalOut, other.startPos, other.hitPos, other.hitObjectRef,
                             {});
    }
}