#include "Parkouring.h"

using namespace ParkourUtility;

int Parkouring::LedgeCheck(RE::NiPoint3 &ledgePoint, RE::NiPoint3 checkDir, float minLedgeHeight, float maxLedgeHeight) {
    const auto player = RE::PlayerCharacter::GetSingleton();
    const auto playerPos = player->GetPosition();

    // Constants adjusted for player scale
    const float startZOffset = 100 * RuntimeVariables::PlayerScale;
    const float playerHeight = 120 * RuntimeVariables::PlayerScale;
    const float minUpCheck = 100 * RuntimeVariables::PlayerScale;
    const float maxUpCheck = (maxLedgeHeight - startZOffset) + 20 * RuntimeVariables::PlayerScale;
    const float fwdCheckStep = 8 * RuntimeVariables::PlayerScale;
    const int fwdCheckIterations = 10;           // 15
    const float minLedgeFlatness = 0.5;          //0.5
    const float playerToLedgeHypotenuse = 0.8f;  // 0.75 - larger is more relaxed, lesser is more strict. Don't set 0

    RE::hkVector4 normalOut(0, 0, 0, 0);

    // Upward raycast to check for headroom
    RE::NiPoint3 upRayStart = playerPos + RE::NiPoint3(0, 0, startZOffset);
    RE::NiPoint3 upRayDir(0, 0, 1);

    float upRayDist = RayCast(upRayStart, upRayDir, maxUpCheck, normalOut, RE::COL_LAYER::kLOS);
    if (upRayDist < minUpCheck) {
        return ParkourType::NoLedge;
    }

    // Forward raycast initialization
    RE::NiPoint3 fwdRayStart = upRayStart + upRayDir * (upRayDist - 10);
    RE::NiPoint3 downRayDir(0, 0, -1);

    bool foundLedge = false;
    float normalZ = 0;

    // Incremental forward raycast to find a ledge
    for (int i = 0; i < fwdCheckIterations; i++) {
        float fwdRayDist = RayCast(fwdRayStart, checkDir, fwdCheckStep * i, normalOut, RE::COL_LAYER::kLOS);
        if (fwdRayDist < fwdCheckStep * i) {
            continue;
        }

        // Downward raycast to detect ledge point
        RE::NiPoint3 downRayStart = fwdRayStart + checkDir * fwdRayDist;
        float downRayDist = RayCast(downRayStart, downRayDir, startZOffset + maxUpCheck, normalOut, RE::COL_LAYER::kLOS);

        ledgePoint = downRayStart + downRayDir * downRayDist;
        normalZ = normalOut.quad.m128_f32[2];

        // Validate ledge based on height and flatness
        if (ledgePoint.z < playerPos.z + minLedgeHeight || ledgePoint.z > playerPos.z + maxLedgeHeight || downRayDist < 10 ||
            normalZ < minLedgeFlatness) {
            continue;
        }

        // Backward ray to check for obstructions behind the vaultable surface
        RE::NiPoint3 backwardRayStart = fwdRayStart + checkDir * (fwdRayDist - 2) + RE::NiPoint3(0, 0, 5);
        const float maxObstructionDistance = 15.0f * RuntimeVariables::PlayerScale;
        float backwardRayDist = RayCast(backwardRayStart, checkDir, maxObstructionDistance, normalOut, RE::COL_LAYER::kLOS);

        if (backwardRayDist > 0 && backwardRayDist < maxObstructionDistance) {
            continue;  // Obstruction behind the vaultable surface
        }

        foundLedge = true;
        break;
    }

    if (!foundLedge) {
        return ParkourType::NoLedge;
    }

    // Ensure there is sufficient headroom for the player to stand
    float headroomBuffer = 10 * RuntimeVariables::PlayerScale;
    RE::NiPoint3 headroomRayStart = ledgePoint + upRayDir * headroomBuffer;
    float headroomRayDist = RayCast(headroomRayStart, upRayDir, playerHeight - headroomBuffer, normalOut, RE::COL_LAYER::kLOS);

    if (headroomRayDist < playerHeight - headroomBuffer) {
        return ParkourType::NoLedge;
    }

    const float ledgePlayerDiff = ledgePoint.z - playerPos.z;
    if (PlayerIsGroundedOrSliding() || PlayerIsSwimming()) {
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
                return ParkourType::Grab;  // Grab ledge out of water, don't jump out like a frog
            }

            return ParkourType::Low;  // Low ledge
        }
        else if (ledgePlayerDiff >= HardCodedVariables::highStepLimit * RuntimeVariables::PlayerScale) {
            if (PlayerIsSwimming()) {
                return ParkourType::Grab;  // Grab ledge out of water, don't step out
            }

            // Additional horizontal and vertical checks for low ledge
            double horizontalDistance = sqrt(pow(ledgePoint.x - playerPos.x, 2) + pow(ledgePoint.y - playerPos.y, 2));
            double verticalDistance = abs(ledgePlayerDiff);

            if (horizontalDistance < verticalDistance * playerToLedgeHypotenuse) {
                return ParkourType::StepHigh;  // High Step
            }
        }
        else {
            if (PlayerIsSwimming()) {
                return ParkourType::Grab;  // Grab ledge out of water, don't step out
            }

            // Additional horizontal and vertical checks for low ledge
            double horizontalDistance = sqrt(pow(ledgePoint.x - playerPos.x, 2) + pow(ledgePoint.y - playerPos.y, 2));
            double verticalDistance = abs(ledgePlayerDiff);

            if (!PlayerIsOnStairs() && horizontalDistance < verticalDistance * playerToLedgeHypotenuse) {
                return ParkourType::StepLow;  // Low Step
            }
        }
    }
    else if (PlayerIsMidairAndNotSliding() && ledgePlayerDiff > HardCodedVariables::grabPlayerAboveLedgeMaxDiff &&
             ledgePlayerDiff <= HardCodedVariables::grabPlayerBelowLedgeMaxDiff * RuntimeVariables::PlayerScale) {
        return ParkourType::Grab;
    }
    return ParkourType::NoLedge;
}
int Parkouring::VaultCheck(RE::NiPoint3 &ledgePoint, RE::NiPoint3 checkDir, float vaultLength, float maxElevationIncrease,
                           float minVaultHeight, float maxVaultHeight) {
    const auto player = RE::PlayerCharacter::GetSingleton();

    if (!PlayerIsGroundedOrSliding()) {
        return ParkourType::NoLedge;
    }

    const auto playerPos = player->GetPosition();

    RE::hkVector4 normalOut(0, 0, 0, 0);

    float headHeight = 120 * RuntimeVariables::PlayerScale;

    // Forward raycast to check for a vaultable surface
    RE::NiPoint3 fwdRayStart = playerPos + RE::NiPoint3(0, 0, headHeight);
    float fwdRayDist = RayCast(fwdRayStart, checkDir, vaultLength, normalOut, RE::COL_LAYER::kLOS);

    if (RuntimeVariables::lastHitObject == RE::COL_LAYER::kTerrain || fwdRayDist < vaultLength) {
        return ParkourType::NoLedge;  // Not vaultable if terrain or insufficient distance
    }

    // Backward ray to check for obstructions behind the vaultable surface
    RE::NiPoint3 backwardRayStart = fwdRayStart + checkDir * (fwdRayDist - 2) + RE::NiPoint3(0, 0, 5);
    const float maxObstructionDistance = 100.0f * RuntimeVariables::PlayerScale;
    float backwardRayDist = RayCast(backwardRayStart, checkDir, maxObstructionDistance, normalOut, RE::COL_LAYER::kLOS);

    if (backwardRayDist > 0 && backwardRayDist < maxObstructionDistance) {
        return ParkourType::NoLedge;  // Obstruction behind the vaultable surface
    }

    // Downward raycast initialization
    int downIterations = /*static_cast<int>(std::floor(vaultLength / 5.0f))*/ 20;
    RE::NiPoint3 downRayDir(0, 0, -1);

    bool foundVaulter = false;
    float foundVaultHeight = -10000.0f;
    bool foundLanding = false;
    float foundLandingHeight = 10000.0f;

    // Incremental downward raycasts
    for (int i = 0; i < downIterations; i++) {
        float iDist = static_cast<float>(i) * 5.0f;
        RE::NiPoint3 downRayStart = playerPos + checkDir * iDist;
        downRayStart.z = fwdRayStart.z;

        float downRayDist = RayCast(downRayStart, downRayDir, headHeight + 100.0f, normalOut, RE::COL_LAYER::kLOS);
        float hitHeight = (fwdRayStart.z - downRayDist) - playerPos.z;

        // Check hit height for vaultable surfaces
        if (hitHeight > maxVaultHeight) {
            return ParkourType::NoLedge;  // Too high to vault
        }
        else if (hitHeight > minVaultHeight && hitHeight < maxVaultHeight) {
            if (hitHeight >= foundVaultHeight) {
                foundVaultHeight = hitHeight;
                foundLanding = false;
            }
            ledgePoint = downRayStart + downRayDir * downRayDist;
            foundVaulter = true;
        }
        else if (foundVaulter && hitHeight < minVaultHeight) {
            foundLandingHeight = std::min(hitHeight, foundLandingHeight);
            foundLanding = true;
        }
    }

    // Final validation for vault
    if (foundVaulter && foundLanding && foundLandingHeight < maxElevationIncrease) {
        ledgePoint.z = playerPos.z + foundVaultHeight;
        if (!PlayerIsOnStairs()) {
            return ParkourType::Vault;  // Vault successful
        }
    }

    return ParkourType::NoLedge;  // Vault failed
}

bool Parkouring::PlaceAndShowIndicator() {
    const bool useIndicators = ModSettings::UseIndicators;
    if (!useIndicators) {
        return false;
    }

    const bool enableStamina = ModSettings::Enable_Stamina_Consumption;
    const bool hasStamina = PlayerHasEnoughStamina();
    const auto ledgeType = RuntimeVariables::selectedLedgeType;

    // Indicators cause crashes, only
    const bool useRed = useIndicators && enableStamina && !hasStamina && !CheckIsVaultActionFromType(ledgeType);

    SKSE::GetTaskInterface()->AddTask([useRed]() {
        auto blueRef = GameReferences::indicatorRef_Blue;
        auto redRef = GameReferences::indicatorRef_Red;
        if (!blueRef || !redRef) {
            return;
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
            return;
        }

        const auto player = RE::PlayerCharacter::GetSingleton();
        if (!player) {
            return;
        }

        if (currentRef->GetParentCell() != player->GetParentCell()) {
            currentRef->MoveTo(player->AsReference());
        }

        currentRef->data.location = RuntimeVariables::ledgePoint + RE::NiPoint3(0, 0, 8.0f);
        currentRef->data.angle = RE::NiPoint3(0, 0, std::atan2(RuntimeVariables::playerDirFlat.x, RuntimeVariables::playerDirFlat.y));
        currentRef->Update3DPosition(true);

        if (RuntimeVariables::IsParkourActive) {
            currentRef->Enable(false);
        }
        else {
            currentRef->Disable();
        }
    });

    return true;
}

int Parkouring::GetLedgePoint() {
    using namespace GameReferences;
    using namespace ModSettings;

    const auto player = RE::PlayerCharacter::GetSingleton();
    //const auto playerPos = player->GetPosition();
    const bool isMoving = player->IsMoving();

    RE::NiPoint3 playerDirFlat = GetPlayerDirFlat(player);

    // Perform ledge or vault checks
    int selectedLedgeType = ParkourType::NoLedge;
    RE::NiPoint3 ledgePoint;

    constexpr int vaultLength = 85;
    constexpr int maxElevationIncrease = 80;

    if (isMoving || !ModSettings::Smart_Parkour_Enabled) {
        selectedLedgeType = VaultCheck(ledgePoint, playerDirFlat, vaultLength, maxElevationIncrease * RuntimeVariables::PlayerScale,
                                       HardCodedVariables::vaultMinHeight * RuntimeVariables::PlayerScale,
                                       HardCodedVariables::vaultMaxHeight * RuntimeVariables::PlayerScale);
    }

    if (selectedLedgeType == ParkourType::NoLedge) {
        selectedLedgeType = LedgeCheck(ledgePoint, playerDirFlat, HardCodedVariables::climbMinHeight * RuntimeVariables::PlayerScale,
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
void Parkouring::InterpolateRefToPosition(const RE::Actor *movingRef, RE::NiPoint3 position, float seconds, bool isRelative) {
    auto vm = RE::BSScript::Internal::VirtualMachine::GetSingleton();
    if (!vm) {
        return;
    }

    /* Calculate speed from cur pos to target dist / time. BUT Read annotations relative to start position. */
    auto curPos = movingRef->GetPosition();
    RE::NiPoint3 relativeTranslatedToWorld = position;

    if (isRelative) {
        const auto facing = RuntimeVariables::playerDirFlat;
        // playerDirFlat = forward vector (x,y,0), normalized
        const float rightX = facing.y;
        const float rightY = -facing.x;

        const auto startPos = RuntimeVariables::PlayerStartPosition;

        const float worldX =
            startPos.x + position.x * rightX * RuntimeVariables::PlayerScale + position.y * facing.x * RuntimeVariables::PlayerScale;
        const float worldY =
            startPos.y + position.x * rightY * RuntimeVariables::PlayerScale + position.y * facing.y * RuntimeVariables::PlayerScale;
        const float worldZ = startPos.z + position.z * RuntimeVariables::PlayerScale;

        relativeTranslatedToWorld = RE::NiPoint3{worldX, worldY, worldZ};
    }

    const auto diff = relativeTranslatedToWorld - curPos;
    auto speed = seconds <= 0 ? 5000 : diff.Length() / seconds;  // Snap to pos if 0 or negative seconds

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

    // Call the Papyrus method
    RE::BSTSmartPointer<RE::BSScript::IStackCallbackFunctor> result;
    vm->DispatchMethodCall1(object,        // the Papyrus ObjectReference instance
                            functionName,  // "TranslateTo"
                            args,          // packed arguments
                            result);
}
void Parkouring::StopInterpolationToPosition() {
    auto movingRef = RE::PlayerCharacter::GetSingleton();
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

void Parkouring::CalculateStartingPosition(int ledgeType) {
    const auto player = RE::PlayerCharacter::GetSingleton();
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
            backOffset = 40;  // Override backward offset
            break;

        case 0:  // Failed (Low Stamina Animation)
            break;
        default:
            logger::error(" >> START POSITION NOT SET, INVALID LEDGE TYPE {} <<", ledgeType);
            return;
    }

    zAdjust = -z * RuntimeVariables::PlayerScale;
    backwardAdjustment = RuntimeVariables::PlayerScale * RuntimeVariables::playerDirFlat * backOffset;

    RuntimeVariables::PlayerStartPosition =
        RE::NiPoint3{RuntimeVariables::ledgePoint.x - backwardAdjustment.x, RuntimeVariables::ledgePoint.y - backwardAdjustment.y,
                     ledgeType == ParkourType::Failed ? player->GetPositionZ() : RuntimeVariables::ledgePoint.z + zAdjust};

    //Parkouring::InterpolateRefToPosition(player, newPosition, 0.1f);
    //player->SetPosition(RuntimeVariables::PlayerStartPosition, true);
}

void Parkouring::UpdateParkourPoint() {
    if (RuntimeVariables::ParkourInProgress) {
        if (GameReferences::currentIndicatorRef)
            GameReferences::currentIndicatorRef->Disable();
        RuntimeVariables::selectedLedgeType = -1;
        return;
    }

    _THREAD_POOL.enqueue([]() {
        RuntimeVariables::IsParkourActive = IsParkourActive();
        RuntimeVariables::PlayerScale = ScaleUtility::GetScale();

        if (!RuntimeVariables::ParkourInProgress) {
            /*Avoid updating the ledge if parkour already started*/
            SKSE::GetTaskInterface()->AddTask([]() { RuntimeVariables::selectedLedgeType = GetLedgePoint(); });
        }
    });

    //logger::info("{}", RE::GetSecondsSinceLastFrame());

    // Indicator stuff
    _THREAD_POOL.enqueue([]() { PlaceAndShowIndicator(); });
}

bool Parkouring::TryActivateParkour() {
    using namespace GameReferences;
    using namespace ModSettings;
    const auto player = RE::PlayerCharacter::GetSingleton();
    const auto LedgeTypeToProcess = RuntimeVariables::selectedLedgeType;
    // Check Is Parkour Active again, make sure condition is still valid during activation
    if (!IsParkourActive() || RuntimeVariables::ParkourInProgress || LedgeTypeToProcess == ParkourType::NoLedge) {
        player->SetGraphVariableInt("SkyParkourLedge", ParkourType::NoLedge);
        return false;
    }

    const bool isMoving = player->IsMoving();
    const bool isVaultAction = CheckIsVaultActionFromType(LedgeTypeToProcess);
    const bool isSwimming = PlayerIsSwimming();
    // const bool isSprinting = player->IsSprinting();

    const auto fallTime = player->GetCharController()->fallTime;
    const bool avoidOnGroundParkour = fallTime > 0.0f;
    const bool avoidMidairParkour = fallTime < 0.17f;
    //logger::info(">> Fall time: {}", fallTime);

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

    /* Cancel if moving, but allow during swimming */
    if (Smart_Parkour_Enabled && isMoving && !isSwimming) {
        if (!isVaultAction) {
            player->SetGraphVariableInt("SkyParkourLedge", ParkourType::NoLedge);
            return false;
        }
    }

    RuntimeVariables::ParkourInProgress = true;

    /* Also pass swimming state for stamina calculation logic */
    ParkourReadyRun(LedgeTypeToProcess, isSwimming);

    return true;
}
void Parkouring::ParkourReadyRun(int32_t ledgeType, bool isSwimming) {
    const auto player = RE::PlayerCharacter::GetSingleton();
    //auto dist = player->GetPosition().GetDistance(RuntimeVariables::ledgePoint);
    //logger::info("Dist: {}", dist);

    player->SetGraphVariableInt("SkyParkourLedge", ledgeType);
    player->NotifyAnimationGraph("moveStop");

    bool success = player->NotifyAnimationGraph("SkyParkour");
    if (success) {
        RuntimeVariables::ParkourActivatedOnce = true;
        /* Always call this, it no longer does an adjustment but sets a reference point to use annotations as offset to it. */
        Parkouring::CalculateStartingPosition(ledgeType);
        /* Swap last leg (Step animations) */
        if (ledgeType == ParkourType::StepHigh || ledgeType == ParkourType::StepLow) {
            RuntimeMethods::SwapLegs();
        }
        /* Steps don't consume stamina anymore */
        else {
            const bool isVaultAction = ParkourUtility::CheckIsVaultActionFromType(ledgeType);
            Parkouring::PostParkourStaminaDamage(player, isVaultAction, isSwimming);
        }
    }
    else {
        /* Parkour Failed for whatever reason */
        ParkourUtility::ToggleControlsForParkour(true);
        RuntimeVariables::ParkourInProgress = false;
    }
}
void Parkouring::PostParkourStaminaDamage(RE::PlayerCharacter *player, bool isVault, bool isSwimming) {
    if (ModSettings::Enable_Stamina_Consumption) {
        if (player->IsGodMode()) {
            return;
        }

        float cost = ParkourUtility::CalculateParkourStamina();

        /* If swimming, fail animation won't play. So no need to flash the bar. Just consume half the stamina cost like vault. */
        if (isVault || isSwimming) {
            // logger::info("cost{}", cost / 2);
            DamageActorStamina(player, cost / 2);
        }
        else if (PlayerHasEnoughStamina()) {
            // logger::info("cost{}", cost);
            DamageActorStamina(player, cost);
        }
        else {
            RE::HUDMenu::FlashMeter(RE::ActorValue::kStamina);
            player->UpdateRegenDelay(RE::ActorValue::kStamina, 2.0f);
        }
    }
}

void Parkouring::SetParkourOnOff(bool turnOn) {
    if (turnOn) {
        if (!ButtonEventListener::GetSingleton()->SinkRegistered)
            ButtonEventListener::Register();
    }
    else {
        if (ButtonEventListener::GetSingleton()->SinkRegistered)
            ButtonEventListener::Unregister();

        ParkourUtility::ToggleControlsForParkour(true);
        RuntimeMethods::ResetRuntimeVariables();

        if (GameReferences::currentIndicatorRef)
            GameReferences::currentIndicatorRef->Disable();
    }
}