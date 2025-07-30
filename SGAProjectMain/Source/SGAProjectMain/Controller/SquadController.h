// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "SquadController.generated.h"

/**
 * 
 */
UCLASS()
class SGAPROJECTMAIN_API ASquadController : public AAIController
{
	GENERATED_BODY()
public:
	ASquadController();
	//컨트롤러가 폰에 빙의될때와 해제될때 호출되는 함수.
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;

	virtual void BeginPlay() override;




public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class UBlackboardData* _blackBoard;



	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class UBehaviorTree* _behaviorTree;

	UPROPERTY()
	TArray<class AEnemyController*> _units;

};
