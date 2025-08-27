// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionTypes.h"	//감각 구분 제어, 감각 자극 관리
#include "EnemyController.generated.h"


UENUM(BlueprintType)
enum class EAIPerceptionSense : uint8
{
	EPS_None UMETA(DisplayName = "EPS_None"),			
	EPS_Sight UMETA(DisplayName = "EPS_Sight"),			
	EPS_Hearing UMETA(DisplayName = "EPS_Hearing"),		
	EPS_Damage UMETA(DisplayName = "EPS_Damage"),		
	EPS_MAX
};


UENUM(BlueprintType)
enum class EAIAlertStep : uint8
{
	None UMETA(DisplayName = "None"),
	LowAlert UMETA(DisplayName = "LowAlert"),
	MediumAlert UMETA(DisplayName = "MediumAlert"),
	HighAlert UMETA(DisplayName = "HighAlert"),
	MAX
};

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

	UFUNCTION()
	void PerceptionUpdated(const TArray<AActor*>& UpdatedActors);

	void HandleSensedSight(AActor* Actor);
	void HandleSensedHearing(FVector hearedLoc);
	void HandleSensedDamage(AActor* Actor);

	FAIStimulus CanSenseActor(AActor* Actor, EAIPerceptionSense AIPerceptionSense);

	void SetNone();
	void SetLowAlert();
	void SetMediumAlert();
	void SetHighAlert();

public:

	UPROPERTY()
	class AEnemy* _pawn;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class UBlackboardData* _blackBoard;



	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class UBehaviorTree* _behaviorTree;

protected:
	UPROPERTY()
	FTimerHandle _timerHandle;

	TArray<class ACharacterBase*> _targets;

	UPROPERTY(VisibleAnywhere ,Category="AIertStep")
	EAIAlertStep _alertStep = EAIAlertStep::None;

	float _soundCheckThreshold = 1.0f;

	UPROPERTY()
	class UAISenseConfig_Sight* _sightConfig;
	class UAISenseConfig_Hearing* _hearingConfig;
	class UAISenseConfig_Damage* _damageSenseConfig;

};
