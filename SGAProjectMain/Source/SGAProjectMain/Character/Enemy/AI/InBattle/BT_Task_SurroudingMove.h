// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BT_Task_SurroudingMove.generated.h"

/**
 * 
 */
UCLASS()
class SGAPROJECTMAIN_API UBT_Task_SurroudingMove : public UBTTaskNode
{
	GENERATED_BODY()

protected:
    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
    
    virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
    /** 태스크가 중단되거나 완료될 때 호출되어 뒷정리를 합니다. */
    virtual void OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult) override;

public:
    // AI가 타겟의 옆으로 얼마나 떨어져서 위치를 잡을지 결정합니다.
    UPROPERTY(EditAnywhere, Category = "AI")
    float FlankDistance = 800.0f;

    // 이상적인 측면 위치 주변에서 실제 이동 가능한 지점을 찾을 반경입니다.
    UPROPERTY(EditAnywhere, Category = "AI")
    float SearchRadius = 500.0f;

    // 타겟 정보를 담고 있는 블랙보드 키입니다.
    UPROPERTY(EditAnywhere, Category = "Blackboard")
    FBlackboardKeySelector TargetKey;
};
