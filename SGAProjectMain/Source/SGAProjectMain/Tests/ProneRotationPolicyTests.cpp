#include "CoreMinimal.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Misc/ScopeExit.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/WorldInitializationValues.h"
#include "UObject/UObjectGlobals.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "InputActionValue.h"
#include "../Character/PlayerCharacter.h"
#include "../Character/HellDiver/HellDiverStateComponent.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FProneRotationPolicyTest,
    "SGA.Character.ProneRotation.Policy",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FProneRotationPolicyTest::RunTest(const FString& Parameters)
{
    const UWorld::InitializationValues IVS = UWorld::InitializationValues()
        .AllowAudioPlayback(false).RequiresHitProxies(false).CreatePhysicsScene(false)
        .CreateNavigation(false).CreateAISystem(false).ShouldSimulatePhysics(false)
        .EnableTraceCollision(false).SetTransactional(false).CreateFXSystem(false);
    UWorld* World = UWorld::CreateWorld(EWorldType::EditorPreview, false,
        MakeUniqueObjectName(GetTransientPackage(), UWorld::StaticClass(), TEXT("ProneRotationPolicy")),
        nullptr, true, ERHIFeatureLevel::Num, &IVS);
    if (!TestNotNull(TEXT("The isolated preview world exists"), World)) return false;
    if (!TestNotNull(TEXT("The engine exists for preview world ownership"), GEngine))
    {
        World->DestroyWorld(false);
        return false;
    }
    GEngine->CreateNewWorldContext(EWorldType::EditorPreview).SetCurrentWorld(World);
    ON_SCOPE_EXIT { World->DestroyWorld(false); GEngine->DestroyWorldContext(World); };

    FActorSpawnParameters Spawn;
    Spawn.ObjectFlags = RF_Transient;
    Spawn.bDeferConstruction = true;
    Spawn.bTemporaryEditorActor = true;
    Spawn.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    const FVector Location(500.f, 500.f, 100.f);
    const FRotator Rotation(0.f, 37.f, 0.f);
    UClass* ActualPlayerClass = LoadClass<APlayerCharacter>(nullptr,
        TEXT("/Game/Blueprints/HellDiver/BP_HellDiver_MK2.BP_HellDiver_MK2_C"));
    if (!TestNotNull(TEXT("The actual player Blueprint class exists"), ActualPlayerClass)) return false;
    APlayerCharacter* Pawn = World->SpawnActor<APlayerCharacter>(ActualPlayerClass, Location, Rotation, Spawn);
    if (!TestNotNull(TEXT("The actual native player pawn exists"), Pawn)) return false;
    Pawn->SetActorTickEnabled(false);
    Pawn->SetActorEnableCollision(false);
    UCharacterMovementComponent* Move = Pawn->GetCharacterMovement();
    UHellDiverStateComponent* State = Pawn->GetStateComponent();
    if (!TestNotNull(TEXT("Native movement exists"), Move) || !TestNotNull(TEXT("Native state exists"), State)) return false;
    State->SetCharacterState(ECharacterState::Proning);
    State->SetWeaponState(EWeaponType::None);
    const FRotator InitialRotation = Pawn->GetActorRotation();
    const FTransform MeshRelative = Pawn->GetMesh()->GetRelativeTransform();

    // Stale flags can come from a previous standing/aiming path.
    const auto DirtyCameraRotationFlags = [&]()
    {
        Pawn->bUseControllerRotationYaw = true;
        Move->bUseControllerDesiredRotation = true;
        Move->bOrientRotationToMovement = true;
    };
    const auto AssertPronePolicy = [&](const TCHAR* Context)
    {
        TestTrue(FString::Printf(TEXT("%s keeps capsule yaw"), Context), Pawn->GetActorRotation().Equals(InitialRotation, 0.001f));
        TestFalse(FString::Printf(TEXT("%s disables immediate controller yaw"), Context), Pawn->bUseControllerRotationYaw);
        TestFalse(FString::Printf(TEXT("%s disables movement-controller yaw"), Context), Move->bUseControllerDesiredRotation);
        TestTrue(FString::Printf(TEXT("%s preserves crawl-facing-movement policy"), Context), Move->bOrientRotationToMovement);
        TestTrue(FString::Printf(TEXT("%s does not counter-rotate the mesh component"), Context),
            Pawn->GetMesh()->GetRelativeTransform().Equals(MeshRelative, 0.001f));
    };

    DirtyCameraRotationFlags();
    Pawn->ViewTurnBack();
    AssertPronePolicy(TEXT("ViewTurnBack"));

    // This public input path reaches ViewTurnBack without waiting for another Look event.
    State->SetActionState(EActionState::Stratagem); // IsFocusing returns false, with no owner/BeginPlay dependency.
    DirtyCameraRotationFlags();
    Pawn->MoveFinish(FInputActionValue(FVector2D::ZeroVector));
    AssertPronePolicy(TEXT("MoveFinish"));

    State->SetActionState(EActionState::None);
    DirtyCameraRotationFlags();
    Pawn->StartFiring(FInputActionValue(true)); // Weapon None skips weapon logic after the real rotation path.
    AssertPronePolicy(TEXT("StartFiring"));

    DirtyCameraRotationFlags();
    Pawn->DefaultLook();
    // Pawn::FaceRotation is the direct controller-yaw route used by controller updates.
    Pawn->FaceRotation(FRotator(0.f, -145.f, 0.f), 1.f / 60.f);
    AssertPronePolicy(TEXT("Look and FaceRotation"));

    DirtyCameraRotationFlags();
    Pawn->SetFocusMove();
    AssertPronePolicy(TEXT("SetFocusMove"));

    // Positive controls ensure the repair did not globally disable standing focus/turn behavior.
    State->SetCharacterState(ECharacterState::Standing);
    Pawn->SetFocusMove();
    TestTrue(TEXT("Standing focus still enables desired controller rotation"), Move->bUseControllerDesiredRotation);
    TestFalse(TEXT("Standing focus still disables movement-facing rotation"), Move->bOrientRotationToMovement);
    Pawn->ViewTurnBack();
    TestTrue(TEXT("Standing ViewTurnBack retains the existing one-degree turn"),
        FMath::IsNearlyEqual(FMath::FindDeltaAngleDegrees(InitialRotation.Yaw, Pawn->GetActorRotation().Yaw), 1.f, 0.001f));
    TestTrue(TEXT("Standing ViewTurnBack still enables desired rotation"), Move->bUseControllerDesiredRotation);
    TestFalse(TEXT("The test never began pawn gameplay"), Pawn->HasActorBegunPlay());
    TestFalse(TEXT("The test never began world gameplay"), World->HasBegunPlay());
    return true;
}

#endif
