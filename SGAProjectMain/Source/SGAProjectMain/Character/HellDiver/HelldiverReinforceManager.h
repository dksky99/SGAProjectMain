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
	// 사망 지점 근처의 하늘에서 드랍포드를 스폰하고, 풀에서 꺼낸 헬다이버를 드랍포드에 주입합니다
	void ReinforceHelldiver(const FVector& deathPoint);

	// 사망한 플레이어의 컨트롤러를 큐에 반환(리스폰 준비)
	void ReturnDeadController(AController* controller);

protected:
	// 시작 시 헬다이버 풀을 미리 스폰하여 비활성화합니다
	void InitSquad();

	// 풀에서 사용 가능한 헬다이버 하나를 꺼냅니다(없으면 nullptr)
	class APlayerCharacter* AcquirePooledDiver();

	// 네비게이션 이용: deathPoint를 기준으로 유효한 지면 좌표를 찾습니다(실패 시 입력 좌표 그대로 반환)
	FVector ComputeGroundPoint(const FVector& deathPoint) const;

	// 일정 주기마다 예산을 1씩 회복합니다
	void AddReinforceBudgetTick();

protected:
	// 풀에 담을 헬다이버 원본 클래스
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game/Units", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<class APlayerCharacter> _hellDiverClass;

	// 미리 스폰해 두는 헬다이버 풀(런타임 관리용)
	UPROPERTY()
	TArray<class APlayerCharacter*> _hellDiverPool;

	// 시작 시 풀의 초기 개수
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game/Units", meta = (AllowPrivateAccess = "true"))
	int32 _initialPoolSize = 5;

	// 플레이어 드랍포드 클래스(착지 후 끌어올림/포제션 담당)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game/Units", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<class AHellDiverDropPod> _playerDropPodClass;

	// 드랍포드 입력 전용 폰 클래스(낙하시 좌우 이동 입력만 담당)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game/Units", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<class AHellDiverDroppodInput> _dropPodInputClass;

	// 드랍포드 스폰 고도(지면에서 위로 올린 Z)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game/Units", meta = (AllowPrivateAccess = "true"))
	float _dropHeight = 10000.0f;

	// 사망 지점 주변 네비 탐색 반경(지면 후보 찾을 때 사용)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game/Units", meta = (AllowPrivateAccess = "true"))
	float _reinforceSearchRadius = 2000.0f;

	// 현재 남은 보강 예산(1회 소모 시 감소)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game/Units", meta = (AllowPrivateAccess = "true"))
	int32 _remainReinforceBudget = 5;

	// 예산 자동 회복 쿨타임(초). 0.0f 이하이면 자동 회복 비활성
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game/Units", meta = (AllowPrivateAccess = "true"))
	float _addReinforceBudgetCoolTime = 120.0f;

	// 예산 회복 타이머
	FTimerHandle _reinforceBudgetTimer;

	// 사망 컨트롤러 대기열(재사용)
	TQueue<AController*> _controllerQ;
};