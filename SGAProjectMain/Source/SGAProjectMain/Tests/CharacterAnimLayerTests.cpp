#include "CoreMinimal.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Animation/AnimExecutionContext.h"
#include "Animation/AnimNodeReference.h"
#include "Components/SkeletalMeshComponent.h"
#include "../Character/CharacterAnimLayerInstance.h"
#include "../Character/CharacterStateComponent.h"
#include "../Character/HellDiver/HellDiverStateComponent.h"
#include "UObject/UnrealType.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCharacterAnimationCompletionTest,
    "SGA.Character.Animation.CompletionBinding",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FCharacterAnimationCompletionTest::RunTest(const FString& Parameters)
{
    auto* State = NewObject<UHellDiverStateComponent>();
    TestTrue(TEXT("A posture request acquires its lock"), State->StartCrouch());
    TestTrue(TEXT("Transition is active before an animation update"), State->IsMotionChanging());
    TestTrue(TEXT("Matching completion releases the lock"), State->FinishMotionChange(State->GetMotionChangeId()));
    TestFalse(TEXT("The posture lock is released"), State->IsMotionChanging());
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCharacterAnimLayerPreviewTest,
    "SGA.Character.Animation.LayerWithoutOwner",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FCharacterAnimLayerPreviewTest::RunTest(const FString& Parameters)
{
    auto* Mesh = NewObject<USkeletalMeshComponent>();
    auto* Layer = NewObject<UCharacterAnimLayerInstance>(Mesh);
    const FAnimUpdateContext Context;
    const FAnimNodeReference Node;
    // Reproduce opening a layer by itself, before a main anim instance is linked.
    Layer->IdleOnUpdate(Context, Node);
    Layer->CycleOnUpdate(Context, Node);
    Layer->SetupStartAnims(Context, Node);
    Layer->UpdateStartAnims(Context, Node);
    Layer->SetupStopAnims(Context, Node);
    Layer->UpdateStopAnims(Context, Node);
    Layer->SetupPivotAnims(Context, Node);
    Layer->UpdatePivotAnims(Context, Node);
    Layer->SetupStanceTransitionAnims(Context, Node);
    Layer->SetupTurnInPlaceEntryState(Context, Node);
    Layer->SetupTurnInPlaceEntryAnims(Context, Node);
    Layer->UpdateTurnInPlaceEntryAnims(Context, Node);
    Layer->SetupJumpFallLandAnim(Context, Node);
    Layer->UpdateJumpFallLandAnim(Context, Node);
    Layer->SetupDodgeStartAnim(Context, Node);
    Layer->UpdateDodgeStartAnim(Context, Node);
    Layer->SetupDodgeLoopAnim(Context, Node);
    Layer->UpdateDodgeLoopAnim(Context, Node);
    Layer->SetupDodgeEndAnim(Context, Node);
    Layer->UpdateDodgeEndAnim(Context, Node);
    TestNull(TEXT("An unlinked layer does not choose a pivot from another stance"), Layer->SelectedPivotAnim());
    TestNull(TEXT("An unlinked layer does not choose a turn from another stance"), Layer->SelectTurnInPlaceAnimation());
    TestTrue(TEXT("An unlinked preview keeps a stable forward direction"),
        Layer->GetProneDirection() == ELocomotionDirections::Forward);
    return true;
}

#endif
