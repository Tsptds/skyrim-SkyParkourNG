#include "Parkouring.h"
#include "Util/ParkourUtility.h"
#include "Listeners/ButtonListener.h"
#include "Listeners/MenuListener.h"
#include "Util/ScaleUtility.h"

#include "_References/GameReferences.h"
#include "_References/ModSettings.h"
#include "_References/ParkourType.h"
#include "_References/RuntimeVariables.h"
#include "_References/HardcodedVariables.h"
#include "_References/RuntimeMethods.h"

using namespace ParkourUtility;

int Parkouring::GetLedgePoint() {
    using namespace GameReferences;
    using namespace ModSettings;

    const auto player = GET_PLAYER;
    //const auto playerPos = player->GetPosition();

    RE::NiPoint3 playerDirFlat = GetPlayerDirFlat(player);

    // Perform ledge or vault checks
    int selectedLedgeType = ParkourType::NoLedge;
    RE::NiPoint3 ledgePoint;

    constexpr int vaultLength = 120;
    constexpr int maxElevationIncrease = 80;

    selectedLedgeType = VaultCheck(ledgePoint, playerDirFlat, vaultLength, maxElevationIncrease * RuntimeVariables::PlayerScale,
                                   HardCodedVariables::vaultMinHeight * RuntimeVariables::PlayerScale,
                                   HardCodedVariables::vaultMaxHeight * RuntimeVariables::PlayerScale);

    if (selectedLedgeType == ParkourType::NoLedge) {
        selectedLedgeType = ClimbCheck(ledgePoint, playerDirFlat, HardCodedVariables::climbMinHeight * RuntimeVariables::PlayerScale,
                                       HardCodedVariables::climbMaxHeight * RuntimeVariables::PlayerScale);
    }
    if (selectedLedgeType == ParkourType::NoLedge) {
        return ParkourType::NoLedge;
    }

    // Don't ever parkour into water, last check before saying this ledge is valid
    float waterLevel;
    player->GetParentCell()->GetWaterHeight(player->GetPosition(), waterLevel);  //Relative to player

    constexpr int validWaterDepth = 10;

    if (ledgePoint.z < waterLevel - validWaterDepth) {
        return ParkourType::NoLedge;
    }

    RuntimeVariables::ledgePoint = ledgePoint;
    RuntimeVariables::playerDirFlat = playerDirFlat;

    return selectedLedgeType;
}

int Parkouring::ClimbCheck(RE::NiPoint3 &ledgePoint, RE::NiPoint3 checkDir, float minLedgeHeight, float maxLedgeHeight) {
    const auto &player = GET_PLAYER;
    const auto &playerPos = player->GetPosition();

    // Constants adjusted for player scale
    const float startZOffset = 100 * RuntimeVariables::PlayerScale;
    const float playerHeight = 120 * RuntimeVariables::PlayerScale;
    const float minUpCheck = 100 * RuntimeVariables::PlayerScale;
    const float maxUpCheck = (maxLedgeHeight - startZOffset) + 20 * RuntimeVariables::PlayerScale;
    const float fwdCheckStep = 5 * RuntimeVariables::PlayerScale;  // 8
    const int fwdCheckIterations = 15;                             // 15
    const float minLedgeFlatness = 0.5;                            //0.5

    // Raycast above player, is there enough room
    RE::NiPoint3 upRayStart = playerPos + RE::NiPoint3(0, 0, startZOffset);
    RE::NiPoint3 upRayDir(0, 0, 1);

    RayCastResult upRay = RayCast(upRayStart, upRayDir, maxUpCheck, COL_LAYER_EXTEND::kClimbObstruction);

    if (upRay.distance < minUpCheck) {
        return ParkourType::NoLedge;
    }

    // Forward raycast initialization
    RE::NiPoint3 fwdRayStart = upRayStart + upRayDir * (upRay.distance - 10);
    RE::NiPoint3 ledgeRayDir(0, 0, -1);

    RayCastResult ledgeRay;
    bool foundLedge = false;
    float normalZ = 0;

    // Incremental forward raycast to find a ledge
    for (int i = 0; i < fwdCheckIterations; i++) {
        RayCastResult fwdRay = RayCast(fwdRayStart, checkDir, fwdCheckStep * i, COL_LAYER_EXTEND::kClimbObstruction);

#ifdef LOG_CLIMB
        LOG("Ledge FWD: {}", PRINT_LAYER(fwdRay.layer));
#endif

        if (fwdRay.distance < fwdCheckStep * i) {
            continue;
        }

        // Downward raycast to detect ledge point
        RE::NiPoint3 ledgeRayStart = fwdRayStart + checkDir * fwdRay.distance;
        ledgeRay = RayCast(ledgeRayStart, ledgeRayDir, startZOffset + maxUpCheck, COL_LAYER_EXTEND::kClimbLedge);

#ifdef LOG_CLIMB
        LOG("Ledge Down: {}", PRINT_LAYER(ledgeRay.layer));
#endif

        if (LAYERS_CLIMB_EXCLUDE.contains(ledgeRay.layer)) {
            continue;
        }

        ledgePoint = ledgeRayStart + ledgeRayDir * ledgeRay.distance;
        normalZ = ledgeRay.normalOut.quad.m128_f32[2];

        // Validate ledge based on height and flatness
        if (ledgePoint.z < playerPos.z + minLedgeHeight || ledgePoint.z > playerPos.z + maxLedgeHeight || ledgeRay.distance < 10 ||
            normalZ < minLedgeFlatness) {
            continue;
        }

        // Check for obstructions behind the Ledge point
        RE::NiPoint3 obstructionCheckStart = fwdRayStart + checkDir * (fwdRay.distance - 2) + RE::NiPoint3(0, 0, 5);
        const float minSpaceRequired = 15.0f * RuntimeVariables::PlayerScale;
        RayCastResult obsRay = RayCast(obstructionCheckStart, checkDir, minSpaceRequired, COL_LAYER_EXTEND::kClimbObstruction);

        if (obsRay.didHit && obsRay.distance < minSpaceRequired) {
            continue;  // Obstruction behind the ledge point
        }

        foundLedge = true;
        break;
    }

    if (!foundLedge) {
        return ParkourType::NoLedge;
    }

    // Ensure there is sufficient headroom for the player to stand
    const float headroomBuffer = 10 * RuntimeVariables::PlayerScale;
    const float headroomPlayerDiff = playerHeight - headroomBuffer;
    RE::NiPoint3 headroomRayStart = ledgePoint + upRayDir * headroomBuffer;
    RayCastResult headroomRay = RayCast(headroomRayStart, upRayDir, headroomPlayerDiff, COL_LAYER_EXTEND::kClimbObstruction);

    if (headroomRay.distance < headroomPlayerDiff) {
        return ParkourType::NoLedge;
    }

    const float ledgePlayerDiff = ledgePoint.z - playerPos.z;
    if (IsSupportGroundedOrSliding(player) || PlayerIsSwimming()) {
        if (ledgePlayerDiff >= HardCodedVariables::highestLedgeLimit * RuntimeVariables::PlayerScale) {
            if (ShouldReplaceMarkerWithFailed()) {
                return ParkourType::Failed;
            }
            return ParkourType::Highest;  // Highest ledge
        }
        else if (ledgePlayerDiff >= HardCodedVariables::highLedgeLimit * RuntimeVariables::PlayerScale) {
            if (ShouldReplaceMarkerWithFailed()) {
                return ParkourType::Failed;
            }
            return ParkourType::High;  // High ledge
        }
        else if (ledgePlayerDiff >= HardCodedVariables::medLedgeLimit * RuntimeVariables::PlayerScale) {
            return ParkourType::Medium;  // Medium ledge
        }
        else if (ledgePlayerDiff >= HardCodedVariables::lowLedgeLimit * RuntimeVariables::PlayerScale) {
            if (PlayerIsSwimming()) {
                return ParkourType::Grab;  // Grab ledge out of water
            }

            return ParkourType::Low;  // Low ledge
        }
        else if (ledgePlayerDiff >= HardCodedVariables::highStepLimit * RuntimeVariables::PlayerScale) {
            if (PlayerIsSwimming()) {
                return ParkourType::Grab;  // Grab ledge out of water
            }

            if (StepsExtraChecks(player, ledgeRay)) {
                return ParkourType::StepHigh;  // High Step
            }
        }
        else {
            if (PlayerIsSwimming()) {
                return ParkourType::Grab;  // Grab ledge out of water, don't step out
            }

            if (StepsExtraChecks(player, ledgeRay)) {
                return ParkourType::StepLow;  // Low Step
            }
        }
    }
    else if (IsSupportUnsupported(player) && GrabExtraChecks(ledgePlayerDiff, ledgeRay)) {
        return ParkourType::Grab;
    }
    return ParkourType::NoLedge;
}
int Parkouring::VaultCheck(RE::NiPoint3 &ledgePoint, RE::NiPoint3 checkDir, float vaultLength, float maxElevationIncrease,
                           float minVaultHeight, float maxVaultHeight) {
    const auto player = GET_PLAYER;

    if (!IsSupportGrounded(player)) {
        return ParkourType::NoLedge;
    }

    if (!VaultExtraChecks(player)) {
        return ParkourType::NoLedge;
    }

    const auto playerPos = player->GetPosition();
    float headHeight = 120 * RuntimeVariables::PlayerScale;

    /* Forward raycast to check if there is an obstruction at head level in vaultLength */

    RE::NiPoint3 fwdRayStart = playerPos + RE::NiPoint3(0, 0, headHeight);
    float minSpaceRequired = 2 * vaultLength * RuntimeVariables::PlayerScale;

    RayCastResult fwdRay = RayCast(fwdRayStart, checkDir, minSpaceRequired, COL_LAYER_EXTEND::kVaultForward);
#ifdef LOG_VAULT
    LOG("Vault FWD: {}", PRINT_LAYER(fwdRay.layer));
#endif

    if (fwdRay.didHit && fwdRay.distance < minSpaceRequired && LAYERS_VAULT_FORWARD_RAY.contains(fwdRay.layer)) {
        return ParkourType::NoLedge;  // Obstruction behind the vaultable surface
    }

    /* Move forward by this steps, and RayCast downwards. If a valid layer is found, mark it. */
    int downIterations = 20;
    RE::NiPoint3 downRayDir(0, 0, -1);
    RayCastResult downRay;

    bool foundVaulter = false;
    float foundVaultHeight = -10000.0f;
    bool foundLanding = false;
    float foundLandingHeight = 10000.0f;
    float vaultableGap = headHeight + 100.0f * RuntimeVariables::PlayerScale;

    // Incremental downward raycasts
    for (int i = 0; i < downIterations; i++) {
        float iDist = static_cast<float>(i) * 5.0f;
        RE::NiPoint3 downRayStart = playerPos + checkDir * iDist;
        downRayStart.z = fwdRayStart.z;

        downRay = RayCast(downRayStart, downRayDir, vaultableGap, COL_LAYER_EXTEND::kVaultDown);

        float hitHeight = (fwdRayStart.z - downRay.distance) - playerPos.z;

        // Check hit height for vaultable surfaces
        if (hitHeight > maxVaultHeight) {
            return ParkourType::NoLedge;  // Too high to vault
        }
        else if (hitHeight > minVaultHeight && hitHeight < maxVaultHeight) {
            if (hitHeight >= foundVaultHeight) {
                foundVaultHeight = hitHeight;
                foundLanding = false;
            }
            ledgePoint = downRayStart + downRayDir * downRay.distance;
            foundVaulter = true;
#ifdef LOG_VAULT
            LOG("Vault Down: {}", PRINT_LAYER(downRay.layer));
#endif
        }
        else if (foundVaulter && hitHeight < minVaultHeight) {
            foundLandingHeight = std::min(hitHeight, foundLandingHeight);
            foundLanding = true;
        }
    }

    // Check if the structure is like a railing by casting an upwards ray on the valid ledge
    const RE::NiPoint3 upRayDir(0, 0, 1);
    const float halfPlayerHeight = headHeight * 0.5f;
    const RayCastResult upRay = RayCast(ledgePoint, upRayDir, halfPlayerHeight, COL_LAYER_EXTEND::kVaultUp);

    if (upRay.didHit) {
        return ParkourType::NoLedge;
    }

    // Final validation for vault
    if (foundVaulter && foundLanding && foundLandingHeight < maxElevationIncrease) {
        ledgePoint.z = playerPos.z + foundVaultHeight;

        return ParkourType::Vault;
    }

    return ParkourType::NoLedge;  // Vault failed
}

void Parkouring::OnStartStop(bool isStop) {
    // IS_START true / IS_STOP false

    const auto &player = GET_PLAYER;
    const auto &ctrl = player->GetCharController();

    if (isStop) {
        ctrl->flags.reset(RE::CHARACTER_FLAGS::kNoSim);
        player->SetGraphVariableInt(SPPF_Ledge, ParkourType::NoLedge);

        // The other graph doesn't see the current graph, interrupt on stop to notify all
        // DO NOT SEND SPPF_STOP OR IT WILL RECURSE INFINITELY, STACK OVERFLOW AND CRASH
        player->NotifyAnimationGraph(SPPF_INTERRUPT);

        RuntimeVariables::RecoveryFramesActive = false;
        RuntimeVariables::ParkourInProgress = false;
    }
    else /* if isStart */ {
        ParkourUtility::StopInteractions(*player);

        // Disable simulation, fixes char controller taking over on hit
        ctrl->flags.set(RE::CHARACTER_FLAGS::kNoSim);
    }

    const auto &ctrlMap = RE::ControlMap::GetSingleton();
    ctrlMap->ToggleControls(RE::ControlMap::UEFlag::kMainFour, isStop);  // Player tab menu & equip. Gets stuck if player uses TFC.
}

bool Parkouring::PlaceAndShowIndicator() {
    const bool useIndicators = ModSettings::Use_Indicators;
    if (!useIndicators) {
        return false;
    }

    const bool enableStamina = ModSettings::Enable_Stamina_Consumption;
    const bool hasStamina = PlayerHasEnoughStamina();
    const auto ledgeType = RuntimeVariables::selectedLedgeType;

    const bool useRed = useIndicators && enableStamina && !hasStamina && !CheckActionRequiresLowEffort(ledgeType);

    auto blueRef = GameReferences::indicatorRef_Blue;
    auto redRef = GameReferences::indicatorRef_Red;
    if (!blueRef || !redRef) {
        return false;
    }

    auto &currentRef = GameReferences::currentIndicatorRef;
    currentRef = useRed ? redRef : blueRef;

    if (useRed) {
        if (blueRef)
            blueRef->Disable();
    }
    else {
        if (redRef)
            redRef->Disable();
    }

    if (!currentRef) {
        return false;
    }

    const auto &player = GET_PLAYER;
    if (!player) {
        return false;
    }

    const auto &playerCell = player->GetParentCell();
    if (!playerCell) {
        return false;
    }

    if (currentRef->GetParentCell() != playerCell) {
        currentRef->SetParentCell(playerCell);
    }

    currentRef->SetPosition(RuntimeVariables::ledgePoint + RE::NiPoint3(0, 0, 8.0f));
    currentRef->Update3DPosition(true);

    if (RuntimeVariables::IsParkourActive) {
        currentRef->Enable(false);
    }
    else {
        currentRef->Disable();
    }

    return true;
}

void Parkouring::InterpolateRefToPosition(const RE::Actor *movingRef, RE::NiPoint3 to, float seconds) {
    auto vm = RE::BSScript::Internal::VirtualMachine::GetSingleton();
    if (!vm) {
        return;
    }

    /* Calculate speed from cur pos to target dist / time. BUT Read annotations relative to start position. */
    auto curPos = movingRef->GetPosition();
    RE::NiPoint3 relativeTranslatedToWorld = to;

    const auto diff = relativeTranslatedToWorld - curPos;

    float mult;
    movingRef->GetGraphVariableFloat(SPPF_SPEEDMULT, mult);
    if (mult <= 0.0f) {
        mult = 1.0f;
    }

    auto speed = seconds <= 0 ? 5000 : diff.Length() / seconds;  // Snap to pos if 0 or negative seconds
    speed *= mult;

    // Wrap movingRef in a Papyrus handle
    auto policy = vm->GetObjectHandlePolicy();
    RE::VMHandle handle = policy->GetHandleForObject(movingRef->GetFormType(), movingRef);
    if (handle == policy->EmptyHandle()) {
        return;
    }

    // Lookup the Papyrus-bound "ObjectReference" instance
    RE::BSFixedString scriptName = "ObjectReference";
    RE::BSFixedString functionName =
        "TranslateTo";  // For SplineTranslateTo, add std::move(float) between rz and speed. Does an overshoot, and pullback. Sometimes too strong.

    RE::BSTSmartPointer<RE::BSScript::Object> object;
    if (!vm->FindBoundObject(handle, scriptName.c_str(), object)) {
        return;
    }

    float px = relativeTranslatedToWorld.x;
    float py = relativeTranslatedToWorld.y;
    float pz = relativeTranslatedToWorld.z;
    float rx = movingRef->data.angle.x;
    float ry = movingRef->data.angle.y;
    float rz = movingRef->data.angle.z;
    float maxRotSpeed = 0.0f;

    // Build the IFunctionArguments with those locals:
    auto args = RE::MakeFunctionArguments(std::move(px),  // afX
                                          std::move(py),  // afY
                                          std::move(pz),  // afZ
                                          std::move(rx),  // afRX
                                          std::move(ry),  // afRY
                                          std::move(rz),  // afRZ
                                          //std::move(100.0f),
                                          std::move(speed), std::move(maxRotSpeed));

    StopInterpolatingRef(movingRef);

    // Call the Papyrus method
    RE::BSTSmartPointer<RE::BSScript::IStackCallbackFunctor> result;
    vm->DispatchMethodCall1(object,        // the Papyrus ObjectReference instance
                            functionName,  // "TranslateTo"
                            args,          // packed arguments
                            result);
}
void Parkouring::StopInterpolatingRef(const RE::Actor *actor) {
    auto movingRef = actor;
    auto vm = RE::BSScript::Internal::VirtualMachine::GetSingleton();
    if (!vm) {
        return;
    }
    auto policy = vm->GetObjectHandlePolicy();
    RE::VMHandle handle = policy->GetHandleForObject(movingRef->GetFormType(), movingRef);
    if (handle == policy->EmptyHandle()) {
        return;
    }

    RE::BSFixedString scriptName = "ObjectReference";
    RE::BSFixedString functionName = "StopTranslation";

    RE::BSTSmartPointer<RE::BSScript::Object> object;
    if (!vm->FindBoundObject(handle, scriptName.c_str(), object)) {
        return;
    }

    auto args = RE::MakeFunctionArguments();

    RE::BSTSmartPointer<RE::BSScript::IStackCallbackFunctor> result;
    vm->DispatchMethodCall1(object,  // the Papyrus ObjectReference instance
                            functionName,
                            args,  // packed arguments
                            result);
}

void Parkouring::CalculateStartingPosition(const RE::Actor *actor, int ledgeType, RE::NiPoint3 &out) {
    float zAdjust = 0;
    float z = 0;
    float backOffset = 55.0f;
    RE::NiPoint3 backwardAdjustment;

    switch (ledgeType) {
        case 8:  // Highest Ledge
            z = HardCodedVariables::highestLedgeElevation - 5;
            break;

        case 7:  // High ledge
            z = HardCodedVariables::highLedgeElevation - 5;
            break;

        case 6:  // Medium ledge
            z = HardCodedVariables::medLedgeElevation - 5;
            break;

        case 5:  // Low ledge
            z = HardCodedVariables::lowLedgeElevation - 5;
            break;

        case 4:  // Step High
            z = HardCodedVariables::stepHighElevation - 5;
            backOffset = 15;  // Override backward offset
            break;

        case 3:  // Step Low
            z = HardCodedVariables::stepLowElevation - 5;
            backOffset = 15;  // Override backward offset
            break;

        case 2:  // Vault
            z = HardCodedVariables::vaultElevation - 5;
            break;

        case 1:  // Grab (Midair or Out of Water)
            z = HardCodedVariables::grabElevation - 5;
            // backOffset = 40;  // Override backward offset
            break;

        case 0:  // Failed (Low Stamina Animation)
            break;
        default:
            ERROR(" >> START POSITION NOT SET, INVALID LEDGE TYPE {} <<", ledgeType);
            return;
    }

    zAdjust = -z * RuntimeVariables::PlayerScale;
    backwardAdjustment = RuntimeVariables::PlayerScale * RuntimeVariables::playerDirFlat * backOffset;

    out = RE::NiPoint3{RuntimeVariables::ledgePoint.x - backwardAdjustment.x, RuntimeVariables::ledgePoint.y - backwardAdjustment.y,
                       ledgeType == ParkourType::Failed ? actor->GetPositionZ() : RuntimeVariables::ledgePoint.z + zAdjust};
}

void Parkouring::InvalidateVars() {
    if (GameReferences::currentIndicatorRef)
        GameReferences::currentIndicatorRef->Disable();
    RuntimeVariables::selectedLedgeType = -1;
}

void Parkouring::UpdateParkourPoint() {
    if (RuntimeVariables::ParkourInProgress) {
        InvalidateVars();
        return;
    }

    _THREAD_POOL.enqueue([]() {
        RuntimeVariables::IsParkourActive = IsParkourActive();
        RuntimeVariables::PlayerScale = ScaleUtility::GetScale();
    });

    if (!RuntimeVariables::ParkourInProgress) {
        /*Avoid updating the ledge if parkour already started*/
        RuntimeVariables::selectedLedgeType = GetLedgePoint();
    }

    // Indicator stuff
    PlaceAndShowIndicator();
}

bool Parkouring::TryActivateParkour() {
    const auto player = GET_PLAYER;
    const auto LedgeTypeToProcess = RuntimeVariables::selectedLedgeType;

    if (LedgeTypeToProcess == ParkourType::NoLedge) {
        return false;
    }

    bool Ongoing;
    if (player->GetGraphVariableBool(SPPF_ONGOING, Ongoing) && Ongoing) {
        return false;
    }

    float turningDelta;
    player->GetGraphVariableFloat("TurnDelta", turningDelta);
    if (turningDelta > 50.0f) {
        return false;
    }

    if (!RuntimeVariables::IsParkourActive || RuntimeVariables::IsMenuOpen) {
        return false;
    }

    const bool isMoving = player->IsMoving();
    const bool lowEffort = CheckActionRequiresLowEffort(LedgeTypeToProcess);
    const bool isSwimming = PlayerIsSwimming();
    // const bool isSprinting = player->IsSprinting();

    const auto fallTime = player->GetCharController()->fallTime;
    const bool avoidOnGroundParkour = fallTime > 0.0f;
    const bool avoidMidairParkour = fallTime < 0.17f;
    //LOG(">> Fall time: {}", fallTime);

    if (LedgeTypeToProcess != ParkourType::Grab) {
        if (avoidOnGroundParkour) {
            return false;
        }
    }
    else {
        if (avoidMidairParkour && !isSwimming) {
            return false;
        }
    }

    /* Cancel if moving, but allow movement during swimming */
    if (ModSettings::Smart_Climb && isMoving && !isSwimming) {
        if (!lowEffort) {
            return false;
        }
    }

    RuntimeVariables::ParkourInProgress = true;

    /* Also pass swimming state for stamina calculation logic */
    ParkourReadyRun(LedgeTypeToProcess, isSwimming);

    return true;
}
void Parkouring::ParkourReadyRun(int32_t ledgeType, bool isSwimming) {
    const auto &player = GET_PLAYER;
    const auto &ctrl = player->GetCharController();
    //auto dist = player->GetPosition().GetDistance(RuntimeVariables::ledgePoint);
    //LOG("Dist: {}", dist);

    /* Another guard for multiple activations */
    if (RuntimeVariables::EnableNotifyWindow) {
        return;
    }
    RuntimeVariables::EnableNotifyWindow = true;
    player->SetGraphVariableInt(SPPF_Ledge, ledgeType);

    RE::NiPoint3 startPos;
    Parkouring::CalculateStartingPosition(player, ledgeType, startPos);

    ctrl->gravity = 0;

    InterpolateRefToPosition(player, startPos, 0.15f);

    _THREAD_POOL.enqueue([player, ctrl, ledgeType, isSwimming, startPos] {
        auto startTime = std::chrono::high_resolution_clock::now();
        long long elapsedMS;
        do {
            elapsedMS =
                std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - startTime).count();

        } while (elapsedMS < 100 && (player->GetPosition().GetDistance(startPos) > 1.0f));

        _TASK_Q([player, ctrl, ledgeType, isSwimming] {
            bool success = player->NotifyAnimationGraph(SPPF_NOTIFY);
            RuntimeVariables::EnableNotifyWindow = false;
            ctrl->gravity = 1;
            StopInterpolatingRef(player);

            if (success) {
                /* Swap last leg (Step animations) */
                if (ledgeType == ParkourType::StepHigh || ledgeType == ParkourType::StepLow) {
                    RuntimeMethods::SwapLegs();
                }
                /* Steps don't consume stamina anymore */
                else {
                    const bool isLowEffort = ParkourUtility::CheckActionRequiresLowEffort(ledgeType);
                    Parkouring::PostParkourStaminaDamage(player, isLowEffort, isSwimming);
                }
            }
            else {
                player->NotifyAnimationGraph(SPPF_STOP);
                RuntimeVariables::ParkourInProgress = false;
            }
        });
    });
}
void Parkouring::PostParkourStaminaDamage(RE::PlayerCharacter *player, bool isLowEffort, bool isSwimming) {
    if (ModSettings::Enable_Stamina_Consumption) {
        if (player->IsGodMode()) {
            return;
        }

        float cost = ParkourUtility::CalculateParkourStamina(player);

        /* If swimming, fail animation won't play. So no need to flash the bar. Just consume half the stamina cost like low effort. */
        if (isLowEffort || isSwimming) {
            // LOG("cost{}", cost / 2);
            DamageActorStamina(player, cost / 2);
        }
        else if (PlayerHasEnoughStamina()) {
            // LOG("cost{}", cost);
            DamageActorStamina(player, cost);
        }
        else {
            RE::HUDMenu::FlashMeter(RE::ActorValue::kStamina);
        }
        player->UpdateRegenDelay(RE::ActorValue::kStamina, 2.0f);
    }
}

void Parkouring::SetParkourOnOff(bool turnOn) {
    if (turnOn) {
        if (!ButtonEventListener::GetSingleton()->SinkRegistered) {
            ButtonEventListener::Register();
            LOG("Processing On");
        }
    }
    else {
        if (ButtonEventListener::GetSingleton()->SinkRegistered) {
            ButtonEventListener::Unregister();
            LOG("Processing Off");
        }

        RuntimeMethods::ResetRuntimeVariables();

        if (GameReferences::currentIndicatorRef) {
            GameReferences::currentIndicatorRef->Disable();
        }
    }
}