// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BT_Task_Patrol_FindNextLoc.generated.h"

/**
 * 
 */
UCLASS()
class SGAPROJECTMAIN_API UBT_Task_Patrol_FindNextLoc : public UBTTaskNode
{
	GENERATED_BODY()

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};
