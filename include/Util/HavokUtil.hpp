#pragma once

namespace HavokUtil {

    RE::BShkbAnimationGraphPtr GetActiveAnimGraph(RE::Actor *actor) {
        if (!actor) return nullptr;

        RE::BSAnimationGraphManagerPtr mngr;
        actor->GetAnimationGraphManager(mngr);
        if (!mngr) return nullptr;

        return mngr->graphs[mngr->GetRuntimeData().activeGraph];
    }

    RE::hkbCharacter *GetHavokCharacter(RE::Actor *actor) {
        const auto &activeGraph = GetActiveAnimGraph(actor);
        if (!activeGraph) return nullptr;

        return &activeGraph->characterInstance;
    }

    bool ValidateBehaviorPatch(RE::Actor *actor) {
        bool behaviorInstalled;

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
}  // namespace HavokUtil