#include "ProneAssetGeometry.h"

#include "Animation/AnimData/IAnimationDataModel.h"
#include "Animation/AnimSequence.h"
#include "Animation/BlendSpace.h"
#include "Animation/Skeleton.h"
#include "Dom/JsonObject.h"
#include "Dom/JsonValue.h"
#include "ReferenceSkeleton.h"

namespace ProneAssetGeometry
{
    using FObject = TSharedRef<FJsonObject>;
    using FValues = TArray<TSharedPtr<FJsonValue>>;

    FObject Vector(const FVector& V)
    {
        FObject Out = MakeShared<FJsonObject>();
        Out->SetNumberField(TEXT("x"), V.X);
        Out->SetNumberField(TEXT("y"), V.Y);
        Out->SetNumberField(TEXT("z"), V.Z);
        return Out;
    }

    FObject Transform(const FTransform& T)
    {
        FObject Out = MakeShared<FJsonObject>();
        Out->SetObjectField(TEXT("translation"), Vector(T.GetTranslation()));
        const FQuat Q = T.GetRotation();
        FObject Rotation = MakeShared<FJsonObject>();
        Rotation->SetNumberField(TEXT("x"), Q.X);
        Rotation->SetNumberField(TEXT("y"), Q.Y);
        Rotation->SetNumberField(TEXT("z"), Q.Z);
        Rotation->SetNumberField(TEXT("w"), Q.W);
        Out->SetObjectField(TEXT("quaternion"), Rotation);
        Out->SetObjectField(TEXT("scale"), Vector(T.GetScale3D()));
        Out->SetObjectField(TEXT("local_x_axis"), Vector(Q.GetAxisX()));
        Out->SetObjectField(TEXT("local_y_axis"), Vector(Q.GetAxisY()));
        Out->SetObjectField(TEXT("local_z_axis"), Vector(Q.GetAxisZ()));
        return Out;
    }

    FObject Geometry(const FReferenceSkeleton& Skeleton, const TArray<FTransform>& ComponentPose)
    {
        FObject Out = MakeShared<FJsonObject>();
        // The actual player mesh uses -90 yaw. Translation is omitted so this is
        // actor-relative geometry, not a claim about a live world-space point.
        const FTransform MeshToActor(FRotator(0.0, -90.0, 0.0));
        const auto Index = [&Skeleton](const TCHAR* Name) { return Skeleton.FindBoneIndex(FName(Name)); };
        const auto Position = [&ComponentPose, &Index](const TCHAR* Name)
        {
            const int32 BoneIndex = Index(Name);
            return BoneIndex != INDEX_NONE ? ComponentPose[BoneIndex].GetTranslation() : FVector::ZeroVector;
        };
        const auto AddDirection = [&Out, &MeshToActor](const TCHAR* Name, const FVector& V)
        {
            const FVector Normal = V.GetSafeNormal();
            Out->SetObjectField(FString(Name) + TEXT("_component"), Vector(Normal));
            const FVector ActorNormal = MeshToActor.TransformVectorNoScale(Normal);
            Out->SetObjectField(FString(Name) + TEXT("_actor"), Vector(ActorNormal));
            if (!FVector(ActorNormal.X, ActorNormal.Y, 0.0).IsNearlyZero())
                Out->SetNumberField(FString(Name) + TEXT("_actor_yaw_degrees"), FMath::RadiansToDegrees(FMath::Atan2(ActorNormal.Y, ActorNormal.X)));
        };

        FValues Bones;
        const TCHAR* Names[] = { TEXT("root"), TEXT("pelvis"), TEXT("spine_01"), TEXT("spine_02"), TEXT("spine_03"),
            TEXT("neck_01"), TEXT("head"), TEXT("head_01"), TEXT("clavicle_l"), TEXT("clavicle_r"),
            TEXT("upperarm_l"), TEXT("upperarm_r"), TEXT("hand_l"), TEXT("hand_r"), TEXT("ik_hand_gun"),
            TEXT("ik_hand_l"), TEXT("ik_hand_r"), TEXT("thigh_l"), TEXT("thigh_r"), TEXT("foot_l"), TEXT("foot_r") };
        for (const TCHAR* Name : Names)
        {
            const int32 BoneIndex = Index(Name);
            if (BoneIndex == INDEX_NONE) continue;
            FObject Bone = MakeShared<FJsonObject>();
            Bone->SetStringField(TEXT("name"), Name);
            Bone->SetNumberField(TEXT("index"), BoneIndex);
            Bone->SetObjectField(TEXT("component_space"), Transform(ComponentPose[BoneIndex]));
            Bone->SetObjectField(TEXT("actor_space"), Transform(ComponentPose[BoneIndex] * MeshToActor));
            Bones.Add(MakeShared<FJsonValueObject>(Bone));
        }
        Out->SetArrayField(TEXT("bones"), Bones);
        const TCHAR* HeadName = Index(TEXT("head")) != INDEX_NONE ? TEXT("head") : TEXT("head_01");
        if (Index(HeadName) != INDEX_NONE && Index(TEXT("foot_l")) != INDEX_NONE && Index(TEXT("foot_r")) != INDEX_NONE)
        {
            const FVector Feet = (Position(TEXT("foot_l")) + Position(TEXT("foot_r"))) * 0.5;
            Out->SetObjectField(TEXT("feet_midpoint_component"), Vector(Feet));
            Out->SetObjectField(TEXT("feet_midpoint_actor"), Vector(MeshToActor.TransformPosition(Feet)));
            AddDirection(TEXT("head_from_feet"), Position(HeadName) - Feet);
        }
        const int32 Pelvis = Index(TEXT("pelvis"));
        if (Pelvis != INDEX_NONE)
        {
            const FQuat Q = ComponentPose[Pelvis].GetRotation();
            AddDirection(TEXT("pelvis_head_axis_minus_x"), -Q.GetAxisX());
            AddDirection(TEXT("pelvis_front_axis_plus_y"), Q.GetAxisY());
            AddDirection(TEXT("pelvis_right_axis_minus_z"), -Q.GetAxisZ());
            if (Index(HeadName) != INDEX_NONE) AddDirection(TEXT("head_from_pelvis"), Position(HeadName) - Position(TEXT("pelvis")));
            if (Index(TEXT("upperarm_l")) != INDEX_NONE && Index(TEXT("upperarm_r")) != INDEX_NONE)
            {
                const FVector Right = Position(TEXT("upperarm_r")) - Position(TEXT("upperarm_l"));
                const FVector ShoulderMid = (Position(TEXT("upperarm_r")) + Position(TEXT("upperarm_l"))) * 0.5;
                const FVector TorsoHead = ShoulderMid - Position(TEXT("pelvis"));
                AddDirection(TEXT("shoulder_right"), Right);
                AddDirection(TEXT("torso_head"), TorsoHead);
                AddDirection(TEXT("torso_front_right_cross_head"), FVector::CrossProduct(Right, TorsoHead));
            }
        }
        bool bFinite = true;
        for (const FTransform& T : ComponentPose) bFinite &= !T.ContainsNaN();
        Out->SetBoolField(TEXT("all_transforms_finite"), bFinite);
        return Out;
    }

    FObject Sequence(UAnimSequence* Animation)
    {
        FObject Out = MakeShared<FJsonObject>();
        Out->SetStringField(TEXT("path"), GetPathNameSafe(Animation));
        if (!Animation) { Out->SetStringField(TEXT("error"), TEXT("Animation could not be loaded.")); return Out; }
        Out->SetStringField(TEXT("skeleton"), GetPathNameSafe(Animation->GetSkeleton()));
        Out->SetNumberField(TEXT("additive_anim_type"), static_cast<int32>(Animation->AdditiveAnimType.GetValue()));
        Out->SetNumberField(TEXT("ref_pose_type"), static_cast<int32>(Animation->RefPoseType.GetValue()));
        Out->SetNumberField(TEXT("ref_frame_index"), Animation->RefFrameIndex);
        Out->SetStringField(TEXT("ref_pose_sequence"), GetPathNameSafe(Animation->RefPoseSeq.Get()));
        const IAnimationDataModel* Model = Animation->GetDataModel();
        const USkeleton* Skeleton = Animation->GetSkeleton();
        if (!Model || !Skeleton) { Out->SetStringField(TEXT("error"), TEXT("Data model or skeleton is unavailable.")); return Out; }
        Out->SetNumberField(TEXT("number_of_keys"), Model->GetNumberOfKeys());
        Out->SetNumberField(TEXT("number_of_frames"), Model->GetNumberOfFrames());
        Out->SetNumberField(TEXT("play_length"), Model->GetPlayLength());
        Out->SetStringField(TEXT("frame_rate"), Model->GetFrameRate().ToPrettyText().ToString());
        const FReferenceSkeleton& RefSkeleton = Skeleton->GetReferenceSkeleton();
        Out->SetNumberField(TEXT("bone_count"), RefSkeleton.GetNum());
        FValues MissingTracks;
        TArray<FTransform> ComponentPose;
        ComponentPose.SetNum(RefSkeleton.GetNum());
        const TArray<FTransform>& ReferencePose = RefSkeleton.GetRefBonePose();
        for (int32 Bone = 0; Bone < RefSkeleton.GetNum(); ++Bone)
        {
            const FName Name = RefSkeleton.GetBoneName(Bone);
            FTransform Local = ReferencePose[Bone];
            if (Model->IsValidBoneTrackName(Name) && Model->GetNumberOfKeys() > 0)
                Local = Model->GetBoneTrackTransform(Name, FFrameNumber(0));
            else MissingTracks.Add(MakeShared<FJsonValueString>(Name.ToString()));
            const int32 Parent = RefSkeleton.GetParentIndex(Bone);
            ComponentPose[Bone] = Parent == INDEX_NONE ? Local : Local * ComponentPose[Parent];
            ComponentPose[Bone].NormalizeRotation();
        }
        Out->SetNumberField(TEXT("sampled_key"), 0);
        Out->SetArrayField(TEXT("reference_fallback_tracks"), MissingTracks);
        Out->SetObjectField(TEXT("raw_nonadditive_geometry"), Geometry(RefSkeleton, ComponentPose));
        // An explicit reference-pose calibration makes the anatomical-axis sign
        // check reproducible for this exact skeleton.
        TArray<FTransform> ReferenceComponentPose;
        ReferenceComponentPose.SetNum(RefSkeleton.GetNum());
        for (int32 Bone = 0; Bone < RefSkeleton.GetNum(); ++Bone)
        {
            const int32 Parent = RefSkeleton.GetParentIndex(Bone);
            ReferenceComponentPose[Bone] = Parent == INDEX_NONE ? ReferencePose[Bone] : ReferencePose[Bone] * ReferenceComponentPose[Parent];
        }
        Out->SetObjectField(TEXT("skeleton_reference_geometry"), Geometry(RefSkeleton, ReferenceComponentPose));
        return Out;
    }
}

bool AuditProneAssetGeometry(TSharedRef<FJsonObject> Result)
{
    using namespace ProneAssetGeometry;
    Result->SetStringField(TEXT("method"), TEXT("Read-only frame-0 raw DataModel tracks; local transforms composed through the skeleton; no additive extraction, IK, runtime retargeting, constraints, or saving."));
    Result->SetStringField(TEXT("actor_space_convention"), TEXT("Actor yaw=0, mesh relative yaw=-90 degrees, zero translation. Actor +X=North/forward, +Y=East/right, +Z=Up."));
    const FString Base = TEXT("/Game/Blueprints/HellDiver/Animation_Locomotion/");
    const FString AlternateBase = TEXT("/Game/Blueprints/HellDiver/Animation/Retargeted_Anim/Prone/Idle/");
    TMap<FString, UAnimSequence*> UniqueSequences;
    FValues BlendSpaces;
    bool bSuccess = true;
    const auto AddSequence = [&UniqueSequences](UAnimSequence* Animation)
    {
        if (Animation) UniqueSequences.Add(Animation->GetPathName(), Animation);
    };
    const auto AddBlendSpace = [&BlendSpaces, &AddSequence, &bSuccess](const FString& Path)
    {
        FObject Item = MakeShared<FJsonObject>();
        Item->SetStringField(TEXT("path"), Path);
        UBlendSpace* Blend = LoadObject<UBlendSpace>(nullptr, *Path);
        if (!Blend)
        {
            Item->SetStringField(TEXT("error"), TEXT("BlendSpace could not be loaded."));
            bSuccess = false;
        }
        else
        {
            Item->SetStringField(TEXT("class"), Blend->GetClass()->GetPathName());
            FValues Samples;
            for (const FBlendSample& Sample : Blend->GetBlendSamples())
            {
                FObject Binding = MakeShared<FJsonObject>();
                Binding->SetObjectField(TEXT("sample_value"), Vector(Sample.SampleValue));
                Binding->SetStringField(TEXT("animation"), GetPathNameSafe(Sample.Animation));
                Binding->SetNumberField(TEXT("rate_scale"), Sample.RateScale);
                if (Sample.Animation)
                {
                    Binding->SetStringField(TEXT("ref_pose_sequence"), GetPathNameSafe(Sample.Animation->RefPoseSeq.Get()));
                    Binding->SetNumberField(TEXT("ref_pose_type"), static_cast<int32>(Sample.Animation->RefPoseType.GetValue()));
                    AddSequence(Sample.Animation);
                    AddSequence(Sample.Animation->RefPoseSeq.Get());
                }
                else bSuccess = false;
                Samples.Add(MakeShared<FJsonValueObject>(Binding));
            }
            Item->SetArrayField(TEXT("samples"), Samples);
        }
        BlendSpaces.Add(MakeShared<FJsonValueObject>(Item));
    };
    AddBlendSpace(Base + TEXT("AimOffset/AO_ProneFwd"));
    AddBlendSpace(Base + TEXT("AimOffset/AO_ProneRight"));
    AddBlendSpace(Base + TEXT("AimOffset/AO_ProneBwd"));
    AddBlendSpace(Base + TEXT("AimOffset/AO_ProneLeft"));
    AddBlendSpace(AlternateBase + TEXT("ABS_Prone_Retargeted"));
    Result->SetArrayField(TEXT("blend_spaces_and_reference_bindings"), BlendSpaces);

    FValues ExplicitCandidates;
    const TArray<FString> Candidates = {
        AlternateBase + TEXT("Hdv_Male_Laying_Left1_Reatergeted"),
        AlternateBase + TEXT("Hdv_Male_Laying_Right1_Reatergeted"),
        AlternateBase + TEXT("Hdv_Male_Lying1_Reatergeted"),
        AlternateBase + TEXT("Hdv_Prone_Idle_Reatergeted"),
        Base + TEXT("Assets/Prone/Helldiver_Anim_Prone_Pistol_Idle_R"),
        Base + TEXT("Assets/Dodge/Aim/Helldiveranim_Prone_Right_Idle"),
        Base + TEXT("Assets/Dodge/Aim/Helldiveranim_Prone_Left_Idle"),
        Base + TEXT("Assets/Dodge/Aim/Helldiveranim_Prone_Bwd_Idle")
    };
    for (const FString& Path : Candidates)
    {
        UAnimSequence* Animation = LoadObject<UAnimSequence>(nullptr, *Path);
        FObject Item = MakeShared<FJsonObject>();
        Item->SetStringField(TEXT("requested_path"), Path);
        Item->SetBoolField(TEXT("loaded"), Animation != nullptr);
        if (Animation) AddSequence(Animation);
        else bSuccess = false;
        ExplicitCandidates.Add(MakeShared<FJsonValueObject>(Item));
    }
    Result->SetArrayField(TEXT("explicit_idle_candidates"), ExplicitCandidates);
    TArray<FString> SortedPaths;
    UniqueSequences.GetKeys(SortedPaths);
    SortedPaths.Sort();
    FValues Sequences;
    for (const FString& Path : SortedPaths)
    {
        FObject Item = Sequence(UniqueSequences.FindChecked(Path));
        bSuccess &= !Item->HasField(TEXT("error"));
        Sequences.Add(MakeShared<FJsonValueObject>(Item));
    }
    Result->SetArrayField(TEXT("sequences"), Sequences);
    Result->SetNumberField(TEXT("unique_sequence_count"), Sequences.Num());
    Result->SetBoolField(TEXT("succeeded"), bSuccess);
    return bSuccess;
}
