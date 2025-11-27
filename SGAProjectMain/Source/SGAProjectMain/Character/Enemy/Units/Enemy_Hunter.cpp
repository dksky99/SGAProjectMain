// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy_Hunter.h"

#include "../BehaviorControlComponent.h"
#include "../../../Controller/EnemyController.h"
#include "../../../Helper/AIActingHelperLibrary.h"
#include "Animation/AnimInstance.h"             // UAnimInstance, Montage_Play(), GetActiveInstanceForMontage()
#include "Animation/AnimMontage.h"              // UAnimMontage

#include "../../CharacterAnimInstance.h"
#include "../../CharacterStateComponent.h"
#include "../../StatComponent.h"
#include "../../../Data/UnitAttackDataAsset.h"


bool AEnemy_Hunter::CheckAbleTryFar(AActor* target)
{
    if (_behaviorControlComponent->GetIsTargetAimingMe())
    {
        return true;
    }
    return false;
}

bool AEnemy_Hunter::TryFar(AActor* target)
{
    if (_behaviorControlComponent->GetIsTargetAimingMe())
    {
        if (TryDodge(target))
        {
            return true;
        }
    }
    return false;
}

bool AEnemy_Hunter::TryDodge(AActor* target)
{

    UCharacterAnimInstance* anim = Cast<UCharacterAnimInstance>(GetMesh()->GetAnimInstance());
    if (_dodgeMontage == nullptr)
        return false;
    if (anim == nullptr)
        return false;
    if (_stateComp->ActionBegin() == false)
        return false;
    bool jumpdirection = FMath::RandBool();

    const float jumpSign = (jumpdirection) ? 1.0f : -1.0f;

    // AI 위치에서 타겟을 바라보는 벡터
    const FVector dirToTarget = (target->GetActorLocation() - GetActorLocation()).GetSafeNormal();

    // 타겟 벡터의 오른쪽(수직) 벡터 계산 (Cross Product)
    const FVector jumpVector = FVector::CrossProduct(dirToTarget, FVector::UpVector).GetSafeNormal();

    // 최종 이동 방향 (오른쪽 또는 왼쪽)
    const FVector FinalMoveDirection = (jumpVector * jumpSign+FVector::UpVector).GetSafeNormal();



    LaunchCharacter(FinalMoveDirection * _jumpPower*0.5f, true, true);

    const float Duration = anim->PlayAnimMontage(_dodgeMontage);

    return true;



}

void AEnemy_Hunter::PartInit()
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
            part->PartStats[0]._onPartDestroyed.AddDynamic(this, &AEnemy_Hunter::Critical);
    }
}


