// Fill out your copyright notice in the Description page of Project Settings.


#include "AIActingHelperLibrary.h"
#include "Math/UnrealMathUtility.h" // FMath::DegreesToRadians 포함

bool UAIActingHelperLibrary::CalculateLaunchDirection(FVector& OutLaunchDirection, const FVector& StartLocation, const FVector& TargetLocation, float LaunchSpeed, bool bPreferHighArc, float GravityMagnitude)
{
    // 시작점에서 목표점까지의 변위 벡터를 계산합니다.
    const FVector Displacement = TargetLocation - StartLocation;

    // 변위 벡터를 수평(XY 평면)과 수직(Z축) 성분으로 분리합니다.
    const FVector DisplacementXY = FVector(Displacement.X, Displacement.Y, 0.0f);

    // 수평 거리(x)와 수직 거리(y)를 구합니다.
    const float x = DisplacementXY.Size();
    const float y = Displacement.Z;

    // 발사 속력의 제곱을 미리 계산해 둡니다. (계산 효율성)
    const float v_sq = FMath::Square(LaunchSpeed);

    // 중력 크기를 양수로 고정합니다. 언리얼의 중력은 보통 음수이므로 절대값을 사용합니다.
    const float g = FMath::Abs(GravityMagnitude);

    // 포물선 궤적 공식(y = x * tan(θ) - (g * x^2) / (2 * v^2 * cos^2(θ)))을 tan(θ)에 대한 2차 방정식으로 변환하기 위한 준비 과정입니다.
    // a*T^2 + b*T + c = 0 형태 (여기서 T = tan(θ))
    const float a = (g * FMath::Square(x)) / (2.0f * v_sq);
    const float b = -x;
    const float c = y + a;

    // 2차 방정식의 판별식(b^2 - 4ac)을 계산합니다. 이 값이 0보다 작으면 해가 없으므로 목표에 도달할 수 없습니다.
    const float Discriminant = FMath::Square(b) - 4.0f * a * c;

    // 판별식이 0보다 작으면 물리적으로 도달할 수 없는 거리이므로 false를 반환하고 함수를 종료합니다.
    if (Discriminant < 0.0f)
    {
        OutLaunchDirection = FVector::ZeroVector; // 방향 벡터를 0으로 초기화합니다.
        return false; // 실패를 알립니다.
    }

    // 판별식의 제곱근을 계산합니다.
    const float SqrtDiscriminant = FMath::Sqrt(Discriminant);

    // 2차 방정식의 두 근(tan의 두 해)을 구합니다. 각각 낮은 궤적과 높은 궤적에 해당합니다.
    const float TanTheta1 = (-b + SqrtDiscriminant) / (2.0f * a);
    const float TanTheta2 = (-b - SqrtDiscriminant) / (2.0f * a);

    // 사용자의 선택(bPreferHighArc)에 따라 두 각도 중 하나를 선택합니다. atan(높은 값)이 더 큰 각도를 반환합니다.
    const float SelectedTanTheta = bPreferHighArc ? FMath::Max(TanTheta1, TanTheta2) : FMath::Min(TanTheta1, TanTheta2);

    // tan 값으로부터 발사 각도를 라디안 단위로 구합니다.
    const float LaunchAngleRadians = FMath::Atan(SelectedTanTheta);

    // 시작점에서 목표점까지의 수평 방향 벡터를 구합니다. (정규화하여 방향만 남김)
    const FVector HorizontalDirection = DisplacementXY.GetSafeNormal();

    // 초기 발사 속도의 수평 성분 크기를 계산합니다. (속력 * cos(각도))
    const float HorizontalSpeed = LaunchSpeed * FMath::Cos(LaunchAngleRadians);

    // 초기 발사 속도의 수직 성분 크기를 계산합니다. (속력 * sin(각도))
    const float VerticalSpeed = LaunchSpeed * FMath::Sin(LaunchAngleRadians);

    // 3D 공간에서의 초기 발사 속도 벡터를 조합합니다. (수평 방향 벡터 * 수평 속력 + 위쪽 방향 벡터 * 수직 속력)
    const FVector LaunchVelocity = (HorizontalDirection * HorizontalSpeed) + (FVector::UpVector * VerticalSpeed);

    // 최종적으로 계산된 속도 벡터를 정규화하여 방향 벡터(단위 벡터)로 만듭니다.
    OutLaunchDirection = LaunchVelocity.GetSafeNormal();

    

    // 성공적으로 계산되었음을 알립니다.
    return true;
}

bool UAIActingHelperLibrary::IsFacingTarget(const FVector& ForwardVector, const FVector& StartLocation, const FVector& TargetLocation, float AngleTolerance)
{
    // 1. 시작점에서 목표점까지의 방향 벡터를 계산하고 정규화합니다.
    const FVector DirectionToTarget = (TargetLocation - StartLocation).GetSafeNormal();

    // 2. 확인할 방향 벡터도 정규화합니다. (안전 장치)
    const FVector NormalizedForward = ForwardVector.GetSafeNormal();

    // 3. 두 방향 벡터를 내적(Dot Product)합니다.
    const float DotProduct = FVector::DotProduct(NormalizedForward, DirectionToTarget);

    // 4. 허용 각도(Degree)를 코사인 값으로 변환합니다.
    // 각도가 0일 때 코사인 값은 1, 90도일 때 0입니다.
    const float ToleranceCosine = FMath::Cos(FMath::DegreesToRadians(AngleTolerance));

    // 5. 내적 결과가 허용 코사인 값보다 크거나 같으면, 허용 각도 내에 있는 것입니다.
    return DotProduct >= ToleranceCosine;
}

EAimStatus UAIActingHelperLibrary::CheckAimAndSuggestCorrection(const AActor* Character, const FVector& TargetLocation, float YawAngleTolerance)
{
    if (!Character)
    {
        return EAimStatus::Misaligned;
    }

    const FVector CharLocation = Character->GetActorLocation();
    const FVector CharForward = Character->GetActorForwardVector();
    const FVector CharUp = Character->GetActorUpVector();

    // 목표를 향하는 전체 3D 방향 벡터
    const FVector DirectionToTarget3D = (TargetLocation - CharLocation).GetSafeNormal();

    // --- 좌우 (Yaw) 정렬 확인 ---
    // 캐릭터의 위쪽 벡터(UpVector)를 법선으로 하는 수평면에 두 벡터를 투영하여
    // 순수하게 좌우 각도 차이만 비교합니다.

    // 1. 캐릭터의 정면 벡터를 수평면에 투영합니다.
    const FVector ForwardHorizontal = FVector::VectorPlaneProject(CharForward, CharUp).GetSafeNormal();

    // 2. 목표 방향 벡터를 수평면에 투영합니다.
    const FVector TargetDirHorizontal = FVector::VectorPlaneProject(DirectionToTarget3D, CharUp).GetSafeNormal();

    // 3. 투영된 벡터들로 내적하여 Yaw 각도를 확인합니다.
    const float YawDot = FVector::DotProduct(ForwardHorizontal, TargetDirHorizontal);
    const float YawToleranceCosine = FMath::Cos(FMath::DegreesToRadians(YawAngleTolerance));

    if (YawDot < YawToleranceCosine)
    {
        // 좌우 조준부터 맞지 않으면, 완전히 빗나간 상태입니다.
        return EAimStatus::Misaligned;
    }

    // --- Yaw가 맞았다면, Pitch까지 맞는지 확인 ---
    // Yaw는 통과했으므로, 이제 전체 3D 벡터를 비교하여 Pitch가 맞는지 확인합니다.
    // 여기서는 매우 엄격한 허용 각도(예: 2도)를 사용해 '완벽한 조준'을 체크합니다.
    const bool bIsFullyAligned = IsFacingTarget(CharForward, CharLocation, TargetLocation, 2.0f);

    if (bIsFullyAligned)
    {
        // 3D 전체 조준이 완벽하면 Aligned 상태입니다.
        return EAimStatus::Aligned;
    }
    else
    {
        // Yaw는 맞았지만 3D 전체 조준은 실패했으므로, Pitch만 조정하면 됩니다.
        return EAimStatus::NeedsPitchCorrection;
    }
}

bool UAIActingHelperLibrary::IsFacingTarget_WithAngle(const FVector& OwnerLoc, const FVector& TargetLoc, const FVector& TargetLook,  float& OutputAngle, float AngleTolerance)
{
    FVector temp = OwnerLoc - TargetLoc;

    float dot = FVector::DotProduct(temp.GetSafeNormal(), TargetLook.GetSafeNormal());

    OutputAngle = FMath::RadiansToDegrees(FMath::Acos(dot));

    if (AngleTolerance < OutputAngle)
        return false;

    return true;
}

FVector UAIActingHelperLibrary::CalculateSpreadDirection(const FVector& Dir, float ErrorAngleDeg)
{
    // 0. 오차 각도가 0 이하면, 계산할 필요 없이 원본 방향만 정규화하여 반환
    if (ErrorAngleDeg <= 0.0f)
    {
        return Dir.GetSafeNormal();
    }

    // 1. FMath::VRandCone은 '라디안' 단위를 사용하므로, 입력받은 '도'(Degree)를 '라디안'(Radian)으로 변환
    const float ErrorAngleRad = FMath::DegreesToRadians(ErrorAngleDeg);

    // 2. FMath::VRandCone을 호출하여 무작위 벡터 생성
    //    이 함수는 Dir을 기준으로 ErrorAngleRad 각도 내에서 균일하게 분포된
    //    무작위 방향 벡터를 반환합니다.
    //    입력되는 Dir이 정규화(Normalized)되어 있어야 하므로 GetSafeNormal()을 사용합니다.
    return FMath::VRandCone(Dir.GetSafeNormal(), ErrorAngleRad);
}

