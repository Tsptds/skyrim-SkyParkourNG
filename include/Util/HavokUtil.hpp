#pragma once
#include "_References/RuntimeVariables.h"

namespace HavokUtil {
    inline static float *g_gameTimeMult = (float *) RELOCATION_ID(508682, 380437).address();  // SGTM static pointer, dereference and use

    inline RayCastResult RayCast(RE::NiPoint3 rayStart, RE::NiPoint3 rayDir, float maxDist, COL_LAYER_EXTEND layerMask,
                                 RE::Actor *actor = GET_PLAYER) {
        RayCastResult result{};
        result.distance = maxDist;

        if (!actor) return result;

        const auto &cell = actor->GetParentCell();
        if (!cell) return result;

        const auto &bhkWorld = cell->GetbhkWorld();
        if (!bhkWorld) return result;

        RE::bhkPickData pickData;
        RE::hkpAllRayHitTempCollector collector;
        const auto &havokWorldScale = RE::bhkWorld::GetWorldScale();

        // Set ray start and end points (scaled to Havok world)
        pickData.rayInput.from = rayStart * havokWorldScale;
        pickData.rayInput.to = (rayStart + rayDir * maxDist) * havokWorldScale;
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

        // Perform the raycast
        if (bhkWorld->PickObject(pickData) && pickData.rayOutput.HasHit()) {
            result.didHit = true;
            result.distance = maxDist * pickData.rayOutput.hitFraction;
            result.normalOut = pickData.rayOutput.normal;
            result.layer = pickData.rayOutput.rootCollidable->GetCollisionLayer();
            result.hitObjectRef = RE::TESHavokUtilities::FindCollidableRef(*pickData.rayOutput.rootCollidable);

            result.hits = pickData.allRayHitTempCollector->hits;
        }

        return result;
    }

    inline RE::BShkbAnimationGraphPtr GetActiveAnimGraph(RE::Actor *actor) {
        if (!actor) return nullptr;

        RE::BSAnimationGraphManagerPtr mngr;
        actor->GetAnimationGraphManager(mngr);
        if (!mngr) return nullptr;

        return mngr->graphs[mngr->GetRuntimeData().activeGraph];
    }

    inline RE::hkbCharacter *GetHavokCharacter(RE::Actor *actor) {
        const auto &activeGraph = GetActiveAnimGraph(actor);
        if (!activeGraph) return nullptr;

        return &activeGraph->characterInstance;
    }

    inline bool ValidateBehaviorPatch(RE::Actor *actor) {
        bool behaviorInstalled{false};

        bool is_FPP;
        actor->GetGraphVariableBool("IsFirstPerson", is_FPP);

        if (is_FPP) {
            actor->GetGraphVariableBool(SPPF_FPP_INSTALLED, behaviorInstalled);
            if (behaviorInstalled) return true;

            RE::DebugMessageBox(
                "SkyParkour Warning\n\n1st Person Behavior is not generated properly\nAnimations will not play\n\nThis is caused by user "
                "error. Your behavior output isn't generated or not overwriting everything else, don't report this as a bug");
        }
        else {
            actor->GetGraphVariableBool(SPPF_TPP_INSTALLED, behaviorInstalled);
            if (behaviorInstalled) return true;

            RE::DebugMessageBox(
                "SkyParkour Warning\n\n3rd Person Behavior is not generated properly\nAnimations will not play\n\nThis is caused by user "
                "error. Your behavior output isn't generated or not overwriting everything else, don't report this as a bug");
        }
        return false;
    }

    class SkyParkourHavokUpdate : public RE::BSAnimationGraphChannel {
        public:
            SkyParkourHavokUpdate(const RE::BSFixedString &name, RE::Actor *owner) {
                const_cast<RE::BSFixedString &>(channelName) = name;
                value = std::bit_cast<uint32_t>(1.0f);
                ChannelOwner = owner;
            }
            void ResetImpl() override {
                value = 0;
            }

            void PollChannelUpdateImpl([[maybe_unused]] bool a_arg1) override {
                if (RuntimeVariables::ParkourInProgress) {
                    if (correctionDone) return;

                    const auto &cl = ChannelOwner->GetCharController();
                    RE::hkVector4 curPos;
                    cl->GetPositionImpl(curPos, true);

                    if (!correctionInitialized) {
                        /* Calculate the missing gap to adjust at every tick once */
                        RE::hkVector4 lpPos = nipoint_to_hkvector(RuntimeVariables::ledgePoint);
                        RE::hkVector4 startPos = nipoint_to_hkvector(RuntimeVariables::startPos);

                        totalMissing = startPos - curPos;

                        missingGap = totalMissing.Length3();
                        correctionInitialized = true;
                    }

                    if (missingGap > 0.1f) {
                        // Assuming Havok tick rate is 30 at all times, even if it isn't. Final movement won't change.
                        RE::hkVector4 nudge = totalMissing * 0.033333f;

                        // Scale to GTM as well
                        if (g_gameTimeMult) {
                            auto SGTM = *g_gameTimeMult;
                            nudge = nudge * (SGTM <= 0 ? 1 : SGTM);
                        }

                        // Set new pos as current pos + the missing bit.
                        RE::hkVector4 newPos = curPos + nudge;

                        cl->SetPositionImpl(newPos, true, false);
                        missingGap -= nudge.Length3();
                    }
                    else {
                        correctionDone = true;
                    }
                }
                else {
                    correctionInitialized = false;
                    correctionDone = false;
                    missingGap = 0.0f;
                    totalMissing = ZERO_VECTOR;
                }
            }

        private:
            RE::Actor *ChannelOwner{nullptr};
            bool correctionInitialized{false};
            bool correctionDone{false};
            float missingGap{};
            RE::hkVector4 totalMissing{};
    };

    static inline void CreateBoundGraphChannels(RE::Actor *act) {
        if (!act) {
            WARN("{}", "Attempted bind channel with null actor, skipped");
            return;
        }
        RE::BSAnimationGraphManagerPtr mgr;
        act->GetAnimationGraphManager(mgr);
        if (!mgr) {
            WARN("Attempted bind channel on {} with no graph manager, skipped", act->GetName());
            return;
        }
        const auto &boundChannels = mgr->boundChannels;

        RE::BSFixedString name{"SkyParkour_PreAdjustState"};

        bool alreadyHas{false};

        for (auto &&i: boundChannels) {
            if (i->channelName == name) {
                alreadyHas = true;
                break;
            }
        }

        if (!alreadyHas) {
            /* I think smart pointer will prevent memory leak. 97% sure. */
            auto channel = new SkyParkourHavokUpdate(name, act);
            RE::BSTSmartPointer<RE::BSAnimationGraphChannel> ptr(channel);

            mgr->boundChannels.push_back(ptr);
            LOG("Havok channel {} bound to {}", name, act->GetName());
            return;
        }
        LOG("{} already has channel {} bound", act->GetName(), name);
    }
}  // namespace HavokUtil