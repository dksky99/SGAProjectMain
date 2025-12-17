// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EaglePlane.generated.h"

UCLASS()
class SGAPROJECTMAIN_API AEaglePlane : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AEaglePlane();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// 컨트롤러가 경로를 넘겨줄 때 호출
	void InitializeFlight(const FVector& targetLocation, const FVector& approachDirection, float startDelay);

	// 폭탄 소켓의 월드 위치 반환
	FVector GetBombSpawnLocation() const;

	// 현재 비행 진행도(0.0f ~ 1.0f)
	float GetFlightProgress() const;

	// ETA계산용
	// "정규화 진행도(t: 0.0f ~ 1.0f)일 때 이글이 어디 있는지" 물어볼 때 사용
	FVector GetPositionAtNormalizedT(float t) const;

	// 정규화 진행도(t: 0.0f ~ 1.0f) 시점에서의 폭탄 소켓 월드 위치 예측
	FVector GetBombSpawnLocationAtNormalizedT(float t) const;

	// 전체 비행 시간 반환
	float GetTotalRunDuration() const { return _totalRunDuration; }

protected:
	// 비행 시작 함수 (타이머 콜백)
	void StartFlight();

protected:
	// 이글 비행기 메쉬
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game/Stratagem/Eagle", meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* _meshComponent;

	// 폭탄이 나가는 기준 소켓 이름
	// 기본값은 에셋에서 실제 이름에 맞게 변경하시면 됩니다. (예: "Bomb_Muzzle")
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Game/Stratagem/Eagle", meta = (AllowPrivateAccess = "true"))
	FName _bombSocketName;

	// 이글 최저 투하 고도 (타겟 Z + 이 값)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Game/Stratagem/Eagle", meta = (AllowPrivateAccess = "true"))
	float _dropAltitude = 1500.0f;

	// 진입/이탈 지점에서 투하 고도보다 얼마나 더 높은지
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Game/Stratagem/Eagle", meta = (AllowPrivateAccess = "true"))
	float _extraHeight = 10000.0f;

	// 타겟으로부터 진입 / 이탈 지점까지의 수평 거리 (대칭 경로)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Game/Stratagem/Eagle", meta = (AllowPrivateAccess = "true"))
	float _pathHalfDistance = 20000.0f;

	// 전체 비행 시간
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Game/Stratagem/Eagle", meta = (AllowPrivateAccess = "true"))
	float _totalRunDuration = 4.0f;

	// 내부 상태
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game/Stratagem/Eagle", meta = (AllowPrivateAccess = "true"))
	FVector _targetLocation;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game/Stratagem/Eagle", meta = (AllowPrivateAccess = "true"))
	FVector _approachDirection;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game/Stratagem/Eagle", meta = (AllowPrivateAccess = "true"))
	FVector _startLocation;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game/Stratagem/Eagle", meta = (AllowPrivateAccess = "true"))
	FVector _centerLocation;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game/Stratagem/Eagle", meta = (AllowPrivateAccess = "true"))
	FVector _exitLocation;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game/Stratagem/Eagle", meta = (AllowPrivateAccess = "true"))
	float _elapsedTime = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game/Stratagem/Eagle", meta = (AllowPrivateAccess = "true"))
	bool _isActive = false;
};
