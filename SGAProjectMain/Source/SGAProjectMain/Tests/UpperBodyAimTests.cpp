#include "CoreMinimal.h"
#include <limits>
#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "Misc/ScopeExit.h"
#include "Animation/AnimInstanceProxy.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/WorldInitializationValues.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/SkeletalMesh.h"
#include "../Character/PlayerCharacter.h"
#include "../Character/HellDiver/HellDiverStateComponent.h"
#include "../Character/Animation/AnimNode_UpperBodyAim.h"
#include "../Character/Animation/UpperBodyAimMath.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FUpperBodyAimRayTest, "SGA.Character.UpperBodyAim.TranslatedMuzzleRay",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FUpperBodyAimRayTest::RunTest(const FString& Parameters)
{
    for (double WorldYaw : {0., 37., 179., -179.})
    for (double Distance : {100., 500., 10000.})
    for (double Heading : {-130., -45., 0., 45., 130.})
    {
        const FQuat WorldRotation(FVector::UpVector, FMath::DegreesToRadians(WorldYaw));
        const FVector Origin(125., -70., 30.);
        const FVector Pivot = Origin + WorldRotation.RotateVector(FVector(15., -20., 125.));
        const FVector Muzzle = Pivot + WorldRotation.RotateVector(FVector(45., 12., 7.));
        const FVector Forward = WorldRotation.RotateVector(FVector::ForwardVector);
        const FVector Target = Pivot + WorldRotation.RotateVector(FRotator(20., Heading, 0.).Vector() * Distance);
        FQuat Q;
        TestTrue(TEXT("A reachable translated muzzle ray has a solution"), UpperBodyAimMath::SolveRayRotation(Pivot, Muzzle, Forward, Target, Q));
        const FVector SolvedMuzzle = Pivot + Q.RotateVector(Muzzle - Pivot);
        const FVector SolvedForward = Q.RotateVector(Forward).GetSafeNormal();
        const FVector Direction = (Target - SolvedMuzzle).GetSafeNormal();
        TestTrue(TEXT("The muzzle ray, rather than a ray from the hand, hits the target"), FVector::DotProduct(SolvedForward, Direction) > .999999);
        TestTrue(TEXT("Shoulder to muzzle distance is preserved"), FMath::IsNearlyEqual((SolvedMuzzle - Pivot).Size(), (Muzzle - Pivot).Size(), .00001));
    }
    FQuat Q;
    TestFalse(TEXT("A target inside the rigid arm's unreachable sphere is rejected"),
        UpperBodyAimMath::SolveRayRotation(FVector::ZeroVector, FVector(100., 0., 0.), FVector::YAxisVector, FVector(1.,0.,0.), Q));
    TestFalse(TEXT("A zero forward vector is rejected"),
        UpperBodyAimMath::SolveRayRotation(FVector::ZeroVector, FVector(1.,0.,0.), FVector::ZeroVector, FVector(100.,0.,0.), Q));
    TestFalse(TEXT("Non-finite target coordinates are rejected"),
        UpperBodyAimMath::SolveRayRotation(FVector::ZeroVector, FVector(1.,0.,0.), FVector::XAxisVector,
            FVector(std::numeric_limits<double>::quiet_NaN(),0.,0.), Q));
    return true;
}

namespace
{
    struct FTestableUpperBodyAim : FAnimNode_UpperBodyAim
    {
        using FAnimNode_UpperBodyAim::InitializeBoneReferences;
        using FAnimNode_UpperBodyAim::EvaluateSkeletalControl_AnyThread;
        using FAnimNode_UpperBodyAim::IsValidToEvaluate;
    };
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FUpperBodyAimPoseTest, "SGA.Character.UpperBodyAim.ActualSkeletonPose",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FUpperBodyAimPoseTest::RunTest(const FString& Parameters)
{
    const UWorld::InitializationValues IVS = UWorld::InitializationValues()
        .AllowAudioPlayback(false).RequiresHitProxies(false).CreatePhysicsScene(false)
        .CreateNavigation(false).CreateAISystem(false).ShouldSimulatePhysics(false)
        .EnableTraceCollision(false).SetTransactional(false).CreateFXSystem(false);
    UWorld* World = UWorld::CreateWorld(EWorldType::EditorPreview, false,
        MakeUniqueObjectName(GetTransientPackage(), UWorld::StaticClass(), TEXT("UpperBodyAimPose")),
        nullptr, true, ERHIFeatureLevel::Num, &IVS);
    if (!TestNotNull(TEXT("Preview world exists"), World) || !TestNotNull(TEXT("Engine exists"), GEngine)) return false;
    GEngine->CreateNewWorldContext(EWorldType::EditorPreview).SetCurrentWorld(World);
    ON_SCOPE_EXIT { World->DestroyWorld(false); GEngine->DestroyWorldContext(World); };
    UClass* PawnClass = LoadClass<APlayerCharacter>(nullptr, TEXT("/Game/Blueprints/HellDiver/BP_HellDiver_MK2.BP_HellDiver_MK2_C"));
    if (!TestNotNull(TEXT("Actual player Blueprint loads"), PawnClass)) return false;
    FActorSpawnParameters Spawn;
    Spawn.ObjectFlags = RF_Transient; Spawn.bDeferConstruction = true; Spawn.bTemporaryEditorActor = true;
    Spawn.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    APlayerCharacter* Pawn = World->SpawnActor<APlayerCharacter>(PawnClass, FVector::ZeroVector, FRotator::ZeroRotator, Spawn);
    if (!TestNotNull(TEXT("Actual player exists"), Pawn)) return false;
    auto* Mesh = Pawn->GetMesh(); Mesh->InitAnim(true);
    UAnimInstance* Anim = Mesh->GetAnimInstance();
    if (!TestNotNull(TEXT("Main animation instance initializes"), Anim)) return false;
    ++GFrameCounter; Mesh->TickAnimation(1.f/60.f, false); Mesh->RefreshBoneTransforms();
    // This isolated solve owns its pose; it does not reach into protected live proxy state.
    FAnimInstanceProxy Proxy(Anim);
    TArray<FBoneIndexType> Required;
    for (int32 I=0; I<Mesh->GetSkeletalMeshAsset()->GetRefSkeleton().GetNum(); ++I) Required.Add(I);
    FBoneContainer Bones(Required, UE::Anim::FCurveFilterSettings(), *Mesh->GetSkeletalMeshAsset());
    if (!TestTrue(TEXT("Required bones are initialized"), Bones.GetCompactPoseNumBones() > 0)) return false;
    const auto Compact = [&](const TCHAR* Name)
    { return Bones.MakeCompactPoseIndex(FMeshPoseBoneIndex(Mesh->GetBoneIndex(Name))); };
    const FCompactPoseBoneIndex Chest = Compact(TEXT("spine_03"));
    const FCompactPoseBoneIndex Hand = Compact(TEXT("hand_r"));
    const FCompactPoseBoneIndex Upper = Compact(TEXT("upperarm_r"));
    const FCompactPoseBoneIndex Lower = Compact(TEXT("lowerarm_r"));
    const FCompactPoseBoneIndex LeftUpper=Compact(TEXT("upperarm_l")), LeftLower=Compact(TEXT("lowerarm_l")), LeftHand=Compact(TEXT("hand_l"));
    if (!TestTrue(TEXT("All required aim bones exist"), Chest != INDEX_NONE && Hand != INDEX_NONE && Upper != INDEX_NONE && Lower != INDEX_NONE
        && LeftUpper != INDEX_NONE && LeftLower != INDEX_NONE && LeftHand != INDEX_NONE)) return false;
    const FVector Forward(0.,1.,0.), Up(0.,0.,1.);
    FMemMark Mark(FMemStack::Get());
    FComponentSpacePoseContext Reference(&Proxy); Reference.Pose.InitPose(&Bones);
    // Use the actual ARP reference pose to identify the model's forward direction per bone.
    const FQuat ChestReference = Reference.Pose.GetComponentSpaceTransform(Chest).GetRotation();
    for (double RootYaw : {-130., -90., 0., 90., 130.})
    for (double Heading : {-80., 35., 80.})
    for (double Pitch : {-40., 20., 40.})
    for (double ExistingChestYaw : {0.,25.})
    {
        FMemMark FrameMark(FMemStack::Get());
        FComponentSpacePoseContext Output(&Proxy); Output.Pose.InitPose(&Bones);
        const FQuat RootRotation(Up, FMath::DegreesToRadians(RootYaw));
        const int32 Count = Bones.GetCompactPoseNumBones();
        TArray<FTransform> Before;
        for (int32 I=0; I<Count; ++I)
        {
            const FCompactPoseBoneIndex Index(I);
            FTransform T = Reference.Pose.GetComponentSpaceTransform(Index);
            T.SetLocation(RootRotation.RotateVector(T.GetLocation()));
            T.SetRotation((RootRotation * T.GetRotation()).GetNormalized());
            Before.Add(T);
            Output.Pose.SetComponentSpaceTransform(Index, T);
        }
        // A preceding authored AO may already have turned the chest. The node must
        // correct its residual instead of adding the entire target rotation twice.
        const FQuat ExistingChestRotation(Up,FMath::DegreesToRadians(ExistingChestYaw));
        const FVector ChestPivot = Before[Chest.GetInt()].GetLocation();
        for (int32 I=Chest.GetInt(); I<Count; ++I)
        {
            bool Descendant = false;
            for (FCompactPoseBoneIndex Index(I); Index != INDEX_NONE; Index = Bones.GetParentBoneIndex(Index))
                if (Index == Chest) { Descendant = true; break; }
            if (!Descendant) continue;
            FTransform& T = Before[I];
            T.SetLocation(ChestPivot + ExistingChestRotation.RotateVector(T.GetLocation() - ChestPivot));
            T.SetRotation((ExistingChestRotation * T.GetRotation()).GetNormalized());
            Output.Pose.SetComponentSpaceTransform(FCompactPoseBoneIndex(I),T);
        }
        FTestableUpperBodyAim Node;
        Node.InitializeBoneReferences(Bones);
        if (!TestTrue(TEXT("The custom node validates the actual skeleton"), Node.IsValidToEvaluate(Mesh->GetSkeletalMeshAsset()->GetSkeleton(), Bones))) return false;
        Node.BodyForward = Forward; Node.BodyUp = Up;
        const FVector RelativeForward = RootRotation.RotateVector(Forward);
        const FVector RelativeRight = FVector::CrossProduct(Up, RelativeForward);
        const FVector Horizontal = FQuat(Up,FMath::DegreesToRadians(Heading)).RotateVector(RelativeForward);
        const FVector AimDirection = Horizontal * FMath::Cos(FMath::DegreesToRadians(Pitch)) + Up * FMath::Sin(FMath::DegreesToRadians(Pitch));
        Node.TargetPosition = Before[Chest.GetInt()].GetLocation() + AimDirection * 1000.;
        const FQuat MuzzleRotation = FRotationMatrix::MakeFromXZ(RelativeForward, Up).ToQuat();
        const FQuat OffsetRotation = Before[Hand.GetInt()].GetRotation().Inverse() * MuzzleRotation;
        const FVector OffsetLocation = Before[Hand.GetInt()].GetRotation().UnrotateVector(RelativeForward * 45. + Up * 7. + RelativeRight * 12.);
        Node.MuzzleInHand = FTransform(OffsetRotation, OffsetLocation);
        TArray<FBoneTransform> Changes;
        Node.EvaluateSkeletalControl_AnyThread(Output, Changes);
        TestTrue(TEXT("The node supplies bone transforms"), Changes.Num() > 3);
        Output.Pose.LocalBlendCSBoneTransforms(Changes, 1.f);
        const FTransform Muzzle = Node.MuzzleInHand * Output.Pose.GetComponentSpaceTransform(Hand);
        const FVector ToTarget = (Node.TargetPosition - Muzzle.GetLocation()).GetSafeNormal();
        TestTrue(TEXT("Same-frame muzzle direction remains on target at every root/turn angle"), FVector::DotProduct(Muzzle.GetUnitAxis(EAxis::X), ToTarget) > .99999);
        const FVector TorsoForward = (Output.Pose.GetComponentSpaceTransform(Chest).GetRotation() * ChestReference.Inverse()).RotateVector(Forward).GetSafeNormal();
        const double Yaw = FMath::RadiansToDegrees(FMath::Atan2(FVector::DotProduct(TorsoForward, RelativeRight), FVector::DotProduct(TorsoForward, RelativeForward)));
        const double Elevation = FMath::RadiansToDegrees(FMath::Asin(FMath::Clamp(FVector::DotProduct(TorsoForward, Up), -1., 1.)));
        TestTrue(TEXT("Torso yaw stays within the configured natural limit"), FMath::Abs(Yaw) <= Node.MaxTorsoYaw + .01);
        TestTrue(TEXT("Torso pitch stays within the configured natural limit"), FMath::Abs(Elevation) <= Node.MaxTorsoPitch + .01);
        TestTrue(TEXT("Existing authored chest yaw is accounted for exactly"),
            FMath::IsNearlyEqual(Yaw,FMath::Clamp(Heading,-double(Node.MaxTorsoYaw),double(Node.MaxTorsoYaw)),.01));
        for (const TCHAR* Name : {TEXT("root"),TEXT("pelvis"),TEXT("thigh_l"),TEXT("thigh_r"),TEXT("foot_l"),TEXT("foot_r")})
        {
            const FCompactPoseBoneIndex Index = Compact(Name);
            if (Index != INDEX_NONE) TestTrue(FString::Printf(TEXT("%s is unchanged"),Name), Output.Pose.GetComponentSpaceTransform(Index).Equals(Before[Index.GetInt()], .0001));
        }
        const double UpperBefore = FVector::Distance(Before[Upper.GetInt()].GetLocation(), Before[Lower.GetInt()].GetLocation());
        const double LowerBefore = FVector::Distance(Before[Lower.GetInt()].GetLocation(), Before[Hand.GetInt()].GetLocation());
        const double UpperAfter = FVector::Distance(Output.Pose.GetComponentSpaceTransform(Upper).GetLocation(), Output.Pose.GetComponentSpaceTransform(Lower).GetLocation());
        const double LowerAfter = FVector::Distance(Output.Pose.GetComponentSpaceTransform(Lower).GetLocation(), Output.Pose.GetComponentSpaceTransform(Hand).GetLocation());
        TestTrue(TEXT("Upper-arm length is preserved"), FMath::IsNearlyEqual(UpperBefore, UpperAfter, .0001));
        TestTrue(TEXT("Forearm length is preserved"), FMath::IsNearlyEqual(LowerBefore, LowerAfter, .0001));
        for (const TCHAR* Name : {TEXT("spine_01"),TEXT("spine_02"),TEXT("spine_03")})
        {
            const FCompactPoseBoneIndex Index=Compact(Name), Parent=Bones.GetParentBoneIndex(Index);
            const FQuat LocalBefore = Before[Parent.GetInt()].GetRotation().Inverse() * Before[Index.GetInt()].GetRotation();
            const FQuat LocalAfter = Output.Pose.GetComponentSpaceTransform(Parent).GetRotation().Inverse() * Output.Pose.GetComponentSpaceTransform(Index).GetRotation();
            TestTrue(FString::Printf(TEXT("%s contributes to torso rotation"),Name), LocalBefore.AngularDistance(LocalAfter) > .0001);
        }
        // The left grip is relative to the newly solved right hand, never last frame's
        // world socket position. Put it inside the left arm's reachable workspace.
        const FTransform RightSolved=Output.Pose.GetComponentSpaceTransform(Hand);
        const FTransform LeftWithoutGrip=Output.Pose.GetComponentSpaceTransform(LeftHand);
        const FVector LeftShoulder=Output.Pose.GetComponentSpaceTransform(LeftUpper).GetLocation();
        const FVector GripPosition=LeftShoulder + (LeftWithoutGrip.GetLocation()-LeftShoulder)*.8 + Up*2.;
        const FTransform DesiredGrip(LeftWithoutGrip.GetRotation(),GripPosition);
        Node.LeftGripInHand=DesiredGrip.GetRelativeTransform(RightSolved);
        const double LeftUpperLength=FVector::Distance(Output.Pose.GetComponentSpaceTransform(LeftUpper).GetLocation(),Output.Pose.GetComponentSpaceTransform(LeftLower).GetLocation());
        const double LeftLowerLength=FVector::Distance(Output.Pose.GetComponentSpaceTransform(LeftLower).GetLocation(),LeftWithoutGrip.GetLocation());
        for (float GripAlpha : {0.f,.5f,1.f})
        {
            FComponentSpacePoseContext BothHands(&Proxy); BothHands.Pose.InitPose(&Bones);
            for (int32 I=0; I<Count; ++I) BothHands.Pose.SetComponentSpaceTransform(FCompactPoseBoneIndex(I),Before[I]);
            Node.LeftHandAlpha=GripAlpha;
            TArray<FBoneTransform> BothChanges;
            Node.EvaluateSkeletalControl_AnyThread(BothHands,BothChanges);
            BothHands.Pose.LocalBlendCSBoneTransforms(BothChanges,1.f);
            const FTransform SolvedLeft=BothHands.Pose.GetComponentSpaceTransform(LeftHand);
            TestTrue(TEXT("Same-frame left grip solve preserves the corrected right hand"),BothHands.Pose.GetComponentSpaceTransform(Hand).Equals(RightSolved,.0001));
            if (GripAlpha==0.f)
                TestTrue(TEXT("Released left hand retains its authored torso-relative pose"),SolvedLeft.Equals(LeftWithoutGrip,.0001));
            if (GripAlpha==1.f)
            {
                TestTrue(TEXT("Full left grip weight reaches the current corrected weapon grip"),SolvedLeft.GetLocation().Equals(GripPosition,.001));
                // IK_left is a positional socket. Its authoring rotation must not flip
                // the wrist, so two different socket orientations at the same point
                // must produce the same evaluated left-hand pose.
                const FTransform SavedGrip=Node.LeftGripInHand;
                Node.LeftGripInHand.SetRotation((FQuat(FVector::UpVector,2.3)*SavedGrip.GetRotation()).GetNormalized());
                FComponentSpacePoseContext DifferentSocketRotation(&Proxy); DifferentSocketRotation.Pose.InitPose(&Bones);
                for (int32 I=0; I<Count; ++I) DifferentSocketRotation.Pose.SetComponentSpaceTransform(FCompactPoseBoneIndex(I),Before[I]);
                TArray<FBoneTransform> RotationChanges;
                Node.EvaluateSkeletalControl_AnyThread(DifferentSocketRotation,RotationChanges);
                DifferentSocketRotation.Pose.LocalBlendCSBoneTransforms(RotationChanges,1.f);
                TestTrue(TEXT("Arbitrary positional socket rotation cannot flip the left wrist"),
                    DifferentSocketRotation.Pose.GetComponentSpaceTransform(LeftHand).Equals(SolvedLeft,.0001));
                Node.LeftGripInHand=SavedGrip;
            }
            const double SolvedUpperLength=FVector::Distance(BothHands.Pose.GetComponentSpaceTransform(LeftUpper).GetLocation(),BothHands.Pose.GetComponentSpaceTransform(LeftLower).GetLocation());
            const double SolvedLowerLength=FVector::Distance(BothHands.Pose.GetComponentSpaceTransform(LeftLower).GetLocation(),SolvedLeft.GetLocation());
            TestTrue(TEXT("Left upper arm never stretches at zero, partial or full grip weight"),FMath::IsNearlyEqual(SolvedUpperLength,LeftUpperLength,.0001));
            TestTrue(TEXT("Left forearm never stretches at zero, partial or full grip weight"),FMath::IsNearlyEqual(SolvedLowerLength,LeftLowerLength,.0001));
            for (const TCHAR* Name : {TEXT("root"),TEXT("pelvis"),TEXT("thigh_l"),TEXT("thigh_r"),TEXT("foot_l"),TEXT("foot_r")})
            {
                const FCompactPoseBoneIndex Index=Compact(Name);
                if (Index != INDEX_NONE) TestTrue(FString::Printf(TEXT("Left grip leaves %s unchanged"),Name),BothHands.Pose.GetComponentSpaceTransform(Index).Equals(Before[Index.GetInt()],.0001));
            }
        }
    }
    return true;
}
#endif
