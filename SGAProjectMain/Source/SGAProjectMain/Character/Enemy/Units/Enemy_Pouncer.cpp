// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy_Pouncer.h"

#include "../../../Controller/EnemyController.h"
#include "../../../Helper/AIActingHelperLibrary.h"
#include "../../../Gun/GunBulletBase.h"
#include "Animation/AnimInstance.h"             // UAnimInstance, Montage_Play(), GetActiveInstanceForMontage()
#include "Animation/AnimMontage.h"              // UAnimMontage

#include "../../CharacterAnimInstance.h"
#include "../../CharacterStateComponent.h"
#include "../../StatComponent.h"
#include "../../../Data/UnitAttackDataAsset.h"

#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "NavigationSystem.h"

#include "Kismet/BlueprintFunctionLibrary.h"
#include "Enemy_Spitter.h"

AEnemy_Pouncer::AEnemy_Pouncer(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{

}

bool AEnemy_Pouncer::CheckAbleTryNear(AActor* target)
{
    if (target == nullptr)
        return false;
    return true;
}

bool AEnemy_Pouncer::CheckAbleTryMiddle(AActor* target)
{
    if (target == nullptr)
        return false;
    if (_isJumpReady == false)
        return false;
    if (UAIActingHelperLibrary::IsFacingTarget(GetActorForwardVector(), GetActorLocation(), target->GetActorLocation()))
        return true;
    return false;
}

bool AEnemy_Pouncer::TryNear(AActor* target)
{
    if (CheckAbleTryNear(target) == false)
        return false;
    if (AttackMelee())
        return true;


    return false;
}

bool AEnemy_Pouncer::TryMiddle(AActor* target)
{

    if (CheckAbleTryMiddle(target) == false)
        return false;
    if(JumpAttack(target))
       return true;
    return false;
}

bool AEnemy_Pouncer::TryFar(AActor* target)
{
    return false;
}


bool AEnemy_Pouncer::JumpAttack(AActor* target)
{
    if (target == nullptr)
        return false;
    if (_isJumpReady == false)
        return false;
    FVector targetLoc;

    FVector muzzleLocation = GetActorLocation();
    targetLoc = target->GetActorLocation();

    FVector direction;
    bool check = UAIActingHelperLibrary::CalculateLaunchDirection(
        direction,
        muzzleLocation,
        targetLoc,
        _jumpPower
    );

    if (check == false)
    {
        return false;
    }

    UE_LOG(LogTemp, Display, TEXT("JumpDirection: %f %f %f"), direction.X, direction.Y, direction.Z);
    DrawDebugLine(GetWorld(), muzzleLocation, muzzleLocation + direction * 500.f, FColor::Yellow, false,1.f, 0, 2.0f);


    UCharacterAnimInstance* anim = Cast<UCharacterAnimInstance>(GetMesh()->GetAnimInstance());


    if (_jumpAttackData->Motion == nullptr)
        return false;
    if (anim == nullptr)
        return false;
    if (_stateComp->ActionBegin() == false)
        return false;

    _isJumpReady = false;

    _curAttackData = _jumpAttackData;
    SetMeleeColisions(_jumpAttackData);

    GetCharacterMovement()->StopMovementImmediately();
    LaunchCharacter(direction * _jumpPower, true, true);

    GetWorld()->GetTimerManager().SetTimer(_jumpTimer, this, &AEnemy_Pouncer::JumpReady, _jumpCooldown, false);
    const float Duration = anim->PlayAnimMontage(_jumpAttackData->Motion);



    return true;



}

bool AEnemy_Pouncer::JumpAttack(FVector target)
{
    FVector start = GetActorLocation();
    FVector targetLoc = target;

    float speed = _jumpPower;

    FVector calculatedVelocity;
    float flightTime;

    if (CalculateLaunchDirectionWithTime(start, targetLoc, speed, calculatedVelocity, flightTime))
    {
        UCharacterAnimInstance* anim = Cast<UCharacterAnimInstance>(GetMesh()->GetAnimInstance());

        if (anim == nullptr)
            return false;
        if (_stateComp->ActionBegin() == false)
            return false;
        if (_jumpAttackData == nullptr)
            return false;
        if (_jumpAttackData->Motion)
        {
            if (flightTime <= 0.0f)
                flightTime = 1.0f;
            float originalAnimLength = _jumpAttackData->Motion->GetPlayLength(); // 원래 애니메이션 길이
            float playRate = originalAnimLength / flightTime;
            anim->PlayAnimMontage(_jumpAttackData->Motion, playRate);

        }

        LaunchCharacter(calculatedVelocity, true, true);
        return true;


    }
    else
    {
        return false;
    }

}


bool AEnemy_Pouncer::CalculateLaunchDirection(const FVector& Start, const FVector& Target, float Speed, FVector& OutLaunchVelocity)
{
    FVector ToTarget = Target - Start;
    float Gravity = GetWorld()->GetGravityZ(); // 일반적으로 -980.f

    // 수평 거리 및 높이 계산
    FVector ToTargetXY = FVector(ToTarget.X, ToTarget.Y, 0.0f);
    float HorizontalDistance = ToTargetXY.Size();
    float VerticalDistance = ToTarget.Z;

    // 중력은 음수이므로 -붙임
    float SpeedSquared = Speed * Speed;
    float GravityAbs = -Gravity;

    float UnderSqrt = FMath::Pow(SpeedSquared, 2) - GravityAbs * (GravityAbs * FMath::Square(HorizontalDistance) + 2 * VerticalDistance * SpeedSquared);

    if (UnderSqrt < 0)
    {
        // 도달 불가능한 경우
        return false;
    }

    // 두 가지 각도가 존재함 (높게/낮게)
    float SqrtPart = FMath::Sqrt(UnderSqrt);

    // 낮은 각도 (빠르게 도달)
    float Angle = FMath::Atan2(SpeedSquared - SqrtPart, GravityAbs * HorizontalDistance);

    // 방향 벡터 구함
    FVector DirectionXY = ToTargetXY.GetSafeNormal();
    FVector LaunchDirection = DirectionXY.RotateAngleAxis(FMath::RadiansToDegrees(Angle), FVector::CrossProduct(DirectionXY, FVector::UpVector));

    // 최종 속도 벡터
    OutLaunchVelocity = LaunchDirection * Speed;

    return true;
}
bool AEnemy_Pouncer::CalculateLaunchDirectionWithTime(const FVector& Start, const FVector& Target, float Speed,FVector& OutLaunchVelocity, float& OutFlightTime)
{
    FVector ToTarget = Target - Start;
    float Gravity = GetWorld()->GetGravityZ(); // 보통 -980.f

    FVector ToTargetXY = FVector(ToTarget.X, ToTarget.Y, 0.0f);
    float HorizontalDistance = ToTargetXY.Size();
    float VerticalDistance = ToTarget.Z;

    float SpeedSquared = Speed * Speed;
    float GravityAbs = -Gravity;

    float UnderSqrt = FMath::Pow(SpeedSquared, 2) - GravityAbs * (GravityAbs * FMath::Square(HorizontalDistance) + 2 * VerticalDistance * SpeedSquared);

    if (UnderSqrt < 0)
    {
        // 도달 불가능
        return false;
    }

    float SqrtPart = FMath::Sqrt(UnderSqrt);

    // 낮은 궤적 (원하면 높은 궤적으로 바꿀 수 있음)
    float Angle = FMath::Atan2(SpeedSquared - SqrtPart, GravityAbs * HorizontalDistance);

    // 방향 벡터 계산
    FVector DirectionXY = ToTargetXY.GetSafeNormal();
    FVector LaunchDirection = DirectionXY.RotateAngleAxis(FMath::RadiansToDegrees(Angle), FVector::CrossProduct(DirectionXY, FVector::UpVector));

    // 최종 속도 벡터
    OutLaunchVelocity = LaunchDirection * Speed;

    // 체공 시간 계산
    float Vy = OutLaunchVelocity.Z;
    float Discriminant = Vy * Vy + 2 * Gravity * VerticalDistance;

    if (Discriminant < 0)
    {
        OutFlightTime = 0.f;
        return false;
    }

    float Time1 = (Vy + FMath::Sqrt(Discriminant)) / -Gravity;
    float Time2 = (Vy - FMath::Sqrt(Discriminant)) / -Gravity;

    // 둘 중 양수인 시간 반환
    OutFlightTime = FMath::Max(Time1, Time2);

    return true;
}

void AEnemy_Spitter::ResetUnit()
{
    Super::ResetUnit();
    _isExplode = false;
}

void AEnemy_Pouncer::PartInit()
{
    ACharacterBase::PartInit();
    if (_statComponent == nullptr)
        return;
    auto partDatas = _statComponent->GetPartDatas();

    if (partDatas->IsEmpty())
        return;
    auto part = partDatas->Find(EBodyPart::Head);
    if (part)
    {
        //기본적인 사망효과.
        if (part->PartStats.IsEmpty() == false)
            part->PartStats[0]._onPartDestroyed.AddDynamic(this, &AEnemy_Pouncer::Critical);
    }
    part = partDatas->Find(EBodyPart::LeftArm);
    if (part)
    {
        //기본적인 사망효과.
        if (part->PartStats.IsEmpty() == false)
            part->PartStats[0]._onPartDestroyed.AddDynamic(this, &AEnemy_Pouncer::Critical);
    }
    part = partDatas->Find(EBodyPart::LeftLeg);
    if (part)
    {
        //기본적인 사망효과.
        if (part->PartStats.IsEmpty() == false)
            part->PartStats[0]._onPartDestroyed.AddDynamic(this, &AEnemy_Pouncer::Critical);
    }
    part = partDatas->Find(EBodyPart::RightArm);
    if (part)
    {
        //기본적인 사망효과.
        if (part->PartStats.IsEmpty() == false)
            part->PartStats[0]._onPartDestroyed.AddDynamic(this, &AEnemy_Pouncer::Critical);
    }
    part = partDatas->Find(EBodyPart::RightLeg);
    if (part)
    {
        //기본적인 사망효과.
        if (part->PartStats.IsEmpty() == false)
            part->PartStats[0]._onPartDestroyed.AddDynamic(this, &AEnemy_Pouncer::Critical);
    }
}


