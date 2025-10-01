// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy_Spitter.h"
#include "../../../Controller/EnemyController.h"
#include "../../../Helper/AIActingHelperLibrary.h"
#include "../../../Gun/GunBulletBase.h"
#include "Animation/AnimInstance.h"             // UAnimInstance, Montage_Play(), GetActiveInstanceForMontage()
#include "Animation/AnimMontage.h"              // UAnimMontage

#include "../../CharacterAnimInstance.h"
#include "../../CharacterStateComponent.h"
#include "../../../Data/UnitAttackDataAsset.h"


AEnemy_Spitter::AEnemy_Spitter(const FObjectInitializer& ObjectInitializer):Super(ObjectInitializer)
{

}

void AEnemy_Spitter::BeginPlay()
{
    Super::BeginPlay();
}

void AEnemy_Spitter::Init_Pooling()
{

}
bool AEnemy_Spitter::CheckAbleTryNear(AActor* target)
{
    if (target == nullptr)
        return false;
    if(UAIActingHelperLibrary::IsFacingTarget(GetActorForwardVector(),GetActorLocation(),target->GetActorLocation()))
        return true;
    return false;
}
bool AEnemy_Spitter::CheckAbleTryMiddle(AActor* target)
{

    if (target == nullptr)
        return false;
    if (UAIActingHelperLibrary::IsFacingTarget(GetActorForwardVector(), GetActorLocation(), target->GetActorLocation()))
        return true;
    return false;
}
bool AEnemy_Spitter::TryNear(AActor* target)
{
    if (CheckAbleTryNear(target) == false)
        return false;
    if (Spit(target))
        return true;

    return false;
}

bool AEnemy_Spitter::TryMiddle(AActor* target)
{
    if (CheckAbleTryMiddle(target) == false)
        return false;
    if (Spit(target))
        return true;

    return false;
}

bool AEnemy_Spitter::TryFar(AActor* target)
{
    return false;
}

bool AEnemy_Spitter::Spit(AActor* target)
{
    if (target == nullptr)
        return false;
    FVector targetLoc;

    FVector muzzleLocation = GetMesh()->GetSocketLocation(TEXT("attach_acid_spray"))+GetActorForwardVector()*10.f;
    targetLoc = target->GetActorLocation();
    float projectileSpeed=3500.f;  //투사체의 속도를 고려하여 사격해야함. 그런데 인스턴스가 필요. 
    FVector direction;
    bool check = UAIActingHelperLibrary::CalculateLaunchDirection(
        direction,
        muzzleLocation,
        targetLoc,
        projectileSpeed
    );

    if (check == false)
    {
        return false;
    }
    _spitDirection = direction;

    UE_LOG(LogTemp, Display, TEXT("SpitDirection: %f %f %f"), direction.X, direction.Y, direction.Z);
    DrawDebugLine(GetWorld(), muzzleLocation, muzzleLocation + direction * 500.f, FColor::Yellow, false, 1.f, 0, 2.0f);

    if (_reservedFunction.IsBound())
        _reservedFunction.Unbind();
    _reservedFunction.BindUObject(this, &AEnemy_Spitter::SpitProjectile);


    UCharacterAnimInstance* anim = Cast<UCharacterAnimInstance>(GetMesh()->GetAnimInstance());


    if (_spitAttack_Animation->Motion==nullptr)
        return false;
    if (anim == nullptr)
        return false;
    if (_stateComp->ActionBegin() == false)
        return false;



    const float Duration = anim->PlayAnimMontage(_spitAttack_Animation->Motion);

    return true;
}


void AEnemy_Spitter::SpitProjectile()
{

    FVector muzzleLocation = GetMesh()->GetSocketLocation(TEXT("attach_acid_spray")) + GetActorForwardVector() * 10.f;
    FVector fireDirection = _spitDirection;

    FRotator fireRotation = fireDirection.Rotation();

    FActorSpawnParameters spawnParams;
    spawnParams.Instigator = this;
    spawnParams.Owner = this; // 총
    spawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn; // 겹치면 움직여서, 아니면 억지로라도 생성

    AGunBulletBase* projectile = GetWorld()->SpawnActor<AGunBulletBase>(_projectileClass, muzzleLocation, _spitDirection.Rotation(), spawnParams);
}
