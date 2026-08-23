#pragma once
#include "_References/RuntimeVariables.h"
#include "ModSettings/ModSettings.hpp"
#include "ParkourUtility.h"
#include "_References/ParkourType.h"
#include "Parkouring.h"
#include "RE/B/bhkCapsuleShape.h"
#include "_References/Compatibility.h"
#include "CrouchSliding.h"

namespace HavokUtil
{
    // inline static float *g_gameTimeMult = (float *) RELOCATION_ID(508682, 380437).address();  // SGTM static pointer, dereference and use

    inline ShapeCastResult ShapeCast(const RE::NiPoint3 &a_start, const RE::NiPoint3 &a_dir, const float a_dist, const float a_radius,
                                     COL_LAYER_EXTEND a_layer, RE::Actor *actor = GET_PLAYER)
    {
        ShapeCastResult result{};
        result.startPos = a_start;
        result.distance = a_dist;
        result.direction = a_dir;
        result.radius = a_radius;

        RE::NiPoint3 end = a_start + a_dir * a_dist;
        result.hitPos = end;

        const RE::TESObjectCELL *cell = actor->GetParentCell();
        if (!cell) return result;

        RE::bhkWorld *bhkWorld = cell->GetbhkWorld();
        if (!bhkWorld) return result;

        RE::hkpWorld *hkWorld = bhkWorld->GetWorld1();
        if (!hkWorld) return result;

        const float scale = RE::bhkWorld::GetWorldScale();

        RE::bhkCapsuleShape *collisionCapsule =
            reinterpret_cast<RE::bhkCapsuleShape *>(RE::MemoryManager::GetSingleton()->Allocate(sizeof(RE::bhkCapsuleShape), 0, false));
        if (!collisionCapsule) return result;

        Offsets::bhkCapsuleShape_ctor(collisionCapsule);  // internally allocates + constructs the real hkpCapsuleShape (0x50 bytes)

        // Start transform, collidable, cast input
        RE::hkTransform startT{};
        startT.translation = RE::hkVector4(a_start.x * scale, a_start.y * scale, a_start.z * scale, 0);
        startT.rotation.col0 = RE::hkVector4(1, 0, 0, 0);
        startT.rotation.col1 = RE::hkVector4(0, 1, 0, 0);
        startT.rotation.col2 = RE::hkVector4(0, 0, 1, 0);

        RE::hkVector4 vertexA(0, 0, 0, 0);
        RE::hkVector4 vertexB(0, 0, 0, 0);
        Offsets::bhkCapsuleShape_SetSize(collisionCapsule, vertexA, vertexB, a_radius * scale);

        auto *capsuleShape = reinterpret_cast<RE::hkpCapsuleShape *>(collisionCapsule->referencedObject.get());

        RE::NiPointer<RE::bhkCapsuleShape> collisionCapsuleHolder(collisionCapsule);
        if (!capsuleShape) return result;

        RE::hkpCollidable coll{};
        coll.shape = capsuleShape;
        coll.shapeKey = static_cast<RE::hkpShapeKey>(-1);
        coll.motion = &startT;  // Both vertices are 0, it's a sphere. So the orientation isn't important for now.
        coll.parent = nullptr;
        coll.ownerOffset = 0;

        RE::CFilter filter{};
        actor->GetCollisionFilterInfo(filter);
        filter.SetCollisionLayer(static_cast<RE::COL_LAYER>(a_layer));
        coll.broadPhaseHandle.collisionFilterInfo = filter;
        /* This creates a new group, not needed. Use the caster actor group so it doesn't collide with self but collides with other */
        // if (auto *cf = RE::bhkCollisionFilter::GetSingleton()) filter.SetSystemGroup(cf->GetNewSystemGroup());

        RE::hkpLinearCastInput input{};
        input.to = VEC3_TO_VEC4(end) * scale;
        input.maxExtraPenetration = 0.01f;
        input.startPointTolerance = 0.01f;

        RE::hkpAllCdPointCollector *collector = Offsets::GetAllCdPointCollector(false, true);
        collector->Reset();  // This holds previous hits, do reset before a new call.

        {
            RE::BSReadLockGuard lock(bhkWorld->worldLock);
            hkWorld->LinearCast(&coll, input, *collector, nullptr);
        }

        if (collector->hits.empty()) return result;

        const RE::hkpRootCdPoint *best = nullptr;
        float bestFrac = (std::numeric_limits<float>::max)();
        for (auto &h: collector->hits)
        {
            float f = h.contact.separatingNormal.quad.m128_f32[3];  // 3rd index is the hit fraction
            if (f < bestFrac)
            {
                bestFrac = f;
                best = &h;
            }
        }
        if (!best) return result;

        const float inv = RE::bhkWorld::GetWorldScaleInverse();

        result.didHit = true;
        result.hitPos = VEC4_TO_VEC3(best->contact.position * inv);
        result.normalOut = VEC4_TO_VEC3(best->contact.separatingNormal);
        result.distance = a_dist * bestFrac;
        result.layer = best->rootCollidableB->GetCollisionLayer();  // B is the hit object
        result.hitObjectRef = RE::TESHavokUtilities::FindCollidableRef(*best->rootCollidableB);

        for (int i = 0; i < collector->hits.size(); ++i)
        {
            result.hits.push_back(collector->hits[i]);
        }

        return result;
    }

    inline RayCastResult RayCast(RE::NiPoint3 rayStart, RE::NiPoint3 rayDir, float maxDist, COL_LAYER_EXTEND layerMask,
                                 RE::Actor *actor = GET_PLAYER)
    {
        RayCastResult result{};
        result.distance = maxDist;
        result.startPos = rayStart;
        result.direction = rayDir;

        RE::NiPoint3 rayEnd = rayStart + rayDir * maxDist;
        result.hitPos = rayEnd;

        if (!actor) return result;

        const auto cell = actor->GetParentCell();
        if (!cell) return result;

        const auto bhkWorld = cell->GetbhkWorld();
        if (!bhkWorld) return result;

        RE::bhkPickData pickData;
        RE::hkpAllRayHitTempCollector collector;
        // collector.Reset();

        const auto havokWorldScale = RE::bhkWorld::GetWorldScale();

        // Set ray start and end points (scaled to Havok world)
        pickData.rayInput.from = rayStart * havokWorldScale;
        pickData.rayInput.to = rayEnd * havokWorldScale;
        pickData.allRayHitTempCollector = &collector;
        // pickData.rayInput.enableShapeCollectionFilter = true;  // Collect the shape if needed

        // Set the collision filter info to exclude the player
        /* hkpCollidable.h, lower 4 bits: CollidesWith, higher 4 bits: BelongsTo */

        //static_cast<uint32_t>(COL_LAYER::kAnimStatic) & ~static_cast<uint32_t>(COL_LAYER::kDoorDetection)

        RE::CFilter cFilter;
        actor->GetCollisionFilterInfo(cFilter);
        cFilter.SetCollisionLayer(static_cast<RE::COL_LAYER>(layerMask));
        pickData.rayInput.filterInfo = cFilter;
        // static_cast<RE::CFilter>(cFilter.filter | static_cast<uint32_t>(layerMask));

        bool picked{false};
        {
            RE::BSReadLockGuard lock(bhkWorld->worldLock);
            picked = bhkWorld->PickObject(pickData) && pickData.rayOutput.HasHit();
        }

        // Perform the raycast
        if (picked)
        {
            result.didHit = true;
            result.distance = maxDist * pickData.rayOutput.hitFraction;
            result.hitPos = rayStart + rayDir * result.distance;
            result.normalOut = pickData.rayOutput.normal;
            result.layer = pickData.rayOutput.rootCollidable->GetCollisionLayer();
            result.hitObjectRef = RE::TESHavokUtilities::FindCollidableRef(*pickData.rayOutput.rootCollidable);

            // Deep copy hits because the collector's internal buffer is local to this function
            for (int i = 0; i < collector.hits.size(); ++i)
            {
                result.hits.push_back(collector.hits[i]);
            }
        }

        return result;
    }

    inline RE::BShkbAnimationGraphPtr GetActiveAnimGraph(RE::Actor *actor)
    {
        if (!actor) return nullptr;

        RE::BSAnimationGraphManagerPtr mngr;
        actor->GetAnimationGraphManager(mngr);
        if (!mngr) return nullptr;

        return mngr->graphs[mngr->GetRuntimeData().activeGraph];
    }

    inline RE::hkbCharacter *GetHavokCharacter(RE::Actor *actor)
    {
        const auto activeGraph = GetActiveAnimGraph(actor);
        if (!activeGraph) return nullptr;

        return &activeGraph->characterInstance;
    }

    inline bool ValidateBehaviorPatch(RE::Actor *actor)
    {
        bool behaviorInstalled{false};

        bool is_FPP;
        actor->GetGraphVariableBool("IsFirstPerson", is_FPP);

        if (is_FPP)
        {
            actor->GetGraphVariableBool(SPPF_FPP_INSTALLED, behaviorInstalled);
            if (behaviorInstalled) return true;
            if (RuntimeVariables::_DidWarnMissingFPP) return false;
            RuntimeVariables::_DidWarnMissingFPP = true;

            RE::DebugMessageBox(
                "SkyParkour Behavior Missing\n\n1st Person Patch isn't generated properly, animations won't play\n\nThis is caused by user "
                "error. Your behavior output isn't generated or not overwriting everything else, don't report this as a bug\n\nCheck "
                "SkyParkourNG.log file for more details.");

            ERROR(
                "1st person behavior missing: The 0_master.hkx file is not properly patched to include proper SkyParkour transitions.\n"
                "Ensure Nemesis/Pandora generates the patches animationdatasinglefile.txt and animationsetdatasinglefile.txt under meshes, "
                "and 0_master.hkx under meshes/actors/character/_1stperson/behaviors\n"
                "Ensure these files are not overwritten by other mods");
        }
        else
        {
            actor->GetGraphVariableBool(SPPF_TPP_INSTALLED, behaviorInstalled);
            if (behaviorInstalled) return true;
            if (RuntimeVariables::_DidWarnMissingTPP) return false;

            RuntimeVariables::_DidWarnMissingTPP = true;
            RE::DebugMessageBox(
                "SkyParkour Behavior Missing\n\n3rd Person Patch isn't generated properly, animations won't play\n\nThis is caused by user "
                "error. Your behavior output isn't generated or not overwriting everything else, don't report this as a bug\n\nCheck "
                "SkyParkourNG.log file for more details.");

            ERROR(
                "3rd person behavior missing: The 0_master.hkx file is not properly patched to include proper SkyParkour transitions.\n"
                "Ensure Nemesis/Pandora generates the patches animationdatasinglefile.txt and animationsetdatasinglefile.txt under meshes, "
                "and 0_master.hkx under meshes/actors/character/behaviors\n"
                "Ensure these files are not overwritten by other mods");
        }
        return false;
    }

    class SkyParkourHavokUpdate : public RE::BSAnimationGraphChannel {
        public:
            SkyParkourHavokUpdate(const RE::BSFixedString &name, RE::Actor *owner)
            {
                const_cast<RE::BSFixedString &>(channelName) = name;
                value = std::bit_cast<uint32_t>(ModSettings::Playback_Speed);
                ChannelOwner = owner;
                INFO("Havok channel {} bound to {}, PrevRefCount: {}", name, owner->GetName(), this->_refCount);
            }
            void ResetImpl() override
            {
                // value = std::bit_cast<uint32_t>(ModSettings::Playback_Speed);

                correctionInitialized = false;
                correctionDone = false;
                missingGap = 0.0f;
                totalMissing = 0;

                // angleCorrectionInitialized = false;
                // angleCorrectionDone = false;
                // missingAngle = 0.0f;
            }

            void PollChannelUpdateImpl([[maybe_unused]] bool a_arg1) override
            {
                if (RuntimeVariables::ParkourInProgress)
                {
                    if (correctionDone /*&& angleCorrectionDone*/) return;

                    const auto cl = ChannelOwner->GetCharController();
                    RE::hkVector4 curPos;
                    cl->GetPositionImpl(curPos, true);

                    if (!correctionInitialized)
                    {
                        /* Calculate the missing gap to adjust at every tick once */
                        RE::hkVector4 lpPos = nipoint_to_hkvector(RuntimeVariables::ledgePoint);
                        RE::hkVector4 startPos = nipoint_to_hkvector(RuntimeVariables::startPos);

                        totalMissing = startPos - curPos;

                        missingGap = totalMissing.Length3();
                        correctionInitialized = true;
                        reset = false;
                    }

                    // if (!angleCorrectionInitialized &&
                    //     std::to_underlying(RuntimeVariables::selectedLedgeType) >= std::to_underlying(ParkourType::Low))
                    // {
                    //     RE::NiPoint3 dir = ParkourUtility::GetActorDirFlat(ChannelOwner);
                    //     RayCastResult angleRay =
                    //         HavokUtil::RayCast(RuntimeVariables::ledgePoint - dir * 5.f, dir, 10.f, COL_LAYER_EXTEND::kClimbObstruction);

                    //     [[unlikely]] if (ModSettings::_Debug_Enabled) { angleRay.Debug_HitPosArrow(COLOR_HEX_Y, 4.f); }

                    //     if (angleRay.didHit)
                    //     {
                    //         {
                    //             float x = angleRay.normalOut.quad.m128_f32[0];
                    //             float y = angleRay.normalOut.quad.m128_f32[1];

                    //             targetAngleZ = std::atan2(-x, -y);
                    //         }

                    //         float curAngleZ = ChannelOwner->data.angle.z;

                    //         missingAngle = targetAngleZ - curAngleZ;
                    //         // normalize to shortest path [-pi, pi]
                    //         missingAngle = fmodf(missingAngle + RE::NI_PI, RE::NI_TWO_PI);
                    //         if (missingAngle < 0.0f) missingAngle += RE::NI_TWO_PI;
                    //         missingAngle -= RE::NI_PI;
                    //     }
                    //     else
                    //     {
                    //         missingAngle = 0;
                    //     }
                    //     angleCorrectionInitialized = true;
                    // }

                    if (!correctionDone && missingGap > 0.1f)
                    {
                        // Assuming Havok tick rate is 30 at all times, even if it isn't. Final movement won't change.
                        RE::hkVector4 nudge = totalMissing * 0.033333f;
                        auto timer = RE::BSTimer::GetSingleton();
                        // Scale to timer as well
                        if (timer)
                        {
                            auto gtm = timer->QGlobalTimeMultiplier();
                            nudge = nudge * (gtm <= 0 ? 1 : gtm);
                        }

                        // Set new pos as current pos + the missing bit.
                        RE::hkVector4 newPos = curPos + nudge;

                        cl->SetPositionImpl(newPos, true, false);
                        missingGap -= nudge.Length3();
                    }
                    else
                    {
                        correctionDone = true;
                    }

                    // if (!angleCorrectionDone)
                    // {
                    //     if (fabsf(missingAngle) > 0.001f)
                    //     {
                    //         float angleNudge = missingAngle * RE::GetSecondsSinceLastFrame();
                    //         auto timer = RE::BSTimer::GetSingleton();
                    //         if (timer)
                    //         {
                    //             auto gtm = timer->QGlobalTimeMultiplier();
                    //             angleNudge *= (gtm <= 0 ? 1 : gtm);
                    //         }
                    //         angleNudge = std::clamp(angleNudge, -fabsf(missingAngle), fabsf(missingAngle));

                    //         float newAngleZ = ChannelOwner->data.angle.z + angleNudge;
                    //         // wrap to [-pi, pi]
                    //         newAngleZ = fmodf(newAngleZ + RE::NI_PI, RE::NI_TWO_PI);
                    //         if (newAngleZ < 0.0f) newAngleZ += RE::NI_TWO_PI;
                    //         newAngleZ -= RE::NI_PI;

                    //         ChannelOwner->data.angle.z = newAngleZ;
                    //         missingAngle -= angleNudge;

                    //         // Counter-rotate the camera's free-look offset so its world yaw doesn't change
                    //         if (ChannelOwner->IsPlayerRef())
                    //         {
                    //             auto cam = RE::PlayerCamera::GetSingleton();
                    //             if (cam && cam->currentState)
                    //             {
                    //                 if (auto tps = skyrim_cast<RE::ThirdPersonState *>(cam->currentState.get()))
                    //                 {
                    //                     tps->freeRotation.x -= angleNudge;
                    //                 }
                    //             }
                    //         }
                    //     }
                    //     else
                    //     {
                    //         angleCorrectionDone = true;
                    //     }
                    // }
                }
                else
                {
                    if (!reset)
                    {
                        ResetImpl();
                        reset = true;
                    }
                }
                if (ModSettings::Parkour_Enabled)
                {
                    Parkouring::UpdateParkourPoint();
                }

                if (Compatibility::TrueDirectionalMovement::found)
                {
                    if (!API_Handles::TDM::IsLockedOn())
                    {
                        if (RuntimeVariables::SlideOngoing)
                        {
                            API_Handles::TDM::SyncTppYaw();
                        }
                    }
                }

                if (ModSettings::ExpSlideTackle)
                {
                    if (RuntimeVariables::SlideOngoing) CrouchSliding::TryKnockCollidedActor(ChannelOwner);
                }
            }

        private:
            RE::Actor *ChannelOwner{nullptr};
            bool correctionInitialized{false};
            bool correctionDone{false};
            float missingGap{};
            RE::hkVector4 totalMissing{};
            bool reset{true};

            // bool angleCorrectionInitialized{false};
            // bool angleCorrectionDone{false};
            // float targetAngleZ{};
            // float missingAngle{};
    };

    static inline void CreateBoundGraphChannels(RE::Actor *act, RE::BSAnimationGraphManagerPtr mgr = nullptr)
    {
        if (!act)
        {
            WARN("{}", "Attempted bind channel with null actor, skipped");
            return;
        }
        if (!mgr)
        {
            act->GetAnimationGraphManager(mgr);
        }

        if (!mgr)
        {
            WARN("Attempted bind channel on {} with no graph manager, skipped", act->GetName());
            return;
        }
        const auto boundChannels = mgr->boundChannels;

        RE::BSFixedString name{SPPF_SPEEDMULT};

        bool alreadyHas{false};

        for (auto &&i: boundChannels)
        {
            if (i->channelName == name)
            {
                alreadyHas = true;
                break;
            }
        }

        if (!alreadyHas)
        {
            /* I think smart pointer will prevent memory leak. 97% sure. */
            auto channel = new SkyParkourHavokUpdate(name, act);
            RE::BSTSmartPointer<RE::BSAnimationGraphChannel> ptr(channel);

            mgr->boundChannels.push_back(ptr);
            return;
        }
        INFO("{} already has channel {} bound", act->GetName(), name);
    }
    static inline bool SetBoundSpeedMult(RE::Actor *act, float value, RE::BSAnimationGraphManagerPtr mgr = nullptr)
    {
        if (!act)
        {
            WARN("{}", "Attempted to set bound value to null actor");
            return false;
        }
        if (!mgr)
        {
            act->GetAnimationGraphManager(mgr);
        }

        if (!mgr)
        {
            WARN("Attempted setting bound value on {} with no graph manager, skipped", act->GetName());
            return false;
        }
        const auto boundChannels = mgr->boundChannels;

        RE::BSFixedString name{SPPF_SPEEDMULT};

        for (auto &&i: boundChannels)
        {
            if (i->channelName == name)
            {
                i.get()->value = std::bit_cast<uint32_t>(value);
                return true;
            }
        }
        return false;
    }
}  // namespace HavokUtil