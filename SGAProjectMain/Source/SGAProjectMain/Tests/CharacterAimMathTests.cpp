#include "CoreMinimal.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "../Character/CharacterAimMath.h"

#include <limits>

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FCharacterAimCardinalAnglesTest,
    "SGA.Character.AimMath.CardinalAngles",
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::EngineFilter)

bool FCharacterAimCardinalAnglesTest::RunTest(const FString& Parameters)
{
    (void)Parameters;
    struct FCase
    {
        FVector Target;
        float ExpectedYaw;
    };

    const FCase Cases[] = {
        {FVector(100.f, 0.f, 0.f), 0.f},
        {FVector(0.f, 100.f, 0.f), 90.f},
        {FVector(0.f, -100.f, 0.f), -90.f},
        {FVector(-100.f, 0.f, 0.f), 180.f},
        {FVector(100.f, 100.f, 0.f), 45.f}
    };

    for (const FCase& Case : Cases)
    {
        CharacterAimMath::FAimAngles Angles;
        TestTrue(TEXT("A nonzero cardinal/diagonal target is valid"),
            CharacterAimMath::TryCalculateAimAngles(FVector::ZeroVector, Case.Target, FRotator::ZeroRotator, Angles));
        TestTrue(FString::Printf(TEXT("Target %s has expected yaw %.1f"), *Case.Target.ToString(), Case.ExpectedYaw),
            FMath::Abs(FMath::FindDeltaAngleDegrees(Angles.Yaw, Case.ExpectedYaw)) < 0.001f);
        TestTrue(TEXT("Level targets have zero pitch"), FMath::IsNearlyZero(Angles.Pitch, 0.001f));
    }

    CharacterAimMath::FAimAngles Up;
    CharacterAimMath::FAimAngles Down;
    TestTrue(TEXT("An elevated target is valid"), CharacterAimMath::TryCalculateAimAngles(
        FVector::ZeroVector, FVector(100.f, 0.f, 100.f), FRotator::ZeroRotator, Up));
    TestTrue(TEXT("An elevated target has +45 pitch"), FMath::IsNearlyEqual(Up.Pitch, 45.f, 0.001f));
    TestTrue(TEXT("A lowered target is valid"), CharacterAimMath::TryCalculateAimAngles(
        FVector::ZeroVector, FVector(100.f, 0.f, -100.f), FRotator::ZeroRotator, Down));
    TestTrue(TEXT("A lowered target has -45 pitch"), FMath::IsNearlyEqual(Down.Pitch, -45.f, 0.001f));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FCharacterAimCoordinateInvarianceTest,
    "SGA.Character.AimMath.CoordinateInvariance",
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::EngineFilter)

bool FCharacterAimCoordinateInvarianceTest::RunTest(const FString& Parameters)
{
    (void)Parameters;
    const FVector Origin(10.f, -20.f, 30.f);
    const FVector Target(110.f, 80.f, 130.f);
    const FRotator ActorRotation(0.f, 20.f, 0.f);
    const FRotator SharedWorldRotation(0.f, 90.f, 0.f);
    const FVector Translation(50000.f, -27000.f, 730.f);

    CharacterAimMath::FAimAngles Before;
    CharacterAimMath::FAimAngles After;
    TestTrue(TEXT("The base world-space target is valid"),
        CharacterAimMath::TryCalculateAimAngles(Origin, Target, ActorRotation, Before));
    TestTrue(TEXT("The transformed world-space target is valid"),
        CharacterAimMath::TryCalculateAimAngles(
            SharedWorldRotation.RotateVector(Origin) + Translation,
            SharedWorldRotation.RotateVector(Target) + Translation,
            FRotator(0.f, 110.f, 0.f), After));

    TestTrue(TEXT("The known relative yaw is 25 degrees"), FMath::IsNearlyEqual(Before.Yaw, 25.f, 0.001f));
    TestTrue(TEXT("The known relative pitch is approximately 35.26439 degrees"),
        FMath::IsNearlyEqual(Before.Pitch, 35.26439f, 0.001f));
    TestTrue(TEXT("A common yaw rotation and translation preserve relative yaw"),
        FMath::IsNearlyEqual(Before.Yaw, After.Yaw, 0.001f));
    TestTrue(TEXT("A common yaw rotation and translation preserve relative pitch"),
        FMath::IsNearlyEqual(Before.Pitch, After.Pitch, 0.001f));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FCharacterAimWorldYawDeltaTest,
    "SGA.Character.AimMath.WorldYawDelta",
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::EngineFilter)

bool FCharacterAimWorldYawDeltaTest::RunTest(const FString& Parameters)
{
    (void)Parameters;
    TestTrue(TEXT("179 to -179 is a positive 2 degree turn"),
        FMath::IsNearlyEqual(CharacterAimMath::CalculateActorYawDelta(179.f, -179.f), 2.f, 0.001f));
    TestTrue(TEXT("-179 to 179 is a negative 2 degree turn"),
        FMath::IsNearlyEqual(CharacterAimMath::CalculateActorYawDelta(-179.f, 179.f), -2.f, 0.001f));
    TestTrue(TEXT("Equivalent world headings have zero delta"),
        FMath::IsNearlyZero(CharacterAimMath::CalculateActorYawDelta(180.f, -180.f), 0.001f));
    TestTrue(TEXT("An unchanged actor has zero delta regardless of the aim target"),
        FMath::IsNearlyZero(CharacterAimMath::CalculateActorYawDelta(30.f, 30.f), 0.001f));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FCharacterAimInvalidInputAndSmoothingTest,
    "SGA.Character.AimMath.InvalidInputAndSmoothing",
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::EngineFilter)

bool FCharacterAimInvalidInputAndSmoothingTest::RunTest(const FString& Parameters)
{
    (void)Parameters;
    const float NotANumber = std::numeric_limits<float>::quiet_NaN();
    CharacterAimMath::FAimAngles Angles;
    TestFalse(TEXT("A target at the aim origin has no usable direction"),
        CharacterAimMath::TryCalculateAimAngles(FVector(10.f), FVector(10.f), FRotator::ZeroRotator, Angles));
    TestTrue(TEXT("An invalid direction leaves finite output angles"), FMath::IsFinite(Angles.Yaw) && FMath::IsFinite(Angles.Pitch));
    TestFalse(TEXT("A nonfinite target is rejected"),
        CharacterAimMath::TryCalculateAimAngles(FVector::ZeroVector, FVector(NotANumber, 0.f, 0.f), FRotator::ZeroRotator, Angles));
    TestTrue(TEXT("Zero delta time preserves the filtered output"),
        FMath::IsNearlyEqual(CharacterAimMath::InterpolateLimitedAimAngle(30.f, -90.f, 0.f), 30.f, 0.001f));
    TestTrue(TEXT("Negative delta time preserves the filtered output"),
        FMath::IsNearlyEqual(CharacterAimMath::InterpolateLimitedAimAngle(30.f, -90.f, -1.f), 30.f, 0.001f));
    TestTrue(TEXT("A nonfinite target cannot poison the filtered output"),
        FMath::IsNearlyEqual(CharacterAimMath::InterpolateLimitedAimAngle(30.f, NotANumber, 1.f), 30.f, 0.001f));
    TestTrue(TEXT("The legacy AO output remains capped at +90"),
        FMath::IsNearlyEqual(CharacterAimMath::InterpolateLimitedAimAngle(0.f, 180.f, 1.f), 90.f, 0.001f));
    TestTrue(TEXT("The legacy AO output remains capped at -90"),
        FMath::IsNearlyEqual(CharacterAimMath::InterpolateLimitedAimAngle(0.f, -180.f, 1.f), -90.f, 0.001f));

    const float FirstFrame = CharacterAimMath::InterpolateLimitedAimAngle(0.f, 60.f, 1.f / 60.f);
    const float SecondFrame = CharacterAimMath::InterpolateLimitedAimAngle(FirstFrame, 60.f, 1.f / 60.f);
    TestTrue(TEXT("A retained interpolation state approaches the target across frames"),
        FirstFrame > 0.f && SecondFrame > FirstFrame && SecondFrame < 60.f);
    return true;
}

#endif
