// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionTypes.h"	//감각 구분 제어, 감각 자극 관리
#include "GenericTeamAgentInterface.h"
#include "../Interface/Targetable.h"
#include "../Character/Enemy/Enemy.h"
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
	LowAlert UMETA(DisplayName = "LowAlert"),	//청각거리 300, 시야거리 1000
	MediumAlert UMETA(DisplayName = "MediumAlert"),//청각거리 500, 시야거리 1500
	HighAlert UMETA(DisplayName = "HighAlert"),// 청각거리 700, 시야거리 2000
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

	void ResetController();

	UFUNCTION()
	void OnTargetDetected(AActor* Actor, FAIStimulus Stimulus);

	UFUNCTION()
	void PerceptionUpdated(const TArray<AActor*>& UpdatedActors);

	void HandleSensedSight(AActor* Actor);
	void HandleSensedHearing(FVector hearedLoc);
	void HandleSensedDamage(AActor* Actor);

	FAIStimulus CanSenseActor(AActor* Actor, EAIPerceptionSense AIPerceptionSense);

	void SetAlertStep(EUnitState unitState);
	void SetNone();
	void SetLowAlert();
	void SetMediumAlert();
	void SetHighAlert();
	void AddTargetActor(class AActor* target);

	void AddAlertStack(float loudness);
	void ReadyToStack();
	void ResetAlertStack();

	float GetAlertStack() { return _curLoudnessStack; }
	class AActor* GetCurTargetActor();
	//블랙보드에 타겟을 갱신할 목적. 지금 적용된타겟보다 더 가까운 위치에 타겟이될수있는 오브젝트가 있다면 그 오브젝트로 바뀌게될 예정.
	class AActor* GetNewTargetActor();
	bool CheckTargetRefresh();
	void RefreshTargets();
	
	void RecieveTarget(AActor* target);
	void RecieveTargetLoc(FVector targetLoc);


	FVector GetTargetLoc() { return _lastSensedLoc; }

	class UBehaviorControlComponent* GetBehaviorControl() { return _behaviorControlComponent; }
protected:
	//타겟이 가능한지 확인한다. 타겟어블을 계승해야하고 Enemy가 아니어야하고 죽어있거나 파괴되어있지 않아야함.
	bool CheckTargetable(AActor* target);
public:

	
	class AEnemy* _pawn;

	class UBlackboardData* _blackBoard;



	class UBehaviorTree* _behaviorTree;

protected:
	//스택이 너무빠른시간내에 다 차오르지 않게하기위해
	UPROPERTY()
	FTimerHandle _alertStackTimer;
	float _alertStackTime = 0.5f;
	bool _isReadyToStack = true;
	//얕은경계와 강한경계때 15초이상 소리를 듣지 못하면 경계치를 리셋
	UPROPERTY()
	FTimerHandle _alertResetTimer;
	float _alertResetTime = 15.0f;

	//타겟
	UPROPERTY(VisibleAnywhere)
	TArray<class AActor*> _targets;


	 AActor* _curTarget=nullptr;

	//경계치에따른 감각 확장.
	UPROPERTY(VisibleAnywhere ,Category="AIertStep")
	EAIAlertStep _alertStep = EAIAlertStep::None;


	//경계치
	float _highAlertThreshold = 5.0f;
	float _curLoudnessStack = 0.0f;

	//마지막 소리가 들린위치
	FVector _lastSensedLoc;


	class UBehaviorControlComponent* _behaviorControlComponent;

	UPROPERTY()
	class UAISenseConfig_Sight* _sightConfig;
	UPROPERTY()
	class UAISenseConfig_Hearing* _hearingConfig;
	UPROPERTY()
	class UAISenseConfig_Damage* _damageSenseConfig;

	FGenericTeamId TeamId;
	
};
