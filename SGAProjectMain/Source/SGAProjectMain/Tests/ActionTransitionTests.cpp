#include "CoreMinimal.h"
#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "Misc/ScopeExit.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/WorldInitializationValues.h"
#include "Animation/AnimMontage.h"
#include "Animation/AnimNode_StateMachine.h"
#include "Animation/AnimSequenceBase.h"
#include "Animation/AnimClassInterface.h"
#include "AnimNodes/AnimNode_RotationOffsetBlendSpace.h"
#include "UObject/UnrealType.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "../Character/PlayerCharacter.h"
#include "../Character/HellDiver/HellDiverStateComponent.h"
#include "../Character/CharacterAnimLayerInstance.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FActionTransitionLockTest, "SGA.Character.Actions.RequestLocks",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FActionTransitionLockTest::RunTest(const FString& Parameters)
{
    auto* State = NewObject<UHellDiverStateComponent>();
    const ECharacterState Stances[] = {ECharacterState::Standing, ECharacterState::Crouching, ECharacterState::Proning};
    for (ECharacterState Source : Stances)
    for (ECharacterState Target : Stances)
    {
        if (Source == Target) continue;
        State->Reset();
        State->SetCharacterState(Source);
        bool Accepted = Target == ECharacterState::Crouching ? State->StartCrouch()
            : Target == ECharacterState::Proning ? State->StartProne()
            : Source == ECharacterState::Proning ? State->FinishProne() : State->FinishCrouch();
        TestTrue(TEXT("Each of six posture requests is accepted"), Accepted);
        TestTrue(TEXT("The request locks immediately"), State->IsMotionChanging());
        TestEqual(TEXT("The source is captured before changing stance"), State->GetMotionChangeSource(), Source);
        const uint32 Token = State->GetMotionChangeId();
        TestFalse(TEXT("Dodge cannot start during a posture change"), State->StartRolling());
        TestFalse(TEXT("Parkour cannot start during a posture change"), State->StartPakour());
        TestFalse(TEXT("Weapon changes cannot start during a posture change"), State->BeginWeaponChange());
        TestFalse(TEXT("Reload cannot start during a posture change"), State->StartReload());
        TestFalse(TEXT("A stale completion cannot release this request"), State->FinishMotionChange(Token - 1));
        TestTrue(TEXT("Matching completion releases this request"), State->FinishMotionChange(Token));
        TestFalse(TEXT("Duplicate completion is ignored"), State->FinishMotionChange(Token));
    }
    State->Reset(); State->SetCharacterState(ECharacterState::Standing);
    TestTrue(TEXT("Weapon change begins"), State->BeginWeaponChange());
    const uint32 WeaponToken = State->GetWeaponChangeId();
    TestFalse(TEXT("Repeated weapon input is rejected"), State->BeginWeaponChange());
    TestFalse(TEXT("Posture change is rejected while changing weapons"), State->StartCrouch());
    TestFalse(TEXT("Dodge is rejected while changing weapons"), State->StartRolling());
    TestFalse(TEXT("Parkour is rejected while changing weapons"), State->StartPakour());
    State->CancelAnimationChanges();
    TestFalse(TEXT("Cancellation releases the weapon lock"), State->IsWeaponChanging());
    TestTrue(TEXT("A new weapon request can start"), State->BeginWeaponChange());
    TestFalse(TEXT("Old callback cannot release a new weapon request"), State->FinishWeaponChange(WeaponToken));
    State->FinishWeaponChange(State->GetWeaponChangeId());
    TestTrue(TEXT("Parkour can start after weapon completion"), State->StartPakour());
    TestFalse(TEXT("Parkour blocks posture"), State->StartProne());
    TestFalse(TEXT("Parkour blocks weapon changes"), State->BeginWeaponChange());
    TestFalse(TEXT("Parkour blocks dodge"), State->StartRolling());
    State->FinishPakour();
    TestTrue(TEXT("Dodge can start after parkour"), State->StartRolling());
    TestTrue(TEXT("Dodge can still enter prone without a second posture montage"), State->StartProne());
    TestFalse(TEXT("Dodge did not acquire a posture lock"), State->IsMotionChanging());
    State->FinishRolling(); State->FinishProne();
    State->Dead();
    TestFalse(TEXT("Death clears posture locks"), State->IsMotionChanging());
    TestFalse(TEXT("Dead pawns cannot change weapons"), State->BeginWeaponChange());
    State->SetLifeState(ELifeState::Alive); State->Reset(); State->SetCharacterState(ECharacterState::Standing);
    State->StartSprint(); State->FinishSprint();
    TestFalse(TEXT("Sprint stop does not acquire a posture lock"), State->IsMotionChanging());
    TestTrue(TEXT("Dodge remains available after sprint stop"), State->StartRolling());
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FActionTransitionPlaybackTest, "SGA.Character.Actions.AssetPlayback",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FActionTransitionPlaybackTest::RunTest(const FString& Parameters)
{
    const UWorld::InitializationValues IVS = UWorld::InitializationValues()
        .AllowAudioPlayback(false).RequiresHitProxies(false).CreatePhysicsScene(false)
        .CreateNavigation(false).CreateAISystem(false).ShouldSimulatePhysics(false)
        .EnableTraceCollision(false).SetTransactional(false).CreateFXSystem(false);
    UWorld* World = UWorld::CreateWorld(EWorldType::EditorPreview, false,
        MakeUniqueObjectName(GetTransientPackage(), UWorld::StaticClass(), TEXT("ActionTransitionPlayback")),
        nullptr, true, ERHIFeatureLevel::Num, &IVS);
    if (!TestNotNull(TEXT("Preview world exists"), World)) return false;
    GEngine->CreateNewWorldContext(EWorldType::EditorPreview).SetCurrentWorld(World);
    ON_SCOPE_EXIT { World->DestroyWorld(false); GEngine->DestroyWorldContext(World); };
    UClass* PawnClass = LoadClass<APlayerCharacter>(nullptr, TEXT("/Game/Blueprints/HellDiver/BP_HellDiver_MK2.BP_HellDiver_MK2_C"));
    if (!TestNotNull(TEXT("Actual player Blueprint loads"), PawnClass)) return false;
    FActorSpawnParameters Spawn;
    Spawn.ObjectFlags = RF_Transient; Spawn.bDeferConstruction = true; Spawn.bTemporaryEditorActor = true;
    Spawn.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    APlayerCharacter* Pawn = World->SpawnActor<APlayerCharacter>(PawnClass, FVector(0,0,100), FRotator::ZeroRotator, Spawn);
    if (!TestNotNull(TEXT("Actual player exists"), Pawn)) return false;
    auto* Mesh = Pawn->GetMesh();
    Mesh->InitAnim(true);
    auto* Anim = Mesh->GetAnimInstance();
    if (!TestNotNull(TEXT("MK5 anim instance initializes"), Anim)) return false;
    const TCHAR* LayerPaths[] = {
        TEXT("/Game/Blueprints/HellDiver/Animation_Locomotion/ABP_Helldiver_Rifle.ABP_Helldiver_Rifle_C"),
        TEXT("/Game/Blueprints/HellDiver/Animation_Locomotion/ABP_Helldiver_Pistol.ABP_Helldiver_Pistol_C"),
        TEXT("/Game/Blueprints/HellDiver/Animation_Locomotion/ABP_Helldiver_Unarmed.ABP_Helldiver_Unarmed_C")};
    const ECharacterState Stances[] = {ECharacterState::Standing, ECharacterState::Crouching, ECharacterState::Proning};
    auto Advance = [&](float Seconds)
    {
        for (int32 Frame = 0; Frame < FMath::CeilToInt(Seconds * 60.f); ++Frame)
        {
            ++GFrameCounter;
            Mesh->TickAnimation(1.f/60.f, false);
            Mesh->RefreshBoneTransforms();
            Pawn->UpdateAnimationChanges();
        }
    };
    for (const TCHAR* Path : LayerPaths)
    {
        UClass* LayerClass = LoadClass<UAnimInstance>(nullptr, Path);
        if (!TestNotNull(TEXT("Weapon layer class loads"), LayerClass)) return false;
        Anim->LinkAnimClassLayers(LayerClass);
        auto* Layer = Cast<UCharacterAnimLayerInstance>(Anim->GetLinkedAnimLayerInstanceByClass(LayerClass));
        if (!TestNotNull(TEXT("Linked layer initializes"), Layer)) return false;
        for (ECharacterState Source : Stances)
        for (ECharacterState Target : Stances)
        {
            if (Source == Target) continue;
            UAnimSequenceBase* Clip = Layer->SelectStanceTransitionAnimation(Source, Target);
            if (!TestNotNull(TEXT("Actual transition clip exists"), Clip)) return false;
            auto* State = Pawn->GetStateComponent();
            State->Reset(); State->SetCharacterState(Source);
            Advance(.1f);
            if (Target == ECharacterState::Proning) State->StartProne();
            else if (Target == ECharacterState::Crouching) State->StartCrouch();
            else if (Source == ECharacterState::Proning) State->FinishProne();
            else State->FinishCrouch();
            Pawn->PrepareMotionChange();
            TestFalse(TEXT("Posture requests never start a montage"), Anim->IsAnyMontagePlaying());
            TestTrue(TEXT("Lock remains active during playback"), State->IsMotionChanging());
            TestFalse(TEXT("Melee cannot interrupt a managed posture transition"), Pawn->AttackMelee());
            Advance(.05f);
            TestTrue(TEXT("Sequence playback keeps the posture lock until the clip ends"), State->IsMotionChanging());
            TestFalse(TEXT("The graph consumes its pending transition request"), Cast<UCharacterAnimInstance>(Anim)->_stanceTransitionRequested);
            TestFalse(TEXT("No posture montage is playing during the transition"), Anim->IsAnyMontagePlaying());
            TestEqual(TEXT("AO is disabled while the transition sequence plays"), Layer->_actionAimAlpha, 0.f);
            bool FoundAO = false;
            for (FStructProperty* Property : IAnimClassInterface::GetFromClass(Layer->GetClass())->GetAnimNodeProperties())
            {
                if (Property->Struct == FAnimNode_RotationOffsetBlendSpace::StaticStruct())
                {
                    FoundAO = true;
                    auto* AO = Property->ContainerPtrToValuePtr<FAnimNode_RotationOffsetBlendSpace>(Layer);
                    TestEqual(TEXT("The actual AO node receives the zero alpha"), AO->Alpha, 0.f);
                }
            }
            TestTrue(TEXT("The real linked layer contains the AO node"), FoundAO);
            Advance(Clip->GetPlayLength() + .5f);
            TestFalse(TEXT("Sequence completion releases posture lock"), State->IsMotionChanging());
            TestEqual(TEXT("AO is restored after the transition"), Layer->_actionAimAlpha, 1.f);
        }
    }
    for (int32 Slot : {0,1,2})
    for (bool Equip : {false,true})
    {
        auto* Clip = Pawn->GetWeaponChangeAnimation(Slot, Equip);
        if (!TestNotNull(TEXT("Retargeted weapon transition clip exists"), Clip)) return false;
        auto* State = Pawn->GetStateComponent();
        State->Reset(); State->SetCharacterState(ECharacterState::Standing);
        State->BeginWeaponChange();
        Pawn->_pendingSwitchIndex = Slot;
        State->SetEquipIndex(Slot);
        Pawn->_playingWeaponChangeId = State->GetWeaponChangeId();
        if (!TestTrue(TEXT("Weapon phase plays on UpperBody slot"), Pawn->PlayWeaponChangePhase(Equip, State->GetWeaponChangeId()))) return false;
        // End an equip phase naturally; interrupt a holster phase before inventory commit.
        if (!Equip) Anim->Montage_Stop(.05f, Pawn->_weaponChangeMontage);
        Advance(Clip->GetPlayLength() + .5f);
        TestFalse(TEXT("Weapon phase completion/interruption releases lock"), State->IsWeaponChanging());
    }
    auto* State = Pawn->GetStateComponent();
    // Existing authored montages must not be passed to the dynamic-montage wrapper.
    auto* WeaponClip = Pawn->GetWeaponChangeAnimation(1, true);
    auto* Authored = UAnimMontage::CreateSlotAnimationAsDynamicMontage(WeaponClip, TEXT("UpperBody"), .1f, .1f, 1.f, 1);
    if (!TestNotNull(TEXT("An UpperBody montage can be constructed"), Authored)) return false;
    TestEqual(TEXT("Authored montages play directly without a second wrapper"), Pawn->PlayChangeAnimation(Authored, TEXT("UpperBody")), Authored);
    Anim->Montage_Stop(0.f, Authored);
    Advance(.1f);
    // Exercise both conduit branches. No actor tick or movement-input simulation is used.
    const int32 Machine = Anim->GetStateMachineIndex(TEXT("Locomotion"));
    auto* RuntimeMachine=const_cast<FAnimNode_StateMachine*>(Anim->GetStateMachineInstance(Machine));
    if (!TestNotNull(TEXT("The actual locomotion machine exists"), RuntimeMachine)) return false;
    const int32 OriginalMaxTransitions=RuntimeMachine->MaxTransitionsPerFrame;
    // In this transient fixture, observe each decision separately. Otherwise Cycle
    // may immediately proceed to Stop in the same frame because no input is simulated.
    RuntimeMachine->MaxTransitionsPerFrame=1;
    for (bool Moving : {false, true})
    {
        State->Reset(); State->SetCharacterState(ECharacterState::Standing);
        Pawn->GetCharacterMovement()->Velocity = FVector::ZeroVector;
        Advance(.2f);
        State->StartCrouch(); Pawn->PrepareMotionChange();
        bool SawTransition=false, SawReturn=false;
        FString History; FName Previous;
        for (int32 Frame=0; Frame<600 && !SawReturn; ++Frame)
        {
            Pawn->GetCharacterMovement()->Velocity = Moving ? FVector(200.f,0.f,0.f) : FVector::ZeroVector;
            Advance(1.f/60.f);
            const FName Name=Anim->GetCurrentStateName(Machine);
            if (Name!=Previous) { History+=Name.ToString()+TEXT(" "); Previous=Name; }
            SawTransition |= Name==TEXT("StanceTransition");
            if (SawTransition && Name==(Moving ? TEXT("Cycle") : TEXT("Idle"))) SawReturn=true;
        }
        TestTrue(TEXT("The request entered the new stance layer"), SawTransition);
        TestTrue(Moving ? TEXT("Moving completion returns through the conduit to Cycle") : TEXT("Stationary completion returns through the conduit to Idle"), SawReturn);
        TestFalse(TEXT("Conduit return leaves no posture lock"), State->IsMotionChanging());
        if (!SawReturn) AddError(FString::Printf(TEXT("Return history=%s moving=%d actualMoving=%d complete=%d velocity=%s"),*History,Moving,Cast<UCharacterAnimInstance>(Anim)->_stanceTransitionMoving,Cast<UCharacterAnimInstance>(Anim)->_stanceTransitionComplete,*Pawn->GetVelocity().ToString()));
    }
    RuntimeMachine->MaxTransitionsPerFrame=OriginalMaxTransitions;
    Pawn->GetCharacterMovement()->Velocity = FVector::ZeroVector;
    State->Reset(); State->SetCharacterState(ECharacterState::Standing); State->StartProne();
    Pawn->PrepareMotionChange();
    Advance(.05f);
    State->CancelAnimationChanges();
    Advance(.3f);
    TestFalse(TEXT("Interrupted stance playback releases lock"), State->IsMotionChanging());
    State->Reset(); State->SetCharacterState(ECharacterState::Standing); State->StartCrouch();
    Pawn->PrepareMotionChange();
    Mesh->SetAnimInstanceClass(nullptr);
    Pawn->UpdateAnimationChanges();
    TestFalse(TEXT("Anim instance removal clears pending locks"), State->IsMotionChanging());
    return true;
}
#endif
