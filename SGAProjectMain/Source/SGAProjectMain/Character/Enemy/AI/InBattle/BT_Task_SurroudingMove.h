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
public:
    UBT_Task_SurroudingMove();

    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
    virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
    virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

protected:
    /** 스트레이프할 대상(플레이어 등)을 가져올 블랙보드 키입니다. */
    UPROPERTY(EditAnywhere, Category = "Blackboard")
    FBlackboardKeySelector TargetActorKey;


    bool _strafeDirection;

    /** 스트레이프를 지속할 시간입니다. */
    UPROPERTY(EditAnywhere, Category = "Node")
    float _angleTolerence;

    /** 태스크 종료 시 캐릭터의 회전 설정을 원래대로 복원하기 위한 플래그 */
    bool bOriginalOrientRotationToMovement;
};
