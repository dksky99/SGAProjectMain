// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "Navigation/PathFollowingComponent.h"
#include "BT_Task_Patrol.generated.h"

/**
 * 
 */
UCLASS()
class SGAPROJECTMAIN_API UBT_Task_Patrol : public UBTTaskNode
{
	GENERATED_BODY()


public:
	UBT_Task_Patrol();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp,
		uint8* NodeMemory) override;

	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory,
		float DeltaSeconds) override;
protected:
	// **UFUNCTION:** 이동 완료/실패 시 AI 컨트롤러가 호출하는 함수
	UFUNCTION()
	void OnMoveCompletedHandler(FAIRequestID RequestID, EPathFollowingResult::Type Result);

protected:

	FVector _targetLocation=FVector::ZeroVector;

	float  _acceptance=0.f;
	bool _findSuccess=false;

	TWeakObjectPtr<UBehaviorTreeComponent> _ownerComp;
};
