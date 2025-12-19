// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EagleStrikeController.generated.h"

UCLASS()
class SGAPROJECTMAIN_API AEagleStrikeController : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AEagleStrikeController();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// 스트라타젬이 컨트롤러를 스폰한 뒤, 초기 설정을 할 때 호출
	void InitializeStrike(float startDelay);

	// 선택된 진입 방향 (XY 평면 정규화 벡터)
	FVector GetApproachDirection() const { return _approachDirection; }

	// 지면에서 폭탄이 떨어져야 하는 지점 목록
	const TArray<FVector>& GetBombImpactPoints() const { return _bombImpactPoints; }

	// 첫 폭탄이 지면에 도달할 때까지 남은 예상 시간(초)을 계산
	float ComputeEtaToFirstImpact() const;

protected:
	// 타겟 기준으로 진입 방향을 선택
	FVector SelectApproachDirection() const;

	// 8방향 벡터를 outDirections에 채워 넣음 (XY 평면, 정규화)
	void BuildApproachDirectionArray(TArray<FVector>& outDirections) const;

	// 해당 방향에 높은 장애물이 있는지 검사
	bool IsDirectionBlockedByTallObstacle(const FVector& directionXY) const;

	// 폭탄 낙하지점 패턴 생성
	void BuildBombImpactPattern();

	// 드롭 타이밍 패턴 생성 (비행 진행도 기준)
	void BuildDropTimingPattern();

	// 현재 비행 진행도에 따라 폭탄을 투하해야 하는지 검사하고, 필요하면 폭탄을 스폰
	void UpdateBombing(float flightProgress);

	// 주어진 낙하지점을 향해 폭탄 1발을 스폰
	void SpawnOneBombTowards(const FVector& impactPos);

protected:
	// 이글 비행기 클래스 (BP에서 지정)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Game/Stratagem/Eagle", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<class AEaglePlane> _eaglePlaneClass;

	// 스폰된 이글 인스턴스 
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game/Stratagem/Eagle", meta = (AllowPrivateAccess = "true"))
	AEaglePlane* _eagleInstance = nullptr;

	// 이글의 진입 방향 (XY 평면 정규화)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game/Stratagem/Eagle", meta = (AllowPrivateAccess = "true"))
	FVector _approachDirection;

	// 타겟 기준으로 얼마나 먼 거리까지 장애물을 검사할지 (수평 거리)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Game/Stratagem/Eagle", meta = (AllowPrivateAccess = "true"))
	float _checkRadius = 4000.0f;

	// "높은 장애물"로 간주할 기준 높이
	// 타겟 Z + _tallObstacleHeight 높이에서 수평 라인트레이스를 쏩니다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Game/Stratagem/Eagle", meta = (AllowPrivateAccess = "true"))
	float _tallObstacleHeight = 1000.0f;

	// 사용할 폭탄 클래스 (BP에서 지정)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Game/Stratagem/Eagle", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<class ABombProjectile> _bombClass;

	// 폭탄 총 개수 (1 또는 짝수)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Game/Stratagem/Eagle", meta = (AllowPrivateAccess = "true"))
	int32 _totalBombCount = 10;

	// 폭탄 속도 (중력 0, 직선 이동 가정)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Game/Stratagem/Eagle", meta = (AllowPrivateAccess = "true"))
	float _bombSpeed = 8000.0f;

	// 살보 간 앞/뒤 간격 (타겟 기준, forward 방향)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Game/Stratagem/Eagle", meta = (AllowPrivateAccess = "true"))
	float _salvoSpacing = 400.0f;

	// 살보 내 좌/우 오프셋 거리
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Game/Stratagem/Eagle", meta = (AllowPrivateAccess = "true"))
	float _lateralOffset = 200.0f;

	// 다음에 쏠 살보 인덱스 (0 기반)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game/Stratagem/Eagle", meta = (AllowPrivateAccess = "true"))
	int32 _nextSalvoIndex = 0;

	// 지면에서 폭탄이 떨어져야 하는 지점 목록
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game/Stratagem/Eagle", meta = (AllowPrivateAccess = "true"))
	TArray<FVector> _bombImpactPoints;

	// 살보(드롭) 타이밍 (0.0f ~ 1.0f, 비행 경로 진행도 기준)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game/Stratagem/Eagle", meta = (AllowPrivateAccess = "true"))
	TArray<float> _salvoDropTimes;

	// 상승 시작 타이밍 (0.0f ~ 1.0f, 비행 경로 진행도 기준)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game/Stratagem/Eagle", meta = (AllowPrivateAccess = "true"))
	float _ascentStartT = 0.5f;

	// 스트라타젬에서 넘겨준 출격 지연 시간 (deployDelay)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game/Stratagem/Eagle", meta = (AllowPrivateAccess = "true"))
	float _startDelay = 0.0f;
};
