#include "CharacterProneRollVerify.h"

#include "CoreGlobals.h"
#include "../../SGAProjectMain/Character/CharacterAnimInstance.h"
#include "../../SGAProjectMain/Character/PlayerCharacter.h"
#include "../../SGAProjectMain/Character/HellDiver/HellDiverStateComponent.h"
#include "Animation/AnimClassInterface.h"
#include "Animation/AnimSequenceBase.h"
#include "Animation/AnimSequence.h"
#include "Animation/AnimData/IAnimationDataModel.h"
#include "AnimNodes/AnimNode_BlendSpacePlayer.h"
#include "Animation/BlendSpace.h"
#include "Components/ChildActorComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Dom/JsonObject.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "UObject/UnrealType.h"

namespace
{
    TSharedRef<FJsonObject> VectorJson(const FVector& Value)
    {
        auto Object = MakeShared<FJsonObject>();
        Object->SetNumberField(TEXT("x"), Value.X);
        Object->SetNumberField(TEXT("y"), Value.Y);
        Object->SetNumberField(TEXT("z"), Value.Z);
        return Object;
    }

    TSharedRef<FJsonObject> TransformJson(const FTransform& Value)
    {
        auto Object = MakeShared<FJsonObject>();
        Object->SetObjectField(TEXT("position"), VectorJson(Value.GetLocation()));
        Object->SetObjectField(TEXT("axis_x"), VectorJson(Value.GetRotation().GetAxisX()));
        Object->SetObjectField(TEXT("axis_y"), VectorJson(Value.GetRotation().GetAxisY()));
        Object->SetObjectField(TEXT("axis_z"), VectorJson(Value.GetRotation().GetAxisZ()));
        Object->SetObjectField(TEXT("scale"), VectorJson(Value.GetScale3D()));
        return Object;
    }

    FVector BoneWorld(const USkeletalMeshComponent* Mesh, FName Name)
    {
        return Mesh->GetComponentTransform().TransformPosition(
            Mesh->GetComponentSpaceTransforms()[Mesh->GetBoneIndex(Name)].GetLocation());
    }

    double AngleDegrees(const FVector& A, const FVector& B)
    {
        if (A.IsNearlyZero() || B.IsNearlyZero()) return 180.0;
        return FMath::RadiansToDegrees(FMath::Acos(FMath::Clamp(FVector::DotProduct(A.GetSafeNormal(), B.GetSafeNormal()), -1.0, 1.0)));
    }

    struct FBodyGeometry
    {
        FVector Center;
        FVector Longitudinal;
        FVector TorsoNormal;
        FVector PelvisPositiveY;
        FVector Head;
        FVector Feet;
    };

    FBodyGeometry ReadBody(const USkeletalMeshComponent* Mesh, double NormalSign)
    {
        FBodyGeometry Value;
        Value.Head = BoneWorld(Mesh, TEXT("head"));
        Value.Feet = (BoneWorld(Mesh, TEXT("foot_l")) + BoneWorld(Mesh, TEXT("foot_r"))) * 0.5;
        Value.Center = BoneWorld(Mesh, TEXT("pelvis"));
        Value.Longitudinal = (Value.Head - Value.Feet).GetSafeNormal2D();
        Value.TorsoNormal = FVector::CrossProduct(
            BoneWorld(Mesh, TEXT("upperarm_r")) - BoneWorld(Mesh, TEXT("upperarm_l")),
            BoneWorld(Mesh, TEXT("spine_03")) - Value.Center).GetSafeNormal() * NormalSign;
        Value.PelvisPositiveY = Mesh->GetComponentTransform().TransformVectorNoScale(
            Mesh->GetComponentSpaceTransforms()[Mesh->GetBoneIndex(TEXT("pelvis"))].GetRotation().GetAxisY()).GetSafeNormal();
        return Value;
    }

    TSharedRef<FJsonObject> PoseJson(const USkeletalMeshComponent* Mesh, const FBodyGeometry& Body)
    {
        auto Object = MakeShared<FJsonObject>();
        Object->SetObjectField(TEXT("pelvis_center_world"), VectorJson(Body.Center));
        Object->SetObjectField(TEXT("head_world"), VectorJson(Body.Head));
        Object->SetObjectField(TEXT("feet_midpoint_world"), VectorJson(Body.Feet));
        Object->SetObjectField(TEXT("longitudinal_xy"), VectorJson(Body.Longitudinal));
        Object->SetObjectField(TEXT("torso_normal_world"), VectorJson(Body.TorsoNormal));
        Object->SetObjectField(TEXT("pelvis_positive_y_world"), VectorJson(Body.PelvisPositiveY));
        auto Bones = MakeShared<FJsonObject>();
        for (const TCHAR* Name : { TEXT("head"), TEXT("pelvis"), TEXT("spine_01"), TEXT("spine_02"), TEXT("spine_03"),
            TEXT("clavicle_r"), TEXT("clavicle_l"), TEXT("upperarm_r"), TEXT("upperarm_l"), TEXT("foot_r"), TEXT("foot_l"),
            TEXT("ball_r"), TEXT("ball_l"), TEXT("hand_r"), TEXT("hand_l") })
        {
            const int32 Index = Mesh->GetBoneIndex(Name);
            if (Index == INDEX_NONE) continue;
            auto Bone = MakeShared<FJsonObject>();
            Bone->SetNumberField(TEXT("index"), Index);
            const FTransform& CS = Mesh->GetComponentSpaceTransforms()[Index];
            Bone->SetObjectField(TEXT("component_space"), TransformJson(CS));
            Bone->SetObjectField(TEXT("world"), TransformJson(CS * Mesh->GetComponentTransform()));
            Bones->SetObjectField(Name, Bone);
        }
        Object->SetObjectField(TEXT("bones"), Bones);
        return Object;
    }

    void DisableHeadAnchor(UAnimInstance* Instance, TArray<TSharedPtr<FJsonValue>>& Disabled)
    {
        const IAnimClassInterface* Class = Instance ? IAnimClassInterface::GetFromClass(Instance->GetClass()) : nullptr;
        if (!Class) return;
        for (const FStructProperty* Property : Class->GetAnimNodeProperties())
        {
            if (!Property || Property->Struct->GetName() != TEXT("AnimNode_ProneHeadAnchor")) continue;
            const FBoolProperty* Enabled = FindFProperty<FBoolProperty>(Property->Struct, TEXT("bEnabled"));
            if (!Enabled) continue;
            void* Node = Property->ContainerPtrToValuePtr<void>(Instance);
            Enabled->SetPropertyValue_InContainer(Node, false);
            Disabled.Add(MakeShared<FJsonValueString>(Instance->GetClass()->GetPathName() + TEXT(":") + Property->GetName()));
        }
    }

    bool ApplyTransientRollCandidate(UAnimInstance* Rifle, TSharedRef<FJsonObject> Result)
    {
        const FStructProperty* Idle = Rifle ? FindFProperty<FStructProperty>(Rifle->GetClass(), TEXT("_proneIdleAnims")) : nullptr;
        if (!Idle)
        {
            Result->SetStringField(TEXT("error"), TEXT("Transient roll candidate could not find Rifle _proneIdleAnims."));
            return false;
        }
        void* IdleValues = Idle->ContainerPtrToValuePtr<void>(Rifle);
        struct FCandidate { const TCHAR* Field; const TCHAR* Asset; };
        const FCandidate Candidates[] = {
            { TEXT("Right"), TEXT("Hdv_Male_Laying_Left1_Reatergeted") },
            { TEXT("Left"), TEXT("Hdv_Male_Laying_Right1_Reatergeted") },
            { TEXT("Backward"), TEXT("Hdv_Male_Lying1_Reatergeted") }
        };
        TArray<TSharedPtr<FJsonValue>> Changes;
        for (const FCandidate& Candidate : Candidates)
        {
            const FObjectPropertyBase* Field = FindFProperty<FObjectPropertyBase>(Idle->Struct, Candidate.Field);
            const FString Path = FString(TEXT("/Game/Blueprints/HellDiver/Animation/Retargeted_Anim/Prone/Idle/")) + Candidate.Asset + TEXT(".") + Candidate.Asset;
            UAnimSequenceBase* Sequence = LoadObject<UAnimSequenceBase>(nullptr, *Path);
            if (!Field || !Sequence)
            {
                Result->SetStringField(TEXT("error"), FString(TEXT("Could not apply transient candidate: ")) + Candidate.Field + TEXT(" -> ") + Path);
                return false;
            }
            auto Row = MakeShared<FJsonObject>();
            Row->SetStringField(TEXT("field"), Candidate.Field);
            Row->SetStringField(TEXT("before"), GetPathNameSafe(Field->GetObjectPropertyValue_InContainer(IdleValues)));
            Field->SetObjectPropertyValue_InContainer(IdleValues, Sequence);
            Row->SetStringField(TEXT("after"), GetPathNameSafe(Field->GetObjectPropertyValue_InContainer(IdleValues)));
            Changes.Add(MakeShared<FJsonValueObject>(Row));
        }
        Result->SetArrayField(TEXT("transient_candidate_idle_sequence_changes"), Changes);
        Result->SetBoolField(TEXT("transient_candidate_applied"), true);
        Result->SetBoolField(TEXT("transient_candidate_aim_offsets_changed"), false);
        return true;
    }

    TSharedRef<FJsonObject> InspectIdleMotionSource(UAnimInstance* Rifle, ELocomotionDirections Direction)
    {
        auto Result = MakeShared<FJsonObject>();
        Result->SetBoolField(TEXT("animated_head_or_spine_source_variation"), false);
        const TCHAR* FieldName = TEXT("Forward");
        if (Direction == ELocomotionDirections::Right) FieldName = TEXT("Right");
        else if (Direction == ELocomotionDirections::Backward) FieldName = TEXT("Backward");
        else if (Direction == ELocomotionDirections::Left) FieldName = TEXT("Left");
        const FStructProperty* Idle = Rifle ? FindFProperty<FStructProperty>(Rifle->GetClass(), TEXT("_proneIdleAnims")) : nullptr;
        const FObjectPropertyBase* Field = Idle ? FindFProperty<FObjectPropertyBase>(Idle->Struct, FieldName) : nullptr;
        UAnimSequence* Sequence = Field ? Cast<UAnimSequence>(Field->GetObjectPropertyValue_InContainer(Idle->ContainerPtrToValuePtr<void>(Rifle))) : nullptr;
        Result->SetStringField(TEXT("sequence"), GetPathNameSafe(Sequence));
        Result->SetBoolField(TEXT("sequence_inspected"), Sequence != nullptr);
        if (!Sequence) return Result;
        Result->SetNumberField(TEXT("play_length_seconds"), Sequence->GetPlayLength());
        Result->SetNumberField(TEXT("sampled_key_count"), Sequence->GetNumberOfSampledKeys());
        const IAnimationDataModel* Model = Sequence->GetDataModel();
        Result->SetBoolField(TEXT("source_data_model_available"), Model != nullptr);
        if (!Model) return Result;
        Result->SetNumberField(TEXT("source_frame_count"), Model->GetNumberOfFrames());
        Result->SetNumberField(TEXT("source_key_count"), Model->GetNumberOfKeys());
        double MaxTranslation = 0.0, MaxRotation = 0.0;
        for (const TCHAR* BoneName : { TEXT("pelvis"), TEXT("spine_01"), TEXT("spine_02"), TEXT("spine_03"), TEXT("head") })
        {
            if (!Model->IsValidBoneTrackName(BoneName)) continue;
            TArray<FTransform> Keys;
            Model->GetBoneTrackTransforms(BoneName, Keys);
            if (Keys.Num() < 2) continue;
            for (const FTransform& Key : Keys)
            {
                MaxTranslation = FMath::Max(MaxTranslation, FVector::Distance(Keys[0].GetTranslation(), Key.GetTranslation()));
                MaxRotation = FMath::Max(MaxRotation, FMath::RadiansToDegrees(Keys[0].GetRotation().AngularDistance(Key.GetRotation())));
            }
        }
        Result->SetNumberField(TEXT("max_source_head_spine_pelvis_translation_variation_cm"), MaxTranslation);
        Result->SetNumberField(TEXT("max_source_head_spine_pelvis_rotation_variation_degrees"), MaxRotation);
        Result->SetBoolField(TEXT("animated_head_or_spine_source_variation"), Model->GetNumberOfKeys() > 1 && (MaxTranslation > 0.01 || MaxRotation > 0.01));
        return Result;
    }

    TArray<TSharedPtr<FJsonValue>> BlendSpaceSnapshot(UAnimInstance* Instance)
    {
        TArray<TSharedPtr<FJsonValue>> Values;
        const IAnimClassInterface* Class = Instance ? IAnimClassInterface::GetFromClass(Instance->GetClass()) : nullptr;
        if (!Class) return Values;
        for (const FStructProperty* Property : Class->GetAnimNodeProperties())
        {
            if (!Property || !Property->Struct->IsChildOf(FAnimNode_BlendSpacePlayerBase::StaticStruct())) continue;
            const auto* Player = Property->ContainerPtrToValuePtr<FAnimNode_BlendSpacePlayerBase>(Instance);
            if (!Player->GetBlendSpace()) continue;
            auto Value = MakeShared<FJsonObject>();
            Value->SetStringField(TEXT("instance"), Instance->GetClass()->GetPathName());
            Value->SetStringField(TEXT("node_property"), Property->GetName());
            Value->SetStringField(TEXT("node_struct"), Property->Struct->GetName());
            Value->SetStringField(TEXT("blendspace"), Player->GetBlendSpace()->GetPathName());
            Value->SetNumberField(TEXT("cached_blend_weight"), Player->GetCachedBlendWeight());
            Value->SetNumberField(TEXT("current_asset_time"), Player->GetCurrentAssetTime());
            Value->SetObjectField(TEXT("input_position"), VectorJson(Player->GetPosition()));
            Value->SetObjectField(TEXT("filtered_position"), VectorJson(Player->GetFilteredPosition()));
            Values.Add(MakeShared<FJsonValueObject>(Value));
        }
        return Values;
    }

    TSharedRef<FJsonObject> UnifiedReferenceSnapshot(UAnimInstance* Layer)
    {
        auto Result = MakeShared<FJsonObject>();
        auto AOs = MakeShared<FJsonObject>();
        UObject* FirstAO = nullptr;
        bool bAOsSame = Layer != nullptr;
        for (const TCHAR* Name : { TEXT("_aoProneFwd"), TEXT("_aoProneRight"), TEXT("_aoProneBwd"), TEXT("_aoProneLeft") })
        {
            const FObjectPropertyBase* Property = Layer ? FindFProperty<FObjectPropertyBase>(Layer->GetClass(), Name) : nullptr;
            UObject* Value = Property ? Property->GetObjectPropertyValue_InContainer(Layer) : nullptr;
            AOs->SetStringField(Name, GetPathNameSafe(Value));
            if (!FirstAO) FirstAO = Value;
            bAOsSame &= Value && Value == FirstAO;
        }
        Result->SetObjectField(TEXT("aim_offsets"), AOs);
        Result->SetBoolField(TEXT("all_four_aim_offsets_share_one_nonnull_asset"), bAOsSame);
        auto Idles = MakeShared<FJsonObject>();
        const FStructProperty* Idle = Layer ? FindFProperty<FStructProperty>(Layer->GetClass(), TEXT("_proneIdleAnims")) : nullptr;
        void* IdleValues = Idle ? Idle->ContainerPtrToValuePtr<void>(Layer) : nullptr;
        UObject* FirstIdle = nullptr;
        bool bIdlesSame = IdleValues != nullptr;
        for (const TCHAR* Name : { TEXT("Forward"), TEXT("Right"), TEXT("Backward"), TEXT("Left") })
        {
            const FObjectPropertyBase* Field = Idle ? FindFProperty<FObjectPropertyBase>(Idle->Struct, Name) : nullptr;
            UObject* Value = Field ? Field->GetObjectPropertyValue_InContainer(IdleValues) : nullptr;
            Idles->SetStringField(Name, GetPathNameSafe(Value));
            if (!FirstIdle) FirstIdle = Value;
            bIdlesSame &= Value && Value == FirstIdle;
        }
        Result->SetObjectField(TEXT("idle_sequences"), Idles);
        Result->SetBoolField(TEXT("all_four_idles_share_one_nonnull_asset"), bIdlesSame);
        return Result;
    }
}

bool EvaluateCharacterProneRollCases(AHellDiver* Pawn, USkeletalMeshComponent* Mesh,
    UCharacterAnimInstance* Anim, UHellDiverStateComponent* State, TSharedRef<FJsonObject> Result)
{
    auto* Player = Cast<APlayerCharacter>(Pawn);
    UChildActorComponent* Camera = Player ? Player->GetCurCamera() : nullptr;
    UCharacterMovementComponent* Movement = Pawn ? Pawn->GetCharacterMovement() : nullptr;
    Result->SetStringField(TEXT("scope"), TEXT("Actual BP/MK5/selected weapon layer evaluated poses; fixed Actor; public camera input; natural longitudinal axis and torso roll geometry. No head or bone pinning."));
    Result->SetStringField(TEXT("limitations"), TEXT("Deferred unpossessed preview fixture; no gameplay actor tick, weapon, terrain, network or rendered visual inspection."));
    Result->SetBoolField(TEXT("assets_saved"), false);
    const bool bCandidate = FParse::Param(FCommandLine::Get(), TEXT("ProneRollCandidate")) ||
        FParse::Param(FCommandLine::Get(), TEXT("VerifyProneRollCandidate"));
    Result->SetBoolField(TEXT("transient_candidate_requested"), bCandidate);
    Result->SetBoolField(TEXT("bone_constraints_added"), false);
    Result->SetBoolField(TEXT("head_position_is_a_pass_criterion"), false);
    Result->SetStringField(TEXT("center_metric"), TEXT("Pelvis horizontal displacement from settled initial prone pose."));
    Result->SetStringField(TEXT("axis_metric"), TEXT("XY normalized head minus mean foot position; compared with settled initial prone axis."));
    Result->SetStringField(TEXT("normal_metric"), TEXT("Primary: pelvis +Y world axis is anatomical body front. Secondary: cross(upperarm_r-upperarm_l, spine_03-pelvis), sign calibrated to standing ActorForward; authored shoulder pose can deviate from pelvis front."));
    Result->SetStringField(TEXT("tolerance_status"), TEXT("Initial diagnostic thresholds; animation quality requires visual review."));
    Result->SetNumberField(TEXT("center_xy_tolerance_cm"), 10.0);
    Result->SetNumberField(TEXT("axis_tolerance_degrees"), 25.0);
    Result->SetNumberField(TEXT("primary_pelvis_front_tolerance_degrees"), 35.0);
    Result->SetNumberField(TEXT("secondary_shoulder_normal_tolerance_degrees"), 60.0);
    Result->SetNumberField(TEXT("delta_seconds"), 1.0 / 60.0);
    Result->SetNumberField(TEXT("frames_per_direction"), 120);
    Result->SetStringField(TEXT("cardinal_step_pass_metric"), TEXT("Settled final geometry; step-transition maxima are diagnostic only. Continuous sweep maxima are tested separately."));
    if (!Player || !Mesh || !Anim || !State || !Camera || !Movement || Pawn->GetController())
    {
        Result->SetStringField(TEXT("error"), TEXT("Expected unpossessed player with actual Mesh, Anim, State and native camera."));
        return false;
    }
    TArray<TSharedPtr<FJsonValue>> BoneNames;
    for (int32 Index = 0; Index < Mesh->GetNumBones(); ++Index)
        BoneNames.Add(MakeShared<FJsonValueString>(Mesh->GetBoneName(Index).ToString()));
    Result->SetArrayField(TEXT("actual_bone_names"), BoneNames);
    for (const TCHAR* Name : { TEXT("head"), TEXT("pelvis"), TEXT("spine_03"), TEXT("upperarm_l"), TEXT("upperarm_r"), TEXT("foot_l"), TEXT("foot_r") })
        if (Mesh->GetBoneIndex(Name) == INDEX_NONE)
        {
            Result->SetStringField(TEXT("error"), FString(TEXT("Missing geometry bone: ")) + Name);
            return false;
        }
    TArray<TSharedPtr<FJsonValue>> Disabled;
    DisableHeadAnchor(Anim, Disabled);
    FString LayerName(TEXT("Rifle"));
    FParse::Value(FCommandLine::Get(), TEXT("VerifyLayer="), LayerName);
    if (LayerName.Equals(TEXT("Rifle"), ESearchCase::IgnoreCase)) LayerName = TEXT("Rifle");
    else if (LayerName.Equals(TEXT("Pistol"), ESearchCase::IgnoreCase)) LayerName = TEXT("Pistol");
    else if (LayerName.Equals(TEXT("Unarmed"), ESearchCase::IgnoreCase)) LayerName = TEXT("Unarmed");
    else
    {
        Result->SetStringField(TEXT("error"), TEXT("VerifyLayer must be Rifle, Pistol or Unarmed."));
        return false;
    }
    const FString LayerPath = FString::Printf(TEXT("/Game/Blueprints/HellDiver/Animation_Locomotion/ABP_Helldiver_%s.ABP_Helldiver_%s_C"), *LayerName, *LayerName);
    UClass* RifleClass = LoadClass<UAnimInstance>(nullptr, *LayerPath);
    UAnimInstance* Rifle = RifleClass ? Anim->GetLinkedAnimLayerInstanceByClass(RifleClass) : nullptr;
    Result->SetStringField(TEXT("verification_weapon_layer"), LayerName);
    Result->SetStringField(TEXT("linked_layer"), GetPathNameSafe(Rifle ? Rifle->GetClass() : nullptr));
    DisableHeadAnchor(Rifle, Disabled);
    if (bCandidate && !ApplyTransientRollCandidate(Rifle, Result)) return false;
    Result->SetObjectField(TEXT("unified_layer_asset_references"), UnifiedReferenceSnapshot(Rifle));
    Result->SetArrayField(TEXT("head_anchor_nodes_disabled_in_transient_fixture_only"), Disabled);
    Result->SetNumberField(TEXT("head_anchor_disable_count"), Disabled.Num());
    Result->SetBoolField(TEXT("existing_head_anchor_disabled_or_absent"), true);
    Camera->SetAbsolute(true, true, true);
    Movement->Velocity = FVector::ZeroVector;
    Movement->SetMovementMode(MOVE_Walking);
    bool bAllFinite = true, bAllTargetsMatch = true;
    float CameraYaw = Pawn->GetActorRotation().Yaw;
    float CameraPitch = 0.f;
    const auto Tick = [&]()
    {
        const FVector Origin = Pawn->GetActorLocation() + FVector(0.f, 0.f, 50.f);
        const FRotator Look(CameraPitch, CameraYaw, 0.f);
        Camera->SetWorldLocationAndRotation(Origin, Look);
        Player->CalcFocusingLoc();
        bAllTargetsMatch &= Pawn->GetFocusingLoc().Equals(Origin + Look.Vector() * 10000.f, 0.1f);
        ++GFrameCounter;
        Mesh->TickAnimation(1.f / 60.f, false);
        Mesh->RefreshBoneTransforms(nullptr);
        for (const FTransform& Bone : Mesh->GetComponentSpaceTransforms()) bAllFinite &= !Bone.ContainsNaN();
    };
    const auto Warm = [&](int32 Frames) { for (int32 Frame = 0; Frame < Frames && bAllFinite; ++Frame) Tick(); };
    State->SetCharacterState(ECharacterState::Standing);
    Warm(120);
    FBodyGeometry Standing = ReadBody(Mesh, 1.0);
    const double CalibrationDot = FVector::DotProduct(Standing.TorsoNormal, Pawn->GetActorForwardVector());
    const double NormalSign = CalibrationDot >= 0.0 ? 1.0 : -1.0;
    Standing = ReadBody(Mesh, NormalSign);
    Result->SetNumberField(TEXT("standing_normal_raw_dot_actor_forward"), CalibrationDot);
    Result->SetNumberField(TEXT("torso_normal_sign"), NormalSign);
    Result->SetObjectField(TEXT("standing_pose"), PoseJson(Mesh, Standing));
    State->SetCharacterState(ECharacterState::Proning);
    Warm(180);
    const FBodyGeometry Initial = ReadBody(Mesh, NormalSign);
    Result->SetObjectField(TEXT("initial_prone_pose"), PoseJson(Mesh, Initial));
    const FVector FixedActorLocation = Pawn->GetActorLocation();
    const FQuat FixedActorRotation = Pawn->GetActorQuat();
    Result->SetObjectField(TEXT("fixed_actor_world"), TransformJson(Pawn->GetActorTransform()));
    Result->SetObjectField(TEXT("fixed_mesh_relative"), TransformJson(Mesh->GetRelativeTransform()));
    struct FDirectionCase { const TCHAR* Name; float Yaw; ELocomotionDirections Direction; FVector Normal; };
    const FDirectionCase Directions[] = {
        { TEXT("Forward"), 0.f, ELocomotionDirections::Forward, -FVector::UpVector },
        { TEXT("Right"), 90.f, ELocomotionDirections::Right, Pawn->GetActorRightVector() },
        { TEXT("Backward"), 180.f, ELocomotionDirections::Backward, FVector::UpVector },
        { TEXT("Left"), -90.f, ELocomotionDirections::Left, -Pawn->GetActorRightVector() },
        { TEXT("ForwardAgain"), 0.f, ELocomotionDirections::Forward, -FVector::UpVector }
    };
    TArray<TSharedPtr<FJsonValue>> Cases;
    bool bAllCasesPassed = true;
    const FFloatProperty* AimYaw = FindFProperty<FFloatProperty>(Anim->GetClass(), TEXT("_actorYaw"));
    const FFloatProperty* RootYaw = FindFProperty<FFloatProperty>(Anim->GetClass(), TEXT("_rootYawOffset"));
    const auto ActorUnchanged = [&]()
    {
        return Pawn->GetActorLocation().Equals(FixedActorLocation, 0.01) && Pawn->GetActorQuat().Equals(FixedActorRotation, 0.0001);
    };
    const auto RecordMotionHold = [&](int32 Frames, ELocomotionDirections Direction)
    {
        const FVector StartHead = BoneWorld(Mesh, TEXT("head"));
        const int32 HeadIndex = Mesh->GetBoneIndex(TEXT("head"));
        const int32 SpineIndex = Mesh->GetBoneIndex(TEXT("spine_03"));
        const FQuat StartHeadRotation = Mesh->GetComponentSpaceTransforms()[HeadIndex].GetRotation();
        const FQuat StartSpineRotation = Mesh->GetComponentSpaceTransforms()[SpineIndex].GetRotation();
        double MaxHeadDisplacement = 0.0, MaxBoneRotation = 0.0;
        for (int32 Frame = 0; Frame < Frames && bAllFinite; ++Frame)
        {
            Tick();
            MaxHeadDisplacement = FMath::Max(MaxHeadDisplacement, FVector::Distance(StartHead, BoneWorld(Mesh, TEXT("head"))));
            const double HeadAngle = FMath::RadiansToDegrees(StartHeadRotation.AngularDistance(Mesh->GetComponentSpaceTransforms()[HeadIndex].GetRotation()));
            const double SpineAngle = FMath::RadiansToDegrees(StartSpineRotation.AngularDistance(Mesh->GetComponentSpaceTransforms()[SpineIndex].GetRotation()));
            MaxBoneRotation = FMath::Max(MaxBoneRotation, FMath::Max(HeadAngle, SpineAngle));
        }
        auto Motion = MakeShared<FJsonObject>();
        Motion->SetNumberField(TEXT("hold_frames"), Frames);
        Motion->SetNumberField(TEXT("max_head_world_motion_cm"), MaxHeadDisplacement);
        Motion->SetNumberField(TEXT("max_head_or_spine_component_rotation_degrees"), MaxBoneRotation);
        Motion->SetStringField(TEXT("criterion"), TEXT("Report fixed-input head/spine motion. Require nonzero variation only when source idle pelvis/head/spine keys contain >0.01cm or >0.01deg authored variation; static authored poses are permitted."));
        const bool bMotion = MaxHeadDisplacement > 0.01 || MaxBoneRotation > 0.01;
        auto Source = InspectIdleMotionSource(Rifle, Direction);
        const bool bRequiresMotion = Source->GetBoolField(TEXT("animated_head_or_spine_source_variation"));
        Motion->SetObjectField(TEXT("idle_source"), Source);
        Motion->SetBoolField(TEXT("nonzero_pose_motion"), bMotion);
        Motion->SetBoolField(TEXT("nonzero_motion_required_by_authored_source"), bRequiresMotion);
        Motion->SetBoolField(TEXT("motion_requirement_satisfied"), !bRequiresMotion || bMotion);
        return Motion;
    };
    for (const FDirectionCase& Direction : Directions)
    {
        CameraYaw = Pawn->GetActorRotation().Yaw + Direction.Yaw;
        double MaxCenterXY = 0.0, MaxAxisAngle = 0.0;
        for (int32 Frame = 0; Frame < 120 && bAllFinite; ++Frame)
        {
            Tick();
            const FBodyGeometry Body = ReadBody(Mesh, NormalSign);
            MaxCenterXY = FMath::Max(MaxCenterXY, FVector::Dist2D(Initial.Center, Body.Center));
            MaxAxisAngle = FMath::Max(MaxAxisAngle, AngleDegrees(Initial.Longitudinal, Body.Longitudinal));
        }
        const FBodyGeometry Body = ReadBody(Mesh, NormalSign);
        const double NormalError = AngleDegrees(Body.TorsoNormal, Direction.Normal);
        const double PelvisFrontError = AngleDegrees(Body.PelvisPositiveY, Direction.Normal);
        const bool bActorUnchanged = ActorUnchanged();
        const bool bDirectionMatches = Anim->GetCurrentState() == ECharacterState::Proning && Anim->GetPronePoseDirection() == Direction.Direction;
        const bool bPassed = bAllFinite && bAllTargetsMatch && bActorUnchanged && bDirectionMatches &&
            FVector::Dist2D(Initial.Center, Body.Center) <= 10.0 && AngleDegrees(Initial.Longitudinal, Body.Longitudinal) <= 25.0 && PelvisFrontError <= 35.0 && NormalError <= 60.0;
        auto Row = PoseJson(Mesh, Body);
        Row->SetStringField(TEXT("direction"), Direction.Name);
        Row->SetNumberField(TEXT("camera_yaw_degrees"), CameraYaw);
        if (AimYaw) Row->SetNumberField(TEXT("actor_relative_aim_yaw"), AimYaw->GetPropertyValue_InContainer(Anim));
        if (RootYaw) Row->SetNumberField(TEXT("root_yaw_offset"), RootYaw->GetPropertyValue_InContainer(Anim));
        Row->SetObjectField(TEXT("expected_torso_normal_world"), VectorJson(Direction.Normal));
        Row->SetNumberField(TEXT("max_pelvis_center_xy_drift_cm"), MaxCenterXY);
        Row->SetNumberField(TEXT("final_pelvis_center_xy_drift_cm"), FVector::Dist2D(Initial.Center, Body.Center));
        Row->SetNumberField(TEXT("max_longitudinal_axis_error_degrees"), MaxAxisAngle);
        Row->SetNumberField(TEXT("final_longitudinal_axis_error_degrees"), AngleDegrees(Initial.Longitudinal, Body.Longitudinal));
        Row->SetNumberField(TEXT("final_torso_normal_error_degrees"), NormalError);
        Row->SetNumberField(TEXT("final_pelvis_positive_y_vs_expected_torso_normal_degrees"), PelvisFrontError);
        Row->SetBoolField(TEXT("actor_transform_unchanged"), bActorUnchanged);
        Row->SetBoolField(TEXT("direction_matches"), bDirectionMatches);
        Row->SetBoolField(TEXT("settled_geometry_passed"), bPassed);
        auto Motion = RecordMotionHold(120, Direction.Direction);
        Row->SetObjectField(TEXT("idle_motion_hold"), Motion);
        Row->SetBoolField(TEXT("passed"), bPassed && Motion->GetBoolField(TEXT("motion_requirement_satisfied")));
        Cases.Add(MakeShared<FJsonValueObject>(Row));
        bAllCasesPassed &= bPassed && Motion->GetBoolField(TEXT("motion_requirement_satisfied"));
    }
    Result->SetArrayField(TEXT("cases"), Cases);

    // Sweep camera direction through the public input path at a human input rate.
    // Each cardinal gets a fixed-input hold; no expected pose or bone transform is written.
    TArray<TSharedPtr<FJsonValue>> Sweeps;
    Result->SetNumberField(TEXT("sweep_degrees_per_second"), 90.0);
    Result->SetNumberField(TEXT("sweep_frames_per_quarter_turn"), 60);
    Result->SetNumberField(TEXT("sweep_cardinal_settle_frames"), 120);
    for (int32 SweepIndex = 0; SweepIndex < 2; ++SweepIndex)
    {
        CameraPitch = 0.f;
        CameraYaw = Pawn->GetActorRotation().Yaw;
        Warm(180);
        double MaxCenterXY = 0.0, MaxAxisAngle = 0.0;
        TSharedPtr<FJsonObject> PeakAxisPose;
        bool bSweepCardinalsPassed = true;
        TArray<TSharedPtr<FJsonValue>> Checkpoints;
        for (int32 Segment = 0; Segment < 4 && bAllFinite; ++Segment)
        {
            const double Sign = SweepIndex == 0 ? 1.0 : -1.0;
            const auto Sample = [&](const TCHAR* Phase, int32 Frame)
            {
                const FBodyGeometry Body = ReadBody(Mesh, NormalSign);
                MaxCenterXY = FMath::Max(MaxCenterXY, FVector::Dist2D(Initial.Center, Body.Center));
                const double AxisAngle = AngleDegrees(Initial.Longitudinal, Body.Longitudinal);
                if (!PeakAxisPose || AxisAngle > MaxAxisAngle)
                {
                    MaxAxisAngle = AxisAngle;
                    PeakAxisPose = PoseJson(Mesh, Body);
                    PeakAxisPose->SetNumberField(TEXT("camera_yaw_degrees"), CameraYaw);
                    PeakAxisPose->SetNumberField(TEXT("camera_pitch_degrees"), CameraPitch);
                    PeakAxisPose->SetNumberField(TEXT("longitudinal_axis_error_degrees"), AxisAngle);
                    PeakAxisPose->SetNumberField(TEXT("pelvis_center_xy_drift_cm"), FVector::Dist2D(Initial.Center, Body.Center));
                    PeakAxisPose->SetNumberField(TEXT("native_prone_pose_direction"), static_cast<int32>(Anim->GetPronePoseDirection()));
                    PeakAxisPose->SetNumberField(TEXT("segment"), Segment);
                    PeakAxisPose->SetNumberField(TEXT("frame_in_phase"), Frame);
                    PeakAxisPose->SetStringField(TEXT("sample_phase"), Phase);
                    if (AimYaw) PeakAxisPose->SetNumberField(TEXT("filtered_aim_yaw"), AimYaw->GetPropertyValue_InContainer(Anim));
                    if (RootYaw) PeakAxisPose->SetNumberField(TEXT("root_yaw_offset"), RootYaw->GetPropertyValue_InContainer(Anim));
                    PeakAxisPose->SetArrayField(TEXT("main_blendspace_players"), BlendSpaceSnapshot(Anim));
                    PeakAxisPose->SetArrayField(TEXT("linked_layer_blendspace_players"), BlendSpaceSnapshot(Rifle));
                }
            };
            for (int32 Frame = 1; Frame <= 60 && bAllFinite; ++Frame)
            {
                const float RelativeYaw = static_cast<float>(Sign * (Segment * 90.0 + Frame * 1.5));
                CameraYaw = Pawn->GetActorRotation().Yaw + FRotator::NormalizeAxis(RelativeYaw);
                Tick();
                Sample(TEXT("Rotate"), Frame);
            }
            for (int32 Frame = 0; Frame < 120 && bAllFinite; ++Frame) { Tick(); Sample(TEXT("Settle"), Frame); }
            const int32 DirectionIndex = SweepIndex == 0 ? (Segment + 1) % 4 : (3 - Segment + 4) % 4;
            const FDirectionCase& Direction = Directions[DirectionIndex];
            const FBodyGeometry Body = ReadBody(Mesh, NormalSign);
            const double NormalError = AngleDegrees(Body.TorsoNormal, Direction.Normal);
            const double PelvisFrontError = AngleDegrees(Body.PelvisPositiveY, Direction.Normal);
            const bool bDirectionMatches = Anim->GetCurrentState() == ECharacterState::Proning && Anim->GetPronePoseDirection() == Direction.Direction;
            const bool bPassed = bDirectionMatches && PelvisFrontError <= 35.0 && NormalError <= 60.0;
            auto Checkpoint = PoseJson(Mesh, Body);
            Checkpoint->SetStringField(TEXT("direction"), Direction.Name);
            Checkpoint->SetNumberField(TEXT("camera_yaw_degrees"), CameraYaw);
            Checkpoint->SetNumberField(TEXT("torso_normal_error_degrees"), NormalError);
            Checkpoint->SetNumberField(TEXT("pelvis_positive_y_vs_expected_torso_normal_degrees"), PelvisFrontError);
            Checkpoint->SetBoolField(TEXT("direction_matches"), bDirectionMatches);
            Checkpoint->SetBoolField(TEXT("passed"), bPassed);
            Checkpoints.Add(MakeShared<FJsonValueObject>(Checkpoint));
            bSweepCardinalsPassed &= bPassed;
        }
        auto Sweep = MakeShared<FJsonObject>();
        Sweep->SetStringField(TEXT("direction"), SweepIndex == 0 ? TEXT("Clockwise_0_90_180_270_360") : TEXT("Counterclockwise_360_270_180_90_0"));
        Sweep->SetNumberField(TEXT("max_pelvis_center_xy_drift_cm"), MaxCenterXY);
        Sweep->SetNumberField(TEXT("max_longitudinal_axis_error_degrees"), MaxAxisAngle);
        if (PeakAxisPose)
        {
            Sweep->SetObjectField(TEXT("peak_axis_pose"), PeakAxisPose);
            Sweep->SetNumberField(TEXT("peak_camera_yaw_degrees"), PeakAxisPose->GetNumberField(TEXT("camera_yaw_degrees")));
            if (AimYaw) Sweep->SetNumberField(TEXT("peak_filtered_aim_yaw"), PeakAxisPose->GetNumberField(TEXT("filtered_aim_yaw")));
        }
        Sweep->SetArrayField(TEXT("settled_cardinal_checkpoints"), Checkpoints);
        const bool bPassed = bAllFinite && bAllTargetsMatch && ActorUnchanged() && bSweepCardinalsPassed && MaxCenterXY <= 10.0 && MaxAxisAngle <= 25.0;
        Sweep->SetBoolField(TEXT("passed"), bPassed);
        Sweeps.Add(MakeShared<FJsonValueObject>(Sweep));
        bAllCasesPassed &= bPassed;
    }
    Result->SetArrayField(TEXT("camera_sweeps"), Sweeps);

    TArray<TSharedPtr<FJsonValue>> PitchCases;
    Result->SetStringField(TEXT("pitch_normal_tolerance_caveat"), TEXT("Anatomical pelvis front retains the primary 35deg roll tolerance. Pitch can bend torso and shoulder-derived normal, so its secondary 60deg authored-pose tolerance adds absolute target pitch (90deg at +/-30)."));
    for (int32 DirectionIndex = 0; DirectionIndex < 4; ++DirectionIndex)
    {
        const FDirectionCase& Direction = Directions[DirectionIndex];
        CameraYaw = Pawn->GetActorRotation().Yaw + Direction.Yaw;
        CameraPitch = 0.f;
        Warm(120);
        for (const float Pitch : { -30.f, 30.f })
        {
            // Gradual one-second pitch change, followed by two seconds to settle.
            const float StartPitch = CameraPitch;
            for (int32 Frame = 1; Frame <= 60 && bAllFinite; ++Frame)
            {
                CameraPitch = FMath::Lerp(StartPitch, Pitch, Frame / 60.f);
                Tick();
            }
            Warm(120);
            const FBodyGeometry Body = ReadBody(Mesh, NormalSign);
            const double AxisError = AngleDegrees(Initial.Longitudinal, Body.Longitudinal);
            const double CenterError = FVector::Dist2D(Initial.Center, Body.Center);
            const double NormalError = AngleDegrees(Body.TorsoNormal, Direction.Normal);
            const double PelvisFrontError = AngleDegrees(Body.PelvisPositiveY, Direction.Normal);
            const double NormalTolerance = 60.0 + FMath::Abs(Pitch);
            const bool bDirectionMatches = Anim->GetCurrentState() == ECharacterState::Proning && Anim->GetPronePoseDirection() == Direction.Direction;
            const bool bPassed = bAllFinite && bAllTargetsMatch && ActorUnchanged() && bDirectionMatches && AxisError <= 25.0 && CenterError <= 10.0 && PelvisFrontError <= 35.0 && NormalError <= NormalTolerance;
            auto Row = PoseJson(Mesh, Body);
            Row->SetStringField(TEXT("direction"), Direction.Name);
            Row->SetNumberField(TEXT("camera_pitch_degrees"), Pitch);
            Row->SetNumberField(TEXT("camera_yaw_degrees"), CameraYaw);
            Row->SetNumberField(TEXT("longitudinal_axis_error_degrees"), AxisError);
            Row->SetNumberField(TEXT("pelvis_center_xy_drift_cm"), CenterError);
            Row->SetNumberField(TEXT("torso_normal_error_degrees"), NormalError);
            Row->SetNumberField(TEXT("pelvis_positive_y_vs_expected_torso_normal_degrees"), PelvisFrontError);
            Row->SetNumberField(TEXT("torso_normal_tolerance_degrees"), NormalTolerance);
            Row->SetBoolField(TEXT("direction_matches"), bDirectionMatches);
            Row->SetBoolField(TEXT("passed"), bPassed);
            PitchCases.Add(MakeShared<FJsonValueObject>(Row));
            bAllCasesPassed &= bPassed;
        }
    }
    Result->SetArrayField(TEXT("pitch_cases"), PitchCases);
    Result->SetBoolField(TEXT("all_bone_transforms_finite"), bAllFinite);
    Result->SetBoolField(TEXT("all_public_camera_targets_match"), bAllTargetsMatch);
    Result->SetBoolField(TEXT("standing_normal_calibration_valid"), FMath::Abs(CalibrationDot) > 0.2);
    return bAllCasesPassed && bAllFinite && bAllTargetsMatch && FMath::Abs(CalibrationDot) > 0.2;
}
