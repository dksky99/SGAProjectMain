#include "AnimNode_UpperBodyAim.h"
#include "UpperBodyAimMath.h"
#include "AnimationRuntime.h"
#include "TwoBoneIK.h"

FAnimNode_UpperBodyAim::FAnimNode_UpperBodyAim()
{
    Root.BoneName = TEXT("root");
    Spine1.BoneName = TEXT("spine_01"); Spine2.BoneName = TEXT("spine_02"); Spine3.BoneName = TEXT("spine_03");
    UpperArm.BoneName = TEXT("upperarm_r"); Hand.BoneName = TEXT("hand_r");
    LeftUpperArm.BoneName = TEXT("upperarm_l");
    LeftLowerArm.BoneName = TEXT("lowerarm_l");
    LeftHand.BoneName = TEXT("hand_l");
}

void FAnimNode_UpperBodyAim::InitializeBoneReferences(const FBoneContainer& Bones)
{
    for (FBoneReference* Bone : {&Root,&Spine1,&Spine2,&Spine3,&UpperArm,&Hand,&LeftUpperArm,&LeftLowerArm,&LeftHand}) Bone->Initialize(Bones);
}

bool FAnimNode_UpperBodyAim::IsValidToEvaluate(const USkeleton* Skeleton, const FBoneContainer& Bones)
{
    for (const FBoneReference* Bone : {&Root,&Spine1,&Spine2,&Spine3,&UpperArm,&Hand})
        if (!Bone->IsValidToEvaluate(Bones)) return false;
    return true;
}

void FAnimNode_UpperBodyAim::EvaluateSkeletalControl_AnyThread(FComponentSpacePoseContext& Output, TArray<FBoneTransform>& Out)
{
    if (TargetPosition.ContainsNaN() || MuzzleInHand.ContainsNaN()) return;
    const FBoneContainer& Bones = Output.Pose.GetPose().GetBoneContainer();
    const int32 Count = Bones.GetCompactPoseNumBones();
    TArray<FTransform, TInlineAllocator<256>> Pose, Reference;
    TArray<bool, TInlineAllocator<256>> Changed;
    Pose.SetNum(Count); Reference.SetNum(Count); Changed.Init(false, Count);
    // Reference rotations identify the model's axes without assuming that local Z is up.
    for (int32 I = 0; I < Count; ++I)
    {
        const FCompactPoseBoneIndex Index(I);
        Pose[I] = Output.Pose.GetComponentSpaceTransform(Index);
        Reference[I] = Bones.GetRefPoseTransform(Index);
        const int32 Parent = Bones.GetParentBoneIndex(Index).GetInt();
        if (Parent != INDEX_NONE) Reference[I] *= Reference[Parent];
    }
    const auto IsChild = [&Bones](int32 Index, int32 Parent)
    {
        for (int32 I = Index; I != INDEX_NONE; I = Bones.GetParentBoneIndex(FCompactPoseBoneIndex(I)).GetInt())
            if (I == Parent) return true;
        return false;
    };
    const auto Rotate = [&](int32 Bone, const FQuat& Q)
    {
        const FVector Pivot = Pose[Bone].GetLocation();
        for (int32 I = Bone; I < Count; ++I)
        {
            if (!IsChild(I, Bone)) continue;
            Pose[I].SetLocation(Pivot + Q.RotateVector(Pose[I].GetLocation() - Pivot));
            Pose[I].SetRotation((Q * Pose[I].GetRotation()).GetNormalized());
            Changed[I] = true;
        }
    };
    const int32 RootIndex = Root.GetCompactPoseIndex(Bones).GetInt();
    const int32 Chest = Spine3.GetCompactPoseIndex(Bones).GetInt();
    const FQuat RootDelta = Pose[RootIndex].GetRotation() * Reference[RootIndex].GetRotation().Inverse();
    const FVector Forward = RootDelta.RotateVector(BodyForward).GetSafeNormal();
    const FVector Up = RootDelta.RotateVector(BodyUp).GetSafeNormal();
    const FVector Right = FVector::CrossProduct(Up, Forward).GetSafeNormal();
    const FVector Direction = (TargetPosition - Pose[Chest].GetLocation()).GetSafeNormal();
    if (Forward.IsNearlyZero() || Up.IsNearlyZero() || Right.IsNearlyZero() || Direction.IsNearlyZero()) return;
    const float Yaw = FMath::Clamp(FMath::RadiansToDegrees(FMath::Atan2(FVector::DotProduct(Direction,Right), FVector::DotProduct(Direction,Forward))), -MaxTorsoYaw, MaxTorsoYaw);
    const float Pitch = FMath::Clamp(FMath::RadiansToDegrees(FMath::Asin(FMath::Clamp(FVector::DotProduct(Direction,Up),-1.,1.))), -MaxTorsoPitch, MaxTorsoPitch);
    const FVector Flat = FQuat(Up, FMath::DegreesToRadians(Yaw)).RotateVector(Forward);
    const FVector Desired = (Flat * FMath::Cos(FMath::DegreesToRadians(Pitch)) + Up * FMath::Sin(FMath::DegreesToRadians(Pitch))).GetSafeNormal();
    const FVector ChestForward = (Pose[Chest].GetRotation() * Reference[Chest].GetRotation().Inverse()).RotateVector(BodyForward).GetSafeNormal();
    const FQuat Torso = FQuat::FindBetweenNormals(ChestForward, Desired);
    // Lower back contributes least; upper spine contributes most. Children move with parents.
    Rotate(Spine1.GetCompactPoseIndex(Bones).GetInt(), FQuat::Slerp(FQuat::Identity,Torso,.2f).GetNormalized());
    Rotate(Spine2.GetCompactPoseIndex(Bones).GetInt(), FQuat::Slerp(FQuat::Identity,Torso,.35f).GetNormalized());
    Rotate(Chest, FQuat::Slerp(FQuat::Identity,Torso,.45f).GetNormalized());
    const int32 Arm = UpperArm.GetCompactPoseIndex(Bones).GetInt();
    const int32 HandIndex = Hand.GetCompactPoseIndex(Bones).GetInt();
    const FQuat RightHandBeforeCorrection = Pose[HandIndex].GetRotation();
    const FTransform Muzzle = MuzzleInHand * Pose[HandIndex];
    FQuat Correction;
    if (UpperBodyAimMath::SolveRayRotation(Pose[Arm].GetLocation(), Muzzle.GetLocation(), Muzzle.GetUnitAxis(EAxis::X), TargetPosition, Correction))
        Rotate(Arm, Correction);
    const FQuat RightHandCorrection = (Pose[HandIndex].GetRotation() * RightHandBeforeCorrection.Inverse()).GetNormalized();
    // A world-space grip sampled before pose evaluation trails this corrected right hand.
    // Resolve the socket's hand-relative transform against the evaluated right hand instead.
    const float GripAlpha = FMath::IsFinite(LeftHandAlpha) ? FMath::Clamp(LeftHandAlpha, 0.f, 1.f) : 0.f;
    if (GripAlpha > 0.f && !LeftGripInHand.ContainsNaN()
        && LeftUpperArm.IsValidToEvaluate(Bones) && LeftLowerArm.IsValidToEvaluate(Bones) && LeftHand.IsValidToEvaluate(Bones))
    {
        const int32 LUpper = LeftUpperArm.GetCompactPoseIndex(Bones).GetInt();
        const int32 LLower = LeftLowerArm.GetCompactPoseIndex(Bones).GetInt();
        const int32 LHand = LeftHand.GetCompactPoseIndex(Bones).GetInt();
        const int32 UpperParent = Bones.GetParentBoneIndex(FCompactPoseBoneIndex(LUpper)).GetInt();
        const FTransform GripTarget = LeftGripInHand * Pose[HandIndex];
        const FVector Shoulder = Pose[LUpper].GetLocation();
        const FVector Elbow = Pose[LLower].GetLocation();
        const FVector Wrist = Pose[LHand].GetLocation();
        const double UpperLength = FVector::Distance(Shoulder, Elbow);
        const double LowerLength = FVector::Distance(Elbow, Wrist);
        if (UpperParent != INDEX_NONE && Bones.GetParentBoneIndex(FCompactPoseBoneIndex(LLower)).GetInt() == LUpper
            && Bones.GetParentBoneIndex(FCompactPoseBoneIndex(LHand)).GetInt() == LLower
            && UpperLength > UE_SMALL_NUMBER && LowerLength > UE_SMALL_NUMBER && !GripTarget.ContainsNaN())
        {
            FTransform SolvedUpper = Pose[LUpper], SolvedLower = Pose[LLower], SolvedHand = Pose[LHand];
            const FVector OldAxis = (Wrist - Shoulder).GetSafeNormal();
            const FVector NewAxis = (GripTarget.GetLocation() - Shoulder).GetSafeNormal();
            // Preserve the posed bend side; a straight chain uses its evaluated orientation.
            FVector Bend = FVector::VectorPlaneProject(Elbow - Shoulder, OldAxis).GetSafeNormal();
            if (Bend.IsNearlyZero())
                Bend = FVector::VectorPlaneProject(Pose[LUpper].GetUnitAxis(EAxis::Z), OldAxis).GetSafeNormal();
            if (Bend.IsNearlyZero())
            {
                FVector Other;
                (OldAxis.IsNearlyZero() ? FVector::ForwardVector : OldAxis).FindBestAxisVectors(Bend, Other);
            }
            FVector PoleDirection = FVector::VectorPlaneProject(Bend, NewAxis).GetSafeNormal();
            if (PoleDirection.IsNearlyZero() && !OldAxis.IsNearlyZero() && !NewAxis.IsNearlyZero())
                PoleDirection = FQuat::FindBetweenNormals(OldAxis, NewAxis).RotateVector(Bend).GetSafeNormal();
            if (PoleDirection.IsNearlyZero()) PoleDirection = Bend;
            const FVector Pole = Shoulder + PoleDirection * (UpperLength + LowerLength);
            AnimationCore::SolveTwoBoneIK(SolvedUpper, SolvedLower, SolvedHand, Pole, GripTarget.GetLocation(),
                UpperLength, LowerLength, false, 1.0, 1.0);
            // IK_left historically specifies position only. Preserve the evaluated wrist
            // orientation relative to the rigid right-hand correction, not socket axes.
            SolvedHand.SetRotation((RightHandCorrection * Pose[LHand].GetRotation()).GetNormalized());
            if (!SolvedUpper.ContainsNaN() && !SolvedLower.ContainsNaN() && !SolvedHand.ContainsNaN())
            {
                TArray<FTransform, TInlineAllocator<256>> Local;
                Local.SetNum(Count);
                for (int32 I = LUpper; I < Count; ++I)
                {
                    if (!IsChild(I, LUpper)) continue;
                    const int32 Parent = Bones.GetParentBoneIndex(FCompactPoseBoneIndex(I)).GetInt();
                    Local[I] = Pose[I].GetRelativeTransform(Pose[Parent]);
                }
                const FQuat SolvedLocalRotations[] = {
                    SolvedUpper.GetRelativeTransform(Pose[UpperParent]).GetRotation(),
                    SolvedLower.GetRelativeTransform(SolvedUpper).GetRotation(),
                    SolvedHand.GetRelativeTransform(SolvedLower).GetRotation() };
                const int32 Chain[] = {LUpper, LLower, LHand};
                for (int32 I = 0; I < 3; ++I)
                    Local[Chain[I]].SetRotation(FQuat::Slerp(Local[Chain[I]].GetRotation(), SolvedLocalRotations[I], GripAlpha).GetNormalized());
                // Keep local offsets and scales at every weight: component-space position lerps
                // would shorten the limbs. Fingers/twist/attachment descendants follow as well.
                for (int32 I = LUpper; I < Count; ++I)
                {
                    if (!IsChild(I, LUpper)) continue;
                    const int32 Parent = Bones.GetParentBoneIndex(FCompactPoseBoneIndex(I)).GetInt();
                    Pose[I] = Local[I] * Pose[Parent];
                    Pose[I].NormalizeRotation();
                    Changed[I] = true;
                }
            }
        }
    }
    for (int32 I=0; I<Count; ++I)
        if (Changed[I]) Out.Emplace(FCompactPoseBoneIndex(I), Pose[I]);
}
