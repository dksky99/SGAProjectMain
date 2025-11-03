// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Enemy.h"
#include "../../Interface/Targetable.h"
#include "BehaviorControlComponent.generated.h"


UENUM(BlueprintType)
enum class EBehaviourType : uint8
{
	Wait,
	Patrol,
	Trace,
	Attack, // 실제로는 아이템이 하는 거지만 AI한테 줄 때는 결과적으로 같은 이야기
	Hit,    // 얻어맞기 (얻어맞은 반응으로써 할 일)
	Death,  // 죽기
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SGAPROJECTMAIN_API UBehaviorControlComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UBehaviorControlComponent();

	void Init();
protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	


	//UnitType
	UFUNCTION(BlueprintPure)
	bool CheckUnitType( EUnitState InType); // 현재 행동 유형 확인하기

	void ChangeUnitType( EUnitState InType); // 유형 전환하기

	EUnitState GetCurUnitType();				// 유형 가져오기

	//BattleType
	bool CheckBattleType( EBattleState InType); // 현재 행동 유형 확인하기

	void ChangeBattleType( EBattleState InType); // 유형 전환하기
	
	EBattleState GetCurBattleType();				// 유형 가져오기


	//타겟액터
	class AActor* GetTargetActor();	// 플레이어 가져오기 = 표적 가져오기

	void SetTargetActor(AActor* target);
	//타겟지점
	FVector GetTargetLoc();	// 플레이어 가져오기 = 표적 가져오기
	void SetTargetLoc(FVector loc);
	// class ACUnit* GetTarget(); // 표적이 범용 대상인 경우


	//타겟과의 거리
	void ChangeDistanceValue(float InValue);

	float GetDistance();

	//경계 임계치
	void ChangeAlertThreshold(float InValue);

	float GetAlertThreshold();



	bool GetIsAbleToAct();	
	void SetIsAbleToAct(bool condition);


	bool GetIsTargetLookAtMe();	
	void SetIsTargetLookAtMe(bool condition);

	bool GetIsTargetAimingMe();
	void SetIsTargetAimingMe(bool condition);

	bool GetIsUnable();	
	void SetIsUnable(bool condition);


	void SetBlackBoard(class UBlackboardComponent* InBlackboard) { blackboard = InBlackboard; }


public:

	UPROPERTY(EditAnywhere)
	FName _unitStateKey = "UnitState";


	UPROPERTY(EditAnywhere)
	FName _battleStateKey = "BattleState";

	UPROPERTY(EditAnywhere)
	FName targetKey = "Target";

	UPROPERTY(EditAnywhere)
	FName targetLocKey = "TargetLoc";

	UPROPERTY(EditAnywhere)
	FName distanceKey = "Distance";


	UPROPERTY(EditAnywhere)
	FName alertThresholdKey = "AlertThreshold";

	UPROPERTY(EditAnywhere)
	FName IsAbleToActKey = "IsAbleToAct";

	UPROPERTY(EditAnywhere)
	FName IsTargetLookAtMeKey = "IsTargetLookAtMe";

	UPROPERTY(EditAnywhere)
	FName IsTargetAimingMeKey = "IsTargetAimingMe";

	UPROPERTY(EditAnywhere)
	FName IsUnableKey = "IsUnable";
protected:

	class UBlackboardComponent* blackboard;

	class AEnemyController* _ownerController;
	class AEnemy* _ownerPawn;
		
};
