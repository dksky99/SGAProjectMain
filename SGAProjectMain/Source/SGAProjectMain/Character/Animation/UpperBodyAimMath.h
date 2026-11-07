#pragma once
#include "CoreMinimal.h"

namespace UpperBodyAimMath
{
    // Rotate a rigid arm about its shoulder so its translated muzzle ray hits Target.
    // Solving the ray parameter first avoids the near-target error of aiming from the hand.
    inline bool SolveRayRotation(const FVector& Pivot, const FVector& Muzzle,
        const FVector& Forward, const FVector& Target, FQuat& Rotation)
    {
        Rotation = FQuat::Identity;
        if (Pivot.ContainsNaN() || Muzzle.ContainsNaN() || Forward.ContainsNaN() || Target.ContainsNaN()) return false;
        const FVector F = Forward.GetSafeNormal();
        const FVector P = Muzzle - Pivot, T = Target - Pivot;
        if (F.IsNearlyZero() || T.IsNearlyZero()) return false;
        const double Along = FVector::DotProduct(P, F);
        const double Discriminant = Along * Along + T.SizeSquared() - P.SizeSquared();
        if (Discriminant < 0.) return false; // Inside the unreachable shoulder/muzzle cone.
        const double Distance = -Along + FMath::Sqrt(Discriminant);
        if (Distance < 0.) return false;
        const FVector Ray = P + Distance * F;
        if (Ray.IsNearlyZero()) return false;
        Rotation = FQuat::FindBetweenNormals(Ray.GetSafeNormal(), T.GetSafeNormal());
        Rotation.Normalize();
        return !Rotation.ContainsNaN();
    }
}
