#pragma once

#include "CoreMinimal.h"

// Pure calculations shared by animation updates and deterministic tests.
// These helpers do not read actors, components, bones, or animation state.
namespace CharacterAimMath
{
    inline float InterpolateFullAimYaw(float Current, float Target, float DeltaSeconds, float Speed = 5.f)
    {
        const float SafeCurrent = FMath::IsFinite(Current) ? FMath::UnwindDegrees(Current) : 0.f;
        if (!FMath::IsFinite(Target) || !FMath::IsFinite(DeltaSeconds) || DeltaSeconds <= 0.f)
        {
            return SafeCurrent;
        }
        const float Delta = FMath::FindDeltaAngleDegrees(SafeCurrent, FMath::UnwindDegrees(Target));
        return FMath::UnwindDegrees(SafeCurrent + Delta * FMath::Clamp(DeltaSeconds * Speed, 0.f, 1.f));
    }

    // Clockwise sectors: forward, right, backward, left.
    inline int32 SelectProneSector(float Yaw, int32 PreviousSector, float Hysteresis = 8.f)
    {
        if (!FMath::IsFinite(Yaw))
        {
            return FMath::Clamp(PreviousSector, 0, 3);
        }
        const float NormalizedYaw = FMath::UnwindDegrees(Yaw);
        const int32 SafePrevious = FMath::Clamp(PreviousSector, 0, 3);
        const float PreviousCenter = SafePrevious * 90.f;
        if (FMath::Abs(FMath::FindDeltaAngleDegrees(PreviousCenter, NormalizedYaw)) <=
            45.f + FMath::Clamp(Hysteresis, 0.f, 40.f))
        {
            return SafePrevious;
        }
        return (FMath::FloorToInt((NormalizedYaw + 45.f) / 90.f) + 4) % 4;
    }

    inline float CalculateProneResidualYaw(float FullYaw, int32 Sector)
    {
        return FMath::IsFinite(FullYaw)
            ? FMath::FindDeltaAngleDegrees(FMath::Clamp(Sector, 0, 3) * 90.f, FullYaw)
            : 0.f;
    }

    inline float ConsumeTurnCurve(float RootYawOffset, float CurveDelta)
    {
        if (!FMath::IsFinite(RootYawOffset) || !FMath::IsFinite(CurveDelta))
        {
            return 0.f;
        }
        if (FMath::Sign(RootYawOffset) != FMath::Sign(CurveDelta))
        {
            return RootYawOffset;
        }
        const float Remaining = RootYawOffset - CurveDelta;
        return FMath::Sign(Remaining) == FMath::Sign(RootYawOffset) ? Remaining : 0.f;
    }

    struct FAimAngles
    {
        float Yaw = 0.f;
        float Pitch = 0.f;
    };

    inline bool TryCalculateAimAngles(
        const FVector& OriginWorld,
        const FVector& TargetWorld,
        const FRotator& ActorWorldRotation,
        FAimAngles& OutAngles)
    {
        OutAngles = FAimAngles{};

        const FVector DirectionWorld = TargetWorld - OriginWorld;
        if (DirectionWorld.ContainsNaN() || ActorWorldRotation.ContainsNaN() || DirectionWorld.IsNearlyZero())
        {
            return false;
        }

        // Preserve the existing actor-relative Euler convention. This is not
        // a pose-relative solution for a rolled or supine character.
        const FRotator RelativeRotation = (DirectionWorld.Rotation() - ActorWorldRotation).GetNormalized();
        if (RelativeRotation.ContainsNaN())
        {
            return false;
        }

        OutAngles.Yaw = static_cast<float>(RelativeRotation.Yaw);
        OutAngles.Pitch = static_cast<float>(RelativeRotation.Pitch);
        return true;
    }

    inline float CalculateActorYawDelta(float PreviousWorldYaw, float CurrentWorldYaw)
    {
        if (!FMath::IsFinite(PreviousWorldYaw) || !FMath::IsFinite(CurrentWorldYaw))
        {
            return 0.f;
        }

        return FMath::FindDeltaAngleDegrees(
            FMath::UnwindDegrees(PreviousWorldYaw),
            FMath::UnwindDegrees(CurrentWorldYaw));
    }

    inline float InterpolateLimitedAimAngle(
        float CurrentAngle,
        float TargetAngle,
        float DeltaSeconds,
        float InterpSpeed = 5.f)
    {
        const float SafeCurrent = FMath::IsFinite(CurrentAngle) ? FMath::Clamp(CurrentAngle, -90.f, 90.f) : 0.f;
        if (!FMath::IsFinite(TargetAngle) || !FMath::IsFinite(DeltaSeconds) || DeltaSeconds <= 0.f || !FMath::IsFinite(InterpSpeed))
        {
            return SafeCurrent;
        }

        return FMath::FInterpTo(SafeCurrent, FMath::Clamp(TargetAngle, -90.f, 90.f), DeltaSeconds, InterpSpeed);
    }
}
