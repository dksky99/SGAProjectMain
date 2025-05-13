// Fill out your copyright notice in the Description page of Project Settings.


#include "H_Relation.h"

bool H_Relation::IsInSight(AActor* user, AActor* target, float maxAngle)
{
    if (!user || !target) return false;

    FVector ForwardVector = user->GetActorForwardVector(); // AI의 정면 벡터
    FVector ToTarget = (target->GetActorLocation() - user->GetActorLocation()).GetSafeNormal(); // AI → Target 방향 벡터

    float DotProduct = FVector::DotProduct(ForwardVector, ToTarget); // 두 벡터의 내적 계산
    float Angle = FMath::RadiansToDegrees(FMath::Acos(DotProduct)); // 내적을 이용해 각도 계산

    return Angle <= maxAngle; // 설정한 각도 이내인지 체크
}

bool H_Relation::IsInSightYaw(AActor* user, AActor* target, float maxAngle)
{
    if (!user || !target) return false;

    FVector ForwardVector = user->GetActorForwardVector(); // AI의 정면 벡터
    FVector ToTarget = (target->GetActorLocation() - user->GetActorLocation());
    ToTarget.Z = ForwardVector.Z;
    ToTarget=ToTarget.GetSafeNormal();
    float DotProduct = FVector::DotProduct(ForwardVector, ToTarget); // 두 벡터의 내적 계산
    float Angle = FMath::RadiansToDegrees(FMath::Acos(DotProduct)); // 내적을 이용해 각도 계산

    return Angle <= maxAngle; // 설정한 각도 이내인지 체크
}

bool H_Relation::IsInDistance(AActor* user, AActor* target, float maxDistance,float minDistance=0.0f)
{
    if (!user || !target) return false;

    return (user->GetDistanceTo(target) <= maxDistance&& user->GetDistanceTo(target) >= minDistance); // 설정한 각도 이내인지 체크
}

FVector H_Relation::LocOfDistanceFromTarget(AActor* user, AActor* target, float distance)
{
    FVector temp = user->GetActorLocation() - target->GetActorLocation();
    temp.Normalize();
    temp*= distance;
    return target->GetActorLocation()+temp;
}

FVector H_Relation::FindFloorFromLoc(AActor* user,float maxDistance, bool useAnotherLoc, FVector loc)
{
    FVector Start = useAnotherLoc?loc: user->GetActorLocation(); // 캐릭터 위치
    FVector End = Start - FVector(0, 0, maxDistance); // 아래 방향으로 1000 유닛

    FHitResult HitResult;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(user); // 캐릭터 자신을 충돌 검사에서 제외

    // 라인 트레이스 실행
    bool bHit = user->GetWorld()->LineTraceSingleByChannel(
        HitResult,
        Start,
        End,
        ECC_GameTraceChannel9, // 보통 ECC_Visibility 또는 ECC_WorldStatic 사용
        Params
    );

    if (bHit)
    {
       

       // UE_LOG(LogTemp, Error, TEXT("Collision Target %s"), *(HitResult.GetActor()->GetName()));
        return HitResult.ImpactPoint; // 충돌한 위치 반환 (땅 좌표)
        
    }

       // UE_LOG(LogTemp, Error, TEXT("Collision Target None"));
    return FVector::ZeroVector; // 충돌이 없으면 (0,0,0) 반환
}
