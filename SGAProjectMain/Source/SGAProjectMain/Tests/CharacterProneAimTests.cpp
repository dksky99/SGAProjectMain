#include "CoreMinimal.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "../Character/CharacterAimMath.h"
#include "../Character/CharacterAnimInstance.h"
#include "Components/SkeletalMeshComponent.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCharacterStanceTransitionTest,
    "SGA.Character.Animation.StanceTransitionSource",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FCharacterStanceTransitionTest::RunTest(const FString& Parameters)
{
    auto* Mesh = NewObject<USkeletalMeshComponent>();
    auto* Anim = NewObject<UCharacterAnimInstance>(Mesh);
    Anim->RecieveCharacterStateChange(static_cast<int32>(ECharacterState::Proning));
    Anim->UpdateStanceState();
    TestTrue(TEXT("Standing to prone records its original stance"),
        Anim->GetStanceTransitionSource() == ECharacterState::Standing);
    TestTrue(TEXT("A stance change is reported on its first update"), Anim->_bIsStateChanged);
    Anim->UpdateStanceState();
    TestFalse(TEXT("The next frame does not replay the state change"), Anim->_bIsStateChanged);
    TestTrue(TEXT("A late layer setup still knows that the transition began standing"),
        Anim->GetStanceTransitionSource() == ECharacterState::Standing);
    Anim->RecieveCharacterStateChange(static_cast<int32>(ECharacterState::Crouching));
    Anim->UpdateStanceState();
    TestTrue(TEXT("The following transition records prone as its source"),
        Anim->GetStanceTransitionSource() == ECharacterState::Proning);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCharacterProneSectorTest,
    "SGA.Character.Animation.ProneSectors",
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::EngineFilter)

bool FCharacterProneSectorTest::RunTest(const FString& Parameters)
{
    TestEqual(TEXT("Forward target selects forward"), CharacterAimMath::SelectProneSector(0.f, 0), 0);
    TestEqual(TEXT("Right target selects right"), CharacterAimMath::SelectProneSector(90.f, 0), 1);
    TestEqual(TEXT("Rear target reaches the formerly unreachable backward pose"),
        CharacterAimMath::SelectProneSector(170.f, 0), 2);
    TestEqual(TEXT("Left target selects left"), CharacterAimMath::SelectProneSector(-90.f, 0), 3);
    TestEqual(TEXT("Rear wrap preserves the backward sector"),
        CharacterAimMath::SelectProneSector(-179.f, 2), 2);
    TestEqual(TEXT("A noisy 46 degree target does not chatter across the boundary"),
        CharacterAimMath::SelectProneSector(46.f, 0), 0);
    TestEqual(TEXT("Passing the deadband selects the next sector"),
        CharacterAimMath::SelectProneSector(54.f, 0), 1);
    TestEqual(TEXT("The reverse crossing retains the right sector"),
        CharacterAimMath::SelectProneSector(44.f, 1), 1);
    TestTrue(TEXT("Rear 170 degree target requires only -10 degrees of local yaw"),
        FMath::IsNearlyEqual(CharacterAimMath::CalculateProneResidualYaw(170.f, 2), -10.f));
    TestTrue(TEXT("Rear wrap computes a small residual instead of a 358 degree turn"),
        FMath::IsNearlyEqual(CharacterAimMath::CalculateProneResidualYaw(-179.f, 2), 1.f));
    const float AcrossSeam = CharacterAimMath::InterpolateFullAimYaw(179.f, -179.f, 1.f / 60.f);
    TestTrue(TEXT("Filtering across the rear seam follows the short arc"), AcrossSeam > 179.f);
    TestTrue(TEXT("An absolute-yaw prone AO can reach its backward center"),
        FMath::Abs(CharacterAimMath::InterpolateFullAimYaw(0.f, 180.f, 1.f)) == 180.f);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCharacterTurnCurveTest,
    "SGA.Character.Animation.TurnCurveConsumption",
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::EngineFilter)

bool FCharacterTurnCurveTest::RunTest(const FString& Parameters)
{
    TestEqual(TEXT("A turn consumes part of a positive offset"),
        CharacterAimMath::ConsumeTurnCurve(30.f, 10.f), 20.f);
    TestEqual(TEXT("A turn consumes part of a negative offset"),
        CharacterAimMath::ConsumeTurnCurve(-30.f, -10.f), -20.f);
    TestEqual(TEXT("Overshooting the end cannot reverse the offset"),
        CharacterAimMath::ConsumeTurnCurve(5.f, 10.f), 0.f);
    TestEqual(TEXT("Negative overshoot cannot reverse the offset"),
        CharacterAimMath::ConsumeTurnCurve(-5.f, -10.f), 0.f);
    TestEqual(TEXT("An unrelated curve cannot increase the existing twist"),
        CharacterAimMath::ConsumeTurnCurve(30.f, -10.f), 30.f);
    return true;
}

#endif
