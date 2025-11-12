// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy_HiveGuard.h"
#include "../BehaviorControlComponent.h"
#include "../../../Controller/EnemyController.h"
#include "../../../Helper/AIActingHelperLibrary.h"
#include "Animation/AnimInstance.h"             // UAnimInstance, Montage_Play(), GetActiveInstanceForMontage()
#include "Animation/AnimMontage.h"              // UAnimMontage

#include "../../CharacterAnimInstance.h"
#include "../../CharacterStateComponent.h"
#include "../../../Data/UnitAttackDataAsset.h"


bool AEnemy_HiveGuard::CheckAbleTryMiddle(AActor* target)
{
    if (_behaviorControlComponent->GetIsTargetAimingMe())
    {
        return true;
    }
    return false;
}


bool AEnemy_HiveGuard::TryMiddle(AActor* target)
{
    if (_behaviorControlComponent->GetIsTargetAimingMe())
    {
        if (StartGuard())
        {
            return true;
        }
    }
    return false;
}


bool AEnemy_HiveGuard::StartGuard()
{
    if (_reservedFunction.IsBound())
        _reservedFunction.Unbind();
    _reservedFunction.BindUObject(this, &AEnemy_HiveGuard::GuardContinue);

    UCharacterAnimInstance* anim = Cast<UCharacterAnimInstance>(GetMesh()->GetAnimInstance());


    if (_guardMontage == nullptr)
        return false;
    if (anim == nullptr)
        return false;
    if (_stateComp->ActionBegin() == false)
        return false;



    const float Duration = anim->PlayAnimMontage(_guardMontage);

    return true;
}

void AEnemy_HiveGuard::GuardContinue()
{
    if (_behaviorControlComponent->GetIsTargetAimingMe() == false)
        return;
    UCharacterAnimInstance* anim = Cast<UCharacterAnimInstance>(GetMesh()->GetAnimInstance());

    // 유효성 검사
    if (anim == nullptr || _guardMontage == nullptr)
    {
        return;
    }

    // 3. 현재 플레이 중인 몽타주가 가드 몽타주인지 확인
    if (anim->GetCurrentActiveMontage() != _guardMontage)
    {
        // 가드 몽타주가 플레이 중이 아니라면 점프할 수 없으므로 함수 종료
        return;
    }

    // 4. 가드를 유지하는 루프 섹션으로 점프
    const FName LoopSectionName = FName(TEXT("GuardLoop")); // 몽타주에 정의된 루프 섹션 이름

    // Montage_JumpToSection 함수를 사용하여 해당 섹션으로 즉시 이동
    anim->Montage_JumpToSection(LoopSectionName, _guardMontage);

}
