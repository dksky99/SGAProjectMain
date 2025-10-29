// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "BT_Task_MoveToDynamicTargetLoc.generated.h"

/**
 * 
 */
UCLASS()
class SGAPROJECTMAIN_API UBT_Task_MoveToDynamicTargetLoc : public UBTTask_BlackboardBase
{
	GENERATED_BODY()
	
public:
    UBT_Task_MoveToDynamicTargetLoc();

    /** 태스크 실행 시 호출됩니다. */
    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

    /** 태스크가 진행 중일 때 매 틱마다 호출됩니다. */
    virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

    /** 태스크가 중단될 때 호출됩니다. */
    virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

protected:

    /** 이동이 완료되었는지 확인합니다. */
    void CheckMoveCompletion(UBehaviorTreeComponent& OwnerComp);

protected:
    /** AI가 목표 위치에 도달했다고 판단할 허용 반경입니다. */
    UPROPERTY(EditAnywhere, Category = "Node")
    float AcceptableRadius;

    /** * true일 경우, AI가 이동 중에도 계속 목표 지점을 바라보게 합니다.
     * (CharacterMovementComponent의 bOrientRotationToMovement가 false로 설정됩니다)
     */


};
