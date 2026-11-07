#include "CharacterAnimationPlaybackVerify.h"
#include "CharacterProneRollVerify.h"
#include "CoreGlobals.h"

#include "Animation/AnimInstance.h"
#include "Animation/AnimClassInterface.h"
#include "Animation/AnimNode_SequencePlayer.h"
#include "Animation/AnimSequenceBase.h"
#include "AssetCompilingManager.h"
#include "../../SGAProjectMain/Character/CharacterAnimInstance.h"
#include "../../SGAProjectMain/Character/HellDiver/HellDiver.h"
#include "../../SGAProjectMain/Character/HellDiver/HellDiverStateComponent.h"
#include "Components/SceneComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Dom/JsonObject.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/World.h"
#include "Engine/WorldInitializationValues.h"
#include "HAL/CriticalSection.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "Misc/OutputDevice.h"
#include "Misc/OutputDeviceRedirector.h"
#include "Misc/ScopeExit.h"
#include "Misc/ScopeLock.h"
#include "UObject/UObjectGlobals.h"
#include "UObject/NoExportTypes.h"
#include "UObject/UnrealType.h"

namespace
{
    class FPlaybackLogCapture final : public FOutputDevice
    {
        FCriticalSection Mutex;
        TArray<FString> Messages;
        int32 Errors = 0;
        int32 DynamicWarnings = 0;
    public:
        FPlaybackLogCapture() { if (GLog) GLog->AddOutputDevice(this); }
        ~FPlaybackLogCapture() { if (GLog) GLog->RemoveOutputDevice(this); }
        virtual bool CanBeUsedOnAnyThread() const override { return true; }
        virtual bool CanBeUsedOnMultipleThreads() const override { return true; }
        virtual void Serialize(const TCHAR* Message, ELogVerbosity::Type Verbosity, const FName& Category) override
        {
            if (Verbosity > ELogVerbosity::Warning) return;
            FScopeLock Guard(&Mutex);
            const FString Text(Message);
            if (Messages.Num() < 100) Messages.Add(Category.ToString() + TEXT(": ") + Text);
            if (Verbosity <= ELogVerbosity::Error) ++Errors;
            if (Text.Contains(TEXT("not dynamic"), ESearchCase::IgnoreCase) ||
                Text.Contains(TEXT("isn't dynamic"), ESearchCase::IgnoreCase) ||
                Text.Contains(TEXT("SetSequence"), ESearchCase::IgnoreCase)) ++DynamicWarnings;
        }
        bool Record(TSharedRef<FJsonObject> Result)
        {
            FScopeLock Guard(&Mutex);
            TArray<TSharedPtr<FJsonValue>> Values;
            for (const FString& Message : Messages) Values.Add(MakeShared<FJsonValueString>(Message));
            Result->SetArrayField(TEXT("evaluation_warnings_and_errors"), Values);
            Result->SetNumberField(TEXT("evaluation_error_count"), Errors);
            Result->SetNumberField(TEXT("dynamic_property_warning_count"), DynamicWarnings);
            return Errors == 0 && DynamicWarnings == 0;
        }
    };

    TSharedRef<FJsonObject> TransformJson(const FTransform& Transform)
    {
        auto Value = MakeShared<FJsonObject>();
        Value->SetStringField(TEXT("translation"), Transform.GetTranslation().ToString());
        Value->SetStringField(TEXT("quaternion"), Transform.GetRotation().ToString());
        Value->SetStringField(TEXT("scale"), Transform.GetScale3D().ToString());
        return Value;
    }

    TArray<TSharedPtr<FJsonValue>> PlayerSnapshot(UAnimInstance* Instance, bool& bHasAssigned)
    {
        TArray<TSharedPtr<FJsonValue>> Values;
        const IAnimClassInterface* Class = Instance ? IAnimClassInterface::GetFromClass(Instance->GetClass()) : nullptr;
        if (!Class) return Values;
        for (const FStructProperty* Property : Class->GetAnimNodeProperties())
        {
            if (!Property || !Property->Struct->IsChildOf(FAnimNode_SequencePlayer::StaticStruct())) continue;
            const auto* Player = Property->ContainerPtrToValuePtr<FAnimNode_SequencePlayer>(Instance);
            UAnimSequenceBase* Sequence = Player->GetSequence();
            if (!Sequence) continue;
            bHasAssigned = true;
            auto Value = MakeShared<FJsonObject>();
            Value->SetStringField(TEXT("node_property"), Property->GetName());
            Value->SetStringField(TEXT("sequence"), Sequence->GetPathName());
            Value->SetNumberField(TEXT("cached_blend_weight"), Player->GetCachedBlendWeight());
            Values.Add(MakeShared<FJsonValueObject>(Value));
        }
        return Values;
    }
}

static bool VerifyCharacterAnimationInternal(TSharedRef<FJsonObject> Result, bool bVerifyProneRoll = false)
{
    FPlaybackLogCapture Capture;
    Result->SetStringField(TEXT("scope"), TEXT("Actual MK5 and selected linked weapon layer in a transient EditorPreview world; no weapon or muzzle accuracy assertion."));
    Result->SetBoolField(TEXT("deferred_construction"), true);
    Result->SetBoolField(TEXT("begin_play_called"), false);
    Result->SetBoolField(TEXT("actor_tick_called"), false);
    Result->SetBoolField(TEXT("world_tick_called"), false);
    Result->SetBoolField(TEXT("assets_saved"), false);
    Result->SetBoolField(TEXT("serial_evaluation_override_for_harness_only"), true);
    Result->SetBoolField(TEXT("runtime_or_asset_threading_setting_changed"), false);
    Result->SetBoolField(TEXT("engine_frame_counter_advanced_for_each_manual_frame"), true);
    const auto Fail = [&Capture, &Result](const FString& Message)
    {
        Result->SetStringField(TEXT("error"), Message);
        Result->SetBoolField(TEXT("passed"), false);
        Capture.Record(Result);
        UE_LOG(LogTemp, Error, TEXT("CharacterAnimationPlaybackVerify: %s"), *Message);
        return false;
    };

    FString LayerName(TEXT("Rifle"));
    FParse::Value(FCommandLine::Get(), TEXT("VerifyLayer="), LayerName);
    if (LayerName.Equals(TEXT("Rifle"), ESearchCase::IgnoreCase)) LayerName = TEXT("Rifle");
    else if (LayerName.Equals(TEXT("Pistol"), ESearchCase::IgnoreCase)) LayerName = TEXT("Pistol");
    else if (LayerName.Equals(TEXT("Unarmed"), ESearchCase::IgnoreCase)) LayerName = TEXT("Unarmed");
    else return Fail(TEXT("VerifyLayer must be Rifle, Pistol or Unarmed."));
    Result->SetStringField(TEXT("verification_weapon_layer"), LayerName);
    const FString LayerPath = FString::Printf(TEXT("/Game/Blueprints/HellDiver/Animation_Locomotion/ABP_Helldiver_%s.ABP_Helldiver_%s_C"), *LayerName, *LayerName);
    UClass* PawnClass = LoadClass<AHellDiver>(nullptr, TEXT("/Game/Blueprints/HellDiver/BP_HellDiver_MK2.BP_HellDiver_MK2_C"));
    UClass* AnimClass = LoadClass<UAnimInstance>(nullptr, TEXT("/Game/Blueprints/HellDiver/Animation_Locomotion/ABP_Helldiver_MK5.ABP_Helldiver_MK5_C"));
    UClass* RifleClass = LoadClass<UAnimInstance>(nullptr, *LayerPath);
    if (!PawnClass || !AnimClass || !RifleClass) return Fail(TEXT("Could not load the exact pawn, MK5 or selected layer class."));
    Result->SetStringField(TEXT("linked_layer"), RifleClass->GetPathName());
    FAssetCompilingManager::Get().FinishAllCompilation();

    const UWorld::InitializationValues IVS = UWorld::InitializationValues()
        .AllowAudioPlayback(false).RequiresHitProxies(false).CreatePhysicsScene(true)
        .CreateNavigation(false).CreateAISystem(false).ShouldSimulatePhysics(false)
        .EnableTraceCollision(true).SetTransactional(false).CreateFXSystem(false);
    UWorld* World = UWorld::CreateWorld(EWorldType::EditorPreview, false,
        MakeUniqueObjectName(GetTransientPackage(), UWorld::StaticClass(), TEXT("CharacterAnimationPlaybackVerify")),
        nullptr, true, ERHIFeatureLevel::Num, &IVS);
    if (!World) return Fail(TEXT("Could not create the isolated EditorPreview world."));
    ON_SCOPE_EXIT { World->DestroyWorld(false); };
    if (World->HasBegunPlay() || World->GetGameInstance()) return Fail(TEXT("Preview world unexpectedly has gameplay initialization."));

    FActorSpawnParameters Spawn;
    Spawn.ObjectFlags = RF_Transient;
    Spawn.bDeferConstruction = true;
    Spawn.bTemporaryEditorActor = true;
    Spawn.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    const FVector InitialPosition(-1000.f, 0.f, 0.f);
    const FRotator Rotation = FRotator::ZeroRotator;
    AHellDiver* Pawn = Cast<AHellDiver>(World->SpawnActor(PawnClass, &InitialPosition, &Rotation, Spawn));
    if (!Pawn || Pawn->HasActorBegunPlay()) return Fail(TEXT("Deferred preview pawn creation failed or unexpectedly began play."));
    Pawn->SetActorTickEnabled(false);
    Pawn->SetActorEnableCollision(false);
    USkeletalMeshComponent* Mesh = Pawn->GetMesh();
    UHellDiverStateComponent* State = Pawn->GetStateComponent();
    if (!Mesh || !State || !Mesh->GetSkeletalMeshAsset()) return Fail(TEXT("Deferred pawn lacks its native Mesh, State or actual skeletal mesh asset."));
    if (Mesh->GetAnimClass() != AnimClass) return Fail(TEXT("Actual BP mesh does not use the expected MK5 animation class."));
    if (!Pawn->GetFocusingLoc().IsNearlyZero()) return Fail(TEXT("Expected the unchanged native focusing target at world origin."));
    if (USceneComponent* Root = Pawn->GetRootComponent())
        if (!Root->IsRegistered()) Root->RegisterComponentWithWorld(World);
    Mesh->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;
    Mesh->bEnableUpdateRateOptimizations = false;
    Mesh->SetUpdateAnimationInEditor(true);
    Mesh->SetComponentTickEnabled(false);
    if (!Mesh->IsRegistered()) Mesh->RegisterComponentWithWorld(World);
    Mesh->SetAnimationMode(EAnimationMode::AnimationBlueprint, false);
    Mesh->SetAnimInstanceClass(AnimClass);
    Mesh->InitAnim(true);
    Mesh->LinkAnimClassLayers(RifleClass);
    UCharacterAnimInstance* Anim = Cast<UCharacterAnimInstance>(Mesh->GetAnimInstance());
    if (!Anim || !Anim->GetLinkedAnimLayerInstanceByClass(RifleClass)) return Fail(TEXT("Actual main or selected linked animation instance was not initialized."));
    const FBoolProperty* MultiThreaded = FindFProperty<FBoolProperty>(UAnimInstance::StaticClass(), TEXT("bUseMultiThreadedAnimationUpdate"));
    if (MultiThreaded)
    {
        MultiThreaded->SetPropertyValue_InContainer(Anim, false);
        if (UAnimInstance* Rifle = Anim->GetLinkedAnimLayerInstanceByClass(RifleClass)) MultiThreaded->SetPropertyValue_InContainer(Rifle, false);
    }
    if (bVerifyProneRoll)
    {
        const bool bCasesPassed = EvaluateCharacterProneRollCases(Pawn, Mesh, Anim, State, Result);
        const bool bLogsClean = Capture.Record(Result);
        Result->SetBoolField(TEXT("pawn_began_play"), Pawn->HasActorBegunPlay());
        Result->SetBoolField(TEXT("world_began_play"), World->HasBegunPlay());
        const bool bPassed = bCasesPassed && bLogsClean && !Pawn->HasActorBegunPlay() && !World->HasBegunPlay();
        Result->SetBoolField(TEXT("passed"), bPassed);
        return bPassed;
    }
    Result->SetStringField(TEXT("pawn_class"), PawnClass->GetPathName());
    Result->SetStringField(TEXT("mesh_asset"), Mesh->GetSkeletalMeshAsset()->GetPathName());
    Result->SetStringField(TEXT("anim_class"), Anim->GetClass()->GetPathName());
    Result->SetStringField(TEXT("linked_layer"), RifleClass->GetPathName());
    Result->SetBoolField(TEXT("weapon_attached"), false);
    Result->SetBoolField(TEXT("right_aim_rig_accuracy_tested"), false);
    Result->SetNumberField(TEXT("frames_per_case"), 120);
    Result->SetNumberField(TEXT("delta_seconds"), 1.0 / 60.0);
    const int32 PelvisIndex = Mesh->GetBoneIndex(TEXT("pelvis"));
    if (PelvisIndex == INDEX_NONE) return Fail(TEXT("Actual mesh has no pelvis bone."));
    const FFloatProperty* AimYaw = FindFProperty<FFloatProperty>(Anim->GetClass(), TEXT("_actorYaw"));
    const FFloatProperty* RootYaw = FindFProperty<FFloatProperty>(Anim->GetClass(), TEXT("_rootYawOffset"));
    if (!AimYaw || !RootYaw) return Fail(TEXT("Expected native diagnostic yaw fields are missing."));

    struct FCase { const TCHAR* Name; ECharacterState Stance; FVector Position; ELocomotionDirections Direction; };
    const FCase Cases[] = {
        {TEXT("StandingForward"), ECharacterState::Standing, {-1000.f,0.f,0.f}, ELocomotionDirections::Forward},
        {TEXT("CrouchingForward"), ECharacterState::Crouching, {-1000.f,0.f,0.f}, ELocomotionDirections::Forward},
        {TEXT("ProneForward"), ECharacterState::Proning, {-1000.f,0.f,0.f}, ELocomotionDirections::Forward},
        {TEXT("ProneRight"), ECharacterState::Proning, {0.f,-1000.f,0.f}, ELocomotionDirections::Right},
        {TEXT("ProneBackward"), ECharacterState::Proning, {1000.f,0.f,0.f}, ELocomotionDirections::Backward},
        {TEXT("ProneLeft"), ECharacterState::Proning, {0.f,1000.f,0.f}, ELocomotionDirections::Left}
    };
    TArray<TSharedPtr<FJsonValue>> Rows;
    FQuat StandingPelvis = FQuat::Identity;
    FVector StandingPelvisLocation = FVector::ZeroVector;
    const TArray<FTransform>& RefBones = Mesh->GetSkeletalMeshAsset()->GetRefSkeleton().GetRefBonePose();
    if (!RefBones.IsValidIndex(PelvisIndex)) return Fail(TEXT("Reference skeleton does not contain the evaluated pelvis index."));
    FTransform ReferencePelvis = RefBones[PelvisIndex];
    for (int32 Parent = Mesh->GetSkeletalMeshAsset()->GetRefSkeleton().GetParentIndex(PelvisIndex); Parent != INDEX_NONE;
        Parent = Mesh->GetSkeletalMeshAsset()->GetRefSkeleton().GetParentIndex(Parent)) ReferencePelvis = ReferencePelvis * RefBones[Parent];
    Result->SetObjectField(TEXT("reference_pelvis_component_space"), TransformJson(ReferencePelvis));
    bool bAllPassed = true;
    for (const FCase& Case : Cases)
    {
        State->SetCharacterState(Case.Stance);
        const bool bMoved = Pawn->SetActorLocationAndRotation(Case.Position, FRotator::ZeroRotator, false, nullptr, ETeleportType::TeleportPhysics);
        Mesh->UpdateComponentToWorld();
        bool bFinite = true;
        for (int32 Frame = 0; Frame < 120; ++Frame)
        {
            // The proxy deliberately runs NativeThreadSafeUpdateAnimation once per
            // GFrameCounter, even when UpdateAnimation is called repeatedly by hand.
            ++GFrameCounter;
            Mesh->TickAnimation(1.f / 60.f, false);
            Mesh->RefreshBoneTransforms(nullptr);
            for (const FTransform& Bone : Mesh->GetComponentSpaceTransforms()) bFinite &= !Bone.ContainsNaN();
            if (!bFinite) break;
        }
        const TArray<FTransform>& Bones = Mesh->GetComponentSpaceTransforms();
        if (!Bones.IsValidIndex(PelvisIndex)) return Fail(TEXT("Evaluated pose has no expected pelvis transform."));
        const FTransform Pelvis = Bones[PelvisIndex];
        if (Case.Stance == ECharacterState::Standing)
        {
            StandingPelvis = Pelvis.GetRotation();
            StandingPelvisLocation = Pelvis.GetTranslation();
        }
        const double PelvisDifference = FMath::RadiansToDegrees(StandingPelvis.AngularDistance(Pelvis.GetRotation()));
        const double PelvisLocationDifference = FVector::Distance(StandingPelvisLocation, Pelvis.GetTranslation());
        const bool bStanceMatches = Anim->GetCurrentState() == Case.Stance;
        const bool bDirectionMatches = Case.Stance != ECharacterState::Proning || Anim->GetPronePoseDirection() == Case.Direction;
        const bool bBodyPoseChanged = Case.Stance != ECharacterState::Proning || PelvisDifference > 5.0;
        const bool bCrouchPoseChanged = Case.Stance != ECharacterState::Crouching || PelvisDifference > 5.0 || PelvisLocationDifference > 5.0;
        bool bHasAssignedSequence = false;
        const auto Players = PlayerSnapshot(Anim->GetLinkedAnimLayerInstanceByClass(RifleClass), bHasAssignedSequence);
        const bool bPositionMatches = Pawn->GetActorLocation().Equals(Case.Position, 0.01f);
        const bool bPassed = bFinite && bStanceMatches && bDirectionMatches && bBodyPoseChanged && bCrouchPoseChanged && bHasAssignedSequence && bPositionMatches;
        auto Row = MakeShared<FJsonObject>();
        Row->SetStringField(TEXT("case"), Case.Name);
        Row->SetStringField(TEXT("requested_actor_location"), Case.Position.ToString());
        Row->SetStringField(TEXT("actor_location"), Pawn->GetActorLocation().ToString());
        Row->SetBoolField(TEXT("actor_location_matches"), bPositionMatches);
        Row->SetBoolField(TEXT("set_actor_location_returned"), bMoved);
        Row->SetStringField(TEXT("target_world"), Pawn->GetFocusingLoc().ToString());
        Row->SetNumberField(TEXT("expected_stance"), static_cast<int32>(Case.Stance));
        Row->SetNumberField(TEXT("actual_stance"), static_cast<int32>(Anim->GetCurrentState()));
        Row->SetNumberField(TEXT("expected_prone_direction"), static_cast<int32>(Case.Direction));
        Row->SetNumberField(TEXT("actual_prone_direction"), static_cast<int32>(Anim->GetPronePoseDirection()));
        Row->SetNumberField(TEXT("actor_relative_aim_yaw"), AimYaw->GetPropertyValue_InContainer(Anim));
        Row->SetNumberField(TEXT("root_yaw_offset"), RootYaw->GetPropertyValue_InContainer(Anim));
        auto Snapshots = MakeShared<FJsonObject>();
        for (const TCHAR* Name : {TEXT("_worldLocation"), TEXT("_aimOriginWorld"), TEXT("_targetPos_World"), TEXT("_targetPos_Mesh")})
        {
            const FStructProperty* Property = FindFProperty<FStructProperty>(Anim->GetClass(), FName(Name));
            if (Property && Property->Struct == TBaseStructure<FVector>::Get())
                Snapshots->SetStringField(Name, Property->ContainerPtrToValuePtr<FVector>(Anim)->ToString());
        }
        for (const TCHAR* Name : {TEXT("_hasAimSnapshot"), TEXT("_hasValidAimDirection")})
            if (const FBoolProperty* Property = FindFProperty<FBoolProperty>(Anim->GetClass(), FName(Name)))
                Snapshots->SetBoolField(Name, Property->GetPropertyValue_InContainer(Anim));
        Row->SetObjectField(TEXT("native_snapshots"), Snapshots);
        Row->SetObjectField(TEXT("pelvis_component_space"), TransformJson(Pelvis));
        Row->SetNumberField(TEXT("pelvis_rotation_difference_from_standing_degrees"), PelvisDifference);
        Row->SetNumberField(TEXT("pelvis_location_difference_from_standing_units"), PelvisLocationDifference);
        Row->SetNumberField(TEXT("pelvis_rotation_difference_from_reference_degrees"),
            FMath::RadiansToDegrees(ReferencePelvis.GetRotation().AngularDistance(Pelvis.GetRotation())));
        Row->SetNumberField(TEXT("pelvis_location_difference_from_reference_units"),
            FVector::Distance(ReferencePelvis.GetTranslation(), Pelvis.GetTranslation()));
        Row->SetArrayField(TEXT("assigned_linked_sequence_players"), Players);
        Row->SetBoolField(TEXT("linked_layer_has_assigned_sequence"), bHasAssignedSequence);
        Row->SetNumberField(TEXT("bone_count"), Bones.Num());
        Row->SetBoolField(TEXT("all_bone_transforms_finite"), bFinite);
        Row->SetBoolField(TEXT("stance_matches"), bStanceMatches);
        Row->SetBoolField(TEXT("direction_matches"), bDirectionMatches);
        Row->SetBoolField(TEXT("prone_differs_from_standing"), bBodyPoseChanged);
        Row->SetBoolField(TEXT("crouch_differs_from_standing"), bCrouchPoseChanged);
        Row->SetBoolField(TEXT("passed"), bPassed);
        Rows.Add(MakeShared<FJsonValueObject>(Row));
        Result->SetArrayField(TEXT("cases"), Rows);
        bAllPassed &= bPassed;
    }
    const bool bLogsClean = Capture.Record(Result);
    Result->SetBoolField(TEXT("pawn_began_play"), Pawn->HasActorBegunPlay());
    Result->SetBoolField(TEXT("world_began_play"), World->HasBegunPlay());
    const bool bPassed = bAllPassed && bLogsClean && !Pawn->HasActorBegunPlay() && !World->HasBegunPlay();
    Result->SetBoolField(TEXT("passed"), bPassed);
    return bPassed;
}

bool VerifyCharacterAnimationPlayback(TSharedRef<FJsonObject> Result)
{
    return VerifyCharacterAnimationInternal(Result, false);
}

bool VerifyCharacterProneRoll(TSharedRef<FJsonObject> Result)
{
    return VerifyCharacterAnimationInternal(Result, true);
}
