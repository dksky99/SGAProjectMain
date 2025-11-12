// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BT_Service_Enemy_InBattle.generated.h"

/**
 * 
 */
UCLASS()
class SGAPROJECTMAIN_API UBT_Service_Enemy_InBattle : public UBTService
{
	GENERATED_BODY()

public:
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Service Properties")
	float _isTargetLookAngle = 45.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Service Properties")
	float _isTargetAimingAngle=5.0f;
};
