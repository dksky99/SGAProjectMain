// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "HellDiverDroppodInput.generated.h"

UCLASS()
class SGAPROJECTMAIN_API AHellDiverDroppodInput : public APawn
{
	GENERATED_BODY()

public:
	AHellDiverDroppodInput();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

public:
	// 헬포드(Actor)를 지정합니다.
	void SetHellPodActor(AActor* hellPodActor) { _hellPodActor = hellPodActor; }

protected:
	void OnMoveX(float axis);
	void OnMoveY(float axis);

	// 매 프레임 헬포드에 축을 보냅니다.
	void PushInputToHellPod();

protected:
	// 헬포드 참조(안전하게 약참조)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game/Stratagem/Sentry")
	TWeakObjectPtr<AActor> _hellPodActor = nullptr;

	// 현재 입력 축
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game/Stratagem/Sentry")
	float _axisX = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game/Stratagem/Sentry")
	float _axisY = 0.0f;
};
