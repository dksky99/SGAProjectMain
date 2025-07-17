// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "EnemyController.generated.h"

/**
 * 
 */
UCLASS()
class SGAPROJECTMAIN_API AEnemyController : public AAIController
{
	GENERATED_BODY()
public:
	AEnemyController();
	//컨트롤러가 폰에 빙의될때와 해제될때 호출되는 함수.
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;

	virtual void BeginPlay() override;

	UFUNCTION()
	void OnTargetDetected(AActor* Actor, FAIStimulus Stimulus);

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class UBlackboardData* _blackBoard;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class UBehaviorTree* _behaviorTree;

protected:
	UPROPERTY()
	FTimerHandle _timerHandle;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite,meta= (AllowPrivateAccess = "true"))
	class UAIPerceptionComponent* _aIPerception;

	UPROPERTY()
	class UAISenseConfig_Sight* _sightConfig;

};
