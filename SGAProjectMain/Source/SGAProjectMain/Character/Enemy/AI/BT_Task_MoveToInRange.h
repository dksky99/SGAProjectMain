// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BT_Task_MoveToInRange.generated.h"

/**
 * 
 */
UCLASS()
class SGAPROJECTMAIN_API UBT_Task_MoveToInRange : public UBTTaskNode
{
	GENERATED_BODY()
public:
	UBT_Task_MoveToInRange();
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
};
