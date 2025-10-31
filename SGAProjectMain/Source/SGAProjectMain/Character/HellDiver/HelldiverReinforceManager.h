// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Containers/Queue.h"
#include "HelldiverReinforceManager.generated.h"

UCLASS()
class SGAPROJECTMAIN_API AHelldiverReinforceManager : public AActor
{
	GENERATED_BODY()
	
public:
	AHelldiverReinforceManager();

protected:
	virtual void BeginPlay() override;

public:
	// 사망 지점 근처의 하늘에서 드랍포드를 스폰합니다
	void ReinforceHelldiver(const FVector& deathPoint);

	// 사망한 플레이어의 컨트롤러를 반환받아 큐에 적재합니다
	void ReturnDeadController(AController* controller);

protected:
	// 주기적으로 예산을 1씩 회복합니다
	void AddReinforceBudgetTick();

protected:
	// 드랍포드 블루프린트를 지정하십시오
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game/Units", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<class AHellDiverDropPod> _playerDropPodClass;

	// 드랍포드가 스폰될 고도(지면 기준 Z 오프셋)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game/Units", meta = (AllowPrivateAccess = "true"))
	float _dropHeight = 10000.0f;

	// 사망 지점 주변 네비게이션 탐색 반경
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game/Units", meta = (AllowPrivateAccess = "true"))
	float _reinforceSearchRadius = 2000.0f;

	// 보급 예산(있으면 감소시키며 사용)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game/Units", meta = (AllowPrivateAccess = "true"))
	int32 _remainReinforceBudget = 5;

	// 예산 자동 회복 쿨타임(초)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game/Units", meta = (AllowPrivateAccess = "true"))
	float _addReinforceBudgetCoolTime = 120.0f;

	// 예산 회복 타이머 핸들
	FTimerHandle _reinforceBudgetTimer;

	// 사망 컨트롤러 대기열
	TQueue<AController*> _controllerQ;
};
