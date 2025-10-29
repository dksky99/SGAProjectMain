// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "BT_Deco_IsTargetLookAtMe.generated.h"

/**
 * 
 */
UCLASS()
class SGAPROJECTMAIN_API UBT_Deco_IsTargetLookAtMe : public UBTDecorator
{
	GENERATED_BODY()

public:
    UBT_Deco_IsTargetLookAtMe();

protected:

        virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;
protected:
    /** 타겟 액터를 가져올 블랙보드 키입니다. (Actor 타입) */
    UPROPERTY(EditAnywhere, Category = "Blackboard")
    FBlackboardKeySelector TargetActorKey;

    /** 타겟의 시선 허용 오차 각도 (도 Degree) */
    UPROPERTY(EditAnywhere, Category = "Condition")
    float AngleTolerance = 15.0f;

    /**
     * 데코레이터의 조건 검사 로직입니다.
     * @param OwnerComp - Behavior Tree 컴포넌트
     * @param NodeMemory - 노드 메모리
     * @return 조건이 충족되면 true, 아니면 false
     */
	
};
