// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DropPod.h"
#include "HellDiverDropPod.generated.h"

/**
 * 
 */
UCLASS()
class SGAPROJECTMAIN_API AHellDiverDropPod : public ADropPod
{
	GENERATED_BODY()

public:
	AHellDiverDropPod();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	// 착지 → 소환 시작
	virtual void SpawnInternalActor(const FVector& SpawnLocation) override;

	// 상승 보간
	void UpdateRaisePlayer(const float dt);

	// 상승 완료 후 Possess
	void FinishRaiseAndPossess();

public:
	// 입력 전용 폰이 매 프레임 호출합니다.
	UFUNCTION(BlueprintCallable, Category = "Game/Stratagem/Pod")
	void ApplyHorizontalInput(const float axisX, const float axisY, const FRotator& controlYawOnly);

protected:
	void UpdateVelocityImmediate();

protected:
	// 파라미터
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game/Stratagem/Pod")
	float _fallSpeed = 2500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game/Stratagem/Pod")
	float _maxHorizontalSpeed = 900.0f;

	// 내부 상태
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game/Stratagem/Pod")
	FVector _desiredVelXY = FVector::ZeroVector;
	
	// 지하 소환 깊이
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game/Stratagem/Pod")
	float _spawnDepth = 220.0f; 

	// 지면으로 올라오는 연출 시간
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game/Stratagem/Pod")
	float _raiseDuration = 1.25f;  

	// 소환 된 액터
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game/Stratagem/Pod")
	class APlayerCharacter* _spawnedPlayer = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game/Stratagem/Pod")
	float _raiseElapsed = 0.0f;
};
