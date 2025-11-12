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

	// 착지 시 부모(ADropPod)가 호출하는 내부 스폰 훅
	virtual void SpawnInternalActor(const FVector& SpawnLocation) override;

	// 지하에서 소환된 플레이어를 지면 위로 올리는 보간 처리
	void UpdateRaisePlayer(const float dt);

	// 상승 완료 후 충돌 활성화 및 컨트롤러 인계
	void FinishRaiseAndPossess();

public:
	// 입력 전용 폰이 매 프레임 호출하여 수평 이동을 즉시 반영
	UFUNCTION(BlueprintCallable, Category = "Game/DropPod/Input")
	void ApplyHorizontalInput(const float axisX, const float axisY, const FRotator& controlYawOnly);

	// 드랍포드의 탑다운 카메라로 시점을 전환한다(BP 카메라 컴포넌트를 사용)
	UFUNCTION(BlueprintCallable, Category = "Game/DropPod/Camera")
	void ActivateOverlookCamera(class AController* controller, const float blendTime = 0.35f);

	// 풀에서 미리 뽑아둔 플레이어를 주입 (스폰 대신 활성화)
	UFUNCTION(BlueprintCallable, Category = "Game/DropPod")
	void SetPreSpawnedPlayer(class APlayerCharacter* pooled) { _player = pooled; }

protected:
	// 낙하/수평 속도를 ProjectileMovement에 즉시 반영
	void UpdateVelocityImmediate();

	// 드랍포드 카메라에서 플레이어 카메라로 전환
	void SmoothBlendToPlayerCamera(class APlayerController* PC);

protected:
	// 이동/낙하 파라미터
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game/DropPod")
	float _fallSpeed = 2500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game/DropPod")
	float _maxHorizontalSpeed = 900.0f;

	// 이동 내부 상태
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game/DropPod")
	FVector _desiredVelXY = FVector::ZeroVector;

	// 소환/상승 파라미터
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game/DropPod")
	float _spawnDepth = 220.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game/DropPod")
	float _raiseDuration = 1.25f;

	// 풀에서 받은 플레이어
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game/DropPod")
	class APlayerCharacter* _player = nullptr;

	// 상승 진행도
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game/DropPod")
	float _raiseElapsed = 0.0f;

	// 카메라/스프링암
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game/DropPod")
	class USpringArmComponent* _springArm = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game/DropPod")
	class UCameraComponent* _camera = nullptr;
};
