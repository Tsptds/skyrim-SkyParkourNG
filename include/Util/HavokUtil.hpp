#pragma once
#include "_References/RuntimeVariables.h"
#include "_References/ModSettings.h"
#include "Parkouring.h"

namespace HavokUtil {
    // inline static float *g_gameTimeMult = (float *) RELOCATION_ID(508682, 380437).address();  // SGTM static pointer, dereference and use

    inline RayCastResult RayCast(RE::NiPoint3 rayStart, RE::NiPoint3 rayDir, float maxDist, COL_LAYER_EXTEND layerMask,
                                 RE::Actor *actor = GET_PLAYER) {
        RayCastResult result{};
        result.distance = maxDist;

        if (!actor) return result;

        const auto cell = actor->GetParentCell();
        if (!cell) return result;

        const auto bhkWorld = cell->GetbhkWorld();
        if (!bhkWorld) return result;

        RE::bhkPickData pickData;
        RE::hkpAllRayHitTempCollector collector;
        const auto havokWorldScale = RE::bhkWorld::GetWorldScale();

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

            // Deep copy hits because the collector's internal buffer is local to this function
            for (int i = 0; i < collector.hits.size(); ++i) {
                result.hits.push_back(collector.hits[i]);
            }
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
        const auto activeGraph = GetActiveAnimGraph(actor);
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
        else {
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
            SkyParkourHavokUpdate(const RE::BSFixedString &name, RE::Actor *owner) {
                const_cast<RE::BSFixedString &>(channelName) = name;
                value = std::bit_cast<uint32_t>(ModSettings::Playback_Speed);
                ChannelOwner = owner;
                LOG("Havok channel {} bound to {}, PrevRefCount: {}", name, owner->GetName(), this->_refCount);
            }
            void ResetImpl() override {
                // value = std::bit_cast<uint32_t>(ModSettings::Playback_Speed);

                correctionInitialized = false;
                correctionDone = false;
                missingGap = 0.0f;
                totalMissing = ZERO_VECTOR;
            }

            void PollChannelUpdateImpl([[maybe_unused]] bool a_arg1) override {
                if (RuntimeVariables::ParkourInProgress) {
                    if (correctionDone) return;

                    const auto cl = ChannelOwner->GetCharController();
                    RE::hkVector4 curPos;
                    cl->GetPositionImpl(curPos, true);

                    if (!correctionInitialized) {
                        /* Calculate the missing gap to adjust at every tick once */
                        RE::hkVector4 lpPos = nipoint_to_hkvector(RuntimeVariables::ledgePoint);
                        RE::hkVector4 startPos = nipoint_to_hkvector(RuntimeVariables::startPos);

                        totalMissing = startPos - curPos;

                        missingGap = totalMissing.Length3();
                        correctionInitialized = true;
                        reset = false;
                    }

                    if (missingGap > 0.1f) {
                        // Assuming Havok tick rate is 30 at all times, even if it isn't. Final movement won't change.
                        RE::hkVector4 nudge = totalMissing * 0.033333f;
                        auto timer = RE::BSTimer::GetSingleton();
                        // Scale to timer as well
                        if (timer) {
                            auto gtm = timer->QGlobalTimeMultiplier();
                            nudge = nudge * (gtm <= 0 ? 1 : gtm);
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
                    if (!reset) {
                        ResetImpl();
                        reset = true;
                    }
                }
                if (ModSettings::Parkour_Enabled) {
                    Parkouring::UpdateParkourPoint();
                }
            }

        private:
            RE::Actor *ChannelOwner{nullptr};
            bool correctionInitialized{false};
            bool correctionDone{false};
            float missingGap{};
            RE::hkVector4 totalMissing{};
            bool reset{true};
    };

    static inline void CreateBoundGraphChannels(RE::Actor *act, RE::BSAnimationGraphManagerPtr mgr = nullptr) {
        if (!act) {
            WARN("{}", "Attempted bind channel with null actor, skipped");
            return;
        }
        if (!mgr) {
            act->GetAnimationGraphManager(mgr);
        }

        if (!mgr) {
            WARN("Attempted bind channel on {} with no graph manager, skipped", act->GetName());
            return;
        }
        const auto boundChannels = mgr->boundChannels;

        RE::BSFixedString name{SPPF_SPEEDMULT};

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
            return;
        }
        LOG("{} already has channel {} bound", act->GetName(), name);
    }
    static inline bool SetBoundSpeedMult(RE::Actor *act, float value, RE::BSAnimationGraphManagerPtr mgr = nullptr) {
        if (!act) {
            WARN("{}", "Attempted to set bound value to null actor");
            return false;
        }
        if (!mgr) {
            act->GetAnimationGraphManager(mgr);
        }

        if (!mgr) {
            WARN("Attempted setting bound value on {} with no graph manager, skipped", act->GetName());
            return false;
        }
        const auto boundChannels = mgr->boundChannels;

        RE::BSFixedString name{SPPF_SPEEDMULT};

        for (auto &&i: boundChannels) {
            if (i->channelName == name) {
                i.get()->value = std::bit_cast<uint32_t>(value);
                return true;
            }
        }
        return false;
    }
}  // namespace HavokUtil